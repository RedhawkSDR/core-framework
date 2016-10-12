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
from omniORB import any as _any
from omniORB import CORBA as _CORBA

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

class helperBase(object):
    def __init__(self):
        pass

    def _cleanUpCurrentState(self):
        if not hasattr(self,'_instanceName'):
            return

        if _domainless._DEBUG == True:
            print "Component: _cleanUpCurrentState() for component " + str(self._instanceName)
        # Remove any connections involving this component
        if _domainless._currentState != None and _domainless._currentState.has_key('Component Connections'):
            # Loop over connections to see if component has any uses side connections
            for connection in _domainless._currentState['Component Connections'].keys():
                if _domainless._currentState['Component Connections'][connection]['Uses Component'] == self:
                    providesComponent = _domainless._currentState['Component Connections'][connection]['Provides Component']
                    try:
                        self.disconnect(providesComponent)
                    except:
                        pass
            # Loop over connections to see if component has any provides side connections
            for connection in _domainless._currentState['Component Connections'].keys():
                if _domainless._currentState['Component Connections'][connection]['Provides Component'] == self:
                    usesComponent = _domainless._currentState['Component Connections'][connection]['Uses Component']
                    try:
                        usesComponent.disconnect(self)
                    except:
                        pass
        # Remove component entry in _currentState
        if _domainless._currentState != None and _domainless._currentState.has_key('Components Running'):
            for component in _domainless._currentState['Components Running'].keys():
                if _domainless._currentState['Components Running'][component] == self:
                    del _domainless._currentState['Components Running'][component]

    def releaseObject(self):
        self._cleanUpCurrentState()

    def reset(self):
        pass

class MessageSink(helperBase):
    def __init__(self, messageId = None, messageFormat = None, messageCallback = None):
        self._messagePort = None
        self._messageId = messageId
        self._messageFormat = messageFormat
        self._messageCallback = messageCallback
        self._providesPortDict = {}
        self._providesPortDict[0] = {}
        numComponentsRunning  = len(_domainless._currentState['Components Running'].keys())
        self._instanceName = "__localMessageSink_" + str(numComponentsRunning+1)
        _domainless._currentState['Components Running'][self._instanceName] = self
        self._buildAPI()

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
            print "MessageSink:getPort(): failed " + str(e)
        return None

    def _buildAPI(self):
        self._providesPortDict[0] = {}
        self._providesPortDict[0]["Port Interface"] = "IDL:ExtendedEvent/MessageEvent:1.0"
        self._providesPortDict[0]["Port Name"] = "msgIn"
        if _domainless._DEBUG == True:
            print "MessageSink:_buildAPI()"
            self.api()

    def api(self):
        print "Component MessageSink :"
        # Determine the maximum length of port names for print formatting
        maxNameLen = 0
        for port in self._providesPortDict.values():
            if len(port['Port Name']) > maxNameLen:
                maxNameLen = len(port['Port Name']) 
        print "Provides (Input) Ports =============="
        print "Port Name" + " "*(maxNameLen-len("Port Name")) + "\tPort Interface"
        print "---------" + " "*(maxNameLen-len("---------")) + "\t--------------"
        if len(self._providesPortDict) > 0:
            for port in self._providesPortDict.values():
                print str(port['Port Name']) + " "*(maxNameLen-len(str(port['Port Name']))) + "\t" + str(port['Port Interface'])
        else:
            print "None"
        print "\n"

    def start(self):
        pass

    def stop(self):
        pass

    def eos(self):
        return False

    def sri(self):
        return None

class MessageSource(helperBase):
    def __init__(self, messageId = None, messageFormat = None):
        self._messagePort = None
        self._messageId = messageId
        self._messageFormat = messageFormat
        self._usesPortDict = {}
        self._usesPortDict[0] = {}
        numComponentsRunning  = len(_domainless._currentState['Components Running'].keys())
        self._instanceName = "__localMessageSource_" + str(numComponentsRunning+1)
        _domainless._currentState['Components Running'][self._instanceName] = self
        self._buildAPI()

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
            print "MessageSource:getPort(): failed " + str(e)
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
            print "MessageSource:getUsesPort(): failed " + str(e)
        return None

    def connect(self,providesComponent, providesPortName=None, usesPortName=None):
        # If passed in object is of type _OutputBase, set uses port IOR string and return
        if isinstance(providesComponent, _OutputBase):
            usesPort_ref = None
            # If only one uses port, select it to connect to output
            if len(self._usesPortDict) == 1:
                usesPortName = self._usesPortDict[0]['Port Name']
            if usesPortName != None:
                usesPort_ref = self.getUsesPort()
                if usesPort_ref != None:
                    portIOR = str(_domainless.orb.object_to_string(usesPort_ref))
                    # determine port type
                    foundUsesPortInterface = None
                    for outputPort in self._usesPortDict.values():
                        if outputPort['Port Name'] == usesPortName:
                            foundUsesPortInterface = outputPort['Port Interface']
                            break
                    if foundUsesPortInterface != None:
                        dataType = foundUsesPortInterface
                        providesComponent.setup(portIOR, dataType=dataType, componentName="__localMessageSource", usesPortName=usesPortName)
                        return
                    raise AssertionError, "MessageSource:connect() failed because usesPortName given was not a valid port for providesComponent"
                else:
                    raise AssertionError, "MessageSource:connect() failed because usesPortName given was not a valid port for providesComponent"
            else:
                raise AssertionError, "MessageSource:connect() failed because usesPortName was not specified ... providesComponent being connect requires it for connection"

        if isinstance(providesComponent, _domainless.Component) == False and \
           isinstance(providesComponent, MessageSink) == False:
            raise AssertionError,"MessageSource:connect() ERROR - connect failed because providesComponent passed in is not of valid type ... valid types include instance of MessageSink, or Component classes"

        global currentState
        if _domainless._DEBUG == True:
            if providesPortName != None:
                print "MessageSource(): connect() providesPortName " + str(providesPortName)
            if usesPortName != None:
                print "MessageSource(): connect() usesPortName " + str(usesPortName)
        portMatchFound = False
        foundProvidesPortInterface = None
        foundProvidesPortName = None
        foundUsesPortName = None
        foundUsesPortInterface = None
        outputPort = self._usesPortDict[0]
        if providesPortName != None:
            for inputPort in providesComponent._providesPortDict.values():
                if inputPort['Port Name'] == providesPortName and \
                   outputPort['Port Interface'] == inputPort['Port Interface']:
                    portMatchFound = True
                    foundProvidesPortInterface = inputPort['Port Interface']
                    foundProvidesPortName = inputPort['Port Name']
                    foundUsesPortInterface = outputPort['Port Interface']
                    foundUsesPortName = outputPort['Port Name']
                    # If match was found, break out of inputPort for loop
                    break
        else:
            for inputPort in providesComponent._providesPortDict.values():
                if outputPort["Port Interface"] == inputPort["Port Interface"]:
                    # If a previous match had been found, connect fails due to ambiguity
                    if portMatchFound == True:
                        raise AssertionError,"MessageSource:connect(): ERROR - connect failed ... multiple ports matched interfaces on connect call ... must specify providesPortName or usesPortName"
                    # First port match found
                    else:
                        portMatchFound = True 
                        foundProvidesPortInterface = inputPort["Port Interface"]
                        foundProvidesPortName = inputPort["Port Name"]
                        foundUsesPortInterface = outputPort["Port Interface"]
                        foundUsesPortName = outputPort["Port Name"]
                        if _domainless._DEBUG == True:
                            print "MessageSource:connect() portMatchFound == True"
                            print "MessageSource:connect() inputPort name " + str(inputPort["Port Name"])
                            print "MessageSource:connect() inputPort interface " + str(inputPort["Port Interface"])
                            print "MessageSource:connect() outputPort name " + str(outputPort["Port Name"])
                            print "MessageSource:connect() outputPort interface " + str(outputPort["Port Interface"])

        if foundProvidesPortName != None and \
            foundUsesPortName != None:
            try:
                usesPort_ref = None
                providesPort_ref = None
                usesPort_handle = self.getUsesPort()
                if (hasattr(providesComponent, 'ref')):
                    providesPort_ref = providesComponent.ref.getPort(str(foundProvidesPortName))
                else:
                    providesPort_ref = providesComponent.getPort(str(foundProvidesPortName))
                if providesPort_ref != None:
                    counter = 0
                    while True:
                        connectionID = self._instanceName+"-"+providesComponent._instanceName+"_"+repr(counter)
                        if not _domainless._currentState['Component Connections'].has_key(connectionID):
                            break
                        counter = counter + 1
                    if _domainless._DEBUG == True:
                        print "MessageSource:connect() calling connectPort() with connectionID " + str(connectionID)
                    usesPort_handle.connectPort(providesPort_ref, str(connectionID))
                    _domainless._currentState['Component Connections'][connectionID] = {}
                    _domainless._currentState['Component Connections'][connectionID]['Uses Port Name'] = str(foundUsesPortName)
                    _domainless._currentState['Component Connections'][connectionID]['Uses Port Interface'] = str(foundUsesPortInterface)
                    _domainless._currentState['Component Connections'][connectionID]['Uses Component'] = self
                    _domainless._currentState['Component Connections'][connectionID]['Provides Port Name'] = str(foundProvidesPortName)
                    _domainless._currentState['Component Connections'][connectionID]['Provides Port Interface'] = str(foundProvidesPortInterface)
                    _domainless._currentState['Component Connections'][connectionID]['Provides Component'] = providesComponent
                    return
            except Exception, e:
                print "MessageSource:connect() failed " + str(e)
        return

    def disconnect(self,providesComponent):
        if _domainless._DEBUG == True:
            print "MessageSource:disconnect()"
        usesPort_ref = None
        usesPort_handle = None
        for id in _domainless._currentState['Component Connections'].keys():
            if _domainless._currentState['Component Connections'][id]['Uses Component']._refid == self._refid and \
               _domainless._currentState['Component Connections'][id]['Provides Component']._refid == providesComponent._refid:
                usesPortName = _domainless._currentState['Component Connections'][id]['Uses Port Name']
                usesPort_ref = None
                usesPort_ref = self.getUsesPort()
                if usesPort_ref != None:
                    usesPort_handle = usesPort_ref._narrow(_CF.Port)
                if usesPort_handle != None:
                    if _domainless._DEBUG == True:
                        print "MessageSource:disconnect(): calling disconnectPort"
                    usesPort_handle.disconnectPort(id)
                    del _domainless._currentState['Component Connections'][id]

    def _buildAPI(self):
        self._usesPortDict[0] = {}
        self._usesPortDict[0]["Port Interface"] = "IDL:ExtendedEvent/MessageEvent:1.0"
        self._usesPortDict[0]["Port Name"] = "msgOut"
        if _domainless._DEBUG == True:
            print "MessageSource:_buildAPI()"
            self.api()

    def api(self):
        print "Component MessageSource :"
        # Determine the maximum length of port names for print formatting
        maxNameLen = 0
        for port in self._usesPortDict.values():
            if len(port['Port Name']) > maxNameLen:
                maxNameLen = len(port['Port Name']) 
        print "Uses (Output) Ports =============="
        print "Port Name" + " "*(maxNameLen-len("Port Name")) + "\tPort Interface"
        print "---------" + " "*(maxNameLen-len("---------")) + "\t--------------"
        if len(self._usesPortDict) > 0:
            for port in self._usesPortDict.values():
                print str(port['Port Name']) + " "*(maxNameLen-len(str(port['Port Name']))) + "\t" + str(port['Port Interface'])
        else:
            print "None"
        print "\n"

    def start(self):
        pass

    def stop(self):
        pass

    def eos(self):
        return False

    def sri(self):
        return None

class _DataPortBase(helperBase):

    def __init__(self, className, portNameAppendix = ""):
        """
        Protected

        Sets self.className and calls parent constructor.

        Defines self.supportedPorts, which determines what interface types are
        supported by inheriting classes.

        """

        self.className = className # for DEBUG/WARNING statements

        self.portNameAppendix = portNameAppendix

        # Note: self._defaultDataFormat needs to be supported in self.supportedPorts
        self._defaultDataFormat = "short"
        self._defaultDataPortName = self._defaultDataFormat + self.portNameAppendix


        helperBase.__init__(self)

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
                            "portType" : "_BULKIO__POA.dataChar",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataChar:1.0",
                                          "Port Name"      : "char"}},
             "short"     : {"bytesPerSample" : 2,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataShort",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataShort:1.0",
                                          "Port Name"      : "short"}},
             "long"      : {"bytesPerSample" : 4,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataLong",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataLong:1.0",
                                          "Port Name"      : "long"}},
             "float"     : {"bytesPerSample" : 4,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataFloat" ,
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataFloat:1.0",
                                          "Port Name"      : "float"}},
             "ulong"     : {"bytesPerSample" : 4,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataUlong" ,
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataUlong:1.0",
                                          "Port Name"      : "uLong"}},
             "double"    : {"bytesPerSample" : 8,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataDouble",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataDouble:1.0",
                                          "Port Name"      : "double"}},
             "longlong"  : {"bytesPerSample" : 8,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataLongLong",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataLongLong:1.0",
                                          "Port Name"      : "longlong"}},
             "octet"     : {"bytesPerSample" : 1,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataOctet",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataOctet:1.0",
                                          "Port Name"      : "octet"}},
             "ulonglong" : {"bytesPerSample" : 8,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataUlongLong",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataUlongLong:1.0",
                                          "Port Name"      : "ulonglong"}},
             "ushort"    : {"bytesPerSample" : 2,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataUshort",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataUshort:1.0",
                                          "Port Name"      : "ushort"}},
             "xml"       : {"bytesPerSample" : 1,
                            "pktSize"        : -1,
                            "portType" : "_BULKIO__POA.dataXML",
                            "portDict" : {"Port Interface" : "IDL:BULKIO/dataXML:1.0",
                                          "Port Name"      : "xml"}}}

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
 

class _SourceBase(_DataPortBase):

    def __init__(self, className, bytesPerPush, dataFormat, data = None):
        """
        Forward parameters to parent constructor.

        className to be used in print statements.

        Set self._dataFormat to dataFormat it it is in the list
        of supported ports, otherwise attempt to guess the format
        based on the data or set the format to self._defaultDataFormat.

        Calls _buildAPI()

        """

        _DataPortBase.__init__(self, 
                               className = className, 
                               portNameAppendix = "Out")

        self.bytesPerPush = int(bytesPerPush)

        if data != None:
            print "WARNING: predicting data source format type by data argument is deprecated.  This will be removed in the next version"

        if dataFormat != None:
            # add support for format byte (which is the same as char)
            if dataFormat.lower() == 'byte':
                dataFormat = 'char'
            if self.supportedPorts.has_key(dataFormat.lower()):
                self._dataFormat = dataFormat.lower()
        else:
            self._dataFormat = None
 
        self._connections = {}
        self._buildAPI()

    def _addConnection(self, portName, arraySrcInst, connectionId):
        """
        When a connection is made, this method should be called 
        to update the running list of array sources.

        """

        pktSize = self._getMetaByPortName("pktSize", portName)
        srcPortType = self._getMetaByPortName("portType", portName)

        connection = {"pktSize"      : pktSize,
                      "srcPortType"  : srcPortType,
                      "arraySrcInst" : arraySrcInst}

        self._connections[connectionId] = connection

    def _addUsesPort(self, port, index):
        """
        Adds the port to self._usesPortDictionary.

        Also sets the pktSize associated with the port.

        """

        self._usesPortDict[index] = port["portDict"]
        self._usesPortDict[index]["Port Name"] += self.portNameAppendix
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

        self._usesPortDict = {}

        index = 0
        if self._dataFormat != None:
            port = self.supportedPorts[self._dataFormat]
            self._srcPortType = port["portType"] 
            self._addUsesPort(port, index)
        else:
            # Create all ports
            for port in self.supportedPorts.values():
                self._addUsesPort(port, index)
                index += 1
        

        if _domainless._DEBUG == True:
            print self.className + ":_buildAPI()"
            self.api()

    def api(self):
        """
        Prints application programming interface (API) information and returns.

        """

        print "Component " + self.className + " :"
        maxNameLen = 0
        # Determine the maximum length of port names for print formatting
        for port in self._usesPortDict.values():
            if len(port['Port Name']) > maxNameLen:
                maxNameLen = len(port['Port Name']) 
        print "Uses (Output) Ports =============="
        print "Port Name" + " "*(maxNameLen-len("Port Name")) + "\tPort Interface"
        print "---------" + " "*(maxNameLen-len("---------")) + "\t--------------"
        if len(self._usesPortDict) > 0:
            for port in self._usesPortDict.values():
                print str(port['Port Name']) + " "*(maxNameLen-len(str(port['Port Name']))) + "\t" + str(port['Port Interface'])
        else:
            print "None"
        print "\n"

    def _setupProvidesComponent(self, usesPortName, providesComponent):
        """
        Handles the passing of port IOR information to a provides component.

        This method is used when connecting to things like Plot modules.

        """

        if usesPortName == None and len(self._usesPortDict.keys()) == 1:
            # If no uses port name is provided, and there is only 1 uses
            # port, use the 1 uses port
            usesPortName = self._usesPortDict[0]["Port Name"]
        elif usesPortName == None and len(self._usesPortDict.keys()) > 1:
            # If no uses port name is provided, and there are multiple uses
            # ports, use the default
            usesPortName = self._defaultDataPortName 

        arraySrcInst = self._createArraySrcInst(self._getMetaByPortName("portType", usesPortName))

        usesPort_ref = arraySrcInst.getPort()

        if usesPort_ref != None:
            portIOR = str(_domainless.orb.object_to_string(usesPort_ref))
            # determine port type
            foundUsesPortInterface = None
            for outputPort in self._usesPortDict.values():
                if outputPort['Port Name'] == usesPortName:
                    foundUsesPortInterface = outputPort['Port Interface']
                    break
            if foundUsesPortInterface != None:
                dataType = foundUsesPortInterface
                providesComponent.setup(portIOR,
                                        dataType=dataType, 
                                        componentName="__local"+self.className, 
                                        usesPortName=usesPortName)
                self._addConnection(usesPortName, arraySrcInst, portIOR)

                return True
            raise AssertionError, self.className+":connect() failed because usesPortName given was not a valid port for providesComponent"
        else:
            raise AssertionError, self.className+":connect() failed because usesPortName given was not a valid port for providesComponent"

    def _determineUsesPortName(self, providesComponent, providesName):
        """
        Attempts to guess the appropriate uses port name.  If a provides
        port is provided, a uses port with a matching interface type is
        returned.  Otherwise, the appropriate uses port is determined
        by calling self._findNameByMatchingInterface.

        """

        # If a provides port name is given, match to the port type 
        # of that interface.
        if providesName:
            providesInterface = self._getInterfaceByName(
                                    providesComponent._providesPortDict,
                                    providesName)
            for usesPort in self._usesPortDict.values():
                if usesPort["Port Interface"] == providesInterface:
                    return usesPort["Port Name"]
            raise Exception, self.className + ":connect() failed.  The specified provides port is of type " + providesInterface + ".  There is no correspinding uses port of this type."

        return self._findNameByMatchingInterface(
                    matchToDict = providesComponent._providesPortDict,
                    portCandidateDict = self._usesPortDict)

    def _findNameByMatchingInterface(self, matchToDict, portCandidateDict):
        """
        Loop through all the ports in matchToDict and portCandidateDict.
        If a matching port type is found, return the corresponding port
        name from portCandiateDict.

        If more than one match is found, or if no matches are found,
        throw an exception.

        """

        matchesFound = 0
        for matchToPort in matchToDict.values():
            for portCandidate in portCandidateDict.values():
                if portCandidate["Port Interface"] == matchToPort["Port Interface"]:
                    portName = portCandidate["Port Name"]
                    matchesFound += 1
            
        if matchesFound == 1:
            return portName
        elif matchesFound == 0: 
            raise Exception, self.className + ":connect() failed.  No matching interface found."
        else:
            raise Exception, self.className + ":connect() failed.  Multiple matching interfaces found.  Must specify port name"


    def connect(self,
                providesComponent, 
                providesPortName=None, 
                usesPortName=None):
        """
        Connect this module to the providesPort specified.  There are 4 main
        sections to this process:

            1.  Determine the name of the uses port.  If one is provided, use
                it; if one is not provided, attempt to guess the appropriate
                uses port based on the interface types.

            2.  Determine the name of the provides port.  If one is provided,
                use it; if one is not provided, find an interface that matches
                the iterface type of the uses port.

            3.  If connectinig to a providesComponent of type _OutputBase, 
                call self._setupProvidesComponent and leave.

            4.  If not connecting to a providesComponent of type _OutputBase,
                make the connection using the self._connectWithKnownNames()
                method.

        """

        # Make sure providesComponent type is valid
        if isinstance(providesComponent, _domainless.Component) == False and \
           isinstance(providesComponent, _OutputBase) == False and \
           isinstance(providesComponent, FileSink) == False and \
           isinstance(providesComponent, DataSink) == False:
            raise AssertionError,self.className+":connect() ERROR - connect failed because providesComponent passed in is not of valid type ... valid types include instance of _OutputBase, FileSink, DataSink, or Component classes"

        # If passed in object is of type _OutputBase, set uses port IOR string and return
        if isinstance(providesComponent, _OutputBase):
            # We are doing something like connecting to a Plot instance
            return self._setupProvidesComponent(usesPortName, providesComponent)

        # If uses port name is not provided, try to guess one
        if usesPortName == None:
            usesPortName = self._determineUsesPortName(providesComponent, 
                                                       providesPortName)

        usesPort = self.getPortByName(usesPortName)["portDict"]

        # If provides port name is not provided, try to guess one
        if providesPortName == None:
            # We know which uses port we want to use, so we only
            # pass one for the method to choose from.
            providesPortName = self._findNameByMatchingInterface(matchToDict = {1: usesPort},
                                                                 portCandidateDict = providesComponent._providesPortDict)

        # Look up the uses/provides interface types now that we knwo the
        # port names.
        usesPortInterface     = usesPort["Port Interface"]
        providesPortInterface = self._getInterfaceByName(providesComponent._providesPortDict, 
                                                         providesPortName)

        # Debug print statements
        if _domainless._DEBUG == True:
            if providesPortName != None:
                print self.className+"(): connect() providesPortName " + str(providesPortName)
            if usesPortName != None:
                print self.className+"(): connect() usesPortName " + str(usesPortName)

        return self._connectWithKnownNames(usesPortName, 
                                           providesPortName, 
                                           usesPortInterface, 
                                           providesPortInterface,
                                           providesComponent)

    def _getInterfaceByName(self, dictOfPorts, portName):
        """
        Provided a dictionary of ports (each entry of the dictionary being
        a dictionary with keys 'Port Name' and 'Port Interface') and a
        portName, itterate through the dictionary to find the interface
        type associated with the portName.

        """

        for port in dictOfPorts.values():
            if port['Port Name'] == portName:
                return port['Port Interface']

        raise Exception, self.className + "::connect() failed.  Port named " + portName + " not found"

    def _connectWithKnownNames(self, 
                              foundUsesPortName, 
                              foundProvidesPortName, 
                              foundUsesPortInterface, 
                              foundProvidesPortInterface,
                              providesComponent):
        """
        Creates a connection between two ports given the metadata associated
        with the port.  This method assumes that the appropriate ports are 
        already know (that is, they have already been specified by the user,
        or another module has already guessed the appropriate ports).

        Creates an array source associated with the connection and adds
        the array source to the module's list of array source.

        """

        if foundUsesPortInterface != foundProvidesPortInterface:
            raise Exception, self.className + "::connect failed.  Uses/provides interface types do not match"
        try:
            usesPort_ref = None
            providesPort_ref = None
            srcPortType = self._getMetaByPortName("portType", foundUsesPortName)
            arraySrcInst = self._createArraySrcInst(srcPortType)
            usesPort_handle = arraySrcInst.getPort()
            providesPort_ref = providesComponent.ref.getPort(str(foundProvidesPortName))
            if providesPort_ref != None:
                counter = 0
                while True:
                    connectionID = self._instanceName+"-"+providesComponent._instanceName+"_"+repr(counter)
                    if not _domainless._currentState['Component Connections'].has_key(connectionID):
                        break
                    counter = counter + 1
                if _domainless._DEBUG == True:
                    print self.className+":connect() calling connectPort() with connectionID " + str(connectionID)
                usesPort_handle.connectPort(providesPort_ref, str(connectionID))
                _domainless._currentState['Component Connections'][connectionID] = {}
                _domainless._currentState['Component Connections'][connectionID]['Uses Port Name'] = str(foundUsesPortName)
                _domainless._currentState['Component Connections'][connectionID]['Uses Port Interface'] = str(foundUsesPortInterface)
                _domainless._currentState['Component Connections'][connectionID]['Uses Component'] = self
                _domainless._currentState['Component Connections'][connectionID]['Provides Port Name'] = str(foundProvidesPortName)
                _domainless._currentState['Component Connections'][connectionID]['Provides Port Interface'] = str(foundProvidesPortInterface)
                _domainless._currentState['Component Connections'][connectionID]['Provides Component'] = providesComponent

                self._addConnection(foundUsesPortName, arraySrcInst, connectionID)

        except Exception, e:
            print self.className+":connect() failed " + str(e)

    def disconnect(self,providesComponent):
        if _domainless._DEBUG == True:
            print self.className + ":disconnect()"
        usesPort_ref = None
        usesPort_handle = None
        for id in _domainless._currentState['Component Connections'].keys():
            if _domainless._currentState['Component Connections'][id]['Uses Component']._refid == self._refid and \
               _domainless._currentState['Component Connections'][id]['Provides Component']._refid == providesComponent._refid:
                usesPortName = _domainless._currentState['Component Connections'][id]['Uses Port Name']
                usesPort_ref = None
                arraySrcInst = self._connections[id]["arraySrcInst"]
                usesPort_ref = arraySrcInst.getPort()
                del self._connections[id]
                if usesPort_ref != None:
                    usesPort_handle = usesPort_ref._narrow(_CF.Port)
                if usesPort_handle != None:
                    if _domainless._DEBUG == True:
                        print self.className + ":disconnect(): calling disconnectPort"
                    usesPort_handle.disconnectPort(id)
                    del _domainless._currentState['Component Connections'][id]

class _SinkBase(_DataPortBase):

    def __init__(self, className):
        """
        Forward parameters to parent constructor.

        className to be used in print statements.

        Calls _buildAPI()

        """

        _DataPortBase.__init__(self, 
                               className = className, 
                               portNameAppendix = "In")

        # Amount of time the self._sink is given to write to the gotEOS flag
        self._sleepTime = 0.001
        self.breakBlock = False

        self._providesPortDict = {}
        self._providesPortDict[0] = {}
        self._sinkPortObject = None
        self._sinkPortType = None
        self._sink = None
        self.ref = self
        self._buildAPI()
        self._refid = _domainless._uuidgen()
        numComponentsRunning  = len(_domainless._currentState['Components Running'].keys())
        # Use one-up counter to make component instance unique
        self._instanceName = "__local" + self.className + "_" + str(numComponentsRunning+1)
        _domainless._currentState['Components Running'][self._instanceName] = self

    def _buildAPI(self):
        """
        Populate the self._providesPortDict with one entry per possible
        port type defined in self.supportedPorts.

        """
        index = 0
        for port in self.supportedPorts.values():
            self._providesPortDict[index] = port["portDict"]
            self._providesPortDict[index]["Port Name"] += self.portNameAppendix
            index += 1

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

    def api(self):
        """
        Prints application programming interface (API) information and returns.

        """

        print "Component " + self.className + " :"
        # Determine the maximum length of port names for print formatting
        maxNameLen = 0
        for port in self._providesPortDict.values():
            if len(port['Port Name']) > maxNameLen:
                maxNameLen = len(port['Port Name']) 
        print "Provides (Input) Ports =============="
        print "Port Name" + " "*(maxNameLen-len("Port Name")) + "\tPort Interface"
        print "---------" + " "*(maxNameLen-len("---------")) + "\t--------------"
        if len(self._providesPortDict) > 0:
            for port in self._providesPortDict.values():
                print str(port['Port Name']) + " "*(maxNameLen-len(str(port['Port Name']))) + "\t" + str(port['Port Interface'])
        else:
            print "None"
        print "\n"
    
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

        _SourceBase.__init__(self, 
                             className = "FileSource", 
                             bytesPerPush = bytesPerPush, 
                             dataFormat = dataFormat) 

        if dataFormat == None:
            print "WARNING: dataFormat not provided for FileSource; defaulting to " + self._defaultDataFormat
            dataFormat = self._defaultDataFormat 
        if self.supportedPorts.has_key(dataFormat):
            self._srcPortType = self.supportedPorts[dataFormat]["portType"]
        else:
            raise Exception, "ERROR: FileSource does not supporte data type " + dataFormat
 
        self._src         = None
        self._runThread   = None
        self._processes   = {}
        self._sampleRate  = sampleRate 
        self._complexData = complexData
        self._SRIKeywords = SRIKeywords
        self._startTime   = startTime
        self._streamID    = streamID
        self._blocking    = blocking 
        self._refid       = _domainless._uuidgen()
        self._sri         = None

        self._srcPortObject = None
        self.setupFileReader()
        numComponentsRunning  = len(_domainless._currentState['Components Running'].keys())
        # Use one-up counter to make component instance unique
        self._instanceName = "__local" + self.className + "_" + str(numComponentsRunning+1)
        _domainless._currentState['Components Running'][self._instanceName] = self

    def __del__(self):
        if _domainless._DEBUG == True:
            print self.className + ": __del__() calling cleanUp"
        self.cleanUp()

    def cleanUp(self):
        for pid in self._processes.keys():
            if _domainless._DEBUG == True:
                print self.className + ": cleanUp() calling __terminate for pid " + str(pid)
            self.__terminate(pid)

    def __terminate(self,pid):
        sp = self._processes[pid]
        for sig, timeout in self._STOP_SIGNALS:
            try:
                # the group id is used to handle child processes (if they 
                # exist) of the component being cleaned up
                if _domainless._DEBUG == True:
                    print self.className + ": __terminate () making killpg call on pid " + str(pid) + " with signal " + str(sig)
                _os.killpg(pid, sig)
            except OSError:
                print self.className + ": __terminate() OSERROR ==============="
                pass
            if timeout != None:
                giveup_time = _time.time() + timeout
            while sp.poll() == None:
                if _time.time() > giveup_time: break
                _time.sleep(0.1)
            if sp.poll() != None: break
        sp.wait()

    def _createArraySrcInst(self, srcPortType):
        return self._src

    def setupFileReader(self):
        try:
            portType = self._srcPortType
            # If input file is a Midas Blue file
            if self._midasFile == True:
                # define source helper component
                self._src = _bluefile_helpers.BlueFileReader(eval(portType))
            # else, input file is binary file
            else:
                self._src = _bulkio_data_helpers.FileSource(eval(portType))
                keywords = []
                for key in self._SRIKeywords:
                    keywords.append(_CF.DataType(key._name, _properties.to_tc_value(key._value,str(key._format))))
            
                self._sri = _BULKIO.StreamSRI(1, 0.0, 1, 0, 0, 0.0, 0, 0, 0,
                                            "defaultStreamID", True, keywords)

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
            print self.className + ":setupFileReader(): failed " + str(e)

    def getUsesPort(self):
        try:
            if self._src != None:
                self._srcPortObject = self._src.getPort()
                return self._srcPortObject
        except Exception, e:
            print self.className + ":getUsesPort(): failed " + str(e)
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
    def __init__(self,filename="", midasFile=False):
        className = "FileSink"

        if _domainless._DEBUG == True:
            print className + ":__init__() filename " + str(filename)
            print className + ":__init__() midasFile " + str(midasFile)
        self._filename = filename
        self._midasFile = midasFile

        _SinkBase.__init__(self, className = className)
   
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

            if self._sink != None and self._sink.outFile != None:
                self._sinkPortObject = self._sink.getPort()
                return self._sinkPortObject
            else:
                return None
        except Exception, e:
            print self.className + ":getPort(): failed " + str(e)
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
                    print self.className + ": " + str(e)

            finally:
                # If this thread dies for some reason, need to release
                # the lock to keep from taking other threads down with it.
                self._sink.port_lock.release()
        else:
            # If the user has yet to call getPort(), the the file sink may
            # not yet exist.
            print "No file writer present, therefore not waiting for EOS.  Is the " + self.className + " module connected?"

class DataSource(_SourceBase):
    def __init__(self, 
                 data         = None, 
                 dataFormat   = None,
                 loop         = False,
                 bytesPerPush = 512000,
                 startTime    = 0.0):

        self.threadExited = None
        
        _SourceBase.__init__(self, 
                             className    = "DataSource", 
                             bytesPerPush = bytesPerPush, 
                             dataFormat   = dataFormat,
                             data         = data)

        self._sampleRate  = None
        self._complexData = None
        self._SRIKeywords = []
        self._sri         = None
        self._startTime   = startTime
        self._loop        = loop
        self._runThread   = None
        self._dataQueue   = _Queue.Queue()

        self.ref = self
        self._refid = _domainless._uuidgen()
        numComponentsRunning  = len(_domainless._currentState['Components Running'].keys())
        # Use one-up counter to make component instance unique
        self._instanceName = "__local" + self.className + "_" + str(numComponentsRunning+1)
        _domainless._currentState['Components Running'][self._instanceName] = self


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
        self._dataQueue.put((data,
                             EOS,
                             streamID,
                             sampleRate,
                             complexData,
                             SRIKeywords,
                             loop))

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
                                                streamID, True, keywords)
                
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
                                                "defaultStreamID", True, [])

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
            except Exception, e:
                print self.className + ":pushData() failed " + str(e)
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

        if srcPortType == "_BULKIO__POA.dataXML":
            if type(data) != str:
                print "data must be a string for the specified data type"
                return
        else:
            if type(data) != list:
                print "data must be a list of values for the specified data type"
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
    def __init__(self):
        _SinkBase.__init__(self, className = "DataSink")

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
            print self.className + ":getPort(): failed " + str(e)
        return None

    def getData(self, length=None, eos_block=False):
        isChar = False
        if self._sink.port_type == _BULKIO__POA.dataChar:
            isChar = True
            
        if self._sink != None:
            self._sink.port_lock.acquire()
            try:
                if length == None:
                    if not eos_block:
                        retval = self._sink.data
                        self._sink.data = []
                    else:
                        while not self._sink.gotEOS and (not self.breakBlock):
                            self._sink.port_lock.release()
                            _time.sleep(self._sleepTime)
                            self._sink.port_lock.acquire()
                        retval = self._sink.data
                        self._sink.data = []
                else:
                    if not eos_block:
                        if len(self._sink.data) >= length:
                            retval = self._sink.data[:length]
                            self._sink.data.__delslice__(0,length)
                        else:
                            while (len(self._sink.data) < length) and (not self._sink.gotEOS) and (not self.breakBlock):
                                self._sink.port_lock.release()
                                _time.sleep(self._sleepTime)
                                self._sink.port_lock.acquire()
                            if len(self._sink.data) >= length:
                                retval = self._sink.data[:length]
                                self._sink.data.__delslice__(0,length)
                            else:
                                retval = self._sink.data
                                self._sink.data = []
                    else:
                        while not self._sink.gotEOS and (not self.breakBlock):
                            self._sink.port_lock.release()
                            _time.sleep(self._sleepTime)
                            self._sink.port_lock.acquire()
                        if length == None:
                            retval = self._sink.data
                            self._sink.data = []
                        else:
                            retval = self._sink.data[:length]
                            self._sink.data.__delslice__(0,length)
            finally:
                self._sink.port_lock.release()
            if isChar:
                newretval = list(_struct.unpack(str(len(retval))+'b',''.join(retval)))
                retval=newretval
            return retval
        else:
            return None

class _OutputBase(helperBase):
    def __init__(self):
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
                print "_OutputBase: __terminate() OSERROR ==============="
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

class probeBULKIO(_OutputBase):
    def __init__(self):

        self._providesPortDict    = {}
        self._providesPortDict[0] = {}
        self._sinkPortObject      = None
        self._sinkPortType        = None
        self._sink                = None
        self.ref                  = self
        self.breakBlock           = False

        self._buildAPI()

        self._refid = _domainless._uuidgen()
        numComponentsRunning  = len(_domainless._currentState['Components Running'].keys())
        # Use one-up counter to make component instance unique
        self._instanceName = "__localprobeBULKIO_" + str(numComponentsRunning+1)
        _domainless._currentState['Components Running'][self._instanceName] = self

    def _buildAPI(self):
        self._providesPortDict[0] = {} 
        self._providesPortDict[0]["Port Interface"] = "IDL:BULKIO/dataChar:1.0" 
        self._providesPortDict[0]["Port Name"] = "charIn"
        self._providesPortDict[1] = {} 
        self._providesPortDict[1]["Port Interface"] = "IDL:BULKIO/dataShort:1.0" 
        self._providesPortDict[1]["Port Name"] = "shortIn"
        self._providesPortDict[2] = {} 
        self._providesPortDict[2]["Port Interface"] = "IDL:BULKIO/dataLong:1.0" 
        self._providesPortDict[2]["Port Name"] = "longIn"
        self._providesPortDict[3] = {} 
        self._providesPortDict[3]["Port Interface"] = "IDL:BULKIO/dataFloat:1.0" 
        self._providesPortDict[3]["Port Name"] = "floatIn"
        self._providesPortDict[4] = {} 
        self._providesPortDict[4]["Port Interface"] = "IDL:BULKIO/dataDouble:1.0" 
        self._providesPortDict[4]["Port Name"] = "doubleIn"
        if _domainless._DEBUG == True:
            print "probeBULKIO:_buildAPI()"
            self.api()

    def api(self):
        print "Component probeBULKIO :"
        # Determine the maximum length of port names for print formatting
        maxNameLen = 0
        for port in self._providesPortDict.values():
            if len(port['Port Name']) > maxNameLen:
                maxNameLen = len(port['Port Name']) 
        print "Provides (Input) Ports =============="
        print "Port Name" + " "*(maxNameLen-len("Port Name")) + "\tPort Interface"
        print "---------" + " "*(maxNameLen-len("---------")) + "\t--------------"
        if len(self._providesPortDict) > 0:
            for port in self._providesPortDict.values():
                print str(port['Port Name']) + " "*(maxNameLen-len(str(port['Port Name']))) + "\t" + str(port['Port Interface'])
        else:
            print "None"
        print "\n"

    def getPort(self, portName):
        if _domainless._DEBUG == True:
            print "probeBULKIO:getPort() portName " + str(portName) + "================================="
        try:
            if portName == "charIn":
                self._sinkPortType = "_BULKIO__POA.dataChar" 
            elif portName == "shortIn":
                self._sinkPortType = "_BULKIO__POA.dataShort" 
            elif portName == "longIn":
                self._sinkPortType = "_BULKIO__POA.dataLong" 
            elif portName == "floatIn":
                self._sinkPortType = "_BULKIO__POA.dataFloat" 
            elif portName == "doubleIn":
               self._sinkPortType = "_BULKIO__POA.dataDouble" 
            else:
                return None

            # Set up output array sink
            self._sink = _bulkio_data_helpers.ProbeSink(eval(self._sinkPortType))

            if self._sink != None:
                self._sinkPortObject = self._sink.getPort()
                return self._sinkPortObject
            else:
                return None

        except Exception, e:
            print "probeBULKIO:getPort(): failed " + str(e)
        return None

    def start(self):
        self.breakBlock = False
        if self._sink != None:
            self._sink.start()

    def stop(self):
        self.breakBlock = True
    
    def eos(self):
        if self._sink == None:
            return False
        return self._sink.gotEOS

    def sri(self):
        if self._sink == None:
            return None
        else:
            return self._sink.sri
    
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
class Plot(_OutputBase):
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
         

    def setup(self, usesPortIOR, dataType=None, componentName=None, usesPortName=None):
        self.usesPortIORString = usesPortIOR
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

class InputFile(FileSource):
    def __init__(self,filename="", dataFormat=None, midasFile=False,sampleRate=1.0,complexData=False,SRIKeywords=[],bytesPerPush=512000,startTime=0.0,streamID=None,blocking=True):
        print "WARNING: InputFile is deprecated.  Use FileSource instead."
        FileSource.__init__(self,filename=filename, dataFormat=dataFormat, midasFile=midasFile,sampleRate=sampleRate,complexData=complexData,SRIKeywords=SRIKeywords,bytesPerPush=bytesPerPush,startTime=startTime,streamID=streamID,blocking=blocking)

class OutputData(DataSink):
    def __init__(self):
        print "WARNING: OutputData is deprecated.  Use DataSink instead."
        DataSink.__init__(self)

class InputData(DataSource):
    def __init__(self, data=None, dataFormat=None,loop=False,bytesPerPush=512000,startTime=0.0):
        print "WARNING: InputData is deprecated.  Use DataSource instead."
        DataSource.__init__(self, data=data, dataFormat=dataFormat,loop=loop,bytesPerPush=bytesPerPush,startTime=startTime)

class OutputFile(FileSink):
    def __init__(self,filename="", midasFile=False):
        print "WARNING: OutputFile is deprecated.  Use FileSink instead."
        FileSink.__init__(self,filename=filename, midasFile=midasFile)

class InputEvents(MessageSource):
    def __init__(self, messageId = None, messageFormat = None):
        print "WARNING: InputEvents is deprecated.  Use MessageSource instead."
        MessageSource.__init__(self,messageId=messageId, messageFormat=messageFormat)

class OutputEvents(MessageSink):
    def __init__(self, messageId = None, messageFormat = None, messageCallback = None):
        print "WARNING: OutputEvents is deprecated.  Use MessageSink instead."
        MessageSink.__init__(self,messageId=messageId, messageFormat=messageFormat, messageCallback=messageCallback)

