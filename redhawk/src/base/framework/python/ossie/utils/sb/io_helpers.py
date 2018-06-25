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
    SDDS_SB = _BULKIO.SDDS_SB
except:
    # Handle case where bulkioInterface may not be installed
    SDDS_SB = 1

import domainless as _domainless
import threading as _threading
import ossie.utils.bulkio.bulkio_helpers as _bulkio_helpers
from ossie.utils.bluefile import bluefile_helpers
from ossie.utils.bulkio import bulkio_data_helpers
import ossie.utils.bluefile.bluefile as _bluefile
from ossie import properties as _properties
from ossie import events as _events
from ossie.cf import CF as _CF
import shlex as _shlex
import time as _time
import signal as _signal
import warnings
import cStringIO, pydoc
import sys as _sys
import os as _os
import subprocess as _subprocess
import Queue as _Queue
import struct as _struct
import logging as _logging
import socket as _socket
from omniORB import any as _any
from omniORB import CORBA as _CORBA
from omniORB import tcInternal
import copy as _copy
import omniORB as _omniORB
import CosEventComm__POA
import traceback

from ossie.utils.model import PortSupplier, OutputBase
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

# Use orb reference from domainless
import domainless

log = _logging.getLogger(__name__)

__all__ = ('DataSink', 'DataSource', 'FileSink', 'FileSource', 'MessageSink',
           'MessageSource','MsgSupplierHelper', 'Plot', 'SRIKeyword', 'compareSRI', 'helperBase',
           'probeBULKIO','createSDDSStreamDefinition', 'DataSourceSDDS',
           'DataSinkSDDS', 'createTimeStamp', 'createSRI', 'compareKeywordLists')

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
        if keyA.id  != keyB.id:
            return False
        if keyA.value._t != keyB.value._t:
            return False
        if keyA.value._v != keyB.value._v:
            return False
    return True

def compareKeywordLists( a, b ):
    for keyA, keyB in zip(a, b):
        if keyA.id  != keyB.id:
            return False
        if keyA.value._t != keyB.value._t:
            return False
        if keyA.value._v != keyB.value._v:
            return False
    return True

def _getAnyValue(key):
    if key._format[0]=='[' and key._format[-1]==']':
        expectedType = _properties.getTypeCode(key._format[1:-1])
        expectedTypeCode = tcInternal.createTypeCode((tcInternal.tv_sequence, expectedType._d, 0))
        return _CORBA.Any(expectedTypeCode, key._value)
    else:
        return _properties.to_tc_value(key._value,str(key._format))

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
        for _identifier, (identifier, uses, provides) in manager.getConnections().items():
            if uses.hasComponent(self) or provides.hasComponent(self):
                usesRef = uses.getReference()
                usesRef.disconnectPort(identifier)
                manager.unregisterConnection(identifier, uses)
        self._sandbox._unregisterComponent(self)

    def reset(self):
        pass

class MessageSink(helperBase, PortSupplier):
    '''
        Received structured messages
        if storeMessages is True, then messages can be retrieved through the getMessages function
            The internal message queue is emptied when messages are retrieved, so if storeMessages is True,
            make sure to regularly retrieve the available messages to empty out the internal list

    '''
    def __init__(self, messageId = None, messageFormat = None, messageCallback = None, storeMessages = False):
        helperBase.__init__(self)
        PortSupplier.__init__(self)
        self._flowOn = False
        self._messagePort = None
        self._messageId = messageId
        self._messageFormat = messageFormat
        self._messageCallback = messageCallback
        self._storeMessages = storeMessages
        self._providesPortDict = {}
        self._providesPortDict['msgIn'] = {
            'Port Interface': 'IDL:ExtendedEvent/MessageEvent:1.0',
            'Port Name': 'msgIn'
            }

    def __del__(self):
        if self._messagePort:
            self._messagePort.terminate()
            self._messagePort = None

    def messageCallback(self, msgId, msgData):
        print msgId, msgData

    def getMessages(self):
        return self._messagePort.getMessages()

    def getPort(self, portName):
        try:
            if self._messageCallback == None:
                self._messageCallback = self.messageCallback
            if  self._messagePort == None:
                self._messagePort = _events.MessageConsumerPort(thread_sleep=0.1, storeMessages = self._storeMessages)
                self._messagePort.registerMessage(self._messageId,
                                             self._messageFormat, self._messageCallback)
            return self._messagePort._this()
        except Exception, e:
            log.error("MessageSink:getPort(): failed " + str(e))
        return None

    def api(self, destfile=None):
        localdef_dest = False
        if destfile == None:
            localdef_dest = True
            destfile = cStringIO.StringIO()

        print >>destfile, "Component MessageSink :"
        PortSupplier.api(self, destfile=destfile)

        if localdef_dest:
            pydoc.pager(destfile.getvalue())
            destfile.close()

    def start(self):
        if self._messagePort :  self._messagePort.startPort()
        self._flowOn = True

    def stop(self):
        if self._messagePort:  self._messagePort.stopPort()
        self._flowOn = False

    def releaseObject(self):
        self.terminate()
        helperBase.releaseObject(self)

    def terminate(self):
        if self._messagePort:  
             self._messagePort.terminate()
             self._messagePort = None

    def eos(self):
        return False

    def sri(self):
        return None

class MessageSource(helperBase, PortSupplier):
    def __init__(self, messageId = None, messageFormat = None):
        '''
        messageId: id for the message (string)
        messageForms: dictionary for the message member types
                key: element id (string)
                value: element type (string)
                
                valid types:
                    boolean, char, double, float, short, long, longlong,
                    octet, string, ulong, ushort, longlong, ulonglong, 
                    [boolean], [char], [double], [float], [short], [long], [longlong],
                    [octet], [string], [ulong], [ushort], [longlong], [ulonglong]
        '''
        helperBase.__init__(self)
        PortSupplier.__init__(self)
        self._flowOn = False
        self._messagePort = None
        self._messageId = messageId
        self._messageFormat = messageFormat

        self._usesPortDict['msgOut'] = {
            'Port Interface': 'IDL:ExtendedEvent/MessageEvent:1.0',
            'Port Name': 'msgOut'
            }

    def _packMessageData(self, data):
        if isinstance(data, dict):
            props = _properties.props_from_dict(data)
        elif isinstance(data, _CORBA.Any):
            props = [_CF.DataType('sb', data)]
        else:
            # Assume it's something that can be mapped to an any
            props = [_CF.DataType('sb', _any.to_any(data))]
        if self._messageFormat:
            for _key in self._messageFormat:
                found_prop = False
                for _prop in props:
                    if _prop.id == _key:
                        found_prop = True
                        if _prop.value._t != _CORBA.TC_null:
                            if '[' in self._messageFormat[_key]:
                                base_type_txt = self._messageFormat[_key].replace('[','').replace(']','')
                                base_type = _properties.getTypeCode(base_type_txt)
                                if _prop.value._t._k != _CORBA.tk_sequence:
                                    raise Exception('Data for item '+_key+' must be a list')
                                if _prop.value._t.content_type() != base_type:
                                    expectedTypeCode = _omniORB.tcInternal.createTypeCode((_omniORB.tcInternal.tv_sequence, base_type._d, 0))
                                    _prop.value = _CORBA.Any(expectedTypeCode, [_properties.to_pyvalue(item, base_type_txt) for item in _prop.value._v])
                            else:
                                _prop.value._t = _properties.getTypeCode(self._messageFormat[_key])
                if not found_prop:
                    raise Exception('Unable to find item '+_key+' in '+str(data))
        return _properties.props_to_any(props)

    def sendMessage(self, msg):
        if not self._flowOn: return
        if hasattr(msg, 'getId'):
            # This is a struct
            messageId = msg.getId()
            payload = _properties.struct_to_any(msg)
        else:
            if self._messageId:
                messageId = self._messageId
            else:
                messageId = 'sb_struct'
            payload = self._packMessageData(msg)

        outgoing = [_CF.DataType(messageId, payload)]
        outmsg = _properties.props_to_any(outgoing)
        self._messagePort.sendMessage(outmsg)

    def getPort(self, portName):
        try:
            if self._messagePort == None:
                 self._messagePort = _events.MessageSupplierPort()
            return self._messagePort._this()
        except Exception, e:
            log.error("MessageSource:getPort(): failed " + str(e))
        return None

    def connectPort(self, connection, connectionId):
        if self._messagePort:
            self._messagePort.connectPort(connection, connectionId)

    def disconnectPort(self, connectionId):
        if self._messagePort:
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

    def api(self, destfile=None):
        localdef_dest = False
        if destfile == None:
            localdef_dest = True
            destfile = cStringIO.StringIO()

        print >>destfile, "Component MessageSource :"
        PortSupplier.api(self, destfile=destfile)

        if localdef_dest:
            pydoc.pager(destfile.getvalue())
            destfile.close()

    def start(self):
        self._flowOn = True

    def stop(self):
        self._flowOn = False

    def eos(self):
        return False

    def sri(self):
        return None


class MsgSupplierHelper(object):
    class Supplier_i(CosEventComm__POA.PushSupplier):
        def disconnect_push_supplier(self):
            pass

    """
    Helper class to send messages to connections of an ExtendedEvent/MessageEvent output Port
    """
    def __init__(self, component):
        self.comp = component
        # holds supplier ports when messages are connected..
        self._msg_ports=[]

    def _packMessageData(self, data):
        if isinstance(data, dict):
            props = _properties.props_from_dict(data)
        elif isinstance(data, _CORBA.Any):
            props = [_CF.DataType('sb', data)]
        else:
            # Assume it's something that can be mapped to an any
            props = [_CF.DataType('sb', _any.to_any(data))]
        return _properties.props_to_any(props)


    def _connectSupplierToEventChannel(self, channel):
        connection = { 'port': channel }

        supplier_admin = channel.for_suppliers()
        proxy_consumer = supplier_admin.obtain_push_consumer()
        connection['proxy_consumer'] = proxy_consumer
        connection['supplier'] = self.Supplier_i()
        proxy_consumer.connect_push_supplier(connection['supplier']._this())

        return connection

    def sendMessage(self, msg, msg_id=None, msg_port=None, restrict=True ):
        # try to find port first...
        _msg_port=None
        msg_ports=[ x for x in self.comp.ports if getattr(x,'_using') and x._using.name == 'MessageEvent' ]
        if len(msg_ports) > 1 and msg_port is None:
            print "Unable to determine message port, please specify the msg_port parameter, available ports: ", [ x._name for x in msg_ports ]
            return False
        
        if msg_port:
            for x in msg_ports:
                if x._name == msg_port:
                    _msg_port = x
                    break
        else:
            _msg_port = msg_ports[0]

        if _msg_port is None:
            print "Unable to determine message port, please specify the msg_port parameter, available ports: ", [ x._name for x in msg_ports ]


        # get current connection for the component and the msg_port
        _evt_connects=[]
        if _msg_port:
            manager = ConnectionManager.instance()
            for k, (identifier, uses, provides) in manager.getConnectionsFor(self.comp).iteritems():
                if uses.getPortName() == _msg_port._name:
                    _evt_connects.append( provides )

        if len(_evt_connects) == 0:
            print "No available registered connections for sending a message."
            return False

        outmsg=msg
        if not isinstance(msg, _CORBA.Any):
            # now look at message structure
            _msg_struct = None
            _msg_structs = [ x for x in self.comp._properties if x.kinds == ['message']  ]
            if len(_msg_structs) > 1 and msg_id is None:
                print "Unable to determine message structure, please specify the msg_id parameter, available structures: ", [ x.id for x in _msg_structs ]
                return False

            if msg_id:
                messageId = msg_id
                for x in _msg_structs:
                    if x.id == msg_id:
                        _msg_struct = x
                        messageId = _msg_struct.id
                        break
            else:
                _msg_struct = _msg_structs[0]
                messageId = _msg_struct.id

            if _msg_struct is None and restrict :
                print "Unable to determine message structure, please specify the msg_id parameter, available structures: ", [ x.id for x in _msg_structs ]
                return False

            payload = self._packMessageData(msg)
            outgoing = [_CF.DataType(messageId, payload)]
            outmsg = _properties.props_to_any(outgoing)


        for evt_conn in _evt_connects:
            conn=None
            ch=evt_conn.getReference()
            for mp in self._msg_ports:
                if mp[0] == ch :
                    conn=mp[1]
            if conn == None:
                conn= self._connectSupplierToEventChannel(ch)
                self._msg_ports.append( (ch, conn ) )

            try:
                conn['proxy_consumer'].push(outmsg)
            except:
                print "WARNING: Unable to send data to: ", conn
                return False

            return True

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
             "sdds"      : {"bytesPerSample" : 1,
                            "pktSize"        : -1,
                            "format"         : 's',
                            "portType" : "_BULKIO__POA.dataSDDS",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataSDDS:1.0",
                                          "Port Name"      : "sdds"}},
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

    def api(self, destfile=None):
        """
        Prints application programming interface (API) information and returns.

        """
        localdef_dest = False
        if destfile == None:
            localdef_dest = True
            destfile = cStringIO.StringIO()

        print >>destfile, "Component " + self.__class__.__name__ + " :"
        PortSupplier.api(self, destfile=destfile)

        if localdef_dest:
            pydoc.pager(destfile.getvalue())
            destfile.close()


class _SourceBase(_DataPortBase):

    def __init__(self, bytesPerPush, dataFormat, data = None, formats=None, subsize=0):
        """
        Forward parameters to parent constructor.

        Set self._dataFormat to dataFormat it it is in the list
        of supported ports, otherwise attempt to guess the format
        based on the data or set the format to self._defaultDataFormat.
        
        Note: the 'data' argument is not used

        Calls _buildAPI()

        """
        _DataPortBase.__init__(self, portNameAppendix = "Out", formats=formats)

        self.bytesPerPush = int(bytesPerPush)

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
            
        self._subsize     = subsize

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
            self.api(destfile=_sys.stdout)

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
            self.api(destfile=_sys.stdout)

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

    def reset(self):
        if self._sink:
            if callable(self._sink.reset):
                self._sink.reset()

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
                 blocking     = True,
                 subsize      = 0,
                 throttle     = False):

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
        self._throttle    = throttle
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
 
        _SourceBase.__init__(self, bytesPerPush = bytesPerPush, dataFormat = dataFormat, subsize=subsize) 
        if self.supportedPorts.has_key(dataFormat):
            self._srcPortType = self.supportedPorts[dataFormat]["portType"]
        else:
            raise Exception, "ERROR: FileSource does not support data type " + dataFormat

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
                self._src = bluefile_helpers.BlueFileReader(eval(portType), throttle=self._throttle)
            # else, input file is binary file
            else:
                self._src = bulkio_data_helpers.FileSource(eval(portType),self._byteswap, portTypes, throttle=self._throttle)
                keywords = []
                for key in self._SRIKeywords:
                    keywords.append(_CF.DataType(key._name, _getAnyValue(key)))

                if self._streamID == None:
                    self._streamID = self._filename.split('/')[-1]

                self._sri = _BULKIO.StreamSRI(1, 0.0, 1, 0, self._subsize, 0.0, 0, 0, 0,
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
    """
      To use a different sink (for custom data processing) for regular files, assign the new class to sinkClass
      To use a different sink for blue files, assign the new class to sinkBlueClass
    """
    def __init__(self,filename=None, midasFile=False, sinkClass=bulkio_data_helpers.FileSink, sinkBlueClass=bluefile_helpers.BlueFileWriter):
        _SinkBase.__init__(self)

        self.sinkClass = sinkClass
        self.sinkBlueClass = sinkBlueClass
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
                self._sink = self.sinkBlueClass(self._filename,eval(self._sinkPortType))
            # else, output file is binary file
            else:
                self._sink = self.sinkClass(self._filename, eval(self._sinkPortType))

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

class DataSinkSDDS(_SinkBase):
    """
    DataSinkSDDS accepts SDDS metadata

    It is the responsibility of the user to consume the SDDS data

    DataSinkSDDS manages attachment Ids under the port (sinkClass) dictionary attachments

    register an attach callback by passing a function to registerAttachCallback
    register an detach callback by passing a function to registerDetachCallback
    """
    def __init__(self, sinkClass=bulkio_data_helpers.SDDSSink):
        _SinkBase.__init__(self, formats=['sdds'])
        self._sink = sinkClass(self)
        self.attach_cb = self.__attach_cb
        self.detach_cb = self.__detach_cb

    def getPort(self, portName):
        return self._sink.getPort()

    def __attach_cb(self, streamDef, user_id):
        print 'attach received: ',streamDef, user_id

    def __detach_cb(self, attachId):
        print 'detach received: ',attachId

    def registerAttachCallback(self, attach_cb_fn):
        """
        The attach function takes two arguments: stream definition, user id
        """
        self.attach_cb = attach_cb_fn

    def registerDetachCallback(self, detach_cb_fn):
        """
        The detach function takes one arguments: attachment id
        """
        self.detach_cb = detach_cb_fn

def createSDDSStreamDefinition(id=None, dataFormat=SDDS_SB, multicastAddress='0.0.0.0',vlan=0,port=1,sampleRate=2,timeTagValid=False,privateInfo=''):
    if id == None:
        id = _socket.gethostname()
    return _BULKIO.SDDSStreamDefinition(id=id, dataFormat=dataFormat, multicastAddress=multicastAddress,vlan=vlan,port=port,sampleRate=sampleRate,timeTagValid=timeTagValid,privateInfo=privateInfo)

class DataSourceSDDS(_SourceBase):
    """
    DataSourceSDDS generates the SDDS metadata and sends it to whichever destination it is connected to.

    It is the responsibility of the user to generate the SDDS data
    """
    def __init__(self):
        """
        Helper to handle the generation of SDDS metadata forwarding
        """
        _SourceBase.__init__(self, bytesPerPush = 0, dataFormat='sdds', formats=['sdds'])
        self._src = bulkio_data_helpers.SDDSSource()
        self._blocking    = True
        self._streamdefs = {}

    def start(self):
        pass

    def stop(self):
        pass

    def attach(self, streamData=None, name=None):
        """
        streamData: type BULKIO.SDDSStreamDefinition
        name: user id (string)

        The return value is the attachment id (use this to detach)

        If there exists more than one connection, then the return value is a list
        of all attachment id's generated

        """
        if streamData == None:
            streamData = createSDDSStreamDefinition()
        if name == None:
            name = _socket.gethostname()+'_user'
        if not isinstance(streamData, _BULKIO.SDDSStreamDefinition):
            raise Exception("streamData must be of type BULKIO.SDDSStreamDefinition")
        if not isinstance(name, str):
            raise Exception("name must be of <type 'str'>")
        retval = self._src.attach(streamData, name)
        if retval:
            self._streamdefs[name] = streamData
        return retval

    def detach(self, attachId=''):
        """
        streamData: type BULKIO.SDDSStreamDefinition
        name: user id (string)
        """
        if not isinstance(attachId, str):
            raise Exception("attachId must be of <type 'str'>")
        self._src.detach(attachId)
        try:
           self._streamdefs.pop(attachid,None)
        except:
            pass

    def _createArraySrcInst(self, srcPortType):
        return self._src

    def getStreamDef( self, name=None, hostip=None, pkts=1000, block=True, returnSddsAnalyzer=True):
        # grab data if stream definition is available 
        sdef =None
        aid=name
        if not aid:
            if len(self._streamdefs) ==  0:
                raise Exception("No attachment have been made, use grabData or call attach")
            
            aid = self._streamdefs.keys()[0]
            print "Defaults to first entry, attach id = ", aid
            sdef = self._streamdefs[aid]
        else:
            sdef = sefl._streamdefs[aid]
            
        if not sdef:
            raise Exception("No SDDS stream definition for attach id:" + aid )
        
        if not hostip:
            hostip = _socket.gethostbyname(_socket.gethostname())

        return self.getData( sdef.multicastAddress, hostip, sdef.port, packets, block=block, returnSDDSAnalyzer=returnSDDSAnalyzer)
        

    def getData( self, mgroup, hostip, port=29495, pkts=1000, pktlen=1080, block=True, returnSddsAnalyzer=True):
        totalRead=0.0
        startTime = _time.time()        
        sock = None
        ismulticast=False
        blen=10240
        bytesRead=0
        requestedBytes=pkts*pktlen
        data=[]
        rawdata=''
        try:
            try:
                ip_class=int(mgroup.split('.')[0])
                if ip_class == '224' or ip_class == '239':
                    ismulticast=True
            except:
                pass

            #print " Capturing ", mgroup, " host ", hostip, " port ", port
            sock = _socket.socket(_socket.AF_INET, _socket.SOCK_DGRAM, _socket.IPPROTO_UDP)
            sock.setsockopt(_socket.SOL_SOCKET, _socket.SO_REUSEADDR, 1)
            sock.bind(("",port))
            if ismulticast:
                mreq=struct.pack('4s4s',_socket.inet_aton(mgroup),_socket.inet_aton(hostip))
                sock.setsockopt(_socket.IPPROTO_IP, _socket.IP_ADD_MEMBERSHIP, mreq)
                print "Capturing Socket Interface: (MULTICAST) Host Interface: " + hostip + " Multicast: " + mgroup + " Port: "+ str(port)
            else:
                print "Capturing Socket Interface: (UDP) Host Interface: " + hostip + " Source Address: " + mgroup + " Port: "+ str(port)
            ncnt=0
            while totalRead < requestedBytes:
                rcvddata = sock.recv(blen,_socket.MSG_WAITALL)
                rawdata=rawdata+rcvddata
                data=data+list(rcvddata)
                totalRead = totalRead + len(rcvddata)
                ncnt += 1
                print " read ", ncnt, " pkt ", len(rcvddata)
        except KeyboardInterrupt,e :
            traceback.print_exc()
            print "Exception during packet capture: " + str(e)
        except Exception, e :
            traceback.print_exc()
            print "Exception during packet capture: " + str(e)
        finally:
            endTime=_time.time()
            deltaTime=endTime -startTime            
        if sock: sock.close()
        print "Elapsed Time: ", deltaTime, "  Total Data (kB): ", totalRead/1000.0, " Rate (kBps): ", (totalRead/1000.0)/deltaTime
        if returnSddsAnalyzer:
            from ossie.utils.sdds import SDDSAnalyzer
            return SDDSAnalyzer( rawdata, pkts, pktlen, totalRead )
        else:
            return data, rawdata, (pktlen,pkts,totalRead)
            

class DataSource(_SourceBase):
    '''
      Soure of Bulk IO data. Supported data format strings:
        char, short, long, float, double, longlong, octet, ushort, ulong, ulonglong
      throttle: when True, data will match sampleRate (provided in the push function)
    '''
    def __init__(self,
                 data         = None,
                 dataFormat   = None,
                 loop         = False,
                 bytesPerPush = 512000,
                 startTime    = 0.0,
                 blocking     = True,
                 subsize      = 0,
                 sri          = None,
                 throttle     = False):
        warnings.warn("DataSource is deprecated, use StreamSource", DeprecationWarning)
        fmts=['char','short','long','float','double','longlong','octet','ushort', 'ulong', 'ulonglong', 'file','xml' ]
        self.threadExited = None

        _SourceBase.__init__(self,
                             bytesPerPush = bytesPerPush,
                             dataFormat   = dataFormat,
                             data         = data,
                             subsize      = subsize,
                             formats=fmts )

        self._sampleRate  = None
        self._onPushSampleRate  = None
        self._complexData = None
        self._streamID = None
        self._SRIKeywords = []
        self._sri         = sri
        self._startTime   = startTime
        self._timePush    = None
        self._blocking    = blocking
        self._loop        = loop
        self._runThread   = None
        self._dataQueue   = _Queue.Queue()
        self._currentSampleTime = self._startTime
        self._throttle = throttle

        # Track unsent packets so that callers can monitor for when all packets
        # have really been sent; checking for an empty queue only tells whether
        # the packet has been picked up by the work thread.
        self._packetsPending = 0
        self._packetsSentCond = _threading.Condition()

        self.ref = self

    def _createArraySrcInst(self, srcPortType):

        if srcPortType != "_BULKIO__POA.dataXML":
            return bulkio_data_helpers.ArraySource(eval(srcPortType))
        else:
            return bulkio_data_helpers.XmlArraySource(eval(srcPortType))


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


    def sri(self):
        if self._sri != None:
            return _copy.copy(self._sri)
        else:
            # make a default sri from the current state
            keywords = []
            try:
                for key in self._SRIKeywords:
                    keywords.append(_CF.DataType(key._name, _getAnyValue(key)))
            except:
                pass
            candidateSri = _BULKIO.StreamSRI(1, 0.0, 1, 0, self._subsize, 0.0, 0, 0, 0,
                                             "defaultStreamID", self._blocking, keywords)
            if self._sampleRate > 0.0:
                candidateSri.xdelta = 1.0/float(self._sampleRate)

            if self._complexData and self._complexData == True:
                candidateSri.mode = 1

            if self._startTime >= 0.0:
                candidateSri.xstart = self._startTime
            return candidateSri

    def sendEOS(self, streamID=None):
        if streamID:
            self.push([],EOS=True, streamID=streamID )
        else:
            if self._sri:
                self.push([],EOS=True, streamID=self._sri.streamID )
            else:
                self.push([],EOS=True )


    def push(self,
             data,
             EOS         = False,
             streamID    = None,
             sampleRate  = None,
             complexData = False,
             SRIKeywords = [],
             loop        = None,
             sri         = None,
             ts          = None ):
        """
        Push an arbitrary data vector
      
        note: sampleRate = 1/xdelta on the SRI
        """

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

        # if no stream id is provided then try and use prior stream id
        if streamID == None:
            if self._sri == None and sri == None:
                if self._streamID != None:
                   streamID = self._streamID
            elif sri != None:
                streamID = sri.streamID
            elif self._sri != None:
                streamID = self._sri.streamID

        # if no stream id is provided then use default
        if streamID == None:
            streamID = "defaultStreamID"

        if sampleRate == None:
            # if no sample rate provide and no sri provide then use prior sample rate if available
            if sri != None:
                if sri.xdelta > 0.0:
                    self._onPushSampleRate = 1.0/sri.xdelta
            if self._onPushSampleRate != None:
                sampleRate = self._onPushSampleRate
        else:
            # if we are given a new sample rate then save this off
            if self._onPushSampleRate == None or ( self._onPushSampleRate != sampleRate ):
                # save off, data consumer thread can be slower so we can use sri
                self._onPushSampleRate = sampleRate

        self._dataQueue.put((data,
                             EOS,
                             streamID,
                             sampleRate,
                             complexData,
                             SRIKeywords,
                             loop,
                             sri,
                             ts))
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
                                                      self._currentSampleTime,
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
            sri         = dataset[7]
            ts          = dataset[8]

            # If loop is set in method call, override class attribute
            if loop != None:
                self._loop = loop
            try:
                if sri == None and self._sri == None :
                    if sampleRate == None : sampleRate=1.0
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
                            keywords.append(_CF.DataType(key._name, _getAnyValue(key)))
                        candidateSri = _BULKIO.StreamSRI(1, 0.0, 1, 0, self._subsize, 0.0, 0, 0, 0,
                                                         streamID, self._blocking, keywords)

                        if sampleRate > 0.0:
                            candidateSri.xdelta = 1.0/float(sampleRate)
                        self._onPushSampleRate = sampleRate

                        if complexData == True:
                            candidateSri.mode = 1
                        else:
                            candidateSri.mode = 0

                        if self._startTime >= 0.0:
                            candidateSri.xstart = self._startTime

                    else:
                        candidateSri = _BULKIO.StreamSRI(1, 0.0, 1, 0, self._subsize, 0.0, 0, 0, 0,
                                                         "defaultStreamID", self._blocking, [])
                else:
                    candidateSri = _copy.copy(self._sri)
                    if sri != None:
                        # user supplied sri
                        candidateSri = sri

                    if streamID and streamID != candidateSri.streamID:
                        candidateSri.streamID = streamID
                        self._streamID = streamID

                    if sampleRate == None:
                        sampleRate = 1.0/candidateSri.xdelta
                        self._sampleRate = sampleRate
                        self._onPushSampleRate = sampleRate
                    else:
                        if sampleRate > 0.0:
                            candidateSri.xdelta = 1.0/float(sampleRate)
                        self._sampleRate = sampleRate
                        self._onPushSampleRate = sampleRate

                    if complexData == True:
                        candidateSri.mode = 1
                        self._complexData = 1

                    self._complexData = candidateSri.mode

                    if self._startTime >= 0.0:
                        candidateSri.xstart = self._startTime

                    # handle keywords
                    if len(SRIKeywords) > 0 :
                        self._SRIKeywords = SRIKeywords
                        # need to keep order for compareSRI
                        ckeys = [ x.id for x in candidateSri.keywords ]
                        keywords = candidateSri.keywords[:]
                        for key in self._SRIKeywords:
                            # if current sri contains they keyword then overwrite else append
                            kw = _CF.DataType(key._name, _getAnyValue(key))
                            if key._name in ckeys:
                                # replace that keyword
                                for x in range(len(keywords)):
                                    if keywords[x].id == kw.id :
                                        keywords[x] = kw
                            else:
                                keywords.append(kw)
                        candidateSri.keywords = keywords

                if self._sri==None or not compareSRI(candidateSri, self._sri):
                    self._sri = _copy.copy(candidateSri)
                    self._pushSRIAllConnectedPorts(sri = self._sri)

                # if we are give a timestamp then use it as is
                if ts != None:
                    self._currentSampleTime = ts

                # Call pushPacket
                # If necessary, break data into chunks of pktSize for each
                # pushPacket
                if len(data) > 0:
                    self._pushPacketsAllConnectedPorts(data,
                                                       self._currentSampleTime,
                                                       EOS,
                                                       streamID)
                    # If loop is set to True, continue pushing data until loop
                    # is set to False or stop() is called
                    while self._loop:
                        self._pushPacketsAllConnectedPorts(data,
                                                           self._currentSampleTime,
                                                           EOS,
                                                           streamID)
                else:
                    self._pushPacketAllConnectedPorts(data,
                                                      self._currentSampleTime,
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
        if isinstance(data, list):
            # Stride through the data by packet size
            for startIdx in xrange(0, len(data), pktSize):
                # Calculate the time offset per packet
                _data = data[startIdx:startIdx+pktSize]
                sampleTimeForPush = len(_data) / float(self._sampleRate)
                if self._sri and self._sri.mode == 1:
                    sampleTimeForPush /= 2.0
                    
                # Only send an EOS with the packet if EOS was given and this is
                # the last packet
                packetEOS = EOS and (startIdx + pktSize) >= len(data)
                self._pushPacket(arraySrcInst,
                                 _data,
                                 currentSampleTime,
                                 packetEOS,
                                 streamID,
                                 srcPortType)
                # covert time stamp if necessary
                if isinstance(currentSampleTime,_BULKIO.PrecisionUTCTime):
                    self._currentSampleTime = currentSampleTime.twsec +  currentSampleTime.tfsec
                    currentSampleTime = self._currentSampleTime

                self._currentSampleTime += sampleTimeForPush
                currentSampleTime += sampleTimeForPush
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

        if isinstance(currentSampleTime,_BULKIO.PrecisionUTCTime):
            T= currentSampleTime
        else:
            T = _BULKIO.PrecisionUTCTime(_BULKIO.TCM_CPU,
                                         _BULKIO.TCS_VALID,
                                         0.0,
                                         int(currentSampleTime),
                                         currentSampleTime - int(currentSampleTime))
        if self._throttle:
            if self._sampleRate != None:
                _time.sleep(len(data)/(self._sampleRate*2.0))

        if srcPortType != "_BULKIO__POA.dataXML":
            bulkio_data_helpers.ArraySource.pushPacket(arraySrcInst,
                                                        data     = data,
                                                        T        = T,
                                                        EOS      = EOS,
                                                        streamID = streamID)
        else:
            bulkio_data_helpers.XmlArraySource.pushPacket(arraySrcInst,
                                                           data     = data,
                                                           EOS      = EOS,
                                                           streamID = streamID)
        if self._throttle:
            if self._sampleRate != None:
                _time.sleep(len(data)/(self._sampleRate*2.0))

    def _pushSRIAllConnectedPorts(self, sri):
        for connection in self._connections.values():
            self._pushSRI(arraySrcInst = connection["arraySrcInst"],
                          srcPortType = connection["srcPortType"],
                          sri = sri)

    def _pushSRI(self, arraySrcInst, srcPortType, sri):
        if srcPortType != "_BULKIO__POA.dataXML":
            #print "_pushSRI ", sri
            bulkio_data_helpers.ArraySource.pushSRI(arraySrcInst, sri)
        else:
            bulkio_data_helpers.XmlArraySource.pushSRI(arraySrcInst, sri)

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
    """
      To use a different sink (for custom data processing), assign the new class to sinkClass
      To use a different sink for XML data, assign the new class to sinkXmlClass
    """
    def __init__(self, sinkClass=bulkio_data_helpers.ArraySink, sinkXmlClass=bulkio_data_helpers.XmlArraySink):
        warnings.warn("DataSink is deprecated, use StreamSink instead", DeprecationWarning)
        fmts=['char','short','long','float','double','longlong','octet','ushort', 'ulong', 'ulonglong', 'file','xml' ]
        _SinkBase.__init__(self, formats=fmts)
        self.sinkClass = sinkClass
        self.sinkXmlClass = sinkXmlClass

    def getPort(self, portName):
        if _domainless._DEBUG == True:
            print self.className + ":getPort() portName " + str(portName) + "================================="
        try:
            self._sinkPortType = self.getPortType(portName)

            # Set up output array sink
            if str(portName) == "xmlIn":
                self._sink = self.sinkXmlClass(eval(self._sinkPortType))
            else:
                self._sink = self.sinkClass(eval(self._sinkPortType))

            if self._sink != None:
                self._sinkPortObject = self._sink.getPort()
                return self._sinkPortObject
            else:
                return None
            pass
        except Exception, e:
            log.error(self.className + ":getPort(): failed " + str(e))
        return None

    def getDataEstimate(self):
        '''
        Returns a structure with the total amount of data available and the 
        number of corresponding timestamps
        '''
        if not self._sink:
            return None
        return self._sink.estimateData()

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
            containing the element index number and the timestamp for that index
        '''
        isChar = self._sink.port_type == _BULKIO__POA.dataChar

        if not self._sink:
            return None
        if eos_block:
            self._sink.waitEOS()
        (retval, timestamps) = self._sink.retrieveData(length=length)
        if not retval:
            if tstamps:
                return ([],[])
            return []
        if isChar:
            # Converts char values into their numeric equivalents
            def from_char(data):
                return [_struct.unpack('b',ch)[0] for ch in data]

            # If SRI is known, and the data is framed, apply conversion on a
            # per-frame basis
            sri = self.sri()
            if sri and sri.subsize > 0:
                retval = [from_char(frame) for frame in retval]
            else:
                retval = from_char(retval)
        if tstamps:
            return (retval, timestamps)
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
    """
      To use a different sink (for custom data processing), assign the new class to sinkClass
    """
    def __init__(self, sinkClass=bulkio_data_helpers.ProbeSink):
        _SinkBase.__init__(self)
        self._sinkClass = sinkClass

    def getPort(self, portName):
        if _domainless._DEBUG == True:
            print "probeBULKIO:getPort() portName " + str(portName) + "================================="
        try:
            self._sinkPortType = self.getPortType(portName)

            # Set up output array sink
            self._sink = self._sinkClass(eval(self._sinkPortType))

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

        plotCommand = str(self._eclipsePath) + "/bin/plotter.sh -portname " + str(self._usesPortName) + " -repid " + str(self._dataType) + " -ior " + str(self.usesPortIORString)
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
      - short, ushort, complexShort, complexUShort
      - float, double, complexFloat, complexDouble
      - long, ulong, complexLong, complexULong
      - longlong, ulonglong, complexLongLong, complexULongLong
      - char, complexChar
      - octet, complexOctet
      - string
      - boolean, complexBoolean
    For sequences, encase the data type in brackets (e.g.: [short])
    '''
    def __init__(self, name, value, format):
        # validate format is legal type to convert to
        if format[0] == '[' and format[-1] == ']':
            if format[1:-1] in _properties.getTypeMap().keys():
                self._name   = name
                self._value  = value
                self._format = format
            else:
                raise RuntimeError("Unsupported format type: " + format)
        elif format in _properties.getTypeMap().keys():
            self._name   = name
            self._value  = value
            self._format = format
        else:
            raise RuntimeError("Unsupported format type: " + format)

def createTimeStamp():
    return _bulkio_helpers.createCPUTimestamp()

def createSRI(streamID='defaultStreamID', sampleRate=1.0, mode=0, blocking=True ):
    xd=1.0
    if sampleRate and sampleRate > 0.0:
        xd = 1.0/float(sampleRate)

    return _BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=xd,
                              xunits=1, subsize=0, ystart=0.0, ydelta=0.0,
                              yunits=0, mode=mode, streamID=streamID, blocking=blocking, keywords=[])
