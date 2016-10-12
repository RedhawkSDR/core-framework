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


import commands as _commands
import os as _os
import copy as _copy
import logging
import weakref
import inspect
from ossie.cf import CF as _CF
from omniORB import any as _any
import string as _string
import struct as _struct
from ossie.utils import log4py as _log4py
from ossie.utils import prop_helpers as _prop_helpers
from ossie.utils import idllib
from ossie.utils import uuid as _uuid
from ossie import parsers as _parsers
from ossie import properties as _properties
from ossie.properties import getCFType 
from ossie.properties import getMemberType
from ossie.cf import ExtendedCF as _ExtendedCF
from ossie.utils.formatting import TablePrinter
import warnings as _warnings
from connect import *

_DEBUG = False 
_trackLaunchedApps = True

_idllib = idllib.IDLLibrary()
if 'OSSIEHOME' in _os.environ:
    _idllib.addSearchPath(_os.path.join(_os.environ['OSSIEHOME'], 'share/idl'))

__MIDAS_TYPE_MAP = {'char'  : ('/SB/8t'),
                    'octet' : ('/SO/8o'),
                    'short' : ('/SI/16t'),
                    'long'  : ('/SL/32t'),
                    'float' : ('/SF/32f'),
                    'double': ('/SD/64f')}

log = logging.getLogger(__name__)
def setDEBUG(debug=False):
    global _DEBUG
    _DEBUG = debug
    
def getDEBUG():
    global _DEBUG
    return _DEBUG

def setTrackApps(track=False):
    global _trackLaunchedApps
    _trackLaunchedApps = track

def getTrackApps():
    global _trackLaunchedApps
    return _trackLaunchedApps

def _uuidgen():
    return str(_uuid.uuid4()) 

class NoMatchingPorts(Exception):
    def __init__(self, *args):
        Exception.__init__(self,*args)

def _convertType(propType, val):
    ''' 
    Convert value from string form to the appropriate numeric type. 
    '''
     
    if propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
        # If value contains an x, it is a hex value (base 16) 
        if val.find('x') != -1:
            newValue = int(val,16)
        else:
            newValue = int(val)
    elif propType in ['double', 'float']:
        newValue = float(val)
    elif propType in ['char', 'string']:
        newValue = str(val)
    elif propType == 'boolean':
        newValue = {"TRUE": True, "FALSE": False}[val.strip().upper()]
    elif propType.find('complex') == 0:
        baseType = getMemberType(propType) 
        real, imag = _prop_helpers.parseComplexString(val, baseType)
        if isinstance(real, basestring):
            real = int(real)
            imag = int(imag)
        newValue = complex(real, imag)
    else:
        newValue = None
    return newValue    

def _getFileContents(filename, fileSystem):
    fileObj = fileSystem.open(filename, True)
    try:
        return fileObj.read(fileObj.sizeOf())
    finally:
        fileObj.close()

def _readProfile(spdFilename, fileSystem):
    spdContents = _getFileContents(spdFilename, fileSystem)
    spd = _parsers.SPDParser.parseString(spdContents)

    xmldir = _os.path.dirname(spdFilename)
    if spd.get_descriptor():
        scdFilename = _os.path.join(xmldir, str(spd.get_descriptor().get_localfile().get_name()))
        scdContents = _getFileContents(scdFilename, fileSystem)
        scd = _parsers.SCDParser.parseString(scdContents)
    else:
        scd = None

    if spd.get_propertyfile():
        prfFilename = _os.path.join(xmldir, str(spd.get_propertyfile().get_localfile().get_name()))
        prfContents = _getFileContents(prfFilename, fileSystem)
        prf = _parsers.PRFParser.parseString(prfContents)
    else:
        prf = None
    
    return spd, scd, prf

def _formatSimple(prop, value,id):
    currVal = str(value)
    # Checks if current prop is an enum
    try:
        if prop._enums != None:
            if value in prop._enums.values():
                currVal += " (enum=" + prop._enums.keys()[prop._enums.values().index(value)] + ")"
    except:
        return currVal
    return currVal

def _formatType(propType):
    return propType + __MIDAS_TYPE_MAP.get(propType, '')


def maxLength(iterable, default=0):
    try:
        return max(len(item) for item in iterable)
    except ValueError:
        return default


class OutputBase(object):
    """
    Special base class for objects that can be used as the provides side of a
    connection, but are created dynamically on connection.
    """
    def setup(self, usesIOR, dataType=None, componentName=None, usesPortName=None):
        raise NotImplementedError, 'OutputBase subclasses must implement setup()'


class CorbaObject(object):
    def __init__(self, ref=None):
        self.ref = ref

    def _matchInterface(self, repid):
        return self.ref._is_a(repid)

    def _getInterface(self):
        return self.ref._NP_RepositoryId

class PortSupplier(object):
    log = log.getChild('PortSupplier')

    def __init__(self):
        self._providesPortDict = {}
        self._usesPortDict = {}

    def _showPorts(self, ports):
        if ports:
            table = TablePrinter('Port Name', 'Port Interface')
            for port in ports.itervalues():
                table.append(port['Port Name'], port['Port Interface'])
            table.write()
        else:
            print "None"

    def api(self):
        print "Provides (Input) Ports =============="
        self._showPorts(self._providesPortDict)
        print

        print "Uses (Output) Ports =============="
        self._showPorts(self._usesPortDict)
        print

    def _getUsesPort(self, name):
        if not name in self._usesPortDict:
            raise RuntimeError, "Component '%s' has no uses port '%s'" % (self._instanceName, name)
        return self._usesPortDict[name]

    def _getDefaultUsesPort(self):
        numPorts = len(self._usesPortDict)
        if numPorts == 1:
            return self._usesPortDict.values()[0]
        elif numPorts == 0:
            raise RuntimeError, "Component '%s' has no uses ports" % self._instanceName
        else:
            raise RuntimeError, "Component '%s' has more than one port, connection is ambiguous" % self._instanceName

    def _matchUsesPort(self, usesPort, connectionId):
        interface = usesPort['Port Interface']

        # First, look for exact matches.
        matches = self._matchExact(interface, self._providesPortDict.values())

        # Only if no exact matches are found, try to check the CORBA interfaces
        # for compatibility.
        if not matches:
            for providesPort in self._providesPortDict.values():
                if self._canConnect(interface, providesPort['Port Interface']):
                    matches.append(providesPort)

        # Convert the matched ports to endpoints.
        return [self._getEndpoint(providesPort, connectionId) for providesPort in matches]

    def _getProvidesPort(self, name):
        if not name in self._providesPortDict:
            raise RuntimeError, "Component '%s' has no provides port '%s'" % (self._instanceName, name)
        return self._providesPortDict[name]

    def _getEndpoint(self, port, connectionId):
        return PortEndpoint(self, port)

    def _canConnect(self, usesInterface, providesInterface):
        if usesInterface == providesInterface:
            return True
        elif usesInterface == 'IDL:ExtendedEvent/MessageEvent:1.0':
            # Special case: a MessageEvent uses port actually narrows the
            # provides port to an EventChannel.
            return self._canConnect('IDL:omg.org/CosEventChannelAdmin/EventChannel:1.0', providesInterface)
        else:
            try:
                idl_interface = _idllib.getInterface(providesInterface)
            except idllib.IDLError:
                return False
            return usesInterface in idl_interface.inherits

    def _matchExact(self, interface, ports):
        return [p for p in ports if p['Port Interface'] == interface]

    def _matchProvidesPort(self, providesPort, connectionId):
        interface = providesPort['Port Interface']

        # First, look for exact matches.
        matches = self._matchExact(interface, self._usesPortDict.values())

        # Only if no exact matches are found, try to check the CORBA interfaces
        # for compatibility.
        if not matches:
            for usesPort in self._usesPortDict.values():
                if self._canConnect(usesPort['Port Interface'], interface):
                    matches.append(usesPort)

        # Convert the matched ports to endpoints.
        return [self._getEndpoint(usesPort, connectionId) for usesPort in matches]

    def connect(self, providesComponent, providesPortName=None, usesPortName=None, connectionId=None):
        """
        This function establishes a connection with a provides-side port. Python will attempt
        to find a matching port automatically. If there are multiple possible matches,
        a uses-side or provides-side port name may be necessary to resolve the ambiguity
        """
        if not connectionId:
            connectionId = str(_uuidgen())
            
        if isinstance(providesComponent, PortSupplier):
            log.trace('Provides side is PortSupplier')
            # Remote side supports multiple ports.
            if providesPortName and usesPortName:
                # Both ports given.
                providesPort = providesComponent._getProvidesPort(providesPortName)
                providesEndpoint = providesComponent._getEndpoint(providesPort, connectionId)
                usesPort = self._getUsesPort(usesPortName)
                usesEndpoint = PortEndpoint(self, usesPort)
            elif providesPortName:
                # Just provides port was given; find first matching uses port.
                providesPort = providesComponent._getProvidesPort(providesPortName)
                providesEndpoint = providesComponent._getEndpoint(providesPort, connectionId)
                try:
                    usesEndpoint = self._matchProvidesPort(providesPort, connectionId)[0]
                except IndexError:
                    raise RuntimeError, "No uses ports that match provides port '%s'" % providesPortName
            elif usesPortName:
                # Just uses port was given; find first matching provides port.
                usesPort = self._getUsesPort(usesPortName)
                usesEndpoint = PortEndpoint(self, usesPort)
                try:
                    providesEndpoint = providesComponent._matchUsesPort(usesPort, connectionId)[0]
                except IndexError:
                    raise RuntimeError, "No provides ports that match uses port '%s'" % usesPortName
            else:
                # No port names given, attempt to negotiate.
                matches = []
                uses = None
                for usesPort in self._usesPortDict.values():
                    uses = PortEndpoint(self, usesPort)
                    matches.extend((uses, provides) for provides in providesComponent._matchUsesPort(usesPort, connectionId))

                if len(matches) > 1:
                    ret_str = "Multiple ports matched interfaces on connect, must specify providesPortName or usesPortName\nPossible matches:\n"
                    for match in matches:
                        ret_str += "  Interface: "+match[0].getInterface()+", component/port:  "+match[0].getName()+"     "+match[1].getName()+"\n"
                    raise RuntimeError, ret_str
                elif len(matches) == 0:
                    if uses:
                        raise NoMatchingPorts('No matching interfaces between '+uses.supplier._instanceName+' and '+providesComponent._componentName)
                    else:
                        raise NoMatchingPorts('No matching interfaces found')
                usesEndpoint, providesEndpoint = matches[0]

        elif isinstance(providesComponent, OutputBase):
            log.trace('Provides side is OutputBase')
            if not usesPortName:
                usesPort = self._getDefaultUsesPort()
                usesPortName = usesPort['Port Name']
            else:
                usesPort = self._getUsesPort(usesPortName)
            usesEndpoint = PortEndpoint(self, usesPort)
            providesComponent.setup(usesEndpoint.getReference(), dataType=usesPort['Port Interface'],
                                    componentName=self._instanceName, usesPortName=usesPortName)
            
            # Underlying connection is handled by OutputBase object, so return early.
            # NB: OutputBase interface does not currently support connection management.
            return
        elif isinstance(providesComponent, CorbaObject):
            log.trace('Provides side is CORBA object')
            if usesPortName:
                # Use usesPortName if provided
                usesPorts = [self._getUsesPort(usesPortName)]
            else:
                usesPorts = self._usesPortDict.values()
            # Try to find a uses interface to connect to the object
            providesInterface = providesComponent._getInterface()
            usesEndpoint = None
            for usesPort in usesPorts:
                if self._canConnect(usesPort['Port Interface'], providesInterface):
                    usesEndpoint = PortEndpoint(self, usesPort)
                    providesEndpoint = ObjectEndpoint(providesComponent, providesInterface)
                    break
            if not usesEndpoint:
                raise NoMatchingPorts("No port matches provides object interface '"+providesInterface+"'")
        else:
            # No support for provides side, throw an exception so the user knows
            # that the connection failed.
            raise TypeError, "Type '%s' is not supported for provides side connection" % (providesComponent.__class__.__name__)
        
        # Make the actual connection from the endpoints
        log.trace("Uses endpoint '%s' has interface '%s'", usesEndpoint.getName(), usesEndpoint.getInterface())
        log.trace("Provides endpoint '%s' has interface '%s'", providesEndpoint.getName(), providesEndpoint.getInterface())
        usesPortRef = usesEndpoint.getReference()
        providesPortRef = providesEndpoint.getReference()

        usesPortRef.connectPort(providesPortRef, connectionId)
        ConnectionManager.instance().registerConnection(connectionId, usesEndpoint, providesEndpoint)

    def _disconnected(self, connectionId):
        pass

    def disconnect(self, providesComponent):
        manager = ConnectionManager.instance()
        for connectionId, (uses, provides) in manager.getConnectionsBetween(self, providesComponent).items():
            usesPortRef = uses.getReference()
            try:
                usesPortRef.disconnectPort(connectionId)
            except:
                pass
            if isinstance(providesComponent, PortSupplier):
                providesComponent._disconnected(connectionId)
            manager.unregisterConnection(connectionId)


class PropertySet(object):
    __log = log.getChild('PropertySet')

    def __init__(self):
        self._properties = []
      
    @property
    def _propertySet(self):
        #DEPRICATED: replaced with _properties, _propertySet did not contain all kinds and actions
        _warnings.warn("'_propertySet' is deprecated", DeprecationWarning)
        try:
            return [p for p in self._properties if 'execparam' in p.kinds or 'configure' in p.kinds]
        except:
            return None
    
    def _findProperty(self, name):
        for prop in self._properties:
            if name in (prop.id, prop.clean_name):
                return prop
        raise KeyError, "Unknown property '%s'" % name
    
    def _itemToDataType(self, name, value):
        prop = self._findProperty(name)
        return _CF.DataType(prop.id, prop.toAny(value))

    def configure(self, props):
        self.__log.trace("configure('%s')", str(props))
        if not self.ref:
            pass
        try:
            # Turn a dictionary of Python values into a list of CF Properties
            props = [self._itemToDataType(k,v) for k,v in props.iteritems()]
        except AttributeError:
            # Assume the exception occurred because props is not a dictionary
            pass
        self.ref.configure(props)
            
    def query(self, props):
        if True:
            self.__log.trace("query('%s')", str(props))
            if self.ref:
                return self.ref.query(props)
            else:
                return None
   
    def api(self, externalPropInfo=None):
        properties = [p for p in self._properties if 'configure' in p.kinds or 'execparam' in p.kinds]
        if not properties:
            return

        table = TablePrinter('Property Name', '(Data Type)', '[Default Value]', 'Current Value')

        # Limit the amount of space between columns of data for display
        table.limit_column(1, 32)
        table.limit_column(2, 24)

        if externalPropInfo:
            # For external props, extract their information
            extId, propId = externalPropInfo
            table.enable_header(False)
        else:
            print "Properties =============="
        for prop in properties:
            if externalPropInfo:
                # Searching for a particular external property
                if prop.clean_name != propId:
                    continue
                name = extId
            else:
                name = prop.clean_name
            writeOnly = False
            if prop.mode != "writeonly":
                currentValue = prop.queryValue()
            else:
                writeOnly = True
                currentValue = "N/A"

            scaType = _formatType(prop.type)
            if prop.type == 'structSeq':
                table.append(name, '('+scaType+')')
                if writeOnly:
                    currentValue = "N"
                for item_count, item in enumerate(currentValue):
                    for member in prop.valueType:
                        _id = str(member[0])
                        scaType = str(member[1])
                        defVal = str(member[2])
                        if not writeOnly:
                            currVal = str(item[member[0]])
                        else:
                            currVal = "N/A"
                        name = ' [%d] %s' % (item_count, _id)
                        scaType = _formatType(scaType)
                        table.append(name, '('+scaType+')', defVal, currVal)
            elif prop.type == 'struct':
                table.append(name, '('+scaType+')')
                for member in prop.members.itervalues():
                    name = ' ' + member.clean_name
                    scaType = _formatType(member.type)
                    if not writeOnly:
                        _currentValue = _formatSimple(member, currentValue[member.id],member.id)
                    else: 
                        _currentValue = "N/A"
                    table.append(' '+name, '('+scaType+')', str(member.defValue), _currentValue)
            else:
                currentValue = _formatSimple(prop, currentValue,prop.id)
                table.append(name, '('+scaType+')', str(prop.defValue), currentValue)

        table.write()


class Service(CorbaObject):
    def __init__(self, ref, profile, spd, scd, prf, instanceName, refid, impl):
        CorbaObject.__init__(self, ref)

        self._profile = profile
        self._refid = refid
        self._spd = spd
        self._scd = scd
        self._prf = prf
        self._impl = impl
        self._instanceName = instanceName
        
         # Add mapping of services operations and attributes
        found = False
        self._repid = self._scd.get_interfaces().interface[0].repid
        try:
            interface = _idllib.getInterface(self._repid)
        except idllib.IDLError:
            log.error("Can't find IDL repo: " + str(self._repid) + " for service: " + str(self._instanceName))
        else:
            self._operations = interface.operations
            self._attributes = interface.attributes
            self._namespace = interface.nameSpace
            self._interface = interface.name
            
        self.populateMemberFunctions()
    
    def _matchInterface(self, repid):
        return repid == self._repid

    def _getInterface(self):
        return self._repid

    def populateMemberFunctions(self):
        objMembers = inspect.getmembers(self.ref)
        # Add member function mapping for each service operation 
        for function in self._operations:
            for name, func  in objMembers:
                if name == function.name:
                    self.__setattr__(name, func)
        
        # Added member function mapping for each service attribute
        for attr in self._attributes:
            for name, func in objMembers:
                # Python attributes start with either _get_ or _set_, so ignore first 5 chars
                if name[5:] == attr.name:
                    self.__setattr__(name, func)


    def log_level(self, newLogLevel=None ):
        if newLogLevel == None:
            return self.ref._get_log_level( newLogLevel )
        else:
            self.ref._set_log_level( newLogLevel )

    def setLogLevel(self, logid, newLogLevel ):
        self.ref.setLogLevel( logid, newLogLevel )

    def getLogConfig(self):
        return self.ref.getLogConfig()

    def setLogConfig(self, new_log_config):
        self.ref.setLogConfig( new_log_config )

    def setLogConfigURL(self, log_config_url):
        self.ref.setLogConfigURL( log_config_url )


class Resource(CorbaObject, PortSupplier, PropertySet):
    __log = log.getChild('Resource')

    def __init__(self, ref=None):
        CorbaObject.__init__(self, ref)
        PortSupplier.__init__(self)
        PropertySet.__init__(self)

    ########################################
    # External Resource API
    def _get_identifier(self):
        if self.ref:
            return self.ref._get_identifier()
        else:
            return None
    
    def _get_started(self):
        if self.ref:
            return self.ref._get_started()
        else:
            return None
    
    def start(self):
        self.__log.trace("start()")
        if self.ref:
            self.ref.start()
            
    def stop(self):
        self.__log.trace("stop()")
        if self.ref:
            self.ref.stop()

    def _get_softwareProfile(self):
        if self.ref:
            return self.ref._get_softwareProfile()
        else:
            return None
            
    def initialize(self):
        self.__log.trace("initialize()")
        if self.ref:
            self.ref.initialize()
            
    def getPort(self, name):
        self.__log.trace("getPort('%s')", name)
        if self.ref:
            return self.ref.getPort(name)
        else:
            return None

    def runTest(self, testid, props):
        if self.ref:
            return self.ref.runTest(testid, props)
        else:
            return None
    
    def releaseObject(self):
        self.__log.trace('releaseObject()')
        if self.ref:
            self.ref.releaseObject()
            # Reset component object reference
            self.ref = None

    def log_level(self, newLogLevel=None ):
        if newLogLevel == None:
            return self.ref._get_log_level()
        else:
            self.ref._set_log_level( newLogLevel )

    def setLogLevel(self, logid, newLogLevel ):
        self.ref.setLogLevel( logid, newLogLevel )

    def getLogConfig(self):
        return self.ref.getLogConfig()

    def setLogConfig(self, new_log_config):
        self.ref.setLogConfig( new_log_config )

    def setLogConfigURL(self, log_config_url):
        self.ref.setLogConfigURL( log_config_url )
    

class Device(Resource):
    def _buildAPI(self):
        self._allocProps = self._getPropertySet(kinds=('allocation',),action=('external','eq','ge','gt','le','lt','ne') )

    def _getAllocProp(self, name):
        for prop in self._allocProps:
            if name in (prop.id, prop.clean_name):
                return prop
        raise KeyError, "No allocation property '%s'" % name

    def _capacitiesToAny(self, props):
        if isinstance(props, dict):
            allocProps = []
            for name, value in props.iteritems():
                prop = self._getAllocProp(name)
                allocProps.append(_CF.DataType(prop.id, prop.toAny(value)))
            return allocProps
        else:
            return props

    def _get_adminState(self):
        if self.ref:
            return self.ref._get_adminState()
        else:
            return None

    def _set_adminState(self, state):
        if self.ref:
            self.ref._set_adminState(state)

    def _get_usageState(self):
        if self.ref:
            return self.ref._get_usageState()
        else:
            return None

    def _get_operationalState(self):
        retval = None
        if self.ref:
            return self.ref._get_operationalState()
        else:
            return None

    def _get_label(self):
        if self.ref:
            return self.ref._get_label()
        else:
            return None

    def _get_compositeDevice(self):
        if self.ref:
            return self.ref._get_compositeDevice()
        else:
            return None

    def allocateCapacity(self, props):
        if not self.ref:
            return None
        allocProps = self._capacitiesToAny(props)
        return self.ref.allocateCapacity(allocProps)

    def deallocateCapacity(self, props):
        if not self.ref:
            return
        allocProps = self._capacitiesToAny(props)
        self.ref.deallocateCapacity(allocProps)

    def api(self):
        print 'Allocation Properties ======'
        if not self._allocProps:
            print 'None'
            return

        table = TablePrinter('Property Name', '(Data Type)', 'Action')
        for prop in self._allocProps:
            table.append(prop.clean_name, '('+prop.type+')', prop.action)
            if prop.type in ('struct', 'structSeq'):
                if prop.type == 'structSeq':
                    structdef = prop.structDef
                else:
                    structdef = prop
                for member in structdef.members.itervalues():
                    table.append('  '+member.clean_name, member.type)
        table.write()
            

class LoadableDevice(Device):
    def load(self, fs, fileName, loadKind):
        if self.ref:
            self.ref.load(fs, fileName, loadKind)

    def unload(self, fileName):
        if self.ref:
            self.ref.unload(fileName)


class ExecutableDevice(LoadableDevice):
    def execute(self, name, options, parameters):
        if self.ref:
            return self.ref.execute(name, options, parameters)
        else:
            return None

    def terminate(self, processID):
        if self.ref:
            self.ref.terminate(processID)


class AggregateDevice(object):
    def _get_devices(self):
        if self.ref:
            return self.ref._get_devices()
        else:
            return None

    def addDevice(self, associatedDevice):
        if self.ref:
            self.ref.addDevice(associatedDevice)

    def removeDevice(self, associatedDevice):
        if self.ref:
            self.ref.removeDevice(associatedDevice)


class ComponentBase(object):
    def __init__(self, spd, scd, prf, instanceName, refid, impl, pid=0, devs=[]):
        self._spd = spd
        self._scd = scd
        self._prf = prf
        self._instanceName = instanceName
        self._refid = refid
        self._impl = impl
        self._configureTable = {}
        self._pid = pid
        self._id = None
        self._devs = devs
        self._properties = self._getPropertySet(kinds=("configure","execparam","allocation","event","message"),
                                          modes=("readwrite","readonly","writeonly"),
                                          action=("external","eq","ge","gt","le","lt","ne"),
                                          includeNil=True)

    def __setattr__(self,name,value):
        # If setting any class attribute except for _properties,
        # Then need to see if name matches any component properties
        # If so, then call configure on the component for the particular property and value
        if name != "_properties":
            try:
                if hasattr(self,"_properties"):
                    propSet = object.__getattribute__(self,"_properties")
                    if propSet != None:
                        for prop in propSet:
                            if name == prop.clean_name:
                                if _DEBUG == True:
                                    print "Component:__setattr__() Setting component property attribute " + str(name) + " to value " + str(value)
                                self._configureSingleProp(prop.id,value)
                                break
                            if name == prop.id:
                                if _DEBUG == True:
                                    print "Component:__setattr__() Setting component property attribute " + str(name) + " to value " + str(value)
                                self._configureSingleProp(name,value)
                                break
            except AttributeError, e:
                # This would be thrown if _propertyies attribute hasn't been set yet
                # This will occur only with setting of class attibutes before _properties has been set
                # This will not affect setting attributes based on component properties since this will
                #   occur after all class attributes have been set
                if str(e).find("_properties") != -1:
                    pass
                else:
                    raise e
        return object.__setattr__(self,name,value)
    
    def __getattribute__(self,name):
        # Called for every access (in alternative to __getattr__)
        if name != "_properties" and hasattr(self,"_properties"):
           propSet = object.__getattribute__(self,"_properties")
           if propSet != None:
               for prop in propSet: 
                   if name == prop.id or name == prop.clean_name:
                       if _DEBUG == True:
                           print 'Component:__getattribute__()', prop
                       return prop
        if name == '_id':
            if object.__getattribute__(self,"_id") == None:
                ref = object.__getattribute__(self,"ref")
                object.__setattr__(self,"_id",ref._get_identifier())
        return object.__getattribute__(self,name)

    def _getPropType(self, prop):
        '''
        Returns the property type.

        If the complex flag is set to true, the type is converted 
        from the primitive type (e.g., long) to the complex type 
        (e.g., complexLong) to support downstream processing.

        '''

        propType = prop.get_type()

        complex = prop.get_complex()
        if complex.lower() == "true":
            propType = _properties.mapComplexType(propType)
        return propType
     
    def _getPropertySet(self, \
                        kinds=("configure","execparam"), \
                        modes=("readwrite", "writeonly", "readonly"), \
                        action=("external"), \
                        includeNil=True):
        """
        A useful utility function that extracts specified property types from
        the PRF file and turns them into a _CF.PropertySet
        """
      
        if _DEBUG == True:
            print "Component: _getPropertySet() kinds " + str(kinds)
            print "Component: _getPropertySet() modes " + str(modes)
            print "Component: _getPropertySet() action " + str(action)
        if not self._prf:
            return []

        propertySet = []
        # Simples
        for prop in self._prf.get_simple():
            if _prop_helpers.isMatch(prop, modes, kinds, action):
                if prop.get_value() == None and includeNil == False:
                    continue
                propType = self._getPropType(prop)
                try:
                    enum = prop.get_enumerations().get_enumeration()
                except:
                    enum = None
                val = prop.get_value()
                if str(val) == str(None):
                    defValue = None
                else:
                    defValue = _convertType(propType, val)
                if prop.get_action():
                    act = prop.get_action().get_type()
                else:
                    act = 'external'

                kindList = []
                for k in prop.get_kind():
                    kindList.append(k.get_kindtype())
                p = _prop_helpers.simpleProperty(id=prop.get_id(), valueType=propType, enum=enum, compRef=weakref.proxy(self), kinds=kindList,defValue=defValue, mode=prop.get_mode(), action=act)
                id_clean = _prop_helpers._cleanId(prop)
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._refid)
                propertySet.append(p)
        # Simple Sequences
        for prop in self._prf.get_simplesequence():
            if _prop_helpers.isMatch(prop, modes, kinds, action):
                values = prop.get_values()
                propType = self._getPropType(prop)
                if values:
                    defValue = [_convertType(propType, val) for val in values.get_value()]
                else:
                    defValue = None
                kindList = []
                for k in prop.get_kind():
                    kindList.append(k.get_kindtype())
                
                p = _prop_helpers.sequenceProperty(id        = prop.get_id(), 
                                                   valueType = propType, 
                                                   kinds     = kindList,
                                                   compRef   = weakref.proxy(self),
                                                   defValue  = defValue, 
                                                   mode      = prop.get_mode())
                id_clean = _prop_helpers._cleanId(prop)
                p.clean_name = _prop_helpers.addCleanName(id_clean, 
                                                          prop.get_id(), 
                                                          self._refid)
                propertySet.append(p)

        # Structures
        for prop in self._prf.get_struct():
            if _prop_helpers.isMatch(prop, modes, kinds, action):
                members = []
                structDefValue = {}
                if prop.get_simple() != None:
                    for simple in prop.get_simple():
                        propType = self._getPropType(simple)
                        val = simple.get_value()
                        if str(val) == str(None):
                            defValue = None
                        else:
                            defValue = _convertType(propType, val)
                        id_clean = _prop_helpers._cleanId(simple)
                        # Add individual property
                        id_clean = _prop_helpers.addCleanName(id_clean, simple.get_id(), self._refid)
                        members.append((simple.get_id(), propType, defValue, id_clean))
                        structDefValue[simple.get_id()] = defValue
                
                hasDefault = False
                for defValue in structDefValue.values():
                    if defValue is not None:
                        hasDefault = True
                        break
                if not hasDefault:
                    structDefValue = None
                kindList = []
                for k in prop.get_configurationkind():
                    kindList.append(k.get_kindtype())
                p = _prop_helpers.structProperty(id=prop.get_id(),
                                                 valueType=members,
                                                 kinds=kindList,
                                                 props=prop.get_simple(),
                                                 compRef=weakref.proxy(self),
                                                 defValue=structDefValue,
                                                 mode=prop.get_mode())
                id_clean = _prop_helpers._cleanId(prop)
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._refid)
                propertySet.append(p)

        # Struct Sequence
        for prop in self._prf.get_structsequence():
            if _prop_helpers.isMatch(prop, modes, kinds, action):
                if prop.get_struct() != None:
                    members = []
                    #get the struct definition
                    for simple in prop.get_struct().get_simple():
                        propType = self._getPropType(simple)
                        val = simple.get_value()
                        if str(val) == str(None):
                            simpleDefValue = None
                        else:
                            simpleDefValue = _convertType(propType, val)
                        id_clean = _prop_helpers._cleanId(simple)
                        # Adds struct member
                        members.append((simple.get_id(), propType, simpleDefValue, id_clean))
                        _prop_helpers.addCleanName(id_clean, simple.get_id(), self._refid)
                    
                    structSeqDefValue = None
                    structValues = prop.get_structvalue()
                    if len(structValues) != 0:
                        structSeqDefValue = []
                        for structValue in structValues:
                            simpleRefs = structValue.get_simpleref()
                            newValue = {}
                            for simpleRef in simpleRefs:
                                value = simpleRef.get_value()
                                id = simpleRef.get_refid()
                                if str(value) == str(None):
                                    _value = None
                                else:
                                    _propType = None
                                    for _id, pt, dv, cv in members:
                                        if _id == str(id):
                                            _propType = pt
                                    _value = _convertType(_propType, value)
                                newValue[str(id)] = _value
                            structSeqDefValue.append(newValue)
                kindList = []
                for k in prop.get_configurationkind():
                    kindList.append(k.get_kindtype())
                p = _prop_helpers.structSequenceProperty(id=prop.get_id(), structID=prop.get_struct().get_id(), valueType=members, kinds=kindList, props=prop.get_struct().get_simple(), compRef=weakref.proxy(self), defValue=structSeqDefValue, mode=prop.get_mode())
                id_clean = _prop_helpers._cleanId(prop)
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._refid)
                propertySet.append(p)

        if _DEBUG == True:
            try:
                print "Component: _getPropertySet() propertySet " + str(propertySet)
            except:
                pass
        return propertySet

    def _parseComponentXMLFiles(self):
        # create a map between prop ids and names
        if self._prf != None:
            self._props = _prop_helpers.getPropNameDict(self._prf)

    #####################################
    
    def eos(self):
        '''
        Returns the value of the most recently received end-of-stream flag over a bulkio port
        '''
        return None
    
    def _query(self, props=[], printResults=False):
        results = self.query(props)
        # If querying all properties, display all property names and values
        propDict = _properties.props_to_dict(results)
        maxNameLen = 0
        if printResults:
            if results != [] and len(props) == 0: 
                for prop in propDict.items():
                    if len(prop[0]) > maxNameLen:
                        maxNameLen = len(prop[0])
                print "_query():"
                print "Property Name" + " "*(maxNameLen-len("Property Name")) + "\tProperty Value"
                print "-------------" + " "*(maxNameLen-len("Property Name")) + "\t--------------"
                for prop in propDict.items():
                    print str(prop[0]) + " "*(maxNameLen-len(str(prop[0]))) + "\t    " + str(prop[1])
        return propDict
    
    # helper function for property changes
    def _configureSingleProp(self, propName, propValue):
        if _DEBUG == True:
            print "Component:_configureSingleProp() propName " + str(propName)
            print "Component:_configureSingleProp() propValue " + str(propValue)

        if not propName in self._configureTable:
            raise AssertionError,'Component:_configureSingleProp() ERROR - Property not found in _configureSingleProp'
        prop = self._configureTable[propName]
        # Will generate a configure call on the component
        prop.configureValue(propValue)
        
    def _buildAPI(self):
        if _DEBUG == True:
            print "Component:_buildAPI()"

        for port in self._scd.get_componentfeatures().get_ports().get_provides():
            name = port.get_providesname()
            if not name:
                continue
            self._providesPortDict[name] = {}
            self._providesPortDict[name]['Port Name'] = str(name)
            self._providesPortDict[name]['Port Interface'] = str(port.get_repid())

        for port in self._scd.get_componentfeatures().get_ports().get_uses():
            name = port.get_usesname()
            if not name:
                continue
            self._usesPortDict[name] = {}
            self._usesPortDict[name]["Port Name"] = str(name)
            self._usesPortDict[name]["Port Interface"] = str(port.get_repid())

        for prop in self._properties:
            if 'configure' in prop.kinds:
                self._configureTable[prop.id] = prop

        if _DEBUG == True:
            try:
                self.api()
            except:
                pass
            
    def _populatePorts(self):
        """Add all port descriptions to the component instance"""
        interface_modules = ['BULKIO', 'BULKIO__POA']
        
        scdPorts = self._scd.componentfeatures.ports
        ports = []

        for uses in scdPorts.uses:
            idl_repid = str(uses.repid)
            try:
                int_entry = _idllib.getInterface(idl_repid)
            except idllib.IDLError:
                log.error("Invalid port descriptor in scd for %s", idl_repid)
                continue

            new_port = _Port(str(uses.usesname), interface=None, direction="Uses", using=int_entry)
            try:
                new_port.generic_ref = self.getPort(str(new_port._name))
                new_port.ref = new_port.generic_ref._narrow(_ExtendedCF.QueryablePort)
                if new_port.ref == None:
                    new_port.ref = new_port.generic_ref._narrow(_CF.Port)
                new_port.extendPort()
    
                idl_repid = new_port.ref._NP_RepositoryId
                try:
                    int_entry = _idllib.getInterface(idl_repid)
                except idllib.IDLError:
                    log.error("Unable to find port description for %s", idl_repid)
                    continue
                new_port._interface = int_entry

                ports.append(new_port)
            except:
                log.error("getPort failed for port name '%s'", new_port._name)
        for provides in scdPorts.provides:
            idl_repid = str(provides.repid)
            try:
                int_entry = _idllib.getInterface(idl_repid)
            except idllib.IDLError:
                log.error("Invalid port descriptor in scd for %s", idl_repid)
                continue
            new_port = _Port(str(provides.providesname), interface=int_entry, direction="Provides")
            try:
                new_port.generic_ref = object.__getattribute__(self,'ref').getPort(str(new_port._name))
                
                # See if interface python module has been loaded, if not then try to import it
                if str(int_entry.nameSpace) not in interface_modules:
                    success = False
                try:
                    pkg_name = (int_entry.nameSpace.lower())+'.'+(int_entry.nameSpace.lower())+'Interfaces'
                    _to = str(int_entry.nameSpace)
                    mod = __import__(pkg_name,globals(),locals(),[_to])
                    globals()[_to] = mod.__dict__[_to]
                    success = True
                except ImportError, msg:
                    pass
                if not success:
                    std_idl_path = _os.path.join(_os.environ['OSSIEHOME'], 'lib/python')
                    for dirpath, dirs, files in _os.walk(std_idl_path):
                        if len(dirs) == 0:
                            continue
                        for directory in dirs:
                            try:
                                _from = directory+'.'+(int_entry.nameSpace.lower())+'Interfaces'
                                _to = str(int_entry.nameSpace)
                                mod = __import__(_from,globals(),locals(),[_to])
                                globals()[_to] = mod.__dict__[_to]
                                success = True
                            except:
                                continue
                            break
                        if success:
                            break
                if not success:
                    continue
        
                interface_modules.append(str(int_entry.nameSpace))
                
                exec_string = 'new_port.ref = new_port.generic_ref._narrow('+int_entry.nameSpace+'.'+int_entry.name+')'
        
                try:
                    exec(exec_string)
                    ports.append(new_port)
                except:
                    continue
            except:
                log.error("getPort failed for port name '%s'", new_port._name)

        return ports

    def _matchUsesPort(self, usesPort, connectionId):
        # Assume the connection is directly to this component.
        if usesPort['Port Interface'] == 'IDL:CF/Resource:1.0':
            return [ObjectEndpoint(self, 'IDL:CF/Resource:1.0')]
        return super(ComponentBase,self)._matchUsesPort(usesPort, connectionId)





class _Port(object):
    """The Port is the gateway into and out of a particular component. A Port has a string name that is unique
        to that port within the context of a particular component.
        
        There are two types of Ports: Uses (output) and Provides (input).
    """
    def __init__(self, name, interface=None, direction="Uses",portType="data",using=None):
        self.extendedFunctionList = []
        self._name = name
        self._interface = interface
        self._using=using
        self._portType = portType    #control or data
        self._direction = direction  #Uses or Provides
        self.generic_ref = None
        self.ref = None
    
    def __getattribute__(self, name):
        try:
            if name in object.__getattribute__(self,'extendedFunctionList'):
                # the intercepted call to a port's supported interface has to be dynamically mapped.
                #  the use of exec is a bit awkward, but retrieves the function pointer
                #  when the base class does not implement __getattr__, __getattribute__, or __call__,
                #  which apparently can happen in the CORBA mapping to Python
                try:
                    retreiveFunc = "functionref = object.__getattribute__(self,'ref')."+name
                    exec(retreiveFunc)
                except:
                    raise AttributeError
                return functionref
            else:
                return object.__getattribute__(self,name)
        except AttributeError:
            raise
    
    @property
    def name(self):
        return self._name
    
    def extendPort(self):
        if self.ref == None:
            return
        # the assumption made here is that the CORBA pointer won't have any functions whose implementation
        #  starts with __; all those would be Python basic services. The use of this list is such that
        #  if the object that ref points to implements any function call that the Port class implements,
        #  then the CORBA pointer one gets called insted of the Port one
        functions = dir(self.ref)
        for function in functions:
            if function[:2] == '__':
                continue
            self.extendedFunctionList.append(function)
