#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import threading
import copy
import time
import sys
import struct

from ossie.cf import CF, ExtendedCF
from ossie.cf.CF import Port
from ossie.utils import uuid
from ossie.properties import simple_property
from ossie.utils.log4py import logging
from redhawk.bitbuffer import bitbuffer

from bulkio.statistics import OutStats
import bulkio.sri
from bulkio import timestamp
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA 
from bulkio.const import MAX_TRANSFER_BYTES
from bulkio.output_streams import *
import traceback

class connection_descriptor_struct(object):
    connection_id = simple_property(id_="connectionTable::connection_id",
                                    name="connection_id",
                                    type_="string")

    stream_id = simple_property(id_="connectionTable::stream_id",
                                name="stream_id",
                                type_="string")

    port_name = simple_property(id_="connectionTable::port_name",
                                name="port_name",
                                type_="string")

    def __init__(self, connection_id="", stream_id="", port_name=""):
        self.connection_id = connection_id
        self.stream_id = stream_id
        self.port_name = port_name

    def __str__(self):
        """Return a string representation of this structure"""
        d = {}
        d["connection_id"] = self.connection_id
        d["stream_id"] = self.stream_id
        d["port_name"] = self.port_name
        return str(d)

    def getId(self):
        return "connectionTable::connection_descriptor"

    def isStruct(self):
        return True

    def getMembers(self):
        return [("connection_id",self.connection_id),("stream_id",self.stream_id),("port_name",self.port_name)]


class OutPort(BULKIO__POA.UsesPortStatisticsProvider):

    class SriMapStruct:
        def __init__( self, sri=None, connections=None, time=None): 
            self.sri=sri
            self.connections = connections #set of connection ID strings that have received this SRI
            self.time=time

    def __init__(self, name, PortTypeClass, PortTransferType, logger=None, dataType=list, bits=0):
        # Backwards-compatibility: accept an element type string for use with
        # struct.calcsize
        if bits == 0:
            bits = struct.calcsize(PortTransferType) * 8
        self.name = name
        self._portLog = logger
        self.PortType = PortTypeClass
        self.PortTransferType=PortTransferType
        self.outConnections = {} # key=connectionId,  value=port
        self.stats = OutStats(self.name, bits=bits)
        self.port_lock = threading.Lock()
        self.sriDict = {} # key=streamID  value=SriMapStruct
        self.filterTable = []

        # Data type class
        self._dataType = dataType
        # Retain noData member for backwards-compatibility
        self.noData = dataType()

        # Determine maximum transfer size in advance
        self._bitSize = bits
        # Multiply by some number < 1 to leave some margin for the CORBA header
        self.maxSamplesPerPush = 8 * int(MAX_TRANSFER_BYTES*.9) / self._bitSize
        # Retain byte size for backwards-compatibility
        self.byteSize = self._bitSize / 8

        self._streams = {}

        if self._portLog == None:
            self._portLog = logging.getLogger("redhawk.bulkio.outport."+name)

    def getLogger(self):
        return self._portLog

    def setLogger(self, logger):
        self._portLog = logger

    def connectPort(self, connection, connectionId):
        if self._portLog:
            self._portLog.trace('bulkio::OutPort  connectPort ENTER ')

        if connection is None:
            raise CF.Port.InvalidPort(1, 'Nil object reference')

        # Attempt to check the type of the remote object to reject invalid
        # types; note this does not require the lock
        repo_id = self.PortType._NP_RepositoryId
        try:
            valid = connection._is_a(repo_id)
        except:
            # If _is_a throws an exception, assume the remote object is
            # unreachable (probably dead)
            raise CF.Port.InvalidPort(1, 'Object unreachable')

        if not valid:
            raise CF.Port.InvalidPort(1, 'Object does not support '+repo_id)

        port = connection._narrow(self.PortType)

        # Acquire the state lock before modifying the container
        with self.port_lock:
            # Prevent duplicate connection IDs
            if str(connectionId) in self.outConnections:
                raise Port.OccupiedPort()

            self.outConnections[str(connectionId)] = port
            self.stats.add(connectionId)

            if self._portLog:
                self._portLog.debug('bulkio::OutPort  CONNECT PORT:%s CONNECTION:%s', self.name, connectionId)

        if self._portLog:
            self._portLog.trace('bulkio::OutPort  connectPort EXIT ')
            
    def disconnectPort(self, connectionId):
        if self._portLog:
            self._portLog.trace('bulkio::OutPort  disconnectPort ENTER ')

        with self.port_lock:
            port = self.outConnections.pop(connectionId, None)
            if not port:
                raise CF.Port.InvalidPort(2, 'No connection '+connectionId)

            for stream_id in self.sriDict.iterkeys():
                if not self._isStreamRoutedToConnection(stream_id, connectionId):
                    continue

                try:
                    self._sendPacket(port, self._dataType(), timestamp.notSet(), True, stream_id)
                except Exception, e:
                    if self._portLog:
                        self._portLog.error("PUSH-PACKET FAILED, PORT/CONNECTION: %s/%s , EXCEPTION: %s", self.name, connectionId, e)

            self.stats.remove(connectionId)
            for key in self.sriDict.keys():
                # if connID exist in set, remove it, otherwise do nothing (that is what discard does)
                self.sriDict[key].connections.discard(connectionId)
            if self._portLog:
                self._portLog.debug( "bulkio::OutPort DISCONNECT PORT:%s CONNECTION:%s", self.name, connectionId)
                self._portLog.trace( "bulkio::OutPort DISCONNECT PORT:%s updatedSriDict%s", self.name, self.sriDict)

        if self._portLog:
            self._portLog.trace('bulkio::OutPort  disconnectPort EXIT ')

    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)

    def setBitSize(self, bitSize):
        self.stats.setBitSize(bitSize)

    def reportConnectionErrors(self, cid):
        retval=False
        if ( self.stats.connectionErrors(cid, 1) < 11 ): retval=True
        return retval
        
    def _get_connections(self):
        currentConnections = []
        self.port_lock.acquire()
        try:
            for id_, port in self.outConnections.items():
                currentConnections.append(ExtendedCF.UsesConnection(id_, port))
        finally:
            self.port_lock.release()
        return currentConnections

    def _get_connectionStatus(self):
        connectionStatus = []
        with self.port_lock:
            for id_, port in self.outConnections.items():
                connectionStatus.append(ExtendedCF.ConnectionStatus(id_, port, True, 'CORBA', []))
        return connectionStatus

    def _get_statistics(self):
        self.port_lock.acquire()
        try:
            recStat = self.stats.retrieve()
        finally:
            self.port_lock.release()
        return recStat

    def _get_state(self):
        self.port_lock.acquire()
        try:
            numberOutgoingConnections = len(self.outConnections)
        finally:
            self.port_lock.release()
        if numberOutgoingConnections == 0:
            return BULKIO.IDLE
        else:
            return BULKIO.ACTIVE

    def _get_activeSRIs(self):
        self.port_lock.acquire()
        try:
            sris = []
            for entry in self.sriDict:
                sris.append(copy.deepcopy(self.sriDict[entry].sri))
        finally:
            self.port_lock.release()
        return sris
    
    def updateConnectionFilter(self, _filterTable):
        self.port_lock.acquire()
        try:
            if _filterTable == None :
                _filterTable = []
            self.filterTable = _filterTable
        finally:
            self.port_lock.release()

    def pushSRI(self, H):
        if self._portLog:
            self._portLog.trace('bulkio::OutPort pushSRI ENTER ')

        with self.port_lock:
            sri = copy.deepcopy(H)
            self.sriDict[H.streamID] = OutPort.SriMapStruct(sri=sri, connections=set()) 
            if not H.streamID in self._streams:
                self._streams[H.streamID] = self._createStream(sri)

            for connId, port in self.outConnections.iteritems():
                if not self._isStreamRoutedToConnection(H.streamID, connId):
                    continue

                try:
                    port.pushSRI(H)
                    self.sriDict[H.streamID].connections.add(connId)
                except Exception, e:
                    if self.reportConnectionErrors(connId) :
                        if self._portLog:
                            self._portLog.error("PUSH-SRI FAILED, PORT/CONNECTION: %s/%s , EXCEPTION: %s", self.name, connId, str(e))

        if self._portLog:
            self._portLog.trace('bulkio::OutPort  pushSRI EXIT ')

    def getStream(self, streamID):
        """
        Get the active stream with the given stream ID.

        Args:
            streamID: String stream identifier.

        Returns:
            Output stream for `streamID` if it exists.
            None if no such stream ID exists.
        """
        with self.port_lock:
            return self._streams.get(streamID, None)

    def getStreams(self):
        """
        Gets the current set of active streams.

        Returns:
            List of output streams.
        """
        with self.port_lock:
            return self._streams.values()

    def createStream(self, stream):
        """
        Creates a new output stream.

        If `stream` is a string, a new output stream is created with stream ID
        `stream` and default values for the SRI. If an output stream with that
        stream ID already exists, it is returned unmodified.

        If `stream` is a BULKIO.StreamSRI, a new output stream is created with
        the same SRI values as `stream`. If an output stream with the same
        stream ID already exists, its SRI is updated to match `stream`.

        Args:
            stream: String stream identifier or BULKIO.StreamSRI.

        Returns:
            Newly-created or updated output stream.
        """
        with self.port_lock:
            if isinstance(stream, BULKIO.StreamSRI):
                # Try to find an existing stream with the same streamID, and
                # update it
                sri = stream
                stream = self._streams.get(sri.streamID, None)
                if stream:
                    # Update the stream's SRI from the argument
                    stream.sri = sri
                    return stream
            else:
                # Assume we were given a stream ID
                stream_id = stream
                stream = self._streams.get(stream_id, None)
                if stream:
                    return stream

                # Create an SRI with the given streamID
                sri = bulkio.sri.create(stream_id)

            # No existing stream was found, create one
            stream = self._createStream(sri)
            self._streams[sri.streamID] = stream
            return stream

    def _createStream(self, sri):
        return OutputStream(sri, self, self._dataType)

    def _pushPacket(self, data, T, EOS, streamID):
        # Prerequisite: caller holds self.port_lock
        packet_size = self._packetSize(data)
        if self._portLog:
            self._portLog.trace("_pushPacket() sending packet size=%d time=%s EOS=%s streamID='%s'", 
                              packet_size, T, EOS, streamID)
        
        for connId, port in self.outConnections.iteritems():
            if not self._isStreamRoutedToConnection(streamID, connId):
                continue
            try:
                if connId not in self.sriDict[streamID].connections and packet_size == 0:
                    # connection is being closed but no data was ever sent, so ignore
                    continue
                if connId not in self.sriDict[streamID].connections:
                    port.pushSRI(self.sriDict[streamID].sri)
                    self.sriDict[streamID].connections.add(connId)
                self._sendPacket(port, data, T, EOS, streamID)
                self.stats.update(packet_size, 0, EOS, streamID, connId)
            except Exception, e:
                if self.reportConnectionErrors(connId)  :
                    if self._portLog:
                        self._portLog.exception("PUSH-PACKET FAILED, PORT/CONNECTION: %s/%s , EXCEPTION: %s", self.name, connId, str(e))

        if EOS:
            if self.sriDict.has_key(streamID):
                tmp = self.sriDict.pop(streamID)
 
    def _sendPacket(self, port, data, T, EOS, streamID):
        port.pushPacket(data, T, EOS, streamID)

    def _isStreamRoutedToConnection(self, streamID, connectionID):
        port_listed = False
        for rule in self.filterTable:
            if rule.port_name != self.name:
                continue
            port_listed = True
            if rule.stream_id == streamID and rule.connection_id == connectionID:
                return True
        return not port_listed

    def pushPacket(self, data, T, EOS, streamID):
        
        if self._portLog:
            self._portLog.trace('bulkio::OutPort  pushPacket ENTER ')

        if not self.sriDict.has_key(streamID):
            sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
            self.pushSRI(sri)

        with self.port_lock:
            self._pushPacket(data, T, EOS, streamID)
            if EOS:
                del self._streams[streamID]

        if self._portLog:
            self._portLog.trace('bulkio::OutPort  pushPacket EXIT ')

    def _reformat(self, data):
        return data

    def _packetSize(self, data):
        return len(data)

class OutNumericPort(OutPort):
    def __init__(self, *args, **kwargs):
        elemType = kwargs.pop('elemType', int)
        OutPort.__init__(self, *args, **kwargs)
        self._elemType = elemType

    def _pushPacket(self, data, T, EOS, streamID):
        # If there is no need to break data into smaller packets, skip straight
        # to the pushPacket call and return.
        elements = len(data)
        if elements <= self.maxSamplesPerPush:
            return OutPort._pushPacket(self, data, T, EOS, streamID);

        sri = self.sriDict[streamID].sri

        # Quantize the push size (in terms of scalars) to the nearest frame,
        # which takes both the complex mode and subsize into account
        item_size = 2 if sri.mode else 1
        frame_size = item_size
        if sri.subsize > 0:
            frame_size *= sri.subsize
        max_samples = int(self.maxSamplesPerPush/frame_size) * frame_size

        # Intialize time for the first subpacket
        packetTime = T

        # Push sub-packets max_samples at a time
        count = len(data)
        for start in xrange(0, count, max_samples):
            # The end index of the packet may exceed the length of the data;
            # the Python slice operator will clamp it to the actual end
            end = start + max_samples

            # Send end-of-stream as false for all sub-packets except for the
            # last one (when the end of the sub-packet goes past the end of the
            # input data), which gets the input EOS.
            if end >= count:
                packetEOS = EOS
            else:
                packetEOS = False

            # Push the current slice of the input data
            OutPort._pushPacket(self, data[start:end], packetTime, packetEOS, streamID);

            # Synthesize the next packet timestamp
            if packetTime.tcstatus == BULKIO.TCS_VALID:
                push_size = min(end, count) - start
                packetTime = packetTime + (push_size/item_size) * sri.xdelta

    def _createStream(self, sri):
        return NumericOutputStream(sri, self, self._dataType, self._elemType)


class OutCharPort(OutNumericPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataChar, OutCharPort.TRANSFER_TYPE, logger, dataType=str, bits=8)

    def _reformat(self, data):
        if isinstance(data, basestring):
            return data
        return struct.pack('%db' % len(data), *data)

class OutOctetPort(OutNumericPort):
    TRANSFER_TYPE = 'B'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataOctet, OutOctetPort.TRANSFER_TYPE, logger, dataType=str, bits=8)

    def _reformat(self, data):
        if isinstance(data, basestring):
            return data
        return struct.pack('%dB' % len(data), *data)

class OutShortPort(OutNumericPort):
    TRANSFER_TYPE = 'h'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataShort, OutShortPort.TRANSFER_TYPE, logger, bits=16)

class OutUShortPort(OutNumericPort):
    TRANSFER_TYPE = 'H'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataUshort, OutUShortPort.TRANSFER_TYPE, logger, bits=16)

class OutLongPort(OutNumericPort):
    TRANSFER_TYPE = 'i'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataLong, OutLongPort.TRANSFER_TYPE, logger, bits=32)

class OutULongPort(OutNumericPort):
    TRANSFER_TYPE = 'I'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataUlong, OutULongPort.TRANSFER_TYPE, logger, bits=32)

class OutLongLongPort(OutNumericPort):
    TRANSFER_TYPE = 'q'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataLongLong, OutLongLongPort.TRANSFER_TYPE, logger, bits=64)

class OutULongLongPort(OutNumericPort):
    TRANSFER_TYPE = 'Q'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataUlongLong, OutULongLongPort.TRANSFER_TYPE, logger, bits=64)

class OutFloatPort(OutNumericPort):
    TRANSFER_TYPE = 'f'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataFloat, OutFloatPort.TRANSFER_TYPE, logger, elemType=float, bits=32)

class OutDoublePort(OutNumericPort):
    TRANSFER_TYPE = 'd'
    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataDouble, OutDoublePort.TRANSFER_TYPE, logger, elemType=float, bits=64)

class OutBitPort(OutNumericPort):
    TRANSFER_TYPE = 'B'

    def __init__(self, name, logger=None):
        OutNumericPort.__init__(self, name, BULKIO.dataBit, OutBitPort.TRANSFER_TYPE, logger, dataType=bitbuffer, bits=1)

    def _sendPacket(self, port, data, T, EOS, streamID):
        data = BULKIO.BitSequence(data.bytes(), len(data))
        port.pushPacket(data, T, EOS, streamID)

    def _reformat(self, data):
        return bitbuffer(data)

    def _createStream(self, sri):
        return BufferedOutputStream(sri, self, bitbuffer)

class OutFilePort(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, logger=None):
        OutPort.__init__(self, name, BULKIO.dataFile, OutFilePort.TRANSFER_TYPE, logger, dataType=str, bits=8)

    def _createStream(self, sri):
        return OutFileStream(sri, self)

class OutXMLPort(OutPort):
    TRANSFER_TYPE = 'c'
    def __init__(self, name, logger=None):
        OutPort.__init__(self, name, BULKIO.dataXML, OutXMLPort.TRANSFER_TYPE, logger, dataType=str, bits=8)

    def pushPacket(self, xml_string, EOS, streamID):
        OutPort.pushPacket(self, xml_string, None, EOS, streamID)

    def _sendPacket(self, port, xml_string, T, EOS, streamID):
        port.pushPacket(xml_string, EOS, streamID)

    def _createStream(self, sri):
        return OutXMLStream(sri, self)


class OutAttachablePort(OutPort):
    class StreamAttachment:
        def __init__(self, connectionId, attachId, inputPort, inStream=None):
            self.connectionId=connectionId
            self.attachId=attachId
            self.inputPort=inputPort
            self.stream=inStream
            self._sa_logger=None

        def setLogger(self, inLogger ):
            self._sa_logger= inLogger

        def setLogger(self, inLogger ):
            self._sa_logger= inLogger

        def detach(self):
            p =  None
            if self.stream:
                p = self.stream.getPort()
            try:
                self.inputPort.detach(self.attachId)
                if p : p.updateStats(self.connectionId)
            except Exception, e:
                if p and p.reportConnectionErrors(self.connectionId)  :
                    if self._sa_logger:
                        self._sa_logger.error("DETACH FAILURE, CONNECTION: %s , EXCEPTION: %s", self.connectionId, str(e))

    class Stream:
        def __init__(self, streamDef, name, streamId=None, streamAttachments=[], sri=None, time=None, port=None):
            self.streamDef=streamDef
            self.name = name
            self.streamId=streamId
            self.streamAttachments=streamAttachments[:]
            self.sri=sri
            self.time=time
            self.port = port
            self._s_logger=None

        def detachAll(self):
            for att in list(self.streamAttachments):
                att.detach()
                self.streamAttachments.remove(att)

        def detachByConnectionId(self, connectionId):
            for att in list(self.streamAttachments):
                if att.connectionId == connectionId and att.inputPort and att.attachId:
                    att.detach()
                    self.streamAttachments.remove(att)

        def detachByAttachId(self, attachId):
            for att in list(self.streamAttachments):
                if att.attachId and att.inputPort and att.attachId == attachId:
                    att.detach()
                    self.streamAttachments.remove(att)

        def detachByAttachIdConnectionId(self, connectionId):
            for att in list(self.streamAttachments):
                if att.attachId and att.inputPort and att.attachId == attachId and att.connectionId == connectionId:
                    att.detach()
                    self.streamAttachments.remove(att)

        def createNewAttachment(self,connectionId, port):
            newAttachment = OutAttachablePort.StreamAttachment(connectionId=connectionId, attachId=None, inputPort=port, inStream=self)
            newAttachment.setLogger(self._s_logger)
            try:
                newAttachment.attachId = port.attach(self.streamDef, self.name)
                self.streamAttachments.append(newAttachment)
            except Exception, e:
                if self._s_logger:
                    self._s_logger.trace( "ATTACH FAILURE, CONNECTION/STREAM %s/%s , EXCEPTION: %s" , connectionId, self.streamDef.id, str(e))
                raise
 
        def hasConnectionId(self, connectionId):
            for att in list(self.streamAttachments):
                if att.connectionId == connectionId:
                    return True 
            return False

        def getPort(self):
            return self.port

        def setPort(self, inPort):
            self.port = inPort

        def setLogger(self, inlogger):
            self._s_logger=inlogger
            for att in self.streamAttachments:
                att.setLogger(inlogger)

        def getConnectionIds(self):
            connectionIds = []
            for att in list(self.streamAttachments):
                connectionIds.append(att.connectionId)
            return connectionIds 

        def updateAttachments(self, expectedAttachments):
            expectedConnectionIds = []
            # Add new attachments that do not already exist
            for att in expectedAttachments:
                if not self.hasConnectionId(att.connectionId):
                    self.createNewAttachment(att.connectionId, att.inputPort)
                expectedConnectionIds.append(att.connectionId)

            # Iterate through attachments and compare to expected connectionIds
            connectionsToRemove = []
            for att in self.streamAttachments:
                existingConnectionId = att.connectionId
                detachConnection = True
                for connId in expectedConnectionIds:
                    if existingConnectionId == connId:
                        detachConnection = False
                        break
                if detachConnection == True:
                    # Store off and apply detach outside of this loop
                    # Removing now will mess up iterator
                    connectionsToRemove.append(existingConnectionId)
                    
            for connId in connectionsToRemove:
                self.detachByConnectionId(connId)
            
        def detachAll(self):
            for att in list(self.streamAttachments):
                att.detach()
                self.streamAttachments.remove(att)


    class StreamContainer:
        def __init__(self, streams=None):
            if streams == None:
                self.streams = []
            else:
               self.streams = streams
            self._sc_logger = None

        def printState(self, title):
            if self._sc_logger:
                self._sc_logger.debug(title)
            for stream in self.streams:
                self.printBlock("Stream", stream.streamId,0)
                for att in stream.streamAttachments:
                    self.printBlock("Attachment",att.attachId,1)
            if self._sc_logger:
                self._sc_logger.debug("")

        def printBlock(self, title, id, indents):
            indent = ""
            for ii in range(indents):
                indent += "    "
            line = "---------------"

            if self._sc_logger:
                self._sc_logger.debug(indent + " |" + line)
                self._sc_logger.debug(indent + " |" + str(title))
                self._sc_logger.debug(indent + " |   '" + str(id) + "'") 
                self._sc_logger.debug(indent + " |" + line)

        def hasStreams(self):
            if len(self.streams) > 0:
                return True
            else:
                return False

        def hasStreamId(self, streamId):
            for stream in self.streams:
                if stream.streamId == streamId:
                    return True
            return False

        def getStreamIds(self):
            streamIds = []
            for stream in self.streams:
                streamIds.append(stream.streamId)
            return streamIds

        def addConnectionToAllStreams(self, connectionId, port):
            for stream in self.streams:
                if not stream.hasConnectionId(connectionId):
                    stream.createNewAttachment(connectionId, port)

        def addConnectionToStream(self, connectionId, port, streamId):
            for stream in self.streams:
                if stream.streamId == streamId:
                    if not stream.hasConnectionId(connectionId):
                        stream.createNewAttachment(connectionId, port)

        def updateSRIForAllStreams(self, currentSRIs):
            for stream in self.streams:
                if currentSRIs.has_key(stream.streamId):
                    stream.sri = currentSRIs[stream.streamId].sri
                    stream.time = currentSRIs[stream.streamId].time

        def updateStreamSRI(self, streamId, sri):
            for stream in self.streams:
                if stream.streamId == streamId:
                    stream.sri = sri

        def updateStreamTime(self, streamId, time):
            for stream in self.streams:
                if stream.streamId == streamId:
                    stream.time = time

        def updateStreamSRIAndTime(self, streamId, sri, time):
            for stream in self.streams:
                if stream.streamId == streamId:
                    stream.sri = sri
                    stream.time = time

        def addStream(self, stream):
            self.streams.append(stream)

        def removeStreamByStreamId(self, streamId):
            for s in list(self.streams):
                if s.streamId == streamId:
                   s.detachAll()
                   self.streams.remove(s)

        def findByStreamId(self, streamId):
            for s in self.streams:
                if s.streamId == streamId:
                    return s
            return None            

        def detachByAttachIdConnectionId(self, attachId=None, connectionId=None):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.connectionId == connectionId and atts.inputPort and atts.attachId and atts.attachId == attachId:
                        atts.detach()
                        stream.streamAttachments.remove(atts)

        def detachAllStreams(self):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.inputPort and atts.attachId:
                        atts.detach()
                        stream.streamAttachments.remove(atts)

        def detachByConnectionId(self, connectionId=None):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.connectionId == connectionId and atts.inputPort and atts.attachId:
                        atts.detach()
                        stream.streamAttachments.remove(atts)

        def detachByAttachId(self, attachId=None):
            for stream in self.streams:
                for atts in list(stream.streamAttachments):
                    if atts.attachId and atts.attachId == attachId and atts.inputPort:
                        atts.detach()

        def findStreamAttachmentsByAttachId(self, attachId):
            attachList = []
            for stream in self.streams:
                for att in stream.streamAttachments:
                    if att.attachId == attachId:
                        attachList.append(att)    
            return attachList

        def setLogger(self, inlogger):
            self._sc_logger = inlogger
            for stream in self.streams:
                stream.setLogger(inlogger)


    TRANSFER_TYPE = 'c'
    def __init__(self, name, max_attachments=None, logger=None, interface=None ):
        OutPort.__init__(self, name, interface, OutAttachablePort.TRANSFER_TYPE, logger, bits=8)
        self.max_attachments = max_attachments
        self.streamContainer = OutAttachablePort.StreamContainer()
        self.sriDict = {} # key=streamID  value=SriMapStruct
        self.filterTable = []
        if not interface:
            if self._portLog:
                self._portLog.error("OutAttachablePort __init__ - an interface must be specified, set to BULKIO.dataSDDS or BULKIO.dataVITA49")
            raise Port.InvalidPort(1, "OutAttachablePort __init__ - an interface must be specified, set to BULKIO.dataSDDS or BULKIO.dataVITA49")
        self.interface=interface # BULKIO port interface (valid options are BULKIO.dataSDDS or BULKIO.dataVITA49)
        self.setLogger(self._portLog)
    
    def setLogger(self, in_logger):
        self._portLog = in_logger;
        self.streamContainer.setLogger(in_logger)

    def _get_state(self):
        self.port_lock.acquire()
        try:
            numberAttachedStreams = len(self._attachedStreams.values())
        finally:
            self.port_lock.release()
        if numberAttachedStreams == 0:
            return BULKIO.IDLE
        else:
            return BULKIO.ACTIVE

    def _get_attachedSRIs(self):
        return self._get_activeSRIs()

    def attachedStreams(self):
        streams = []
        for stream in self.streamContainer.streams:
            streams.append(stream.streamDef)
        return streams 

    def attachmentIds(self):
        ids = []
        for stream in self.streamContainer.streams:
            for atts in stream.streamAttachments:
                ids.append(atts.attachId)
        return ids 

    def attachmentIds(self,streamId):
        ids = []
        for stream in self.streamContainer.streams:
            if stream.streamId == streamId:
                for atts in stream.streamAttachments:
                    ids.append(atts.attachId)
                break
        return ids

    def connectPort(self, connection, connectionId):
        OutPort.connectPort( self, connection, connectionId )
        self.port_lock.acquire()
        try:
            try:
                portListed = False
                port = self.outConnections[str(connectionId)]

                if self._portLog:
                    self._portLog.trace("bulkio::OutAttachablePort connectPort(), Filter Table %s" % self.filterTable)
                for ftPtr in self.filterTable:
                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connectionId):
                        desiredStreamId = ftPtr.stream_id
                        self.streamContainer.addConnectionToStream(connectionId,port,desiredStreamId)

                if not portListed:
                    self.streamContainer.addConnectionToAllStreams(connectionId,port) 

                self.updateSRIForAllConnections()
            except Exception, e:
                if self._portLog:
                    self._portLog.error("CONNECTION FAILED, CONNECTION %s , EXCEPTION: %s" ,  connectionId, str(e))
                raise Port.InvalidPort(1, "Invalid Port for Connection ID:" + str(connectionId) )
        finally:
            self.port_lock.release()
        self.streamContainer.printState("After connectPort")
    
    def disconnectPort(self, connectionId):
        self.port_lock.acquire()
        try:
            try:
                self.streamContainer.detachByConnectionId(connectionId)
            except Exception, e:
                if self._portLog:
                    self._portLog.error("Unable to detach from stream before disconnecting port,  Connection: %s , Exception: %s", str(connectionId), str(e))

            if not self.outConnections.has_key(connectionId):
                if self._portLog:
                    self._portLog.warn("bulkio::OutAttachablePort  disconnectPort() - connectionId " + str(connectionId) + " is not contained in list of outConnections")
            else:
                self.outConnections.pop(connectionId, None)
                for key in self.sriDict.keys():
                    # if connID exist in set, remove it, otherwise do nothing (that is what discard does)
                    self.sriDict[key].connections.discard(connectionId)
                if self._portLog:
                    self._portLog.debug( "bulkio::OutAttachablePort DISCONNECT PORT:" + str(self.name) + " CONNECTION:" + str(connectionId))
                    self._portLog.trace( "bulkio::OutAttachablePort DISCONNECT PORT:" + str(self.name) + " updated sriDict" + str(self.sriDict))
        finally:
            self.port_lock.release()
        self.streamContainer.printState("After disconnectPort")

    def detach(self, attachId=None, connectionId=None):
        if self._portLog:
            self._portLog.trace("bulkio::OutAttachablePort, DETACH ENTER ")

        self.port_lock.acquire()
        try:
            if connectionId:
                for stream in self.streamContainer.streams:
                    stream.detachByConnectionId(connectionId)

            if attachId:
                for stream in self.streamContainer.streams:
                    for atts in list(stream.streamAttachments):
                        if atts.attachId == attachId:
                            atts.detach(attachId)
                            stream.streamAttachments.pop(atts)

            if not attachId and not connectionId:
                for stream in self.streamContainer.streams:
                    for atts in list(stream.streamAttachments):
                        atts.detach()
                self.streamContainer = OutAttachablePort.StreamContainer()
                self.streamContainer.setLogger(self._portLog)

        finally:
            self.port_lock.release()

        if self._portLog:
            self._portLog.trace("bulkio::OutAttachablePort, DETACH EXIT ")

    def attach(self, streamData, name):
        # Eventually deprecate attach() method for output port
        self.streamContainer.removeStreamByStreamId(streamData.id)
        self.addStream(streamData)
        return ""

    def updateStream(self, streamData):
        self.port_lock.acquire()
        streamId = streamData.id
        if (not self.streamContainer.hasStreamId(streamId)):
            return False;

        self.streamContainer.removeStreamByStreamId(streamId)
        self.port_lock.release()
        return self.addStream(streamData)


    def addStream(self, streamData):
        if self._portLog:
            self._portLog.trace("bulkio::OutAttachablePort, addStream ENTER ")

        ids = []
        self.port_lock.acquire()
        try:
            if self.streamContainer.hasStreamId(streamData.id):
                return False;

            stream = OutAttachablePort.Stream(streamDef=streamData, name="", streamId=streamData.id)
            stream.setLogger(self._portLog)
                                           

            portListed = False
            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:

                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == stream.streamId):
                        try:
                           if self.sriDict.has_key(stream.streamId):
                              sriMap = self.sriDict[stream.streamId]
                              stream.sri = sriMap.sri
                              stream.time = sriMap.time
                           stream.createNewAttachment(connId,port)
                        except Exception, e:
                           if  self.reportConnectionErrors(connId) :
                               if self._portLog:
                                   self._portLog.error("ATTACH FAILED, PORT/CONNECTION %s/%s , EXCEPTION: %s" , str(self.name), str(connId), str(e))

            if not portListed: 
                if self.sriDict.has_key(stream.streamId):
                    sriMap = self.sriDict[stream.streamId]
                    stream.sri = sriMap.sri
                    stream.time = sriMap.time
                for connId,port in self.outConnections.items():
                    try:
                       stream.createNewAttachment(connId,port)
                    except Exception, e:
                        if  self.reportConnectionErrors(connId) :
                            if self._portLog:
                                self._portLog.error("ATTACH FAILED, PORT/CONNECTION %s/%s , EXCEPTION: %s" , str(self.name), str(connId), str(e))

            self.streamContainer.addStream(stream) 
        
        finally:
            self.port_lock.release()

        for atts in stream.streamAttachments:
            ids.append(atts.attachId)
            if self._portLog:
                self._portLog.debug("bulkio.OutAttachablePort addStream()  PORT, ATTACH COMPLETED ID " + str(atts.attachId) + " CONNECTION ID:" + str(atts.connectionId))

        if self._portLog:
            self._portLog.trace("bulkio::OutAttachablePort, addStream EXIT ")
       
        self.streamContainer.printState("After addStream")
        return True

    def removeStream(self, streamId):
        self.streamContainer.removeStreamByStreamId(streamId)
        self.streamContainer.printState("After removeStream")

    def getStreamDefinition(self, attachId):
        streamDefList = []
        for stream in self.streamContainer.streams:
            for atts in stream.streamAttachments:
                if atts.attachId == attachId:
                    streamDefList.append(stream.streamDef)
        return streamDefList 

    def getUser(self, attachId):
        nameList = []
        for stream in self.streamContainer.streams:
            for atts in stream.streamAttachments:
                if atts.attachId == attachId:
                    nameList.append(stream.name)
        return nameList 
    
    def pushSRI(self, H, T):
        if self._portLog:
            self._portLog.trace("bulkio::OutAttachablePort, PUSH-SRI ENTER ")

        self.port_lock.acquire()
        try:
            sri = copy.deepcopy(H)
            sriTime = copy.deepcopy(T)
            self.sriDict[H.streamID] = OutPort.SriMapStruct(sri=sri, connections=set(), time=sriTime)
            portListed = False
            self.streamContainer.updateStreamSRIAndTime(H.streamID, sri, sriTime)

            for connId, port in self.outConnections.items():
                for ftPtr in self.filterTable:

                    # check if port was listed in connection filter table
                    if ftPtr.port_name == self.name:
                        portListed = True

                    if (ftPtr.port_name == self.name) and (ftPtr.connection_id == connId) and (ftPtr.stream_id == H.streamID):
                        try:
                            if port != None:
                                port.pushSRI(H, T)
                                self.sriDict[H.streamID].connections.add(connId)
                        except Exception, e:
                            if  self.reportConnectionErrors(connId) :
                                if self._portLog:
                                    self._portLog.exception("PUSH-SRI (attachable) FAILED, PORT/CONNECTION %s/%s , EXCEPTION: %s ", str(self.name), connId, str(e))

            if not portListed:
                for connId, port in self.outConnections.items():
                    try:
                        if port != None:
                            port.pushSRI(H, T)
                            self.sriDict[H.streamID].connections.add(connId)
                    except Exception, e:
                        if  self.reportConnectionErrors(connId) :
                            if self._portLog:
                                self._portLog.exception("PUSH-SRI (attachable) FAILED, PORT/CONNECTION %s/%s , EXCEPTION: %s ", str(self.name), connId, str(e))
        finally:
            self.port_lock.release() 

        if self._portLog:
            self._portLog.trace("bulkio::OutAttachablePort, PUSH-SRI EXIT ")

    def updateConnectionFilter(self, _filterTable):
        self.port_lock.acquire()
        try:
            if _filterTable == None :
                _filterTable = []
            self.filterTable = _filterTable

            #1. loop over filterTable
                #A. ignore other port_names
                #B. create mapping of streamid->connections(attachments) 

            hasPortEntry = False
            streamsFound = {}
            streamAttachments = {}
            # Populate streamsFound 
            knownStreamIds = self.streamContainer.getStreamIds()
            for id in knownStreamIds:
                streamsFound[id] = False

            # Iterate through each filterTable entry and capture state
            for entry in self.filterTable:
                if entry.port_name != self.name:
                    continue

                hasPortEntry = True
                if entry.connection_id in self.outConnections.keys():
                    connectedPort = self.outConnections.get(entry.connection_id)
                else:
                    if self._portLog:
                        self._portLog.trace("bulkio::OutAttachablePort, updateConnectionFilter() Unable to find connected port with connectionId: " + entry.connection_id)
                    continue

                if self.streamContainer.hasStreamId(entry.stream_id):
                    streamsFound[entry.stream_id] = True
                    expectedAttachment = OutAttachablePort.StreamAttachment(entry.connection_id, None, connectedPort)
                    if not streamAttachments.has_key(entry.stream_id):
                        streamAttachments[entry.stream_id] = []
                    streamAttachments[entry.stream_id].append(expectedAttachment)

            for streamId, expectedAttachements in streamAttachments.iteritems():
                foundStream = self.streamContainer.findByStreamId(streamId)
                if foundStream:
                    foundStream.updateAttachments(expectedAttachements)
                else:
                    if self._portLog:
                        self._portLog.warn("bulkio::OutAttachablePort, updateConnectionFilter() Unable to locate stream definition for streamId: " +streamId)

        
            if hasPortEntry:
                # If there's a valid port entry, we need to detach unmentioned streams
                for streamId,found in streamsFound.items():
                    if not found:
                        stream = self.streamContainer.findByStreamId(streamId)
                        if stream:
                            stream.detachAll()
            else:
                # No port entry == All connections on
                for connId, port in self.outConnections.items():
                    self.streamContainer.addConnectionToAllStreams(connId,port)

            self.updateSRIForAllConnections()

        finally:
            self.port_lock.release()
            self.streamContainer.printState("After updateFilterTable")

    def updateSRIForAllConnections(self):
        # Iterate through stream objects in container
        #   Check if sriDict has stream entry
        #     Yes: Check that ALL connections are listed in sriDict entry
        #          Update currentSRI
        #     No:  PushSRI on all attachment ports
        #          Update currentSRI

        # Iterate through all registered streams
        for stream in self.streamContainer.streams:
            streamConnIds = stream.getConnectionIds()

            # Check if sriDict has entry for StreamId
            if self.sriDict.has_key(stream.streamId):
                sriMap = self.sriDict[stream.streamId]

                # Check if all connections on the streams have pushed SRI
                currentSRIConnIds = sriMap.connections
                for connId in streamConnIds:

                    # If not found, pushSRI and update currentSRIs container
                    if not connId in currentSRIConnIds:

                        # Grab the port
                        if self.outConnections.has_key(connId):
                            connectedPort = self.outConnections[connId]
                            # Push sri and update sriMap 
                            connectedPort.pushSRI(sriMap.sri, sriMap.time)
                            sriMap.connections.add(connId)
                        else:
                            if self._portLog:
                                self._portLog.debug("updateSRIForAllConnections() Unable to find connected port with connectionId: " + connId)

class OutSDDSPort(OutAttachablePort):
    def __init__(self, name, max_attachments=None, logger=None ):
        OutAttachablePort.__init__(self, name, max_attachments, logger, interface=BULKIO.dataSDDS)

class OutVITA49Port(OutAttachablePort):
    def __init__(self, name, max_attachments=None, logger=None ):
        OutAttachablePort.__init__(self, name, max_attachments, logger, interface=BULKIO.dataVITA49)
