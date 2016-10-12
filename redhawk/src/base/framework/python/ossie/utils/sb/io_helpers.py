#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#


try:
    from bulkio.bulkioInterfaces import BULKIO as _BULKIO
    from bulkio.bulkioInterfaces import BULKIO__POA as _BULKIO__POA
except:
    # Handle case where bulkioInterface may not be installed
    pass
import domainless as _domainless
import threading as _threading
import ossie.utils.bulkio.bulkio_helpers as _bulkio_helpers
import ossie.utils.bluefile.bluefile_helpers as _bluefile_helpers
import ossie.utils.bulkio.bulkio_data_helpers as _bulkio_data_helpers
import ossie.utils.bluefile.bluefile as _bluefile
from ossie import properties as _properties
from ossie import events as _events
from ossie.cf import CF as _CF
import shlex as _shlex
import time as _time
import signal as _signal
import os as _os
import subprocess as _subprocess
import Queue as _Queue
import struct as _struct
import logging as _logging
from omniORB import any as _any
from omniORB import CORBA as _CORBA

from ossie.utils.model import PortSupplier, OutputBase
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

# Use orb reference from domainless
import domainless

log = _logging.getLogger(__name__)

__all__ = ('DataSink', 'DataSource', 'FileSink', 'FileSource', 'MessageSink',
           'MessageSource','Plot', 'SRIKeyword', 'compareSRI', 'helperBase',
           'probeBULKIO')

def compareSRI(a, b):
    '''
    Compare the content of two SRI objects
    '''
    if a.hversion != b.hversion:
        return False
    if a.xstart != b.xstart:
        return False
    if a.xdelta != b.xdelta:
        return False
    if a.xunits != b.xunits:
        return False
    if a.subsize != b.subsize:
        return False
    if a.ystart != b.ystart:
        return False
    if a.ydelta != b.ydelta:
        return False
    if a.yunits != b.yunits:
        return False
    if a.mode != b.mode:
        return False
    if a.streamID != b.streamID:
        return False
    if a.blocking != b.blocking:
        return False
    if len(a.keywords) != len(b.keywords):
        return False
    for keyA, keyB in zip(a.keywords, b.keywords):
        if keyA.value._t != keyB.value._t:
            return False
        if keyA.value._v != keyB.value._v:
            return False
    return True

def _checkComplex(data):
    for item in data:
        if isinstance(item, complex):
            return True
    return False

class helperBase(object):
    def __init__(self):
        # Create a unique instance identifier.
        self._refid = str(uuid4())
        self._sandbox = _domainless._getSandbox()

        # Create a unique instance name and register with the sandbox.
        baseName = '__local' + self.__class__.__name__
        self._instanceName = self._sandbox._createInstanceName(baseName)
        self._sandbox._registerComponent(self)

    def releaseObject(self):
        # Break any connections involving this component.
        manager = ConnectionManager.instance()
        for identifier, (uses, provides) in manager.getConnections().items():
            if uses.hasComponent(self) or provides.hasComponent(self):
                usesRef = uses.getReference()
                usesRef.disconnectPort(identifier)
                manager.unregisterConnection(identifier)
        self._sandbox._unregisterComponent(self)

    def reset(self):
        pass

class MessageSink(helperBase, PortSupplier):
    def __init__(self, messageId = None, messageFormat = None, messageCallback = None):
        helperBase.__init__(self)
        PortSupplier.__init__(self)
        self._messagePort = None
        self._messageId = messageId
        self._messageFormat = messageFormat
        self._messageCallback = messageCallback
        self._providesPortDict = {}
        self._providesPortDict['msgIn'] = {
            'Port Interface': 'IDL:ExtendedEvent/MessageEvent:1.0',
            'Port Name': 'msgIn'
            }

    def messageCallback(self, msgId, msgData):
        print msgId, msgData

    def getPort(self, portName):
        try:
            if self._messageCallback == None:
                self._messageCallback = self.messageCallback
            self._messagePort = _events.MessageConsumerPort(thread_sleep=0.1)
            self._messagePort.registerMessage(self._messageId,
                                         self._messageFormat, self._messageCallback)
            return self._messagePort._this()
        except Exception, e:
            log.error("MessageSink:getPort(): failed " + str(e))
        return None

    def api(self):
        print "Component MessageSink :"
        PortSupplier.api(self)

    def start(self):
        pass

    def stop(self):
        pass

    def eos(self):
        return False

    def sri(self):
        return None

class MessageSource(helperBase, PortSupplier):
    def __init__(self, messageId = None, messageFormat = None):
        helperBase.__init__(self)
        PortSupplier.__init__(self)
        self._messagePort = None
        self._messageId = messageId
        self._messageFormat = messageFormat

        self._usesPortDict['msgOut'] = {
            'Port Interface': 'IDL:ExtendedEvent/MessageEvent:1.0',
            'Port Name': 'msgOut'
            }

    def sendMessage(self, msg):
        outmsg = None
        if hasattr(msg, 'getId'): # this is a struct
            outgoing = [_CF.DataType(id=msg.getId(),value=_properties.struct_to_any(msg))]
            outmsg = _properties.props_to_any(outgoing)
        elif type(msg) == dict: # this is a dictionary
            payload = []
            for entry in msg:
                payload.append(_CF.DataType(id=str(entry),value=_any.to_any(msg[entry])))
            outgoing = [_CF.DataType(id="sb_struct",value=_properties.props_to_any(payload))]
            outmsg = _properties.props_to_any(outgoing)
        elif isinstance(msg,_CORBA.Any): # this is an Any
            payload = [_CF.DataType(id="sb",value=msg)]
            outgoing = [_CF.DataType(id="sb_struct",value=_properties.props_to_any(payload))]
            outmsg = _properties.props_to_any(outgoing)
        else: # assume it's something that can be mapped to an any
            payload = [_CF.DataType(id="sb",value=_any.to_any(msg))]
            outgoing = [_CF.DataType(id="sb_struct",value=_properties.props_to_any(payload))]
            outmsg = _properties.props_to_any(outgoing)
        self._messagePort.sendMessage(outmsg)

    def getPort(self, portName):
        try:
            self._messagePort = _events.MessageSupplierPort()
            return self._messagePort._this()
        except Exception, e:
            log.error("MessageSource:getPort(): failed " + str(e))
        return None

    def connectPort(self, connection, connectionId):
        self._messagePort.connectPort(connection, connectionId)

    def disconnectPort(self, connectionId):
        self._messagePort.disconnectPort(connectionId)

    def getUsesPort(self):
        try:
            if self._messagePort == None:
                return self.getPort('')
            else:
                return self._messagePort._this()
        except Exception, e:
            log.error("MessageSource:getUsesPort(): failed " + str(e))
        return None

    def api(self):
        print "Component MessageSource :"
        PortSupplier.api(self)

    def start(self):
        pass

    def stop(self):
        pass

    def eos(self):
        return False

    def sri(self):
        return None

class _DataPortBase(helperBase, PortSupplier):

    def __init__(self, portNameAppendix = "", formats=None):
        """
        Protected

        Sets self.className and calls parent constructor.

        Defines self.supportedPorts, which determines what interface types are
        supported by inheriting classes.

        """
        helperBase.__init__(self)
        PortSupplier.__init__(self)

        self.className = self.__class__.__name__ # for DEBUG/WARNING statements

        self.portNameAppendix = portNameAppendix

        # Note: self._defaultDataFormat needs to be supported in self.supportedPorts
        self._defaultDataFormat = "short"
        self._defaultDataPortName = self._defaultDataFormat + self.portNameAppendix


        # top-level keys should be all lower-case
        #
        # To add support for a new interface type, simply add it to the
        # dictionary following the format of the other entries.  The
        # current list of entries represents supported interfaces that
        # have been verified to work; this list does not include all
        # interfaces that might work.
        self.supportedPorts = {
             "char"      : {"bytesPerSample" : 1,
                            "pktSize"        : -1,
                            "format"         : 'b',
                            "portType" : "_BULKIO__POA.dataChar",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataChar:1.0",
                                          "Port Name"      : "char"}},
             "short"     : {"bytesPerSample" : 2,
                            "pktSize"        : -1,
                            "format"         : 'h',
                            "portType" : "_BULKIO__POA.dataShort",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataShort:1.0",
                                          "Port Name"      : "short"}},
             "long"      : {"bytesPerSample" : 4,
                            "pktSize"        : -1,
                            "format"         : 'i',
                            "portType" : "_BULKIO__POA.dataLong",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataLong:1.0",
                                          "Port Name"      : "long"}},
             "float"     : {"bytesPerSample" : 4,
                            "pktSize"        : -1,
                            "format"         : 'f',
                            "portType" : "_BULKIO__POA.dataFloat" ,
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataFloat:1.0",
                                          "Port Name"      : "float"}},
             "ulong"     : {"bytesPerSample" : 4,
                            "pktSize"        : -1,
                            "format"         : 'I',
                            "portType" : "_BULKIO__POA.dataUlong" ,
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataUlong:1.0",
                                          "Port Name"      : "ulong"}},
             "double"    : {"bytesPerSample" : 8,
                            "pktSize"        : -1,
                            "format"         : 'd',
                            "portType" : "_BULKIO__POA.dataDouble",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataDouble:1.0",
                                          "Port Name"      : "double"}},
             "longlong"  : {"bytesPerSample" : 8,
                            "pktSize"        : -1,
                            "format"         : 'q',
                            "portType" : "_BULKIO__POA.dataLongLong",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataLongLong:1.0",
                                          "Port Name"      : "longlong"}},
             "octet"     : {"bytesPerSample" : 1,
                            "pktSize"        : -1,
                            "format"         : 'B',
                            "portType" : "_BULKIO__POA.dataOctet",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataOctet:1.0",
                                          "Port Name"      : "octet"}},
             "ulonglong" : {"bytesPerSample" : 8,
                            "pktSize"        : -1,
                            "format"         : 'Q',
                            "portType" : "_BULKIO__POA.dataUlongLong",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataUlongLong:1.0",
                                          "Port Name"      : "ulonglong"}},
             "ushort"    : {"bytesPerSample" : 2,
                            "pktSize"        : -1,
                            "format"         : 'H',
                            "portType" : "_BULKIO__POA.dataUshort",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataUshort:1.0",
                                          "Port Name"      : "ushort"}},
             "file"      : {"bytesPerSample" : 1,
                            "pktSize"        : -1,
                            "format"         : 's',
                            "portType" : "_BULKIO__POA.dataFile",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataFile:1.0",
                                          "Port Name"      : "file"}},
             "xml"       : {"bytesPerSample" : 1,
                            "pktSize"        : -1,
                            "format"         : 's',
                            "portType" : "_BULKIO__POA.dataXML",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataXML:1.0",
                                          "Port Name"      : "xml"}}}

        # Allow subclasses to support only a subset of formats
        if formats is not None:
            for name in self.supportedPorts.keys():
                if name not in formats:
                    del self.supportedPorts[name]

    def _getMetaByPortName(self, key, portName):
        """
        Search through self.supportedPorts for the port with 'Port Name'
        equal to portName.  When the port is found, return the value
        associated with the key 'key' of that port.

        """

        for port in self.supportedPorts.values():
            if port["portDict"]["Port Name"] == portName:
                return port[key]


    def getPortByName(self, portName):
        """
        Returns the entry of self.supportedPorts associated with portName.

        """

        for port in self.supportedPorts.values():
            if port["portDict"]["Port Name"] == portName:
                return port

        # portName not found in self.supportedPorts.  This should never happen
        # as the portName should be set via self.supportedPorts.
        raise Exception, "Port name " + portName + " not found."

    def api(self):
        """
        Prints application programming interface (API) information and returns.

        """
        print "Component " + self.__class__.__name__ + " :"
        PortSupplier.api(self)


class _SourceBase(_DataPortBase):

    def __init__(self, bytesPerPush, dataFormat, data = None, formats=None):
        """
        Forward parameters to parent constructor.

        Set self._dataFormat to dataFormat it it is in the list
        of supported ports, otherwise attempt to guess the format
        based on the data or set the format to self._defaultDataFormat.

        Calls _buildAPI()

        """
        _DataPortBase.__init__(self, portNameAppendix = "Out", formats=formats)

        self.bytesPerPush = int(bytesPerPush)

        if data != None:
            log.warn("Predicting data source format type by data argument is deprecated.  This will be removed in the next version")

        if dataFormat != None:
            # add support for format byte (which is the same as char)
            if dataFormat.lower() == 'byte':
                dataFormat = 'char'
            if self.supportedPorts.has_key(dataFormat.lower()):
                self._dataFormat = dataFormat.lower()
            else:
                self._dataFormat = None
        else:
            self._dataFormat = None

        self._connections = {}
        self._buildAPI()

    def _addConnection(self, portName, arraySrcInst):
        """
        When a connection is made, this method should be called
        to update the running list of array sources.

        """

        pktSize = self._getMetaByPortName("pktSize", portName)
        srcPortType = self._getMetaByPortName("portType", portName)

        connection = {"pktSize"      : pktSize,
                      "srcPortType"  : srcPortType,
                      "arraySrcInst" : arraySrcInst}

        self._connections[portName] = connection

    def _addUsesPort(self, port):
        """
        Adds the port to self._usesPortDictionary.

        Also sets the pktSize associated with the port.

        """
        name = port['portDict']['Port Name'] + self.portNameAppendix
        self._usesPortDict[name] = port["portDict"]
        self._usesPortDict[name]["Port Name"] = name
        # Determine number of elements per pushPacket
        # NOTE: complex data is treated real data with alternating real and
        # imaginary values
        self._pktSize = self.bytesPerPush / port["bytesPerSample"]
        port["pktSize"] = self._pktSize

    def _buildAPI(self):
        """
        Private

        Populates self._srcPortType, and self.usesPortDict using the
        self.supportedPorts dictionary as indexed by the self._dataFormat

        """
        if self._dataFormat != None:
            port = self.supportedPorts[self._dataFormat]
            self._srcPortType = port["portType"]
            self._addUsesPort(port)
        else:
            for port in self.supportedPorts.values():
                self._addUsesPort(port)


        if _domainless._DEBUG == True:
            print self.className + ":_buildAPI()"
            self.api()

    def getPort(self, name):
        if name in self._connections:
            arraySrcInst = self._connections[name]['arraySrcInst']
        else:
            srcPortType = self._getMetaByPortName("portType", name)
            arraySrcInst = self._createArraySrcInst(srcPortType)
            self._addConnection(name, arraySrcInst)
        return arraySrcInst.getPort()


class _SinkBase(_DataPortBase):

    def __init__(self, formats=None):
        """
        Forward parameters to parent constructor.

        Calls _buildAPI()

        """
        _DataPortBase.__init__(self, portNameAppendix = "In", formats=formats)

        # Amount of time the self._sink is given to write to the gotEOS flag
        self._sleepTime = 0.001
        self.breakBlock = False

        self._sinkPortObject = None
        self._sinkPortType = None
        self._sink = None
        self.ref = self
        self._buildAPI()

    def _buildAPI(self):
        """
        Populate the self._providesPortDict with one entry per possible
        port type defined in self.supportedPorts.

        """
        for port in self.supportedPorts.values():
            name = port['portDict']['Port Name'] + self.portNameAppendix
            self._providesPortDict[name] = port["portDict"]
            self._providesPortDict[name]["Port Name"] = name

        if _domainless._DEBUG == True:
            print self.className + ":_buildAPI()"
            self.api()

    def getPortType(self, portName):
        """
        Uses self.getPortByName to retrieve the prt type
        associated with the portName.

        """

        return self.getPortByName(portName)["portType"]

    def start(self):
        pass

    def stop(self):
        """
        Sets a flag to break the module out of a loop that
        is waiting for an EOS flag (if applicable).

        """

        self.breakBlock = True

    def eos(self):
        """
        Returns the end of sequence (EOS) state of self._sink, or false if no
        self._sink exists.

        """

        if self._sink == None:
            return False
        return self._sink.gotEOS

    def sri(self):
        """
        Returns the signal related information (SRI) associated with of
        self._sink, or false if no self._sink exists.

        """
        if self._sink == None:
            return None
        else:
            return self._sink.sri


class FileSource(_SourceBase):
    def __init__(self,
                 filename     = "",
                 dataFormat   = None,
                 midasFile    = False,
                 sampleRate   = 1.0,
                 complexData  = False,
                 SRIKeywords  = [],
                 bytesPerPush = 512000,
                 startTime    = 0.0,
                 streamID     = None,
                 blocking     = True):

        self._filename = filename
        self._midasFile = midasFile
        self._src         = None
        self._runThread   = None
        self._sampleRate  = sampleRate 
        self._complexData = complexData
        self._SRIKeywords = SRIKeywords
        self._startTime   = startTime
        self._streamID    = streamID
        self._blocking    = blocking 
        self._sri         = None
        self._byteswap    = False
        self._defaultDataFormat = '16t'

        # the following data format code is messy because the arguments to the filesource can be either from the list
        # of reserved type names ('short','char','octet','long','ulong','longlong','ulonglong','float','double')
        # or from a combined string (<bits><type><complex?><reversed?>
        # for the follow-on functionality expects a type declaration from the reserved type name list
        # and flags to determine whether or not it's reversed or complex
        
        if dataFormat == None:
            log.warn("dataFormat not provided for FileSource; defaulting to " + self._defaultDataFormat)
            dataFormat = self._defaultDataFormat
            if complexData:
                dataFormat = dataFormat + 'c'

        reserved_type_names = ['short','char','octet','long','ulong','longlong','ulonglong','float','double']
        if dataFormat.lower() in reserved_type_names:
            if dataFormat.lower() == 'short':
                dataFormat = '16t'
            elif dataFormat.lower() == 'char':
                dataFormat = '8t'
            elif dataFormat.lower() == 'octet':
                dataFormat = '8u'
            elif dataFormat.lower() == 'long':
                dataFormat = '32t'
            elif dataFormat.lower() == 'ulong':
                dataFormat = '32u'
            elif dataFormat.lower() == 'longlong':
                dataFormat = '64t'
            elif dataFormat.lower() == 'ulonglong':
                dataFormat = '64u'
            elif dataFormat.lower() == 'float':
                dataFormat = '32f'
            elif dataFormat.lower() == 'double':
                dataFormat = '64f'
            if complexData:
                dataFormat = dataFormat + 'c'

        if self._midasFile:
            hdr, data = _bluefile.read(self._filename, list)
            if hdr['format'].endswith('B'):
                dataFormat = 'char'
            elif hdr['format'].endswith('I'):
                dataFormat = 'short'
            elif hdr['format'].endswith('L'):
                dataFormat = 'long'
            elif hdr['format'].endswith('F'):
                dataFormat = 'float'
            elif hdr['format'].endswith('D'):
                dataFormat = 'double'
            if hdr['data_rep']=='EEEI':
                self._byteswap = False
            else:
                self._byteswap = True
            if len(hdr['format']) > 0 and hdr['format'][0]=='C':
                self._complexData = True
            else:
                self._complexData = False
        else:
            if 'c' in dataFormat:
                self._complexData = True
            if 'r' in dataFormat:
                self._byteswap = True
            if '8' in dataFormat:
                if 'u' in dataFormat:
                    dataFormat = 'octet'
                elif 't' in dataFormat:
                    dataFormat = 'char'
            if '16' in dataFormat:
                if 'u' in dataFormat:
                    dataFormat = 'ushort'
                elif 't' in dataFormat:
                    dataFormat = 'short'
            if '32' in dataFormat:
                if 'u' in dataFormat:
                    dataFormat = 'ulong'
                elif 't' in dataFormat:
                    dataFormat = 'long'
                elif 'f' in dataFormat:
                    dataFormat = 'float'
            if '64' in dataFormat:
                if 'u' in dataFormat:
                    dataFormat = 'ulonglong'
                elif 't' in dataFormat:
                    dataFormat = 'longlong'
                elif 'f' in dataFormat:
                    dataFormat = 'double'
 
        _SourceBase.__init__(self, bytesPerPush = bytesPerPush, dataFormat = dataFormat) 
        if self.supportedPorts.has_key(dataFormat):
            self._srcPortType = self.supportedPorts[dataFormat]["portType"]
        else:
            raise Exception, "ERROR: FileSource does not supporte data type " + dataFormat

        self._srcPortObject = None
        self.setupFileReader()

    def _createArraySrcInst(self, srcPortType):
        return self._src

    def setupFileReader(self):
        portTypes = {}
        for usesPort in self._usesPortDict:
            portTypes[usesPort] = {}
            portTypes[usesPort]['Port Interface']=self._usesPortDict[usesPort]['Port Interface']
            portTypes[usesPort]['Port Name']=self._usesPortDict[usesPort]['Port Name']
            foundType = False
            for support in self.supportedPorts:
                if self.supportedPorts[support]['portDict']['Port Interface'] == portTypes[usesPort]['Port Interface']:
                    portTypes[usesPort]['Port Type']=eval(self.supportedPorts[support]['portType'])
                    foundType = True
        try:
            portType = self._srcPortType
            # If input file is a Midas Blue file
            if self._midasFile == True:
                # define source helper component
                self._src = _bluefile_helpers.BlueFileReader(eval(portType))
            # else, input file is binary file
            else:
                self._src = _bulkio_data_helpers.FileSource(eval(portType),self._byteswap, portTypes)
                keywords = []
                for key in self._SRIKeywords:
                    keywords.append(_CF.DataType(key._name, _properties.to_tc_value(key._value,str(key._format))))

                if self._streamID == None:
                    self._streamID = self._filename.split('/')[-1]

                self._sri = _BULKIO.StreamSRI(1, 0.0, 1, 0, 0, 0.0, 0, 0, 0,
                                            self._streamID, True, keywords)

                if self._sampleRate > 0.0:
                    self._sri.xdelta = 1.0/float(self._sampleRate)

                if self._complexData == True:
                    self._sri.mode = 1
                else:
                    self._sri.mode = 0

                if self._streamID != None:
                    self._sri.streamID = self._streamID

                if self._startTime >= 0.0:
                    self._sri.xstart = self._startTime

                self._sri.blocking = self._blocking

        except Exception, e:
            log.error(self.className + ":setupFileReader(): failed " + str(e))

    def getUsesPort(self):
        try:
            if self._src != None:
                self._srcPortObject = self._src.getPort()
                return self._srcPortObject
        except Exception, e:
            log.error(self.className + ":getUsesPort(): failed " + str(e))
        return None

    def start(self):
        if self._src != None:
            # read file
            if self._midasFile == True:
                self._runThread = _threading.Thread(target=self._src.run,args=(self._filename,self._pktSize,self._streamID))
            else:
                self._runThread = _threading.Thread(target=self._src.run,args=(self._filename,self._sri,self._pktSize))
            self._runThread.setDaemon(True)
            self._runThread.start()

    def stop(self):
        # Notify the run method to stop looping and pushing out data
        if self._midasFile == True:
            self._src.done = True
        else:
            self._src.EOS = True

class FileSink(_SinkBase):
    def __init__(self,filename=None, midasFile=False):
        _SinkBase.__init__(self)

        if _domainless._DEBUG == True:
            print className + ":__init__() filename " + str(filename)
            print className + ":__init__() midasFile " + str(midasFile)
        self._filename = filename
        self._midasFile = midasFile

    def getPort(self, portName):
        try:
            self._sinkPortType = self.getPortType(portName)

            # Set up file writer
            # If output file is a Midas Blue file
            if self._midasFile == True:
                # define source helper component
                self._sink = _bluefile_helpers.BlueFileWriter(self._filename,eval(self._sinkPortType))
            # else, output file is binary file
            else:
                self._sink = _bulkio_data_helpers.FileSink(self._filename, eval(self._sinkPortType))

            if self._sink != None:
                self._sinkPortObject = self._sink.getPort()
                return self._sinkPortObject
            else:
                return None
        except Exception, e:
            log.error(self.className + ":getPort(): failed " + str(e))
        return None

    def waitForEOS(self):
        """
        Blocks until an end of stream (EOS) is received by the file writer.

        This can be used to allow use of this module in scripts.
        Without a wait statement, a script could end before the file writer
        is done writing its data.

        """

        if self._sink != None:
            self._sink.port_lock.acquire()
            try:
                try:
                    while not self._sink.gotEOS and (not self.breakBlock):
                        self._sink.port_lock.release()
                        # give the file writer an opportunity to write to
                        # the gotEOS flag.
                        _time.sleep(self._sleepTime)
                        self._sink.port_lock.acquire()

                except Exception, e:
                    log.error(self.className + ": " + str(e))

            finally:
                # If this thread dies for some reason, need to release
                # the lock to keep from taking other threads down with it.
                self._sink.port_lock.release()
        else:
            # If the user has yet to call getPort(), the the file sink may
            # not yet exist.
            log.warn("No file writer present, therefore not waiting for EOS.  Is the " + self.className + " module connected?")

class DataSource(_SourceBase):
    def __init__(self,
                 data         = None,
                 dataFormat   = None,
                 loop         = False,
                 bytesPerPush = 512000,
                 startTime    = 0.0,
                 blocking     = True):

        self.threadExited = None

        _SourceBase.__init__(self,
                             bytesPerPush = bytesPerPush,
                             dataFormat   = dataFormat,
                             data         = data)

        self._sampleRate  = None
        self._complexData = None
        self._SRIKeywords = []
        self._sri         = None
        self._startTime   = startTime
        self._blocking    = blocking
        self._loop        = loop
        self._runThread   = None
        self._dataQueue   = _Queue.Queue()

        # Track unsent packets so that callers can monitor for when all packets
        # have really been sent; checking for an empty queue only tells whether
        # the packet has been picked up by the work thread.
        self._packetsPending = 0
        self._packetsSentCond = _threading.Condition()

        self.ref = self

    def _createArraySrcInst(self, srcPortType):

        if srcPortType != "_BULKIO__POA.dataXML":
            return _bulkio_data_helpers.ArraySource(eval(srcPortType))
        else:
            return _bulkio_data_helpers.XmlArraySource(eval(srcPortType))


    def start(self):
        self._exitThread = False
        if self._runThread == None:
            self._runThread = _threading.Thread(target=self.pushThread)
            self._runThread.setDaemon(True)
            self._runThread.start()
        elif not self._runThread.isAlive():
            self._runThread = _threading.Thread(target=self.pushThread)
            self._runThread.setDaemon(True)
            self._runThread.start()

    def push(self,
             data,
             EOS         = False,
             streamID    = "defaultStreamID",
             sampleRate  = 1.0,
             complexData = False,
             SRIKeywords = [],
             loop        = None):

        # Detect whether or not any of the data is of type complex
        _complexData = _checkComplex(data)

        # If complex values are present, interleave the data as scalar values
        # and set the complex flag
        if _complexData:
            if self._dataFormat in ('octet', 'short', 'ushort', 'long', 'ulong', 'longlong', 'ulonglong'):
                itemType = int
            else:
                itemType = float
            data = _bulkio_helpers.pythonComplexListToBulkioComplex(data, itemType)
            complexData = True

        self._dataQueue.put((data,
                             EOS,
                             streamID,
                             sampleRate,
                             complexData,
                             SRIKeywords,
                             loop))
        self._packetQueued()

    def _packetQueued(self):
        self._packetsSentCond.acquire()
        self._packetsPending += 1
        self._packetsSentCond.release()

    def _packetSent(self):
        self._packetsSentCond.acquire()
        try:
            if self._packetsPending == 0:
                raise AssertionError, 'Packet sent but no packets pending'
            self._packetsPending -= 1
            if self._packetsPending == 0:
                self._packetsSentCond.notifyAll()
        finally:
            self._packetsSentCond.release()

    def pushThread(self):
        self.settingsAcquired = False
        self.threadExited = False
        # Make sure data passed in is within min/max bounds on port type
        # and is a valid type
        currentSampleTime = self._startTime
        while not self._exitThread:
            exitInputLoop = False
            while not exitInputLoop:
                try:
                    dataset = self._dataQueue.get(timeout=0.1)
                    exitInputLoop = True
                    settingsAcquired = True
                except:
                    if self._exitThread:
                        exitInputLoop = True
            if self._exitThread:
                if self.settingsAcquired:
                    self._pushPacketAllConnectedPorts([],
                                                      currentSampleTime,
                                                      EOS,
                                                      streamID)
                    self._packetSent()
                self.threadExited = True
                return

            data        = dataset[0]
            EOS         = dataset[1]
            streamID    = dataset[2]
            sampleRate  = dataset[3]
            complexData = dataset[4]
            SRIKeywords = dataset[5]
            loop        = dataset[6]

            # If loop is set in method call, override class attribute
            if loop != None:
                self._loop = loop
            try:
                self._sampleRate  = sampleRate
                self._complexData = complexData
                self._SRIKeywords = SRIKeywords
                self._streamID    = streamID
                candidateSri      = None
                # If any SRI info is set, call pushSRI
                if streamID != None or \
                  sampleRate != None or \
                  complexData != None or \
                  len(SRIKeywords) > 0:
                    keywords = []
                    for key in self._SRIKeywords:
                        keywords.append(_CF.DataType(key._name, _properties.to_tc_value(key._value,str(key._format))))
                    candidateSri = _BULKIO.StreamSRI(1, 0.0, 1, 0, 0, 0.0, 0, 0, 0,
                                                     streamID, self._blocking, keywords)

                    if sampleRate > 0.0:
                        candidateSri.xdelta = 1.0/float(sampleRate)

                    if complexData == True:
                        candidateSri.mode = 1
                    else:
                        candidateSri.mode = 0

                    if self._startTime >= 0.0:
                        candidateSri.xstart = self._startTime
                else:
                    candidateSri = _BULKIO.StreamSRI(1, 0.0, 1, 0, 0, 0.0, 0, 0, 0,
                                                     "defaultStreamID", self._blocking, [])

                if self._sri==None or not compareSRI(candidateSri, self._sri):
                    self._sri = candidateSri
                    self._pushSRIAllConnectedPorts(sri = self._sri)

                # Call pushPacket
                # If necessary, break data into chunks of pktSize for each
                # pushPacket
                if len(data) > 0:
                    self._pushPacketsAllConnectedPorts(data,
                                                       currentSampleTime,
                                                       EOS,
                                                       streamID)
                    # If loop is set to True, continue pushing data until loop
                    # is set to False or stop() is called
                    while self._loop:
                        self._pushPacketsAllConnectedPorts(data,
                                                           currentSampleTime,
                                                           EOS,
                                                           streamID)
                else:
                    self._pushPacketAllConnectedPorts(data,
                                                      currentSampleTime,
                                                      EOS,
                                                      streamID)
                self._packetSent()
            except Exception, e:
                log.warn(self.className + ":pushData() failed " + str(e))
        self.threadExited = True

    def _pushPacketsAllConnectedPorts(self,
                                      data,
                                      currentSampleTime,
                                      EOS,
                                      streamID):

        for connection in self._connections.values():
            self._pushPackets(arraySrcInst      = connection["arraySrcInst"],
                              data              = data,
                              currentSampleTime = currentSampleTime,
                              EOS               = EOS,
                              streamID          = streamID,
                              srcPortType       = connection["srcPortType"],
                              pktSize           = connection["pktSize"])

    def _pushPacketAllConnectedPorts(self,
                                     data,
                                     currentSampleTime,
                                     EOS,
                                     streamID):

        for connection in self._connections.values():
            self._pushPacket(arraySrcInst      = connection["arraySrcInst"],
                             data              = data,
                             currentSampleTime = currentSampleTime,
                             EOS               = EOS,
                             streamID          = streamID,
                             srcPortType       = connection["srcPortType"])

    def _pushPackets(self,
                     arraySrcInst,
                     data,
                     currentSampleTime,
                     EOS,
                     streamID,
                     srcPortType,
                     pktSize):

        # If necessary, break data into chunks of pktSize for each pushPacket
        if str(type(data)) == "<type 'list'>":
            while len(data) > 0:
                _EOS = EOS
                if len(data) >= pktSize and EOS == True:
                    _EOS = False
                self._pushPacket(arraySrcInst,
                                 data[:pktSize],
                                 currentSampleTime,
                                 _EOS,
                                 streamID,
                                 srcPortType)
                dataSize = len(data[:pktSize])
                if self._sri != None:
                    if self._sri.mode == 1:
                        dataSize = dataSize / 2
                currentSampleTime = currentSampleTime + dataSize/self._sampleRate
                data = data[pktSize:]
        else:
            self._pushPacket(arraySrcInst,
                             data,
                             currentSampleTime,
                             EOS,
                             streamID,
                             srcPortType)

    def _pushPacket(self,
                    arraySrcInst,
                    data,
                    currentSampleTime,
                    EOS,
                    streamID,
                    srcPortType):

        if srcPortType == "_BULKIO__POA.dataXML" or srcPortType == "_BULKIO__POA.dataFile":
            if type(data) != str:
                log.error("data must be a string for the specified data type")
                return
        else:
            if type(data) != list:
                log.error("data must be a list of values for the specified data type")
                return
        if len(data)>0:
                data = _bulkio_helpers.formatData(data,
                                                  BULKIOtype=eval(srcPortType))

        T = _BULKIO.PrecisionUTCTime(_BULKIO.TCM_CPU,
                                     _BULKIO.TCS_VALID,
                                     0.0,
                                     int(currentSampleTime),
                                     currentSampleTime - int(currentSampleTime))
        if srcPortType != "_BULKIO__POA.dataXML":
            _bulkio_data_helpers.ArraySource.pushPacket(arraySrcInst,
                                                        data     = data,
                                                        T        = T,
                                                        EOS      = EOS,
                                                        streamID = streamID)
        else:
            _bulkio_data_helpers.XmlArraySource.pushPacket(arraySrcInst,
                                                           data     = data,
                                                           EOS      = EOS,
                                                           streamID = streamID)

    def _pushSRIAllConnectedPorts(self, sri):
        for connection in self._connections.values():
            self._pushSRI(arraySrcInst = connection["arraySrcInst"],
                          srcPortType = connection["srcPortType"],
                          sri = sri)

    def _pushSRI(self, arraySrcInst, srcPortType, sri):
        if srcPortType != "_BULKIO__POA.dataXML":
            _bulkio_data_helpers.ArraySource.pushSRI(arraySrcInst, sri)
        else:
            _bulkio_data_helpers.XmlArraySource.pushSRI(arraySrcInst, sri)

    def waitAllPacketsSent(self, timeout=None):
        """
        Wait until all of the packets queued on this source have been pushed to
        all connected ports. If timeout is given, it should be the maximum
        number of seconds to wait before giving up.
        """
        self._packetsSentCond.acquire()
        try:
            # Assume no spurious signals will occur, so we can defer to the
            # timeout handling of Python's Condition object.
            if self._packetsPending > 0:
                self._packetsSentCond.wait(timeout)
        finally:
            self._packetsSentCond.release()

    def stop(self):
        self._exitThread = True
        self._loop = False
        if self.threadExited != None:
            timeout_count = 10
            while not self.threadExited:
                _time.sleep(0.1)
                timeout_count -= 1
                if timeout_count < 0:
                    raise AssertionError, self.className + ":stop() failed to exit thread"

class DataSink(_SinkBase):
    def getPort(self, portName):
        if _domainless._DEBUG == True:
            print self.className + ":getPort() portName " + str(portName) + "================================="
        try:
            self._sinkPortType = self.getPortType(portName)

            # Set up output array sink
            if str(portName) == "xmlIn":
                self._sink = _bulkio_data_helpers.XmlArraySink(eval(self._sinkPortType))
            else:
                self._sink = _bulkio_data_helpers.ArraySink(eval(self._sinkPortType))

            if self._sink != None:
                self._sinkPortObject = self._sink.getPort()
                return self._sinkPortObject
            else:
                return None
            pass
        except Exception, e:
            log.error(self.className + ":getPort(): failed " + str(e))
        return None

    def getData(self, length=None, eos_block=False, tstamps=False):
        '''
        Returns either an array of the received data elements or a tuple containing the received list
        and their associated time stamps

        Parameters
        ----------
        length: number of elements that are requested
        eos_block: setting to True creates a blocking call until eos is received
        tstamps: setting to True makes the return value a tuple, where the first
            element is the data set and the second element is a series of tuples
            containing the element index number of and timestamp
        '''
        isChar = self._sink.port_type == _BULKIO__POA.dataChar

        if not self._sink:
            return None
        if eos_block:
            self._sink.waitEOS()
        (retval, timestamps) = self._sink.retrieveData(length=length)
        if isChar:
            newretval = list(_struct.unpack(str(len(retval))+'b',''.join(retval)))
            retval=newretval
        if tstamps:
            return (retval,timestamps)
        else:
            return retval

    def stop(self):
        super(DataSink,self).stop()
        if self._sink:
            self._sink.stop()

class _OutputBase(helperBase):
    def __init__(self):
        helperBase.__init__(self)
        self.usesPortIORString = None
        self._providesPortDict = {}
        self._processes = {}
        self._STOP_SIGNALS = ((_signal.SIGINT, 1),
                              (_signal.SIGTERM, 5),
                              (_signal.SIGKILL, None))

    def __del__(self):
        if _domainless._DEBUG == True:
            print "_OutputBase: __del__() calling cleanUp"
        self.cleanUp()

    def cleanUp(self):
        for pid in self._processes.keys():
            if _domainless._DEBUG == True:
                print "_OutputBase: cleanUp() calling __terminate for pid " + str(pid)
            self.__terminate(pid)

    def start(self):
        pass

    def stop(self):
        pass

    def __terminate(self,pid):
        sp = self._processes[pid]
        for sig, timeout in self._STOP_SIGNALS:
            try:
                # the group id is used to handle child processes (if they
                # exist) of the component being cleaned up
                if _domainless._DEBUG == True:
                    print "_OutputBase: __terminate () making killpg call on pid " + str(pid) + " with signal " + str(sig)
                _os.killpg(pid, sig)
            except OSError:
                log.error("_OutputBase: __terminate() OSERROR ===============")
                pass
            if timeout != None:
                giveup_time = _time.time() + timeout
            while sp.poll() == None:
                if _time.time() > giveup_time: break
                _time.sleep(0.1)
            if sp.poll() != None: break
        sp.wait()

    def setup(self,portIOR, dataType=None, componentName=None, usesPortName=None):
        pass

class probeBULKIO(_SinkBase):
    def __init__(self):
        _SinkBase.__init__(self)

    def getPort(self, portName):
        if _domainless._DEBUG == True:
            print "probeBULKIO:getPort() portName " + str(portName) + "================================="
        try:
            self._sinkPortType = self.getPortType(portName)

            # Set up output array sink
            self._sink = _bulkio_data_helpers.ProbeSink(eval(self._sinkPortType))

            if self._sink != None:
                self._sinkPortObject = self._sink.getPort()
                return self._sinkPortObject
            else:
                return None

        except Exception, e:
            log.error("probeBULKIO:getPort(): failed " + str(e))
        return None

    def receivedStreams(self):
        if self._sink == None:
            print None
        else:
            print "Current streams:"
            for entry in self._sink.valid_streams:
                average_packet = self._sink.received_data[entry][0]/float(self._sink.received_data[entry][1])
                print "Received "+str(self._sink.received_data[entry][0])+" on stream ID "+entry+" with mean packet length of "+str(average_packet)
            print "Terminated streams:"
            for entry in self._sink.invalid_streams:
                average_packet = self._sink.received_data[entry][0]/float(self._sink.received_data[entry][1])
                print "Received "+str(self._sink.received_data[entry])+" on stream ID "+entry+" with mean packet length of "+str(average_packet)

# Plot class requires the following:
# - Eclipse Redhawk IDE must be installed
# - Environment variable RH_IDE must be set defining the path to the main eclipse directory (/data/eclipse for example)
class Plot(OutputBase, _OutputBase):
    def __init__(self,usesPortName=None):
        if _domainless._DEBUG == True:
            print "Plot:__init__()"
        _OutputBase.__init__(self)
        self.usesPortIORString = None
        self._usesPortName = usesPortName
        self._dataType = None
        self._eclipsePath = None
        if _os.environ.has_key("RH_IDE"):
            self._eclipsePath = str(_os.environ["RH_IDE"])
        else:
            raise AssertionError, "Plot():__init__() ERROR - must set environment variable RH_IDE or call IDELocation()"

    def __del__(self):
        if _domainless._DEBUG == True:
            print "Plot: __del__() calling _OutputBase.__del__()"
        _OutputBase.__del__(self)

    def cleanUp(self):
        if _domainless._DEBUG == True:
            print "Plot: cleanUp() calling _OutputBase:cleanUp()"
        _OutputBase.cleanUp(self)

    def __terminate(self,pid):
        if _domainless._DEBUG == True:
            print "Plot: __terminate() calling _OutputBase:__terminate()"
        _OutputBase.__terminate(self,pid)

    def plot(self):
        if _domainless._DEBUG == True:
            print "Plot:plot()"

        # Error checking before launching plot
        if self.usesPortIORString == None:
            raise AssertionError, "Plot:plot() ERROR - usesPortIORString not set ... must call connect() on this object from another component"
        if self._usesPortName == None:
            raise AssertionError, "Plot:plot() ERROR - usesPortName not set ... must call connect() on this object from another component"
        if self._dataType == None:
            raise AssertionError, "Plot:plot() ERROR - dataType not set ... must call connect() on this object from another component"

        plotCommand = str(self._eclipsePath) + "/bin/plotter.sh -portname " + str(self._usesPortName) + " -repid " + str(self._dataType) + " -handler gov.redhawk.ui.port.nxmplot -ior " + str(self.usesPortIORString)
        if _domainless._DEBUG == True:
            print "Plot:plotCommand " + str(plotCommand)
        args = _shlex.split(plotCommand)
        if _domainless._DEBUG == True:
            print "Plot:args " + str(args)
        try:
            dev_null = open('/dev/null','w')
            sub_process = _subprocess.Popen(args,stdout=dev_null,preexec_fn=_os.setpgrp)
            pid = sub_process.pid
            self._processes[pid] = sub_process
        except Exception, e:
            raise AssertionError, "Plot:plot() Failed to launch plotting due to %s" % ( e)


    def setup(self, usesPort, dataType=None, componentName=None, usesPortName=None):
        self.usesPortIORString = domainless.orb.object_to_string(usesPort)
        self._dataType = dataType
        self._usesPortName = usesPortName

        if _domainless._DEBUG == True:
            print "Plot:setup()"
        self.plot()

class SRIKeyword(object):
    '''
    This is used in the Input series as the element in the SRIKeywords list
    name and value correspond to the id/value pair
    format is a string that describes the data type casting that needs to happen
      - short, ushort
      - float, double
      - long, ulong
      - longlong, ulonglong
      - char
      - octet
      - string
      - boolean
    '''
    def __init__(self, name, value, format):
        self._name   = name
        self._value  = value
        self._format = format
