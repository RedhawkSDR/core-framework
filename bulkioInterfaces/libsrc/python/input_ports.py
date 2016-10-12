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
import collections
import copy
import time

from ossie.utils import uuid
from bulkio.statistics import InStats
from bulkio import sri
from bulkio import timestamp
from bulkio import const
from ossie.cf.CF import Port

from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA #@UnusedImport


class InPort:
    DATA_BUFFER=0
    TIME_STAMP=1
    END_OF_STREAM=2
    STREAM_ID=3
    SRI=4
    SRI_CHG=5
    QUEUE_FLUSH=6
    _TYPE_ = 'c'

    # Backwards-compatible DataTransfer type can still be unpacked like a tuple
    # but also supports named fields
    DataTransfer = collections.namedtuple('DataTransfer', 'dataBuffer T EOS streamID SRI sriChanged inputQueueFlushed')

    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None,  maxsize=100, PortTransferType=_TYPE_ ):
        self.name = name
        self.logger = logger
        self.queue = collections.deque()
        self._maxSize = maxsize
        self.port_lock = threading.Lock()
        self._not_full = threading.Condition(self.port_lock)
        self._not_empty = threading.Condition(self.port_lock)
        self._breakBlock = False
        self.stats =  InStats(name, PortTransferType)
        self.blocking = False
        self.sri_cmp = sriCompare
        self.newSriCallback = newSriCallback
        self.sriChangeCallback = sriChangeCallback
        self.sriDict = {} # key=streamID, value=StreamSRI

        _cmpMsg  = "DEFAULT"
        _newSriMsg  = "EMPTY"
        _sriChangeMsg  = "EMPTY"
        if sriCompare != sri.compare:
            _cmpMsg  = "USER_DEFINED"
        if newSriCallback:
            _newSriMsg  = "USER_DEFINED"
        if sriChangeCallback:
            _sriChangeMsg  = "USER_DEFINED"

        if self.logger:
            self.logger.debug( "bulkio::InPort CTOR port:" + str(name) +
                          " Blocking/MaxInputQueueSize " + str(self.blocking) + "/"  + str(maxsize) +
                          " SriCompare/NewSriCallback/SriChangeCallback " +  _cmpMsg + "/" + _newSriMsg + "/" + _sriChangeMsg );

    def setNewSriListener(self, newSriCallback):
        self.port_lock.acquire()
        try:
            self.newSriCallback = newSriCallback
        finally:
            self.port_lock.release()

    def setSriChangeListener(self, sriChangeCallback):
        self.port_lock.acquire()
        try:
            self.sriChangeCallback = sriChangeCallback
        finally:
            self.port_lock.release()

    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)

    def _get_statistics(self):
        self.port_lock.acquire()
        try:
            return self.stats.retrieve()
        finally:
            self.port_lock.release()

    def _get_state(self):
        self.port_lock.acquire()
        try:
            if len(self.queue) == 0:
                return BULKIO.IDLE
            elif len(self.queue) == self._maxSize:
                return BULKIO.BUSY
            else:
                return BULKIO.ACTIVE
        finally:
            self.port_lock.release()

    def _get_activeSRIs(self):
        self.port_lock.acquire()
        try:
            return [self.sriDict[entry][0] for entry in self.sriDict]
        finally:
            self.port_lock.release()

    def getCurrentQueueDepth(self):
        self.port_lock.acquire()
        try:
            return len(self.queue)
        finally:
            self.port_lock.release()

    def getMaxQueueDepth(self):
        self.port_lock.acquire()
        try:
            return self._maxSize
        finally:
            self.port_lock.release()

    #set to -1 for infinite queue
    def setMaxQueueDepth(self, newDepth):
        self.port_lock.acquire()
        try:
            self._maxSize = int(newDepth)
        finally:
            self.port_lock.release()

    def unblock(self):
        self.port_lock.acquire()
        try:
            self._breakBlock = False
        finally:
            self.port_lock.release()

    def block(self):
        self.port_lock.acquire()
        try:
            self._breakBlock = True
            self._not_empty.notifyAll()
        finally:
            self.port_lock.release()

    # Provide standard interface for start/stop
    startPort = unblock
    stopPort = block

    def pushSRI(self, H):

        if self.logger:
            self.logger.trace( "bulkio::InPort pushSRI ENTER (port=" + str(self.name) +")" )

        self.port_lock.acquire()
        try:
            if H.streamID not in self.sriDict:
                sriChanged = True
                if self.logger:
                    self.logger.debug( "pushSRI PORT:" + str(self.name) + " NEW SRI:" + str(H.streamID) )
                if self.newSriCallback:
                    self.newSriCallback( H )
                self.sriDict[H.streamID] = (copy.deepcopy(H), True)
                if H.blocking:
                    self.blocking = True
            else:
                sri, sriChanged = self.sriDict[H.streamID]
                if self.sri_cmp:
                    if not self.sri_cmp(sri, H):
                        self.sriDict[H.streamID] = (copy.deepcopy(H), True)
                        if H.blocking:
                            self.blocking = True
                        if self.sriChangeCallback:
                            self.sriChangeCallback( H )
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace( "bulkio::InPort pushSRI EXIT (port=" + str(self.name) +")" )


    def pushPacket(self, data, T, EOS, streamID):

        if self.logger:
            self.logger.trace( "bulkio::InPort pushPacket ENTER (port=" + str(self.name) +")" )

        self.port_lock.acquire()
        try:
            if self._maxSize == 0:
                if self.logger:
                    self.logger.trace( "bulkio::InPort pushPacket EXIT (port=" + str(self.name) +")" )
                return
            if self.sriDict.has_key(streamID):
                sri, sriChanged = self.sriDict[streamID]
                if EOS:
                    self.sriDict[streamID] = (sri, True)
                else:
                    self.sriDict[streamID] = (sri, False)
            else:
                # Create a default SRI for the stream ID
                if self.logger:
                    self.logger.warn("bulkio::InPort pushPacket received data for stream '%s' with no SRI", streamID)
                sri = BULKIO.StreamSRI(1, 0.0, 1.0, 1, 0, 0.0, 0.0, 0, 0, streamID, False, [])
                if self.newSriCallback:
                    self.newSriCallback(sri)
                self.sriDict[streamID] = (sri, False)
                sriChanged = True

            queueFlushed = False
            flagEOS = False
            if self.blocking:
                while len(self.queue) >= self._maxSize:
                    self._not_full.wait()
            else:
                # Flush the queue if not using infinite queue (maxSize == -1), blocking is not on, and
                #  current length of queue is >= maxSize
                if len(self.queue) >= self._maxSize and self._maxSize > -1:
                    queueFlushed = True
                    if self.logger:
                        self.logger.debug("bulkio::InPort pushPacket PURGE INPUT QUEUE (SIZE=%d)", len(self.queue))

                    foundSRIChanged = False
                    for data, T, EOS, streamID, sri, sriChangeHappened, inputQueueFlushed in self.queue:
                        if foundSRIChanged and flagEOS:
                            break
                        if sriChangeHappened:
                            sriChanged = True
                            foundSRIChanged = True
                        if EOS:
                            flagEOS = True
                    self.queue.clear()

            if flagEOS:
                EOS = True
            packet = (data, T, EOS, streamID, copy.deepcopy(sri), sriChanged, queueFlushed)
            self.stats.update(self._packetSize(data), float(len(self.queue))/float(self._maxSize), EOS, streamID, queueFlushed)
            if self.logger:
                self.logger.trace("bulkio::InPort pushPacket NEW Packet (QUEUE=%d)", len(self.queue))
            self.queue.append(packet)

            # Let one waiting getPacket call know there is a packet available
            self._not_empty.notify()
        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace( "bulkio::InPort pushPacket EXIT (port=" + str(self.name) +")" )

    def getPacket(self, timeout=const.NON_BLOCKING):

        if self.logger:
            self.logger.trace( "bulkio::InPort getPacket ENTER (port=" + str(self.name) +")" )

        self.port_lock.acquire()
        try:
            if timeout < 0.0:
                while not self.queue and not self._breakBlock:
                    self._not_empty.wait()
            elif timeout > 0.0:
                # Determine the absolute time at which the timeout expires
                end = time.time() + timeout
                while not self.queue and not self._breakBlock:
                    # Calculate remaining timeout
                    remain = end - time.time()
                    if remain <= 0.0:
                        break
                    self._not_empty.wait(remain)

            if not self.queue:
                return InPort.DataTransfer(None, None, None, None, None, None, None)

            data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed = self.queue.popleft()

            # Let one waiting pushPacket call know there is space available
            self._not_full.notify()

            if EOS:
                if self.sriDict.has_key(streamID):
                    (a,b) = self.sriDict.pop(streamID)
                    if sri.blocking:
                        stillBlock = False
                        for _sri, _sriChanged in self.sriDict.values():
                            if _sri.blocking:
                                stillBlock = True
                                break
                        if not stillBlock:
                            self.blocking = False
            return InPort.DataTransfer(data, T, EOS, streamID, sri, sriChanged, inputQueueFlushed)
        finally:
            self.port_lock.release()

            if self.logger:
                self.logger.trace( "bulkio::InPort getPacket EXIT (port=" + str(self.name) +")" )

    def _packetSize(self, data):
        return len(data)


class InCharPort(InPort, BULKIO__POA.dataChar):
    _TYPE_ = 'c'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InCharPort._TYPE_ )

class InOctetPort(InPort, BULKIO__POA.dataOctet):
    _TYPE_ = 'B'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InOctetPort._TYPE_ )

class InShortPort(InPort, BULKIO__POA.dataShort):
    _TYPE_ = 'h'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InShortPort._TYPE_ )

class InUShortPort(InPort, BULKIO__POA.dataUshort):
    _TYPE_ = 'H'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InUShortPort._TYPE_ )

class InLongPort(InPort, BULKIO__POA.dataLong):
    _TYPE_ = 'i'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InLongPort._TYPE_ )

class InULongPort(InPort, BULKIO__POA.dataUlong):
    _TYPE_ = 'I'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InULongPort._TYPE_ )

class InLongLongPort(InPort, BULKIO__POA.dataLongLong):
    _TYPE_ = 'q'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InLongLongPort._TYPE_ )


class InULongLongPort(InPort, BULKIO__POA.dataUlongLong):
    _TYPE_ = 'Q'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InULongLongPort._TYPE_ )


class InFloatPort(InPort, BULKIO__POA.dataFloat):
    _TYPE_ = 'f'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InFloatPort._TYPE_ )


class InDoublePort(InPort, BULKIO__POA.dataDouble):
    _TYPE_ = 'd'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InDoublePort._TYPE_ )


class InFilePort(InPort, BULKIO__POA.dataFile):
    _TYPE_ = 'd'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InFilePort._TYPE_ )

    def _packetSize(self, data):
        # For statistics, consider the entire URL a single element
        return 1


class InXMLPort(InPort, BULKIO__POA.dataXML):
    _TYPE_ = 'd'
    def __init__(self, name, logger=None, sriCompare=sri.compare, newSriCallback=None, sriChangeCallback=None, maxsize=100 ):
        InPort.__init__(self, name, logger, sriCompare, newSriCallback, sriChangeCallback, maxsize, InXMLPort._TYPE_ )

    def pushPacket(self, xml_string, EOS, streamID):
        # Insert a None for the timestamp and use parent implementation
        InPort.pushPacket(self, xml_string, None, EOS, streamID)

class InAttachablePort:
    _TYPE_='b'
    def __init__(self, name, logger=None, attachDetachCallback=None, sriCmp=sri.compare, timeCmp=timestamp.compare, PortType = _TYPE_, newSriCallback=None, sriChangeCallback=None,interface=None):
        self.name = name
        self.logger = logger
        self.port_lock = threading.Lock()
        self.sri_query_lock = threading.Lock()
        self._attachedStreams = {} # key=attach_id, value = (streamDef, userid)
        self.stats = InStats(name, PortType )
        self.sriDict = {} # key=streamID, value=(StreamSRI, PrecisionUTCTime)
        self.attachDetachCallback = attachDetachCallback
        self.newSriCallback = newSriCallback
        self.sriChangeCallback = sriChangeCallback
        self.sri_cmp = sriCmp
        self.time_cmp = timeCmp
        self.sriChanged = False
        if not interface:
            if self.logger:
                self.logger.error("InAttachablePort __init__ - an interface must be specified, set to BULKIO.dataSDDS or BULKIO.dataVITA49")
            raise Port.InvalidPort(1, "InAttachablePort __init__ - an interface must be specified, set to BULKIO.dataSDDS or BULKIO.dataVITA49")
        self.interface=interface # BULKIO port interface (valid options are BULKIO.dataSDDS or BULKIO.dataVITA49)
        self.setNewAttachDetachListener(attachDetachCallback)
        if self.logger:
            self.logger.debug("bulkio::InAttachablePort CTOR port:" + str(self.name) + " using interface " + str(self.interface))

    def setNewAttachDetachListener(self, attachDetachCallback ):
        self.port_lock.acquire()
        try:
            self.attachDetachCallback = attachDetachCallback

            # Set _attach_cb
            try:
                self._attach_cb = getattr(attachDetachCallback, "attach")
                if not callable(self._attach_cb):
                    self._attach_cb = None
            except AttributeError:
                self._attach_cb = None

            # Set _detach_cb
            try:
                self._detach_cb = getattr(attachDetachCallback, "detach")
                if not callable(self._detach_cb):
                    self._detach_cb = None
            except AttributeError:
                self._detach_cb = None

        finally:
            self.port_lock.release()

    def setNewSriListener(self, newSriCallback):
        self.port_lock.acquire()
        try:
            self.newSriCallback = newSriCallback
        finally:
            self.port_lock.release()

    def setSriChangeListener(self, sriChangeCallback):
        self.port_lock.acquire()
        try:
            self.sriChangeCallback = sriChangeCallback
        finally:
            self.port_lock.release()

    def setBitSize(self, bitSize):
        self.stats.setBitSize(bitSize)

    def enableStats(self, enabled):
        self.stats.setEnabled(enabled)

    def updateStats(self, elementsReceived, queueSize, streamID):
        self.port_lock.acquire()
        try:
            self.stats.update(elementsReceived, queueSize, streamID)
        finally:
            self.port_lock.release()

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
            numAttachedStreams = len(self._attachedStreams.values())
        finally:
            self.port_lock.release()
        if numAttachedStreams == 0:
            return BULKIO.IDLE
        # default behavior is to limit to one connection
        elif numAttachedStreams == 1:
            return BULKIO.BUSY
        else:
            return BULKIO.ACTIVE

    def _get_attachedSRIs(self):
        sris = []
        self.sri_query_lock.acquire()
        try:
            for entry in self.sriDict:
                # First value of sriDict entry is the StreamSRI object
                sris.append(copy.deepcopy(self.sriDict[entry][0]))
        finally:
            self.sri_query_lock.release()
        return sris

    def _get_usageState(self):
        self.port_lock.acquire()
        try:
            numAttachedStreams = len(self._attachedStreams.values())
        finally:
            self.port_lock.release()
        if numAttachedStreams == 0:
            return self.interface.IDLE
        # default behavior is to limit to one connection
        elif numAttachedStreams == 1:
            return self.interface.BUSY
        else:
            return self.interface.ACTIVE

    def _get_attachedStreams(self):
        return [x[0] for x in self._attachedStreams.values()]

    def _get_attachmentIds(self):
        return self._attachedStreams.keys()

    def attach(self, streamDef, userid):

        if self.logger:
            self.logger.trace("bulkio::InAttachablePort attach ENTER  (port=" + str(self.name) +")" )
            self.logger.debug("InAttachablePort.attach() - ATTACH REQUEST, STREAM/USER"  + str(streamDef) + '/' + str(userid))

        attachId = None
        self.port_lock.acquire()
        try:
            try:
                if self.logger:
                    self.logger.debug("InAttachablePort.attach() - CALLING ATTACH CALLBACK, STREAM/USER"  + str(streamDef) + '/' + str(userid) )
                if self._attach_cb != None:
                    attachId = self._attach_cb(streamDef, userid)
            except Exception, e:
                if self.logger:
                    self.logger.error("InAttachablePort.attach() - ATTACH CALLBACK EXCEPTION : " + str(e) + " STREAM/USER"  + str(streamDef) + '/' + str(userid) )
                raise self.interface.AttachError(str(e))

            if attachId == None:
                attachId = str(uuid.uuid4())

            self._attachedStreams[attachId] = (streamDef, userid)

        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.debug("InAttachablePort.attach() - ATTACH COMPLETED,  ID:" + str(attachId) + " STREAM/USER: " + str(streamDef) + '/' + str(userid))
            self.logger.trace("bulkio::InAttachablePort attach EXIT (port=" + str(self.name) +")" )

        return attachId

    def detach(self, attachId):

        if self.logger:
            self.logger.trace("bulkio::InAttachablePort detach ENTER (port=" + str(self.name) +")" )
            self.logger.debug("InAttachablePort.detach() - DETACH REQUESTED, ID:" + str(attachId) )

        self.port_lock.acquire()
        try:
            if not self._attachedStreams.has_key(attachId):

                if self.logger:
                    self.logger.debug("InAttachablePort.detach() - DETACH UNKNOWN ID:" + str(attachId) )

                if attachId:
                    raise self.interface.DetachError("Stream %s not attached" % str(attachId))
                else:
                    raise self.interface.DetachError("Cannot detach Unkown ID")

            attachedStreamDef, refcnf = self._attachedStreams[attachId]

            #
            # Deallocate capacity here if applicable
            #
            try:
                if self.logger:
                    self.logger.debug("InAttachablePort.detach() - CALLING DETACH CALLBACK, ID:" + str(attachId) )

                if self._detach_cb != None:
                    self._detach_cb(attachId)
            except Exception, e:
                if self.logger:
                    self.logger.error("InAttachablePort.detach() - DETACH CALLBACK EXCEPTION: " + str(e) )
                raise self.interface.DetachError(str(e))

            # Remove the attachment from our list
            del self._attachedStreams[attachId]

        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.debug("InAttachablePort.detach() - DETACH SUCCESS, ID:" + str(attachId) )
            self.logger.trace("bulkio::InAttachablePort detach EXIT (port=" + str(self.name) +")" )

    def getStreamDefinition(self, attachId):
        try:
            return self._attachedStreams[attachId][0]
        except KeyError:
            raise self.interface.StreamInputError("Stream %s not attached" % attachId)

    def getUser(self, attachId):
        try:
            return self._attachedStreams[attachId][1]
        except KeyError:
            raise self.interface.StreamInputError("Stream %s not attached" % attachId)

    def _get_activeSRIs(self):
        self.sri_query_lock.acquire()
        try:
            activeSRIs = [self.sriDict[entry][0] for entry in self.sriDict]
        finally:
            self.sri_query_lock.release()
        return activeSRIs

    def pushSRI(self, H, T):

        if self.logger:
            self.logger.trace("bulkio::InAttachablePort pushSRI ENTER (port=" + str(self.name) +")" )

        self.port_lock.acquire()
        try:
            if H.streamID not in self.sriDict:
                if self.newSriCallback:
                    self.newSriCallback( H )
                # Disable querying while adding a new SRI
                self.sri_query_lock.acquire()
                try:
                    self.sriDict[H.streamID] = (copy.deepcopy(H), copy.deepcopy(T))
                finally:
                    self.sri_query_lock.release()
            else:
                cur_H, cur_T = self.sriDict[H.streamID]
                s_same = False
                if self.sri_cmp:
                    s_same = self.sri_cmp(cur_H, H)

                t_same = False
                if self.time_cmp:
                    t_same = self.time_cmp(cur_T, T)

                self.sriChanged = ( s_same == False )  or  ( t_same == False )
                if self.sriChanged and self.sriChangeCallback:
                    self.sriChangeCallback( H )
                # Disable querying while adding a new SRI
                self.sri_query_lock.acquire()
                try:
                    self.sriDict[H.streamID] = (copy.deepcopy(H), copy.deepcopy(T))
                finally:
                    self.sri_query_lock.release()

        finally:
            self.port_lock.release()

        if self.logger:
            self.logger.trace("bulkio::InAttachablePort pushSRI EXIT (port=" + str(self.name) +")" )

class InSDDSPort(BULKIO__POA.dataSDDS,InAttachablePort):
    def __init__(self, name, logger=None, attachDetachCallback=None, sriCmp=None, timeCmp=None, PortType = 'b', newSriCallback=None, sriChangeCallback=None ):
        InAttachablePort.__init__(self, name, logger, attachDetachCallback, sriCmp, timeCmp, PortType, newSriCallback, sriChangeCallback, interface=BULKIO.dataSDDS)

class InVITA49Port(BULKIO__POA.dataVITA49,InAttachablePort):
    def __init__(self, name, logger=None, attachDetachCallback=None, sriCmp=None, timeCmp=None, PortType = 'b', newSriCallback=None, sriChangeCallback=None ):
        InAttachablePort.__init__(self, name, logger, attachDetachCallback, sriCmp, timeCmp, PortType, newSriCallback, sriChangeCallback, interface=BULKIO.dataVITA49)
