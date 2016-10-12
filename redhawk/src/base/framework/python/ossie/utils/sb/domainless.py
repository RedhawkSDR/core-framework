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

"""
Python command line environment where Redhawk components can be launched, connected, and operated without a Domain Manager or a Device Manager

The Component class is used to manage a component
  - The constructor can take an SPD XML file or a component name found in the defined SDRROOT.
  - SCD (XML) describes the interfaces for the instantiated component.
  - Properties are attributes of the instantiated object.
  - The destructor cleans up the launched component process.
  - connect() is called on the component with the uses port and the component with the provides port is the argument
    - Ambiguities can be resolved with usesPortName and/or providesPortName.

Helpers are provided to move data into and out of component objects
  - DataSource and DataSink are used to push vectors from Python to components and back.
  - FileSource and FileSink are used to push data from a file into components and back.
  - The Plot class provides a way to display data from a particulare port.
    * NOTE:Plot class requires the following:
      - Eclipse Redhawk IDE must be installed
      - Environment variable RH_IDE must be set defining the path to the main eclipse directory (/data/eclipse for example)

Examples of use:

    # List available components in SDRROOT
    catalog()

    # Show current list of components running and component connections
    show()

    # Launch component from component name
    a = Component(\"TestComponent\")

    # Launch component passing in path to SPD XML file of component
    b = Component(\"/var/redhawk/sdr/dom/components/TestComponent/TestComponent.spd.xml\")

    # Inspect interfaces and property for a given component
    a.api()

    # Provide file input to a component and plot output from a given output port
    a = FileSource(\"<input filename here>\",\"<data type here>\")
    b = Component(\"SomeComponent\")
    c = Plot()
    a.connect(b)
    b.connect(c,usesPortName=\"Some Port\") # usesPortName must be specified for Plot(), connect() launches plot display window
    start() # calls start on all deployed components and helpers
    stop() # calls stop on all deployed components and helpers

    # Continuously pushing an array of data to a plot
    # a.stop() stops the loop
    a = DataSource()   # default data type is short
    b = DataSink()
    a.connect(b,usesPortName=\"shortOut\")
    data = range(1000)
    a.push(data,loop=True)
    retval = b.getData(100)            # block until 100 elements are received, then return the received elements
    retval = b.getData(eos_block=True) # block until EOS is received, then return all available data
"""
# the _<pkg> is there to keep someone importing the sandbox from seeing the package
import atexit as _atexit
import commands as _commands
import copy as _copy
import os as _os
import fnmatch as _fnmatch
import signal as _signal
import ossie.utils.popen as _popen
import sys as _sys
import time as _time
import ossie.parsers.spd as _SPDParser
import ossie.parsers.prf as _PRFParser
import ossie.parsers.scd as _SCDParser
import ossie.parsers.sad as _SADParser
from ossie import properties as _properties
from ossie.properties import __TYPE_MAP
from ossie.utils import prop_helpers as _prop_helpers
from ossie.utils import type_helpers as _type_helpers
from ossie.cf import CF as _CF
from ossie.cf import CF__POA as _CF__POA
from ossie.cf import ExtendedCF as _ExtendedCF
from omniORB import any as _any
from omniORB import CORBA as _CORBA
import CosNaming as _CosNaming
import CosNaming__POA as _CosNaming__POA# This must be imported after importing omniORB
import io_helpers as _io_helpers
import xml.dom.minidom as _minidom
from ossie.utils.sca import importIDL as _importIDL
import string as _string
import struct as _struct

_std_idl_path="/usr/local/share/idl/ossie"
_std_idl_include_path="/usr/local/share/idl"

_interface_list = []
_loadedInterfaceList = False

__MIDAS_TYPE_MAP = {'char'  : ('/SB/8t'),
                    'octet' : ('/SO/8o'),
                    'short' : ('/SI/16t'),
                    'long'  : ('/SL/32t'),
                    'float' : ('/SF/32f'),
                    'double': ('/SD/64f')}

_STOP_SIGNALS = ((_signal.SIGINT, 1),
                 (_signal.SIGTERM, 5),
                 (_signal.SIGKILL, None))

_STOP_SIGNALS_DEBUGGER = ((_signal.SIGINT, 0.5),
                 (_signal.SIGTERM, 0.5),
                 (_signal.SIGKILL, None))

_DEBUG = False

connectedIDE = False
IDE_REF = None
def setIDE_REF(ref):
    global IDE_REF
    global connectedIDE
    IDE_REF = ref
    connectedIDE = True
    _populateFromExternalNC()

def getIDE_REF():
    return IDE_REF
    
def _populateFromExternalNC():
    global _currentState
    _currentState['Components Running'] = {}
    _currentState['Component Connections']= {}
    if IDE_REF != None:
        resources = IDE_REF._get_registeredResources()
        # Instantiate local Component instances of Component running in IDE sandbox
        for resc in resources:
            comp = Component(componentDescriptor=resc.profile,instanceName=resc.resource._get_identifier(),refid=resc.resource._get_identifier(),autoKick=False,componentObj=resc.resource)
            _currentState['Components Running'][comp._instanceName] = comp 

        # Determine current connection state of components running
        for component in _currentState['Components Running'].values():
            for port in component._ports:
                if port._direction == "Uses" and hasattr(port,"_get_connections"):
                    for connection in port._get_connections():
                        connectionId = connection.connectionId
                        connectionProvidesPortObj = connection.port 
                        _currentState['Component Connections'][connectionId] = {}
                        _currentState['Component Connections'][connectionId]['Uses Component'] = component
                        _currentState['Component Connections'][connectionId]['Uses Port Name'] = port._name
                        for entry in component._usesPortDict.values():
                            if entry['Port Name'] == port._name:
                                _currentState['Component Connections'][connectionId]['Uses Port Interface'] = entry['Port Interface'] 
                                break
                        # Loop over all components to find a matching port
                        for providesComponent in _currentState['Components Running'].values():
                            if not component.ref._is_equivalent(providesComponent.ref):
                                for providesPort in providesComponent._ports:
                                    if providesPort._direction == "Provides":
                                        if connectionProvidesPortObj._is_equivalent(providesPort.ref):
                                            _currentState['Component Connections'][connectionId]['Provides Component'] = providesComponent
                                            _currentState['Component Connections'][connectionId]['Provides Port Name'] = providesPort._name
                                            for entry in providesComponent._providesPortDict.values():
                                                if entry['Port Name'] == providesPort._name:
                                                    _currentState['Component Connections'][connectionId]['Provides Port Interface'] = entry['Port Interface'] 
                                                    break

# Initialize globals
_currentState = {}
_currentState['Components Running'] = {}
_currentState['Component Connections']= {}
_currentState['SDRROOT'] = None

# Prepare the ORB
orb = _CORBA.ORB_init()
_obj_poa = orb.resolve_initial_references("RootPOA")
_poaManager = _obj_poa._get_the_POAManager()
_poaManager.activate()

def setDEBUG(debug=False):
    global _DEBUG
    _DEBUG = debug 

def getDEBUG():
    return _DEBUG

def _cleanUpLaunchedComponents():
    if connectedIDE == False:
        # Remove stored Component references
        for connection in _currentState['Component Connections'].keys():
            usesComp = _currentState['Component Connections'][connection]['Uses Component']
            portName = _currentState['Component Connections'][connection]['Uses Port Name']
            try:
                port = usesComp.getPort(portName)
                port.disconnectPort(connection)
            except:
                pass
            _currentState['Component Connections'][connection]['Uses Component'] = None
            _currentState['Component Connections'][connection]['Provides Component'] = None
        for component in _currentState['Components Running'].keys():
            try:
                _currentState['Components Running'][component].stop()
            except Exception,e:
                continue
        for component in _currentState['Components Running'].keys():
            try:
                _currentState['Components Running'][component].releaseObject()
            except:
                if _DEBUG:
                    print component,"raised an exception while exiting"
            _currentState['Components Running'][component] = None

_atexit.register(_cleanUpLaunchedComponents)

def reset():
    '''
    Set all components to their initial running condition
    '''
    connectionList = [] 
    connectionEntry = { \
        'USES_REFID': None, \
        'USES_PORT_NAME': None, \
        'PROVIDES_REFID': None, \
        'PROVIDES_PORT_NAME': None \
    }
    for connection in _currentState['Component Connections'].keys():
        entry = connectionEntry.copy()
        usesRefid = _currentState['Component Connections'][connection]['Uses Component']._refid
        entry['USES_REFID'] = usesRefid
        usesPortName = _currentState['Component Connections'][connection]['Uses Port Name']
        entry['USES_PORT_NAME'] = usesPortName
        providesRefid = _currentState['Component Connections'][connection]['Provides Component']._refid
        entry['PROVIDES_REFID'] = providesRefid
        providesPortName = _currentState['Component Connections'][connection]['Provides Port Name']
        entry['PROVIDES_PORT_NAME'] = providesPortName 
        connectionList.append(entry)

    # Bring down current component process and re-launch it
    for component in _currentState['Components Running'].keys():
        _currentState['Components Running'][component].reset()

    for connection in connectionList:
        usesComp = None
        usesName = None
        providesComp = None
        providesName = None
        for component in _currentState['Components Running'].keys():
            if connection['USES_REFID'] == _currentState['Components Running'][component]._refid:
                usesComp = _currentState['Components Running'][component]
                usesName = connection['USES_PORT_NAME']
            if connection['PROVIDES_REFID'] == _currentState['Components Running'][component]._refid:
                providesComp = _currentState['Components Running'][component]
                providesName = connection['PROVIDES_PORT_NAME']
        if usesComp != None and usesName != None and \
           providesComp != None and providesName != None:
            usesComp.connect(providesComp, providesPortName=providesName, usesPortName=usesName)

def IDELocation(location=None):
    if location == None:
        if _os.environ.has_key("RH_IDE"):
            if _DEBUG:
                print "IDELocation(): RH_IDE environment variable is set to " + str(_os.environ["RH_IDE"])
            return str(_os.environ["RH_IDE"])
        else:
            if _DEBUG:
                print "IDELocation(): WARNING - RH_IDE environment variable is not set so plotting will not work"
            return None
    else:
        if _os.path.isdir(location):
            _os.environ["RH_IDE"] = str(location)
            if _DEBUG:
                print "IDELocation(): setting RH_IDE environment variable " + str(location)
            return str(location)
        else:
            if _DEBUG:
                print "IDELocation(): ERROR - invalid location passed in for RH_IDE environment variable"
            return None

def start():
    if _DEBUG == True:
        print "start():" 
    for comp in _currentState['Components Running'].items():
        componentName = comp[0]
        component = comp[1]
        if component != None:
            if _DEBUG == True:
                print "start(): Calling start on component " + str(componentName) 
            component.start()

def stop():
    if _DEBUG == True:
        print "stop():" 
    for comp in _currentState['Components Running'].items():
        componentName = comp[0]
        component = comp[1]
        if component != None:
            if _DEBUG == True:
                print "stop(): Calling stop on component " + str(componentName) 
            component.stop()

def redirectSTDOUT(filename):
    if _DEBUG == True:
        print "redirectSTDOUT(): redirecting stdout/stderr to filename " + str(filename)
    if filename != None:
        dirname = _os.path.dirname(filename)
        if len(dirname) == 0 or \
           (len(dirname) > 0 and _os.path.isdir(dirname)):
            try:
                f = open(filename,'w')
                # Send stdout and stderr to provided filename
                _sys.stdout = f 
                _sys.stderr = f 
            except Exception, e:
                print "redirectSTDOUT(): ERROR - Unable to open file " + str(filename) + " for writing stdout and stderr " + str(e)
        
def show():
    '''
    Show current list of components running and component connections
    '''
    if connectedIDE == True:
        _populateFromExternalNC()
    print "Components Running:"
    print "------------------"
    for component in _currentState['Components Running'].keys():
        print component, _currentState['Components Running'][component]
    print "\n"
    print "Component Connections:"
    print "---------------------"
    for connection in _currentState['Component Connections'].values():
        print str(connection['Uses Component']._instanceName) + " -> " + str(connection['Provides Component']._instanceName)
    
    print "\n"
    print "SDRROOT (if set to None it defaults to environment variable $SDRROOT):"
    print "---------------------"
    print _currentState['SDRROOT']
    print "\n"

def getComponent(name):
    '''
    Retrieve a pointer to a running component instance
    '''
    if connectedIDE == True:
        _populateFromExternalNC()
    if _currentState['Components Running'].has_key(name):
        return _currentState['Components Running'][name]
    return None

def generateSADXML(waveform_name):
    '''
    generate a SAD XML string describing the current sandbox
    '''
    import ossie.utils.redhawk.sad_template as sad_template
    if _DEBUG == True:
        print "generateSadFileString(): generating SAD XML string for given waveform name " + str(waveform_name)
    Sad_template = sad_template.sad()
    initial_file = Sad_template.template
    with_id = initial_file.replace('@__UUID__@',_uuidgen())
    # Need waveform name
    with_name = with_id.replace('@__NAME__@',waveform_name)
    # Loop over components to define individual componentfile and componentplacement entries
    componentfiles = ''
    partitioning = '' 
    for component in _currentState['Components Running'].values():
        # Exclude local input file , output file , and output array components from the sad file
        if component._instanceName.find("__localFileSource") == -1 and \
           component._instanceName.find("__localFileSink") == -1 and \
           component._instanceName.find("__localDataSource") == -1 and \
           component._instanceName.find("__localDataSink") == -1:
            relativePathIndex = component._profile.find("/components")
            if relativePathIndex != -1:
                relativePathFilename = component._profile[relativePathIndex:]
            else:
                relativePathFilename = component._profile
            componentfiles += Sad_template.componentfile.replace('@__SPDFILE__@',relativePathFilename)
            componentfiles = componentfiles.replace('@__SPDFILEID__@', component._spdFileId)
            partitioning += Sad_template.componentplacement.replace('@__COMPONENTNAME__@',component._instanceName)
            partitioning = partitioning.replace('@__COMPONENTINSTANCE__@',component._refid)
            partitioning = partitioning.replace('@__SPDFILEID__@',component._spdFileId)
    with_componentfiles = with_name.replace('@__COMPONENTFILE__@',componentfiles)
    with_partitioning = with_componentfiles.replace('@__COMPONENTPLACEMENT__@',partitioning)
    # Set Assembly Controller to first component in list
    assemblycontroller = ''
    if len(_currentState['Components Running'].keys()) > 0:
        key = _currentState['Components Running'].keys()[0]
        component = _currentState['Components Running'][key]
        assemblycontroller = Sad_template.assemblycontroller.replace('@__COMPONENTINSTANCE__@',component._refid)
    with_ac = with_partitioning.replace('@__ASSEMBLYCONTROLLER__@', assemblycontroller)
    # Loop over connections
    connectinterface = ''
    for connection in _currentState['Component Connections'].values():
        usesport = Sad_template.usesport.replace('@__PORTNAME__@',connection['Uses Port Name'])
        usesport = usesport.replace('@__COMPONENTINSTANCE__@',connection['Uses Component']._refid)
        if connection['Provides Port Name'] == "CF:Resource":
            # component support interface
            providesport = Sad_template.componentsupportedinterface.replace('@__PORTINTERFACE__@',connection['Provides Port Interface'])
            providesport = providesport.replace('@__COMPONENTINSTANCE__@',connection['Provides Component']._refid)
        else:
            providesport = Sad_template.providesport.replace('@__PORTNAME__@',connection['Provides Port Name'])
            providesport = providesport.replace('@__COMPONENTINSTANCE__@',connection['Provides Component']._refid)
        connectinterface += Sad_template.connectinterface.replace('@__USESPORT__@',usesport)
        connectinterface = connectinterface.replace('@__PROVIDESPORT__@',providesport)
        connectinterface = connectinterface.replace('@__CONNECTID__@',_uuidgen())
    with_connections = with_ac.replace('@__CONNECTINTERFACE__@',connectinterface)
    # External ports are ignored
    with_connections = with_connections.replace('@__EXTERNALPORTS__@',"")
    sadString = with_connections
    if _DEBUG == True:
        print "generateSadFileString(): returning SAD XML string " + str(sadString)
    return sadString

class overloadContainer:
    def __init__(self, id, value=None, type=None):
        self.id = id
        self.value = value
        self.type = type

def overloadProperty(component, simples=None, simpleseq=None, struct=None, structseq=None):
    if len(component._propertySet) > 0:
        allProps = dict([(str(prop.id),prop) for prop in component._propertySet])
        for entry in component._propertySet:
            if entry.mode == "readonly":
                continue
            for overload in simples:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    if entry.valueType == 'string' or entry.valueType == 'char':
                        setattr(component, entry.clean_name, overload.value)
                    elif entry.valueType == 'boolean':
                        setattr(component, entry.clean_name, bool(overload.value))
                    elif entry.valueType == 'ulong' or entry.valueType == 'short' or entry.valueType == 'octet' or \
                         entry.valueType == 'ushort' or entry.valueType == 'long' or entry.valueType == 'longlong' or \
                         entry.valueType == 'ulonglong':
                        setattr(component, entry.clean_name, int(overload.value))
                    elif entry.valueType == 'float' or entry.valueType == 'double':
                        setattr(component, entry.clean_name, float(overload.value))
                    else:
                         print "the proposed overload (id="+entry.id+") is not of a supported type ("+entry.valueType+")"
            for overload in simpleseq:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    if entry.valueType == 'string' or entry.valueType == 'char':
                        setattr(component, entry.clean_name, overload.value)
                    elif entry.valueType == 'boolean':
                        setattr(component, entry.clean_name, [bool(s) for s in overload.value])
                    elif entry.valueType == 'ulong' or entry.valueType == 'short' or entry.valueType == 'octet' or \
                         entry.valueType == 'ushort' or entry.valueType == 'long' or entry.valueType == 'longlong' or \
                         entry.valueType == 'ulonglong':
                        stuff=[int(s) for s in overload.value]
                        setattr(component, entry.clean_name, stuff)
                    elif entry.valueType == 'float' or entry.valueType == 'double':
                        setattr(component, entry.clean_name, [float(s) for s in overload.value])
                    else:
                         print "the proposed overload (id="+entry.id+") is not of a supported type ("+entry.valueType+")"
            for overload in struct:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    structValue = {}
                    for simple in entry.valueType:
                        if overload.value.has_key(str(simple[0])):
                            if len(simple) == 3:
                                clean_name = str(simple[0])
                            else:
                                clean_name = str(simple[3])
                            if simple[1] == 'string' or simple[1] == 'char':
                                structValue[clean_name] = overload.value[clean_name]
                            elif simple[1] == 'boolean':
                                structValue[clean_name] = bool(overload.value[clean_name])
                            elif simple[1] == 'ulong' or simple[1] == 'short' or simple[1] == 'octet' or \
                                 simple[1] == 'ushort' or simple[1] == 'long' or simple[1] == 'longlong' or \
                                 simple[1] == 'ulonglong':
                                structValue[clean_name] = int(overload.value[clean_name])
                            elif simple[1] == 'float' or simple[1] == 'double':
                                structValue[clean_name] = float(overload.value[clean_name])
                            else:
                                print "the proposed overload (id="+entry.id+") is not of a supported type ("+entry.valueType+")"
                            
                    setattr(component, entry.clean_name, structValue)
            for overload in structseq:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    structSeqValue = []
                    for overloadedValue in overload.value:
                        structValue = {}
                        for simple in entry.valueType:
                            if overloadedValue.has_key(str(simple[0])):
                                if len(simple) == 3:
                                    clean_name = str(simple[0])
                                else:
                                    clean_name = str(simple[3])
                                if simple[1] == 'string' or simple[1] == 'char':
                                    structValue[clean_name] = overloadedValue[clean_name]
                                elif simple[1] == 'boolean':
                                    structValue[clean_name] = bool(overloadedValue[clean_name])
                                elif simple[1] == 'ulong' or simple[1] == 'short' or simple[1] == 'octet' or \
                                    simple[1] == 'ushort' or simple[1] == 'long' or simple[1] == 'longlong' or \
                                    simple[1] == 'ulonglong':
                                    structValue[clean_name] = int(overloadedValue[clean_name])
                                elif simple[1] == 'float' or simple[1] == 'double':
                                    structValue[clean_name] = float(overloadedValue[clean_name])
                                else:
                                    print "the proposed overload (id="+entry.id+") is not of a supported type ("+entry.valueType+")"
                        structSeqValue.append(structValue)
                    setattr(component, entry.clean_name, structSeqValue)
        for prop in allProps:
            dV = allProps[prop].defValue
            if dV == None:
                continue
            if allProps[prop].mode != "readonly":
                setattr(component, allProps[prop].clean_name, allProps[prop].defValue)

def loadSADFile(filename, props={}):
    '''
    Load the graph/configuration described in the SAD file onto the sandbox
    '''
    if _DEBUG == True:
        print "loadSADFile(): loading in SAD file " + str(filename)
    try:
        if type(props) != dict:
            print "loadSADFile(): props argument must be a dictionary. Ignoring overload"
            props = {}
        sadFile = open(filename,'r')
        sadFileString = sadFile.read()
        sad = _SADParser.parseString(sadFileString)
        if _DEBUG == True:
            print "loadSADFile(): waveform ID " + str(sad.get_id())
            print "loadSADFile(): waveform name " + str(sad.get_name())
        launchedComponents = []
        validRequestedComponents = {} 
        # Loop over each <componentfile> entry to determine SPD filenames and which components are kickable
        for component in sad.componentfiles.get_componentfile():
            if _DEBUG == True:
                print "loadSADFile(): COMPONENT FILE localfile " + str(component.get_localfile().get_name()) 
                print "loadSADFile(): COMPONENT FILE id " + str(component.get_id()) 
                print "loadSADFile(): COMPONENT FILE type " + str(component.get_type()) 
            spdFilename = _spdFileExists(component.get_localfile().get_name())
            if spdFilename != None:
                validRequestedComponents[component.get_id()] = spdFilename

        # Need to determine which component is the assembly controller
        assemblyControllerRefid = None
        if sad.assemblycontroller != None:
            assemblyControllerRefid = sad.assemblycontroller.get_componentinstantiationref().get_refid()
            if _DEBUG == True:
                print "loadSADFile(): ASSEMBLY CONTROLLER component instantiation ref " + str(sad.assemblycontroller.get_componentinstantiationref().get_refid())
        if assemblyControllerRefid == None:
            print "loadSADFile(): WARNING sad file did not specify an assembly controller"

        # Loop over each <componentplacement> entry to determine actual instance name for component
        # NOTE: <componentplacement> can also occur within <hostcollocation> elements if that exists
        #       so that needs to be checked also
        componentPlacements = sad.partitioning.get_componentplacement()
        for hostCollocation in sad.get_partitioning().get_hostcollocation():
            componentPlacements.extend(hostCollocation.get_componentplacement())

        configurable = {}
        for component in componentPlacements:
            if _DEBUG == True:
                print "loadSADFile(): COMPONENT PLACEMENT component spd file id " + str(component.componentfileref.refid)
                print "loadSADFile(): COMPONENT PLACEMENT component instantiation id " + str(component.get_componentinstantiation()[0].id_)
                print "loadSADFile(): COMPONENT PLACEMENT component name " + str(component.get_componentinstantiation()[0].get_usagename())
            # If component has a valid SPD file (isKickable), launch it
            refid = component.componentfileref.refid
            if validRequestedComponents.has_key(refid):
                instanceName = component.get_componentinstantiation()[0].get_usagename()
                componentName = _os.path.basename(validRequestedComponents[refid] ).split('.')[0]
                instanceID = component.get_componentinstantiation()[0].id_
                if _DEBUG == True:
                    print "loadSADFile(): launching component " + str(instanceName)
                properties=component.get_componentinstantiation()[0].get_componentproperties()
                #simples
                spd = validRequestedComponents[refid]
                parsed_spd = _SPDParser.parse(spd)
                pathToComponentXML = _os.path.dirname(spd)
                prfFilename = pathToComponentXML+'/'+parsed_spd.get_propertyfile().get_localfile().name
                _prf = _PRFParser.parse(prfFilename)
                execprops = []
                configurable[instanceName] = []
                for prop_check in _prf.get_simple():
                    if prop_check.get_kind()[0].get_kindtype() == 'execparam':
                        execprops.append(str(prop_check.get_id()))
                    if prop_check.get_kind()[0].get_kindtype() == 'configure':
                        if prop_check.get_mode() == 'readwrite' or prop_check.get_mode() == 'writeonly':
                            configurable[instanceName].append(str(prop_check.get_id()))
                for prop_check in _prf.get_simplesequence():
                    if prop_check.get_kind()[0].get_kindtype() == 'configure':
                        if prop_check.get_mode() == 'readwrite' or prop_check.get_mode() == 'writeonly':
                            configurable[instanceName].append(str(prop_check.get_id()))
                for prop_check in _prf.get_struct():
                    if prop_check.get_mode() == 'readwrite' or prop_check.get_mode() == 'writeonly':
                        configurable[instanceName].append(str(prop_check.get_id()))
                for prop_check in _prf.get_structsequence():
                    if prop_check.get_mode() == 'readwrite' or prop_check.get_mode() == 'writeonly':
                        configurable[instanceName].append(str(prop_check.get_id()))
                if properties != None:
                    simples = properties.get_simpleref()
                else:
                    simples = [] 
                simple_exec_vals = {}
                for simple in simples:
                    if not (simple.refid in execprops):
                        continue
                    overload_value = str(simple.value)
                    if simple.refid in props and assemblyController:
                        overload_value = props[simple.refid]
                        props.pop(simple.refid)
                    container = overloadContainer(str(simple.refid),overload_value)
                    simple_exec_vals[container.id] = container.value
                newComponent = Component(componentName,instanceName,instanceID,launchedFromSADFile=True,execparams=simple_exec_vals)
                if newComponent != None:
                    launchedComponents.append(newComponent)

        # Set up component connections
        if sad.connections:
            for connection in sad.connections.get_connectinterface():
                if connection != None:
                    connectionID = None
                    if connection.get_id() != "":
                        connectionID = connection.get_id()
                    if _DEBUG == True:
                        print "loadSADFile(): CONNECTION INTERFACE: connection ID " + str(connection.get_id())
                    usesPortComponent = None
                    usesPortName = None
                    providesPortComponent = None
                    providesPortName = None
                    # Check for uses port
                    if connection.get_usesport() != None:
                        if _DEBUG == True:
                            print "loadSADFile(): CONNECTION INTERFACE: uses port name " + str(connection.get_usesport().get_usesidentifier())
                            print "loadSADFile(): CONNECTION INTERFACE: uses port component ref " + str(connection.get_usesport().get_componentinstantiationref().get_refid())
                        usesPortName = connection.get_usesport().get_usesidentifier() 
                        usesPortComponentRefid = connection.get_usesport().get_componentinstantiationref().get_refid()
                        # Loop through launched components to find one containing the uses port to be connected
                        for component in launchedComponents:
                            if component._refid == usesPortComponentRefid:
                                usesPortComponent = component
                                break

                        # Look for end point of the connection
                        # Check for provides port 
                        if connection.get_providesport() != None:
                            if _DEBUG == True:
                                print "loadSADFile(): CONNECTION INTERFACE: provides port name " + str(connection.get_providesport().get_providesidentifier())
                                print "loadSADFile(): CONNECTION INTERFACE: provides port component ref " + str(connection.get_usesport().get_componentinstantiationref().get_refid())
                            providesPortName = connection.get_providesport().get_providesidentifier() 
                            providesPortComponentRefid = connection.get_providesport().get_componentinstantiationref().get_refid()
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    break
                        elif connection.get_componentsupportedinterface() != None:
                            if _DEBUG == True:
                                print "loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port interface " + str(connection.get_componentsupportedinterface().get_supportedidentifier())
                                print "loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port component ref " + str(connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid())
                            providesPortComponentRefid = connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid()
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    break
                        elif connection.get_findby() != None:
                            if _DEBUG == True:
                                print "loadSADFile(): CONNECTION INTERFACE: findby " + str(connection.get_componentsupportedinterface())

                        # Make connection
                        if usesPortComponent != None and providesPortComponent != None:
                            if _DEBUG == True:
                                print "loadSADFile(): calling connect on usesPortComponent"
                                if providesPortName != None:
                                    print "loadSADFile(): passing in to connect() providesPortName " + str(providesPortName)
                                if usesPortName != None:
                                    print "loadSADFile(): passing in to connect()  usesPortName " + str(usesPortName)
                                if connectionID != None:
                                    print "loadSADFile(): passing in to connect() connectionID " + str(connectionID)
                            if providesPortName != None and usesPortName != None and connectionID != None: 
                                usesPortComponent.connect(providesPortComponent, providesPortName,usesPortName,connectionID)
                            elif usesPortName != None and connectionID != None:
                                usesPortComponent.connect(providesPortComponent, usesPortName=usesPortName,connectionID=connectionID)
                            elif usesPortName != None:
                                usesPortComponent.connect(providesPortComponent, usesPortName=usesPortName)
                            else:
                                usesPortComponent.connect(providesPortComponent)

        # configure properties for launched component
        for component in componentPlacements:
            refid = component.componentfileref.refid
            assemblyController = False
            sandboxComponent = None
            if validRequestedComponents.has_key(refid):
                instanceID = component.get_componentinstantiation()[0].id_
                componentProps = None
                if len(launchedComponents) > 0:
                    for comp in launchedComponents:
                        if instanceID == comp._refid:
                            componentProps = comp._configRef
                            if instanceID == assemblyControllerRefid:
                                assemblyController = True
                            sandboxComponent = comp
                            break

            if sandboxComponent != None and \
               componentProps != None:
                properties=component.get_componentinstantiation()[0].get_componentproperties()
                if properties != None:
                    #simples
                    simples = properties.get_simpleref()
                    simple_vals = []
                    for simple in simples:
                        if not (simple.refid in configurable[sandboxComponent._instanceName]):
                            continue
                        overload_value = str(simple.value)
                        if simple.refid in props and assemblyController:
                            overload_value = props[simple.refid]
                            props.pop(simple.refid)
                        simple_vals.append(overloadContainer(str(simple.refid),overload_value))
                    #simple sequences
                    simpleseqs = properties.get_simplesequenceref()
                    simpleseq_vals = []
                    if simpleseqs != None:
                        for simpleseq in simpleseqs:
                            if not (simpleseq.refid in configurable[sandboxComponent._instanceName]):
                                continue
                            values_vals = [str(value) for value in simpleseq.get_values().get_value()]
                            if simpleseq.refid in props and assemblyController:
                                values_vals = props[simpleseq.refid]
                                props.pop(simpleseq.refid)
                            simpleseq_vals.append(overloadContainer(str(simpleseq.refid),values_vals))
                    #structs
                    structs = properties.get_structref()
                    struct_vals = []
                    if structs != None:
                        for struct in structs:
                            if not (struct.refid in configurable[sandboxComponent._instanceName]):
                                continue
                            simples = struct.get_simpleref()
                            value = {}
                            for simple in simples:
                                value[str(simple.refid)] = str(simple.value)
                            if struct.refid in props and assemblyController:
                                value = props[struct.refid]
                                props.pop(struct.refid)
                            struct_vals.append(overloadContainer(str(struct.refid),value))
                    #struct sequences
                    structseqs = properties.get_structsequenceref()
                    structseq_vals = []
                    if structseqs != None:
                        for structseq in structseqs:
                            if not (structseq.refid in configurable[sandboxComponent._instanceName]):
                                continue
                            values_vals = []
                            for struct_template in structseq.get_structvalue():
                                simples = struct_template.get_simpleref()
                                value = {}
                                for simple in simples:
                                    value[str(simple.refid)] = str(simple.value)
                                values_vals.append(value)
                            if structseq.refid in props and assemblyController:
                                values_vals = props[structseq.refid]
                                props.pop(structseq.refid)
                            structseq_vals.append(overloadContainer(str(structseq.refid),values_vals))
                    if len(sandboxComponent._propertySet) > 0:
                        overloadProperty(sandboxComponent, simple_vals, simpleseq_vals, struct_vals, structseq_vals)
            if assemblyController and len(props) > 0 :
                prop_types = {}
                prop_types['simple'] = []
                prop_types['simpleseq'] = []
                prop_types['struct'] = []
                prop_types['structseq'] = []
                prop_set = sandboxComponent._prf.get_simple()
                prop_types['simple'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                prop_set = sandboxComponent._prf.get_simplesequence()
                prop_types['simpleseq'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                prop_set = sandboxComponent._prf.get_struct()
                prop_types['struct'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                prop_set = sandboxComponent._prf.get_structsequence()
                prop_types['structseq'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                prop_to_pop = []
                for prop in props:
                    for prop_check in sandboxComponent._propertySet:
                        if prop_check.id == prop:
                            for prop_type_check in prop_types:
                                if prop in prop_types[prop_type_check][0]:
                                    prop_types[prop_type_check][1].append(overloadContainer(str(prop), props[prop]))
                                    prop_to_pop.append(str(prop))
                for prop_to_pop_iter in prop_to_pop:
                    props.pop(prop_to_pop_iter)
                overloadProperty(sandboxComponent, prop_types['simple'][1], prop_types['simpleseq'][1], prop_types['struct'][1], prop_types['structseq'][1])
            
        launchedComponents = []
        if len(props) != 0:
            for prop in props:
                print "Overload property '"+prop+"' was ignored because it is not on the Assembly Controller"
        return True
    except Exception, e:
        print "loadSADFile(): ERROR - Failed to load sad file " + str(filename) + " " + str(e)
        return False

def _uuidgen():
    return _commands.getoutput('uuidgen')

def setSDRROOT(newRoot):
    '''
    Change the SDRROOT location where sandbox looks for components and devices
    '''
    global _currentState
    # Validates new root
    if _os.path.isdir(newRoot):
        if _os.path.isdir(newRoot+"/dom"):
            if _os.path.isdir(newRoot+"/dev"):
                if _DEBUG == True:
                    print "setSDRROOT(): setting SDRROOT to " + str(newRoot)
                _currentState['SDRROOT'] = newRoot
            else:
                raise AssertionError, "setSDRROOT(): ERROR - invalid SDRROOT, dev directory does not exist"
        else:
            raise AssertionError, "setSDRROOT(): ERROR - invalid SDRROOT, dom directory does not exist"
    else:
        raise AssertionError, "setSDRROOT(): ERROR - invalid SDRROOT, directory does not exist"

def catalog(searchPath=None, printResults=False, returnSPDs=False):
    '''
    Lists all available components in $SDRROOT
    '''
    if connectedIDE == False:
        global _currentState
        if _currentState['SDRROOT'] == None:
            if _DEBUG == True:
                print "catalog(): defaulting local SDRROOT value to environment variable SDRROOT"
            if _os.environ.has_key("SDRROOT") == True:
                _currentState['SDRROOT'] = _os.environ["SDRROOT"]
            else:
                print "catalog() WARNING - No components found. SDRROOT not specified in python shell with setSDRROOT() or as environment variable"
                return None
        if not searchPath:
            if _DEBUG == True:
                print "catalog(): defaulting component search path to $SDRROOT/dom/components"
            searchPath = _currentState['SDRROOT'] + '/dom/components/'
    spdFilesWithFullPath = []
    componentNames = []

    if connectedIDE == False:
        files = []
        for root, dirs, fnames in _os.walk(searchPath):
            for filename in _fnmatch.filter(fnames, "*spd.xml"):
                files.append(_os.path.join(root,filename))
        for file_entry in files:
            try:
                tmp = _SPDParser.parse(file_entry)
            except:
                print "Could not parse", file_entry
                continue
            spdFilesWithFullPath.append(file_entry)
            componentNames.append(str(tmp.get_name()))
    else:
        if IDE_REF != None:
            profiles = IDE_REF._get_availableProfiles()
            for file_entry in profiles: 
                spdFilesWithFullPath.append(file_entry)
                componentNames.append(_os.path.basename(file_entry).split('.')[0])

    if printResults == True:
        print "catalog(): available components found in search path " + str(searchPath) + " ====================="
        count = 0
        for component in componentNames:
            print componentNames[count]
            count = count + 1

    if returnSPDs:
        return spdFilesWithFullPath
    else:
        return componentNames

# Given an spd filename, determine if it exists in the current SDRROOT
# filename may be relative path or full path
# spd filename returned is the full path
def _spdFileExists(filename, fileMgr=None):
    if filename == None or len(filename) == 0:
        raise AssertionError,"_spdFileExists(): ERROR - Need to enter spd filename"
    else:
        # check to see if full path has been provided
        validSPD = False
        if _os.path.isfile(filename):
            try:
                tmp = _SPDParser.parse(filename)
                validSPD = True
            except:
                pass
        if validSPD:
            if _DEBUG == True:
                print "_spdFileExists(): filename " + str(filename) + " does exist" 
            return filename 
        # check to see if relative path has been provided (i.e. path is under $SDRROOT/dom)
        else:
            if fileMgr == None: 
                # Make sure SDRROOT is set in python shell
                if _currentState['SDRROOT'] == None:
                    if _os.environ.has_key("SDRROOT") == True:
                        _currentState['SDRROOT'] = _os.environ["SDRROOT"]
                        if _DEBUG == True:
                            print "_spdFileExists(): local SDRROOT not set ... defaulting to environment variable SDRROOT with value " + str(_currentState['SDRROOT'])
                    else:
                        print "_spdFileExists() SPD File " + str(filename) + " not found. "
                        print "_spdFileExists() WARNING - relative path for file requires SDRROOT be set to locate ... local SDRROOT can be set calling setSDRROOT() or setting environment variable SDRROOT"
                        return None 
                spd_path = str(_currentState['SDRROOT']) + "/dom" + str(filename)
                if _os.path.isfile(spd_path):
                    if _DEBUG == True:
                        print "_spdFileExists(): filename " + spd_path + " does exist" 
                    return spd_path
            else:
                # If SCA file manager passed in, use that to see if file exists
                if fileMgr != None:
                    try:
                        fileList = fileMgr.list(filename)
                        if fileList != []:
                            return filename
                    except:
                        pass

    if _DEBUG == True:
        print "_spdFileExists(): filename " + str(filename) + " does not exist" 
    return None 

def _readSPDFile(filename, fileMgr=None):
    if fileMgr is not None:
        spdFile = fileMgr.open(filename, True)
        spdContents = spdFile.read(spdFile.sizeOf())
        spdFile.close()
        return _SPDParser.parseString(spdContents)
    else:
        return _SPDParser.parse(filename)

class _NamingContextStub(_CosNaming__POA.NamingContextExt):
    """
    Class used to generate a NamingContext object.  It extends the 
    NamingContextExt rather than the NameContext so it can work with Java as 
    well with the other implementations.  
    
    The only additional method that had to be implemented to support this 
    parent class was the to_name(name) method.  Java implementations invoke 
    this method to generate a list of NamingComponent objects based on some
    string format explained below.
     
    """
    # use the default implementation for everything but bind
    component = None

    def __init__(self):
        _NamingContextStub.component = None

    def bind(self, name, object):
        _NamingContextStub.component = object

    def rebind(self, name, object):
        _NamingContextStub.component = object

    def to_name(self, name):
        """
        Returns a list of _CosNaming.NameComponent from the incoming string.
        
        The name is a string used to generate NameComponent objects.  The 
        NameComponent object takes 2 optional arguments: id and kind.  The 
        string can have one or more names as follow:
        
            id1.kind1/id2.kind2/id3./.kind4
        
        Where the first two objects have the id and the kind.  The third object
        does not have a kind defined, and the fourth object does not have an 
        id.  Both, the id and the kind, can use a '\' as a escape character to
        include either the '.' or the '/' as part of them.  For instance the 
        following pair is valid:
        
            my\.id.my\/kind ==> ("my.id" "my/kind")
        
        If an error is found while parsing the string, then a ValueError is 
        raised and an empty list is returned.
        
        """
        names = []
        # Before splitting the name, replace all the escapes
        data = {'DOT_TOKEN':'\.', 'SLASH_TOKEN':'\/'}

        try:
            # if a escape dot is found replace it with the DOT_TOKEN
            name = name.replace('\.', '%(DOT_TOKEN)s')
            # if a escape slash is found replace it with the SLASH_TOKEN
            name = name.replace('\/', '%(SLASH_TOKEN)s')

            # now that we are not 'escaping' we can split the name into what 
            # will become NameComponent objects
            items = name.split('/')
            # for evert NameComponent, get the id and the kind
            for item in items:
                # Separate the id and the kind
                end = item.find('.')
                if end >= 0:
                    id_ = item[0:end]
                    kind_ = item[end + 1:]
                    # replace the DOT_TOKEN and the SLASH_TOKEN back 
                    id_ = id_ % data
                    kind_ = kind_ % data
                    # create the NameComponent object and append it to the list
                    names.append(_CosNaming.NameComponent(id_, kind_))

            return names
        except:
            raise ValueError("_NamingContextStub:to_name() '%s' is an invalid name" % name)

        return []

class _DeviceManagerStub(_CF__POA.DeviceManager):
    """
    Proxy used to provide launched Devices a point with which to register
    """

    device = None
    service = None

    def registerDevice(self, registeringDevice):
        _DeviceManagerStub.device = registeringDevice

    def unregisterDevice(self, registeringDevice):
        _DeviceManagerStub.device = None

    def registerService(self, registeringService, name):
        _DeviceManagerStub.service = registeringService

    def unregisterService(self, registeringService, name):
        _DeviceManagerStub.service = None

class _componentBase(object):
    '''
      componentDescriptor can be either the name of the component or the absolute file path
    '''
    def __init__(self, componentDescriptor=None,instanceName=None,refid=None):
        self._componentName = None
        self._profile = None
        self._propertySet = None
        self._instanceName = None
        self._sub_process = None
        self.pid = None
        self.debugger_p = None
        self.dependencies = []

        if componentDescriptor != None:
            # Check to see if componentDescriptor is a valid SPD file
            if _spdFileExists(componentDescriptor,self._fileManager) != None:
                self._profile = componentDescriptor
                try:
                    tmp = _readSPDFile(self._profile, self._fileManager)
                except:
                    print "Could not parse", componentDescriptor
                    return
                self._componentName = str(tmp.get_name())
            # else componentDescriptor is the name of the component 
            else:
                self._componentName = componentDescriptor
                if not self._isKickableComponent():
                    raise AssertionError, "Component:__init__() ERROR - need to provide valid component name or spd file to constructor"
        else:
            raise AssertionError, "Component:__init__() ERROR - None type not valid"

        if instanceName == None or len(instanceName)==0:
            numComponentsRunning  = len(_currentState['Components Running'].keys())
            # Use one-up counter to make component instance unique
            self._instanceName = self._componentName.replace('.','_') + "_" + str(numComponentsRunning+1)
            #self._instanceName = self._componentName + "_" + str(numComponentsRunning+1)
        else:
            # Makes sure instance Name is unique
            if instanceName in _currentState['Components Running'].keys():
                raise AssertionError, "Component:__init__() ERROR - User specified instance name already in use"
            else:
                self._instanceName = instanceName
        self._spdFileId = self._instanceName + _uuidgen()
        if _DEBUG == True:
            print "Component: __init__() self._componentName " + str(self._componentName)
            print "Component: __init__() self._instanceName " + str(self._instanceName)
        if refid == None:
            self._refid = _uuidgen()
        else:
            # Makes sure refid is unique
            for comp in _currentState['Components Running'].keys():
                if refid == _currentState['Components Running'][comp]._refid:
                    raise AssertionError, "Component:__init__() ERROR - User specified refid already in use"
            self._refid = refid

    def _cleanUpProcess(self):
        if _DEBUG == True:
            print "Component: _cleanUpProcess() for component " + str(self._componentName)
        if self.pid != None:
            self._terminate(self.pid)

    def _cleanUpCurrentState(self):
        if _DEBUG == True:
            print "Component: _cleanUpCurrentState() for component " + str(self._componentName)
        # Remove any connections involving this component
        if _currentState != None and _currentState.has_key('Component Connections'):
            # Loop over connections to see if component has any uses side connections
            for connection in _currentState['Component Connections'].keys():
                if _currentState['Component Connections'][connection]['Uses Component'] == self:
                    providesComponent = _currentState['Component Connections'][connection]['Provides Component']
                    try:
                        self.disconnect(providesComponent)
                    except:
                        pass
            # Loop over connections to see if component has any provides side connections
            for connection in _currentState['Component Connections'].keys():
                if _currentState['Component Connections'][connection]['Provides Component'] == self:
                    usesComponent = _currentState['Component Connections'][connection]['Uses Component']
                    try:
                        usesComponent.disconnect(self)
                    except:
                        pass
        # Remove component entry in _currentState
        if _currentState != None and _currentState.has_key('Components Running'):
            for component in _currentState['Components Running'].keys():
                if _currentState['Components Running'][component] == self:
                    del _currentState['Components Running'][component]

    def _terminate(self,pid):
        if self.debugger_p == None:
            tmp_stop_signals = _STOP_SIGNALS
        else:
            tmp_stop_signals = _STOP_SIGNALS_DEBUGGER
        for sig, timeout in tmp_stop_signals:
            try:
                # the group id is used to handle child processes (if they 
                # exist) of the component being cleaned up
                if _DEBUG == True:
                    print "Component: _terminate () making killpg call on pid " + str(pid) + " with signal " + str(sig)
                _os.killpg(pid, sig)
            except OSError:
                print "Component: _terminate() OSERROR ==============="
                pass
            if timeout != None:
                giveup_time = _time.time() + timeout
            while self._sub_process.poll() == None:
                if _time.time() > giveup_time: break
                _time.sleep(0.1)
            if self._sub_process.poll() != None: break
        if self.debugger_p == None:
            self._sub_process.wait()
        else:
            self.debugger_p.terminate()
            self.debugger_p.wait()
            self.debugger_p.kill()
        # Reset component object reference
        self.ref = None

    # This method determines if a given component name exists under the current SDRROOT
    # If it does, the SPD file is located for the given component
    def _isKickableComponent(self):
        if self._componentName == None:
            raise AssertionError,"Component:_isKickableComponent() ERROR: Need to set component name"
        spdFiles = catalog(None, False, True)
        if spdFiles != None:
            for file in spdFiles: 
                try:
                    tmp = _readSPDFile(file, self._fileManager)
                except:
                    print "Could not parse", file
                    return
                componentName = str(tmp.get_name())
                if componentName == self._componentName:            
                    self._profile = file
                    if _DEBUG == True:
                        print "Component:_isKickableComponent() component " + str(self._componentName) + " is kickable"
                    return True
        if _DEBUG == True:
            print "Component:_isKickableComponent() component " + str(self._componentName) + " is not kickable"
        return False

    def _verifySharedLibrary(self, localFilePath):
        matchesPattern = False
        # check to see if it's a C shared library
        status, output = _commands.getstatusoutput('nm '+localFilePath)
        if status == 0:
            # This is a C library
            libpath = _os.path.dirname(_os.path.abspath(localFilePath))
            try:
                path = _os.environ['LD_LIBRARY_PATH'].split(':')
            except KeyError:
                path = []
            if not libpath in path:
                path.append(libpath)
                _os.environ['LD_LIBRARY_PATH'] = ':'.join(path)
            matchesPattern = True
        else:
            # This is not a C library
            pass
        # check to see if it's a python module
        try:
            currentdir = _os.getcwd()
            subdirs = localFilePath.split('/')
            currentIdx = 0
            if len(subdirs) != 1:
                aggregateChange = ''
                while currentIdx != len(subdirs)-1:
                    aggregateChange += subdirs[currentIdx]+'/'
                    currentIdx += 1
                _os.chdir(aggregateChange)
            foundRoot = False
            for entry in _sys.path:
                if entry == '.':
                    foundRoot = True
                    break
            if not foundRoot:
                _sys.path.append('.')
            importFile = subdirs[-1]
            path = _os.environ['PYTHONPATH'].split(':')
            newFileValue = ''
            if importFile[-3:] == '.py':
                exec('import '+importFile[:-3])
                newFileValue = importFile[:-3]
            elif importFile[-4:] == '.pyc':
                exec('import '+importFile[:-4])
                newFileValue = importFile[:-4]
            else:
                exec('import '+importFile)
                newFileValue = importFile
            foundValue = False
            for entry in path:
                if entry == newFileValue:
                    foundValue = True
                    break
            if not foundValue:
                _os.environ['PYTHONPATH'] = _os.environ['PYTHONPATH']+':'+currentdir+'/'+aggregateChange+':'
            matchesPattern = True
        except:
            # This is not a python module
            pass
        _os.chdir(currentdir)
        # check to see if it's a java package
        status, output = _commands.getstatusoutput('file '+localFilePath)
        if localFilePath[-4:] == '.jar' and 'Zip' in output:
            currentdir = _os.getcwd()
            subdirs = localFilePath.split('/')
            path = ''
            if _os.environ.has_key('CLASSPATH'):
                path = _os.environ['CLASSPATH'].split(':')
            candidatePath = currentdir+'/'+localFilePath
            foundValue = False
            for entry in path:
                if entry == candidatePath:
                    foundValue = True
                    break
            if not foundValue:
                if _os.environ.has_key('CLASSPATH'):
                    _os.environ['CLASSPATH'] = _os.environ['CLASSPATH']+':'+candidatePath+':'
                else:
                    _os.environ['CLASSPATH'] = candidatePath+':'
            matchesPattern = True
        else:
            # This is not a Java package
            pass
        # it matches no patterns. Assume that it's a set of libraries
        if not matchesPattern:
            try:
                _os.environ['LD_LIBRARY_PATH'] = _os.environ['LD_LIBRARY_PATH']+':'+localFilePath+':'
            except KeyError:
                _os.environ['LD_LIBRARY_PATH'] = localFilePath+':'

    def _kick(self, execparams={}, debugger=None):
        if _DEBUG == True:
            print "Component:_kick() execparams " + str(execparams)
        global _currentState
        # Default execparams values
        if len(execparams.keys()) == 0:
            execparams = self._getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
            execparams = dict([(x.id, x.defValue) for x in execparams])
            if _DEBUG == True:
                print "Component:_kick() no execparams specified ... using default " + str(execparams)

        # get the type
        type = self._scd.get_componenttype()

        # Prepare the NamingContext stub
        if _os.environ.has_key("NAMING_CONTEXT_IOR") == False and IDE_REF == None:
            nc_stub = _NamingContextStub()
            _obj_poa.activate_object(nc_stub)
            nc_stub_var = nc_stub._this()
        elif IDE_REF != None:
            nc_stub_var = IDE_REF._get_namingContext()
        else: # NAMING_CONTEXT_IOR exists
            nc_stub_var = orb.string_to_object(_os.environ["NAMING_CONTEXT_IOR"]) 
        if IDE_REF == None:
            dm_stub = _DeviceManagerStub()
        else:
            dm_stub = None

        # Prepare the exec params
        naming_context_ior = orb.object_to_string(nc_stub_var)
        component_identifier = self._refid
        if component_identifier == None:
            component_identifier = self._spd.get_id()

        # if the user passed in an impl id, try to get the entry point from that impl
        if self._impl != None:
            impl_found = False
            for impl in self._spd.get_implementation():
                if impl.get_id() == self._impl:
                    entry_point = impl.get_code().get_entrypoint()
                    if (entry_point[0] != '/'):
                        entry_point = _os.path.join(_os.path.dirname(self._profile), entry_point)
                    if _DEBUG == True:
                        print "Component:_kick() Running Entry Point: %s" % entry_point
                        print "Component:_kick() Binding to Name: %s" % self._instanceName
                        print "Component:_kick() Simulated Naming Context IOR: %s" % naming_context_ior
                    if not _os.path.exists(entry_point):
                        raise AssertionError, "Component:_kick() Component implementation %s is missing entry_point %s" % (self._impl, impl.get_code().get_entrypoint())
                    impl_found = True
                    self.dependencies = impl.get_dependency()
                    break
            # The id was not part of the SPD file
            if not impl_found:
                raise ValueError("Component:_kick() ID %s was not found" % self._impl)
            
        # else try each impl until we can find an entry point for one of them
        else:
            impl_found = False
            for impl in self._spd.get_implementation():
                entry_point = impl.get_code().get_entrypoint()
                if (entry_point[0] != '/'):
                    entry_point = _os.path.join(_os.path.dirname(self._profile), entry_point)
                if _DEBUG == True:
                    print "Component:_kick() Running Entry Point: %s" % entry_point
                    print "Component:_kick() Binding to Name: %s" % self._instanceName
                    print "Component:_kick() Simulated Naming Context IOR: %s" % naming_context_ior
                if _os.path.exists(entry_point):
                    impl_found = True
                    self.dependencies = impl.get_dependency()
                    break
            if not impl_found:
                raise AssertionError, "Component:_kick() Could not find an entry point"

        for dependency in self.dependencies:
            softpkg = dependency.get_softpkgref()
            if softpkg == None:
                continue
            filename = softpkg.get_localfile()
            local_filename = _os.getenv('SDRROOT')+'/dom/'+filename.name
            dep_spd = _SPDParser.parse(local_filename)
            dep_impl = softpkg.get_implref()
            dep_localfile = None
            if dep_impl != None:
                for impl in dep_spd.get_implementation():
                    if dep_impl.get_refid() == impl.get_id():
                        dep_localfile = impl.get_code().get_localfile().name
                        break
            else:
                for impl in dep_spd.get_implementation():
                    try:
                        dep_localfile = impl.get_code().get_localfile().name
                        if dep_localfile != None:
                            break
                    except:
                        continue
            dependency_local_filename = local_filename[:local_filename.rfind('/')+1] + dep_localfile
            self._verifySharedLibrary(dependency_local_filename)
            

        argids = []
        if self._scd.get_componenttype() in ("device", "loadabledevice", "executabledevice"):
            # Prepare a DeviceManager stub
            if IDE_REF == None:
                dm_stub_var = dm_stub._this()
            else:
                dm_stub_var = IDE_REF._get_deviceManager()
            device_manager_ior = orb.object_to_string(dm_stub_var)

            # Prepare the exec params
            args = [ entry_point,
                     "DEVICE_ID", component_identifier,
                     "DEVICE_LABEL", self._spd.get_name(),
                     "DEVICE_MGR_IOR", device_manager_ior,
                     "PROFILE_NAME", self._profile ]
            argids = ["DEVICE_ID", "DEVICE_LABEL", "DEVICE_MGR_IOR", "PROFILE_NAME"]
        elif self._scd.get_componenttype() in ("resource",):
            args = [ entry_point,
                     "COMPONENT_IDENTIFIER", component_identifier,
                     "NAMING_CONTEXT_IOR", naming_context_ior,
                     "NAME_BINDING", self._instanceName ]
            argids = ["COMPONENT_IDENTIFIER", "NAMING_CONTEXT_IOR", "NAME_BINDING"]
        elif self._scd.get_componenttype() in ("service",):
            if IDE_REF == None:
                dm_stub_var = dm_stub._this()
            else:
                dm_stub_var = IDE_REF._get_deviceManager()
            device_manager_ior = orb.object_to_string(dm_stub_var)
            args = [ entry_point,
                     "DEVICE_MGR_IOR", device_manager_ior,
                     "SERVICE_NAME", self._spd.get_name() ]
            argids = ["DEVICE_MGR_IOR", "SERVICE_NAME"]
        else:
            raise ValueError("Component:_kick() Unexpected component type")

        for ex_id, ex_val in execparams.items():
            if ex_id in argids:
                if _DEBUG == True:
                    print "Component: Ignoring execparam '%s' because it is a duplicate entry (reserved id)" % ex_id
                continue
            if (ex_val != None):
                argids.append(ex_id)
                args.append(ex_id)
                args.append(str(ex_val))
        if _DEBUG == True:
            print "Component:_kick() args passed to _popen.Popen " + str(args)
        
        exec_file = _os.path.join(_os.getcwd(), entry_point)

        if self.ref == None:
            try:
                self._sub_process = _popen.Popen(args,
                                                executable=exec_file,
                                                cwd=_os.getcwd(),
                                                preexec_fn=_os.setpgrp)
            except Exception, e:
                raise AssertionError, "Component:_kick() Failed to launch component implementation %s due to %s" % (self._impl, e)
        
            if debugger:
                if debugger == 'gdb':
                    term = None
                    gdb = None
                    status,output = _commands.getstatusoutput('which gdb')
                    if status == 0:
                        gdb = output
                    else:
                        print "debugger "+str(debugger)+" cannot be found. Launching component without debugger"
                    if gdb:
                        status,output = _commands.getstatusoutput('which xterm')
                        if status == 0:
                            term = 'xterm'
                        if not term:
                            print "xterm cannot be found (needed to run gdb session). Launching component without debugger"
                    if term and gdb:
                        attach = str(gdb)+' '+exec_file+' '+str(self._sub_process.pid)
                        self.debugger_p = _popen.Popen([term, '-e', attach], executable=term)
                else:
                    print "debugger "+str(debugger)+" not supported"
            # Wait up to 10 seconds for the component to bind
            if self._timeout is None:
                timeout = 10.0
            else:
                timeout = self._timeout
            self.ref = None
            sleepIncrement = 0.1
            if self._scd.get_componenttype() in ("device", "loadabledevice", "executabledevice"):
                while _DeviceManagerStub.device == None:
                    if _DEBUG == True:
                        print "Component:_kick() Waiting for _DeviceManagerStub device %s" % self._instanceName, timeout
                    _time.sleep(sleepIncrement)
                    timeout -= sleepIncrement
                    if timeout < 0:
                        raise AssertionError, "Component:_kick() Device implementation %s did not register with device manager" % self._impl
                self.ref = _DeviceManagerStub.device
                self.ref = self.ref._narrow(_CF.Device)
                if self.ref._narrow(_CF.LoadableDevice):
                    self.ref = self.ref._narrow(_CF.LoadableDevice)
                if self.ref._narrow(_CF.ExecutableDevice):
                    self.ref = self.ref._narrow(_CF.ExecutableDevice)
            elif self._scd.get_componenttype() in ("resource",):
                while _NamingContextStub.component == None:
                    if _DEBUG == True:
                        print "Component:_kick() Waiting for _NamingContextStub component %s" % self._instanceName, timeout
                    _time.sleep(sleepIncrement)
                    timeout -= sleepIncrement
                    if timeout < 0:
                        raise AssertionError, "Component:_kick() Component implementation %s did not bind to name server" % self._impl
                self.ref = _NamingContextStub.component
                self.ref = self.ref._narrow(_CF.Resource)
            elif self._scd.get_componenttype() in ("service",):
                while _DeviceManagerStub.service == None:
                    if _DEBUG == True:
                        print "Component:_kick() Waiting for _DeviceManagerStub service %s" % self._instanceName, timeout
                    _time.sleep(sleepIncrement)
                    timeout -= sleepIncrement
                    if timeout < 0:
                        raise AssertionError, "Component:_kick() Service implementation %s did not register with device manager" % self._impl
                self.ref = _DeviceManagerStub.service

            self.pid = self._sub_process.pid
        self._spdFileId = self._instanceName + "_" + _uuidgen()
        if _DEBUG == True:
            print "Component:_kick() Launched component " + self._instanceName + " with pid " + str(self.pid)
        _currentState['Components Running'][self._instanceName] = self

    def __setattr__(self,name,value):
        # If setting any class attribute except for _propertySet,
        # Then need to see if name matches any component properties
        # If so, then call configure on the component for the particular property and value
        if name != "_propertySet":
            try:
                if hasattr(self,"_propertySet"):
                    propSet = object.__getattribute__(self,"_propertySet")
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
                # This would be thrown if _propertySet attribute hasn't been set yet
                # This will occur only with setting of class attibutes before _propertySet has been set
                # This will not affect setting attributes based on component properties since this will
                #   occur after all class attributes have been set
                if str(e).find("_propertySet") != -1:
                    pass
                else:
                    raise e
        return object.__setattr__(self,name,value)

    def __getattr__(self,name):
        # Need to see if name matches any component properties
        # If so, then call query on the component for the property and return that value 
        if name != "_propertySet" and hasattr(self,"_propertySet"):
            propSet = object.__getattribute__(self,"_propertySet")
            if propSet != None:
                for prop in propSet: 
                    if name == prop.clean_name:
                        queryResults = self.query([_CF.DataType(id=str(prop.id),value=_any.to_any(None))])[0]
                        if (queryResults != None):
                            currentValue = queryResults.value._v
                            if _DEBUG == True:
                                print "Component:__getattr__() query returns component property value " + str(currentValue) + " for property " + str(name)
                            return currentValue 
        return object.__getattribute__(self,name)

    def __getattribute__(self,name):
        try:
            if name != "_propertySet" and hasattr(self,"_propertySet"):
                propSet = object.__getattribute__(self,"_propertySet")
                if propSet != None:
                    for prop in propSet: 
                        if name == prop.id or name == prop.clean_name:  
                            return prop
            return object.__getattribute__(self,name)
        except AttributeError:
            raise

class Component(_componentBase):
    """
    This representation provides a proxy to a running component. The CF::Resource api can be accessed
    directly by accessing the members of the class
    
      componentDescriptor can be either the name of the component or the absolute file path
    
    A simplified access to properties is available through:
        Component.<property id> provides read/write access to component properties
    
    """
    def __init__(self,componentDescriptor=None,instanceName=None,refid=None,autoKick=True,componentObj=None,impl=None,launchedFromSADFile=False,debugger=None,execparams={},timeout=None,*args,**kwargs):
        self._profile = ''
        self._spd = None
        self._scd = None 
        self._prf = None 
        self._execparams = execparams
        self._impl = impl
        self._ports = []
        self._providesPortDict = {}
        self._configureTable = {}
        self._usesPortDict = {}
        self.ref = componentObj
        self._autoKick = autoKick
        self._fileManager = None
        self._debugger = debugger
        self._timeout = timeout
        if connectedIDE == True and IDE_REF != None:
            self._fileManager = IDE_REF._get_fileManager()

        _componentBase.__init__(self, componentDescriptor, instanceName, refid)
        if connectedIDE == True:
            if IDE_REF != None and autoKick == True:
                rescFactory = IDE_REF.getResourceFactoryByProfile(self._profile)
                self.ref = rescFactory.createResource(self._instanceName, [])

        try:
            if len(self._componentName)>0:
                self._parseComponentXMLFiles()
                self._buildAPI()
                if self._autoKick == True and self.ref == None:
                    self._kick(execparams=self._execparams,debugger=self._debugger)
                    self.initialize()
                    self._populatePorts()
                elif self.ref != None:
                    self._populatePorts()
                    _currentState['Components Running'][self._instanceName] = self 
            else:
                raise AssertionError, "Component:__init__() ERROR - Failed to instantiate invalid component % " % (str(self._componentName))

        except Exception, e:
            print "Component:__init__() ERROR - Failed to instantiate component " + str(self._componentName) + " with exception " + str(e)

        # build a configure call to initialize all properties to their default values
        self._configRef = []
        if (connectedIDE == False or componentObj == None) and self._propertySet != None:
            for prop in self._propertySet:
                if prop.type == 'struct':
                    if prop.mode != 'readonly':
                        ref = _copy.deepcopy(prop.propRef)
                        refList = []
                        for simple in prop.members.values():
                            simpleRef = _copy.deepcopy(simple.propRef)
                            if simple.defValue == None:
                                simpleRef.value = _any.to_any(None)
                            else:
                                simpleRef.value._v = _copy.deepcopy(simple.defValue)
                            refList.append(simpleRef)
                        ref.value._v = refList
                        self._configRef.append(ref)
                elif prop.type == 'structSeq':
                    if prop.mode != 'readonly':
                        if prop.defValue != None:
                            ref = _copy.deepcopy(prop.propRef)
                            structRefs = []
                            for dict in prop.defValue:
                                newProp = _prop_helpers.structProperty(id=prop.structID, valueType=prop.valueType, compRef=prop.compRef, mode=prop.mode)
                                newPropRef = newProp.propRef
                                newPropRef.value._v = [_copy.deepcopy(member.propRef) for member in newProp.members.values()]
                                simpleRefs = []
                                for simpleRef in newPropRef.value.value():
                                    try:
                                        if dict[simpleRef.id] == None:
                                            simpleRef.value = _any.to_any(None)
                                        else:
                                            simpleRef.value._v = dict[simpleRef.id]
                                    except:
                                        oldType = simpleRef.value._t
                                        simpleRef.value = _any.to_any(None)
                                        for _id, _type, _defValue in prop.valueType:
                                            if _id == simpleRef.id:
                                                if _defValue != None:
                                                    simpleRef.value._v = _defValue
                                                    simpleRef.value._t = oldType    
                                    simpleRefs.append(_copy.deepcopy(simpleRef))
                                tmpRef = _CORBA.Any(_CORBA.TypeCode("IDL:CF/Properties:1.0"), simpleRefs)
                                structRefs.append(_copy.deepcopy(tmpRef))
                            ref.value._v = structRefs
                            self._configRef.append(ref)
                else:
                    if prop.mode != 'readonly':
                        ref = _copy.deepcopy(prop.propRef)
                        if prop.defValue == None:
                            ref.value = _any.to_any(None)
                        else:
                            ref.value._v = _copy.deepcopy(prop.defValue)
                        if prop.id != 'structSeq':
                            self._configRef.append(ref)
            # When launching from SAD file, component needs to be configured after connections have been made
            # to replicate Domain-based ApplicationFactory behavior
            if launchedFromSADFile == False:
                newConfigRef = []
                for entry in self._configRef:
                    if entry.value._v != None:
                        newConfigRef.append(entry)
                self.configure(newConfigRef)

        # Take in property name/value pairs passed into the constructor
        # Add them as attributes to the class 
        # Call configure for the property name setting it to the provided value
        for arg in kwargs.items():
            self.__setattr__(arg[0],arg[1])
            self._configureSingleProp(arg[0],arg[1])

    def reset(self):
        self.releaseObject()
        if connectedIDE == True:
            if IDE_REF != None:
                rescFactory = IDE_REF.getResourceFactoryByProfile(self._profile[1:])
                self.ref = rescFactory.createResource(self._instanceName, [])
            _populateFromExternalNC()
        else:
            self._kick(execparams=self._execparams)
        self.initialize()
        self._populatePorts()

    def _isMatch(self, prop, modes, kinds, actions):
        if prop.get_mode() == None:
            m = "readwrite"
        else:
            m = prop.get_mode()
        matchMode = (m in modes)

        if prop.__class__ in (_PRFParser.simple, _PRFParser.simpleSequence):
            if prop.get_action() == None:
                a = "external"
            else:
                a = prop.get_action().get_type()
            matchAction = (a in actions)

            matchKind = False
            if prop.get_kind() == None:
                k = ["configure"]
            else:
                k = prop.get_kind()
            for kind in k:
                if kind.get_kindtype() in kinds:
                    matchKind = True

            # If kind is both configure and allocation, then action is a match
            # Bug 295
            foundConf = False
            foundAlloc = False
            for kind in k:
                if "configure" == kind.get_kindtype():
                    foundConf = True
                if "allocation" == kind.get_kindtype():
                    foundAlloc = True
            if foundConf and foundAlloc:
                matchAction = True
                
        elif prop.__class__ in (_PRFParser.struct, _PRFParser.structSequence):
            matchAction = True # There is no action, so always match

            matchKind = False
            if prop.get_configurationkind() == None:
                k = ["configure"]
            else:
                k = prop.get_configurationkind()
            for kind in k:
                if kind.get_kindtype() in kinds:
                    matchKind = True

            if k in kinds:
                matchKind = True


        return matchMode and matchKind and matchAction

    def _getPropertySet(self, kinds=("configure",), \
                             modes=("readwrite", "writeonly", "readonly"), \
                             action="external", \
                             includeNil=True):
        if _DEBUG == True:
            print "Component: _getPropertySet() kinds " + str(kinds)
            print "Component: _getPropertySet() modes " + str(modes)
            print "Component: _getPropertySet() action " + str(action)
        """
        A useful utility function that extracts specified property types from
        the PRF file and turns them into a _CF.PropertySet
        """
        propertySet = []
        translation = 48*"_"+_string.digits+7*"_"+_string.ascii_uppercase+6*"_"+_string.ascii_lowercase+133*"_"

        # Simples
        for prop in self._prf.get_simple(): 
            if self._isMatch(prop, modes, kinds, (action,)):
                if prop.get_value() == None and includeNil == False:
                    continue
                propType = prop.get_type()
                val = prop.get_value()
                if str(val) == str(None):
                    defValue = None
                else:
                    if propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
                        # If value contains an x, it is a hex value (base 16) 
                        if val.find('x') != -1:
                            defValue = int(val,16)
                        else:
                            defValue = int(val)
                    elif propType in ['double', 'float']:
                        defValue = float(val)
                    elif propType in ['char', 'string']:
                        defValue = str(val)
                    elif propType == 'boolean':
                        defValue = {"TRUE": True, "FALSE": False}[val.strip().upper()]
                    else:
                        defValue = None
                p = _prop_helpers.simpleProperty(id=prop.get_id(), valueType=propType, compRef=self, defValue=defValue, mode=prop.get_mode())
                prop_id = prop.get_name()
                if prop_id == None:
                    prop_id = prop.get_id()
                id_clean = str(prop_id).translate(translation)
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._refid)
                self._configureTable[prop.get_id()] = p
                propertySet.append(p)

                # If property has enumerations, stores them 
                if prop.enumerations != None:
                    _prop_helpers._addEnumerations(prop, p.clean_name)

        # Simple Sequences
        for prop in self._prf.get_simplesequence():
            if self._isMatch(prop, modes, kinds, (action,)):
                values = prop.get_values()
                propType = prop.get_type()
                defValue = None
                strValue = ""
                if str(values) != str(None):
                    defValue = []
                    defValues = values.get_value()
                    for val in defValues:
                        if propType in ['long', 'longlong', 'short', 'ulong', 'ulonglong', 'ushort']:
                            # If value contains an x, it is a hex value (base 16) 
                            if val.find('x') != -1:
                                defValue.append(int(val,16))
                            else:
                                defValue.append(int(val))
                        elif propType in ['double', 'float']:
                            defValue.append(float(val))
                        elif propType == 'string':
                            defValue.append(str(val))
                        elif propType == 'boolean':
                            defValue.append({"TRUE": True, "FALSE": False}[val.strip().upper()])
                        elif propType == 'octet':
                            strValue += _struct.pack('B', int(val))
                        elif propType == 'char':
                            strValue += str(val)
                # Octets and chars need to be passed as string instead of lists
                if len(strValue) > 0:
                    defValue = strValue
                
                p = _prop_helpers.sequenceProperty(id=prop.get_id(), valueType=propType, compRef=self, defValue=defValue, mode=prop.get_mode())
                prop_id = prop.get_name()
                if prop_id == None:
                    prop_id = prop.get_id()
                id_clean = str(prop_id).translate(translation)
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._refid)
                self._configureTable[prop.get_id()] = p
                propertySet.append(p)

        # Structures
        for prop in self._prf.get_struct():
            if self._isMatch(prop, modes, kinds, (action,)):
                members = []
                if prop.get_simple() != None:
                    for simple in prop.get_simple():
                        propType = simple.get_type()
                        val = simple.get_value()
                        if str(val) == str(None):
                            defValue = None
                        else:
                            if propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
                                # If value contains an x, it is a hex value (base 16) 
                                if val.find('x') != -1:
                                    defValue = int(val,16)
                                else:
                                    defValue = int(val)
                            elif propType in ['double', 'float']:
                                defValue = float(val)
                            elif propType in ['char', 'string']:
                                defValue = str(val)
                            elif propType == 'boolean':
                                defValue = {"TRUE": True, "FALSE": False}[val.strip().upper()]
                            else:
                                defValue = None
                        prop_id = simple.get_name()
                        if prop_id == None:
                            prop_id = simple.get_id()
                        id_clean = str(prop_id).translate(translation)
                        # Checks for enumerated properties
                        if simple.enumerations != None:
                            _prop_helpers._addEnumerations(simple, id_clean)
                        # Add individual property
                        id_clean = _prop_helpers.addCleanName(id_clean, simple.get_id(), self._refid)
                        members.append((simple.get_id(), propType, defValue, id_clean))
                p = _prop_helpers.structProperty(id=prop.get_id(), valueType=members, compRef=self, mode=prop.get_mode())
                prop_id = prop.get_name()
                if prop_id == None:
                    prop_id = prop.get_id()
                id_clean = str(prop_id).translate(translation)
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._refid)
                self._configureTable[prop.get_id()] = p
                propertySet.append(p)

        # Struct Sequence
        for prop in self._prf.get_structsequence():
            if self._isMatch(prop, modes, kinds, (action,)):
                if prop.get_struct() != None:
                    members = []
                    #get the struct definition
                    for simple in prop.get_struct().get_simple():
                        propType = simple.get_type()
                        val = simple.get_value()
                        if str(val) == str(None):
                            simpleDefValue = None
                        else:
                            if propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
                                # If value contains an x, it is a hex value (base 16) 
                                if val.find('x') != -1:
                                    simpleDefValue = int(val,16)
                                else:
                                    simpleDefValue = int(val)
                            elif propType in ['double', 'float']:
                                simpleDefValue = float(val)
                            elif propType in ['char', 'string']:
                                simpleDefValue = str(val)
                            elif propType == 'boolean':
                                simpleDefValue = {"TRUE": True, "FALSE": False}[val.strip().upper()]
                            else:
                                simpleDefValue = None
                        prop_id = simple.get_name()
                        if prop_id == None:
                            prop_id = simple.get_id()
                        id_clean = str(prop_id).translate(translation)
                        # Checks for enumerated properties
                        if simple.enumerations != None:
                            _prop_helpers._addEnumerations(simple, id_clean)   
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
                                    _value = None
                                    if _propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
                                        # If value contains an x, it is a hex value (base 16) 
                                        if value.find('x') != -1:
                                            _value = int(value,16)
                                        else:
                                            _value = int(value)
                                    elif _propType in ['double', 'float']:
                                        _value = float(value)
                                    elif _propType in ['char', 'string']:
                                        _value = str(value)
                                    elif _propType == 'boolean':
                                        _value = {"TRUE": True, "FALSE": False}[value.strip().upper()]
                                    else:
                                        _value = None    
                                newValue[str(id)] = _value
                            structSeqDefValue.append(newValue)
                p = _prop_helpers.structSequenceProperty(id=prop.get_id(), structID=prop.get_struct().get_id(), valueType=members, compRef=self, defValue=structSeqDefValue, mode=prop.get_mode())
                prop_id = prop.get_name()
                if prop_id == None:
                    prop_id = prop.get_id()
                id_clean = str(prop_id).translate(translation)
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._refid)
                self._configureTable[prop.get_id()] = p
                propertySet.append(p)

        if _DEBUG == True:
            try:
                print "Component: _getPropertySet() propertySet " + str(propertySet)
            except:
                pass
        return propertySet

    ########################################
    # External Resource API
    def _get_identifier(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_identifier()
            except:
                pass
        return retval
    
    def _get_started(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_started()
            except:
                pass
        return retval
    
    def start(self):
        if _DEBUG == True:
            print "Component: start()"
        if self.ref:
            try:
                self.ref.start()
            except:
                raise
    
    def stop(self):
        if _DEBUG == True:
            print "Component: stop()"
        if self.ref:
            try:
                self.ref.stop()
            except:
                raise
    
    def initialize(self):
        if _DEBUG == True:
            print "Component: initialize()"
        if self.ref:
            try:
                self.ref.initialize()
            except:
                raise
    
    def releaseObject(self):
        if _DEBUG == True:
            print "Component: releaseObject()"
        # Update _currentState list of connections and components
        if connectedIDE == False:
            self._cleanUpCurrentState()
        if self.ref:
            try:
                self.ref.releaseObject()
            except:
                raise
        # Remove component process
        if connectedIDE == False:
            self._cleanUpProcess()
    
    def getPort(self, name):
        if _DEBUG == True:
            print "Component: getPort()"
        retval = None
        if self.ref:
            try:
                retval = self.ref.getPort(name)
            except:
                raise
        return retval
    
    def configure(self, props):
        if _DEBUG == True:
            print "Component: configure() props " + str(props)
        if self.ref:
            try:
                self.ref.configure(props)
            except:
                raise
    
    def query(self, props):
        if _DEBUG == True:
            print "Component: query() props " + str(props)
        retval = None
        if self.ref:
            try:
                retval = self.ref.query(props)
            except:
                raise
        return retval
    
    def runTest(self, testid, props):
        retval = None
        if self.ref:
            try:
                retval = self.ref.query(testid, props)
            except:
                raise
        return retval
    
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

        prop = self._configureTable[propName]
        if (prop != None):
            #will generate a configure call on the component
            prop.configureValue(propValue)
        else:
            raise AssertionError,'Component:_configureSingleProp() ERROR - Property not found in _configureSingleProp'

    def _buildAPI(self):
        if _DEBUG == True:
            print "Component:_buildAPI()"
        count = 0
        for port in self._scd.get_componentfeatures().get_ports().get_provides():
            if port.get_providesname() != None:
                self._providesPortDict[count] = {}
                self._providesPortDict[count]["Port Name"] = port.get_providesname()
                self._providesPortDict[count]["Port Interface"] = port.get_repid()
                count = count + 1

        count = 0
        for port in self._scd.get_componentfeatures().get_ports().get_uses():
            if port.get_usesname() != None:
                self._usesPortDict[count] = {}
                self._usesPortDict[count]["Port Name"] = port.get_usesname()
                self._usesPortDict[count]["Port Interface"] = port.get_repid()
                count = count + 1

        self._propertySet = self._getPropertySet()

        if _DEBUG == True:
            try:
                self.api()
            except:
                pass

    def api(self):
        '''
        Inspect interfaces and properties for the component
        '''
        if _DEBUG == True:
            print "Component:api()"
        print "Component [" + str(self._componentName) + "]:"
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
        
        if globals().has_key('__TYPE_MAP'):  
            typeMap = globals()['__TYPE_MAP']
            midasTypeMap = globals()['__MIDAS_TYPE_MAP']
        maxNameLen = len("Property Name")
        maxSCATypeLen = len("(Data Type)")
        maxDefaultValueLen = len("[Default Value]") 
        propList = []
        for prop in self._propertySet:
            id = _copy.deepcopy(prop.id)
            clean_name = _copy.deepcopy(prop.clean_name)
            mode = _copy.deepcopy(prop.mode)
            propType = _copy.deepcopy(prop.type)
            propRef = _copy.deepcopy(prop.propRef)
            defValue = _copy.deepcopy(prop.defValue)
            
            if mode != "writeonly":
                if propType == 'struct':
                    currentValue = prop.members
                else:
                    currentValue = _copy.deepcopy(prop.queryValue())
            else:
                currentValue = "N/A"

            scaType = str(propType)
            if midasTypeMap.has_key(scaType):
                scaType = scaType + midasTypeMap[scaType]

            propList.append([clean_name,scaType,defValue,currentValue]) 
            # Keep track of the maximum length of strings in each column for print formatting
            if len(str(clean_name)) > maxNameLen:
                maxNameLen = len(str(clean_name))
            if len(str(scaType)) > maxSCATypeLen:
                maxSCATypeLen = len(str(scaType))
            if len(str(propRef.value.value())) > maxDefaultValueLen:
                maxDefaultValueLen = len(str(propRef.value.value()))

        # Limit the amount of space between columns of data for display
        if maxSCATypeLen > len("\t\t\t\t".expandtabs()):
            maxSCATypeLen = len("\t\t\t\t".expandtabs())
        if maxDefaultValueLen > len("\t\t\t".expandtabs()):
            maxDefaultValueLen = len("\t\t\t".expandtabs())
        if len(propList) > 0:
            print "Properties =============="
            print "Property Name" + " "*(maxNameLen-len("Property Name"))+ "\t(Data Type)" + " "*(maxSCATypeLen-len("(Data Type)")) + "\t\t[Default Value]" + " "*(maxDefaultValueLen-len("[Default Value]")) +"\t\tCurrent Value"
            print "-------------" + " "*(maxNameLen-len("Property Name"))+ "\t-----------" + " "*(maxSCATypeLen-len("(Data Type)")) + "\t\t---------------" + " "*(maxDefaultValueLen-len("[Default Value]")) + "\t\t-------------"
            for clean_name, propType, defValue, currentValue in propList:
                name = str(clean_name)
                if len(name) > maxNameLen:
                    name = str(clean_name)[0:maxNameLen-3] + '...'

                scaType = str(propType)
                if len(scaType) > maxSCATypeLen:
                    scaType = scaType[0:maxSCATypeLen-3] + '...'

                if scaType == 'structSeq':
                    currVal = ''
                    defVal = ''
                    print name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal + " "*(maxDefaultValueLen-len(defVal)) + "\t" + currVal
                elif scaType == 'struct':
                    currVal = ''
                    defVal = ''
                    print name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal + " "*(maxDefaultValueLen-len(defVal)) + "\t" + currVal
                    
                    for member in currentValue.values():
                        _id = _copy.deepcopy(member.id)
                        _propType = _copy.deepcopy(member.type)
                        _defValue = _copy.deepcopy(member.defValue)
                        _currentValue = _copy.deepcopy(member.queryValue())
                        
                        name = str(member.clean_name)
                        if len(name) > maxNameLen:
                            name = name[0:maxNameLen-3] + '...'
                            
                        scaType = str(_propType)
                        if midasTypeMap.has_key(scaType):
                            scaType = scaType + midasTypeMap[scaType]
                             
                        if len(scaType) > maxSCATypeLen+1:
                            scaType = scaType[0:maxSCATypeLen-3] + '...'
                        
                        defVal = str(_defValue)    
                        if defVal != None and len(defVal) > maxDefaultValueLen:
                            defVal = defVal[0:maxDefaultValueLen-3] + '...'
                        
                        currVal = str(_currentValue)
                        if currVal != None and len(currVal) > maxDefaultValueLen:
                            currVal = currVal[0:maxDefaultValueLen-3] + '...'
                                             
                        # Checks if the current prop is an enum
                        if _id in _prop_helpers._enums.keys():
                            for en in _prop_helpers._enums[_id].enumeration:
                                if _currentValue == en.get_value():
                                    currVal += " (enum=" + en.get_label() + ")"
                            
                        print '  ' + name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal  + " "*(maxDefaultValueLen-len(defVal)) + "\t\t" + currVal
                else:
                    defVal = str(defValue)
                    # If this prop is stored as a string and is of type octet or char, 
                    # that means it is an octet or char sequence
                    # Convert from string to list value for display
                    if type(defValue) == str:
                        if propType.find('octet') != -1:
                            defVal = str(list([ord(x) for x in defValue]))
                        elif propType.find('char') != -1:
                            defVal = str(list([x for x in defValue]))
          
                    if defVal != None and len(defVal) > maxDefaultValueLen:
                        defVal = defVal[0:maxDefaultValueLen-3] + '...'
                    
                    currVal = str(currentValue)
                    if currVal != None and len(currVal) > maxDefaultValueLen:
                        currVal = currVal[0:maxDefaultValueLen-3] + '...'

                    # Checks if current prop is an enum
                    if clean_name in _prop_helpers._enums.keys():
                        for en in _prop_helpers._enums[clean_name].enumeration:
                            if currentValue == en.get_value():
                                currVal += " (enum=" + en.get_label() + ")"
                    
                    print name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal  + " "*(maxDefaultValueLen-len(defVal)) + "\t\t" + currVal
            
    def _parseComponentXMLFiles(self):
        if _DEBUG == True:
            print "Component: _parseComponentXMLFiles()"
        if connectedIDE == False:
            if self._profile != None and \
               self._componentName != None:
                # SPD File
                self._spd = _SPDParser.parse(self._profile)

                pathToComponentXML = _os.path.dirname(self._profile)
                # PRF File
                if _os.path.isfile(pathToComponentXML+'/'+self._spd.get_propertyfile().get_localfile().name):
                    self._prfFilename = pathToComponentXML+'/'+self._spd.get_propertyfile().get_localfile().name
                else:
                    self._prfFilename = None
                if self._prfFilename != None:
                    self._prf = _PRFParser.parse(self._prfFilename)

                # SCD File
                if _os.path.isfile(pathToComponentXML+'/'+self._spd.get_descriptor().get_localfile().name):
                    self._scdFilename = pathToComponentXML+'/'+self._spd.get_descriptor().get_localfile().name 
                else:
                    self._scdFilename = None
                if self._scdFilename != None:
                    self._scd = _SCDParser.parse(self._scdFilename)
        else:
            if self._fileManager != None and len(self._profile) > 0:
                # SPD File
                spdFile = self._fileManager.open(self._profile,True)
                self._spdContents = spdFile.read(spdFile.sizeOf())
                spdFile.close()
                if self._spdContents != None:
                    self._spd = _SPDParser.parseString(self._spdContents)

                # PRF File
                doc_spd = _minidom.parseString(self._spdContents)
                localfiles = doc_spd.getElementsByTagName('localfile')
                for localfile in localfiles:
                    if localfile.parentNode.tagName == 'propertyfile':
                        self.__setattr__('_prf_path',object.__getattribute__(self,'_profile')[:object.__getattribute__(self,'_profile').rfind('/')+1]+str(localfile.getAttribute('name')))
                    if localfile.parentNode.tagName == 'descriptor':
                        self.__setattr__('_scd_path',object.__getattribute__(self,'_profile')[:object.__getattribute__(self,'_profile').rfind('/')+1]+str(localfile.getAttribute('name')))
                prfFile = self._fileManager.open(object.__getattribute__(self,'_prf_path'), True)
                self._prfContents = prfFile.read(prfFile.sizeOf())
                prfFile.close()
                if self._prfContents != None:
                    self._prf = _PRFParser.parseString(self._prfContents)

                # SCD File
                scdFile = self._fileManager.open(object.__getattribute__(self,'_scd_path'), True)
                self._scdContents = scdFile.read(scdFile.sizeOf())
                scdFile.close()
                if self._scdContents != None:
                    self._scd = _SCDParser.parseString(self._scdContents)

        # create a map between prop ids and names
        if self._prf != None:
            self._props = _prop_helpers.getPropNameDict(self._prf)

    def connect(self,providesComponent, providesPortName=None, usesPortName=None, connectionID=None):
        '''
        This function establishes a connection with a provides-side port. Python will attempt
        to find a matching port automatically. If there are multiple possible matches,
        a uses-side or provides-side port name may be necessary to resolve the ambiguity
        '''
        # If passed in object is of type _OutputBase, set uses port IOR string and return
        if isinstance(providesComponent, _io_helpers._OutputBase):
            usesPort_ref = None
            if usesPortName == None and len(self._usesPortDict.values()) == 1:
                usesPortName = self._usesPortDict.values()[0]['Port Name']
            if usesPortName != None:
                usesPort_ref = self.getPort(str(usesPortName))
                if usesPort_ref != None:
                    portIOR = str(orb.object_to_string(usesPort_ref))

                    # determine port type
                    foundUsesPortInterface = None
                    for outputPort in self._usesPortDict.values():
                        if outputPort['Port Name'] == usesPortName:
                            foundUsesPortInterface = outputPort['Port Interface']
                            break
                    if foundUsesPortInterface != None:
                        dataType = foundUsesPortInterface
                        providesComponent.setup(portIOR, dataType=dataType, componentName=self._componentName, usesPortName=usesPortName)
                        return
                    raise AssertionError, "Component:connect() failed because usesPortName given was not a valid port for providesComponent"
                else:
                    raise AssertionError, "Component:connect() failed because usesPortName given was not a valid port for providesComponent"
            else:
                raise AssertionError, "Component:connect() failed because usesPortName was not specified ... providesComponent being connect requires it for connection"

        if isinstance(providesComponent,Component) == False and \
           isinstance(providesComponent, _io_helpers._OutputBase) == False and \
           isinstance(providesComponent, _io_helpers.MessageSink) == False and \
           isinstance(providesComponent, _io_helpers._SinkBase) == False:
            raise AssertionError,"Component:connect() ERROR - connect failed because providesComponent passed in is not of valid type ... valid types include instance of _OutputBase,DataSink,FileSink, or Component classes"

        # Perform connect between this Component instance and a given Component instance called providesComponent 
        global _currentState
        if _DEBUG == True:
            print "Component: connect()"
            if providesPortName != None:
                print "Component: connect() providesPortName " + str(providesPortName)
            if usesPortName != None:
                print "Component: connect() usesPortName " + str(usesPortName)
            if connectionID != None:
                print "Component: connect() connectionID " + str(connectionID)
        portMatchFound = False 
        multipleMatchesFound = False
        foundProvidesPortInterface = None
        foundProvidesPortName = None
        foundUsesPortName = None 
        foundUsesPortInterface = None 
        # If both uses and provides port name are provided no need to search
        if usesPortName != None and providesPortName != None:
            foundUsesPortName = usesPortName
            foundProvidesPortName = providesPortName
        # If just provides port name is provided, find first matching port interface
        elif providesPortName != None:
            for outputPort in self._usesPortDict.values():
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
                if portMatchFound == True:
                    # If match was found, break out of outputPort for loop
                    break
        # If just uses port name is provided, find first matching port interface
        elif usesPortName != None:
            for outputPort in self._usesPortDict.values():
                if outputPort['Port Name'] == usesPortName:
                    # If output port is of type CF:Resource, then input port is just component
                    if outputPort['Port Interface'].find("CF/Resource") != -1:
                        portMatchFound = True
                        foundProvidesPortInterface = outputPort['Port Interface'] 
                        foundProvidesPortName = None 
                        foundUsesPortInterface = outputPort['Port Interface']  
                        foundUsesPortName = outputPort['Port Name'] 
                    # else need to find matching provides port interface
                    else:
                        for inputPort in providesComponent._providesPortDict.values():
                            if outputPort['Port Name'] == usesPortName and \
                               outputPort['Port Interface'] == inputPort['Port Interface']:
                                portMatchFound = True
                                foundProvidesPortInterface = inputPort['Port Interface']
                                foundProvidesPortName = inputPort['Port Name']
                                foundUsesPortInterface = outputPort['Port Interface']
                                foundUsesPortName = outputPort['Port Name']
                                # If match was found, break out of inputPort for loop
                                break
                if portMatchFound == True:
                    # If match was found, break out of outputPort for loop
                    break
        # If no port names provided, find matching port interfaces
        # If more than one port interface matches, then connect fails
        else:
            for outputPort in self._usesPortDict.values():
                # If output port is of type CF:Resource, then input port is just component
                if outputPort['Port Interface'].find("CF/Resource") != -1:
                    # If a previous match had been found, connect fails due to ambiguity
                    if portMatchFound == True:
                        raise AssertionError,"Component:connect(): ERROR - connect failed ... multiple ports matched interfaces on connect call ... must specify providesPortName or usesPortName" 
                    # First port match found
                    else:                        
                        portMatchFound = True
                        foundProvidesPortInterface = inputPort['Port Interface']
                        foundProvidesPortName = inputPort['Port Name']
                        foundUsesPortInterface = None 
                        foundUsesPortName = None 
                        continue

                for inputPort in providesComponent._providesPortDict.values():
                    if outputPort["Port Interface"] == inputPort['Port Interface']:
                        # If a previous match had been found, connect fails due to ambiguity
                        if portMatchFound == True:
                            raise AssertionError, "Component:connect(): ERROR connect failed ... multiple ports matched interfaces on connect call ... must specify providesPortName or usesPortName" 
                        # First port match found
                        else:                        
                            portMatchFound = True
                            foundProvidesPortInterface = inputPort['Port Interface']
                            foundProvidesPortName = inputPort['Port Name']
                            foundUsesPortInterface = outputPort['Port Interface']
                            foundUsesPortName = outputPort['Port Name']

        if _DEBUG == True:
            if foundUsesPortName != None:
                print "Component:connect() foundUsesPortName " + str(foundUsesPortName)
            if foundUsesPortInterface != None:
                print "Component:connect() foundUsesPortInterface " + str(foundUsesPortInterface)
            if foundProvidesPortName != None:
                print "Component:connect() foundProvidesPortName " + str(foundProvidesPortName)
            if foundProvidesPortInterface != None:
                print "Component:connect() foundProvidesPortInterface " + str(foundProvidesPortInterface)

        if (foundUsesPortName != None) and \
           (foundProvidesPortName != None or foundUsesPortInterface.find("CF/Resource")):             
            try:
                usesPort_ref = None
                usesPort_handle = None
                providesPort_ref = None
                usesPort_ref = self.getPort(str(foundUsesPortName))
                if usesPort_ref != None:
                    usesPort_handle = usesPort_ref._narrow(_CF.Port)

                if foundProvidesPortName != None:
                    providesPort_ref = providesComponent.getPort(str(foundProvidesPortName))
                elif foundUsesPortInterface.find("CF/Resource") != -1:
                    providesPort_ref = providesComponent.ref._narrow(_CF.Resource)
                if providesPort_ref != None:
                    if connectionID == None:
                        counter = 0
                        while True:
                            connectionID = self._instanceName+"-"+providesComponent._instanceName+"_"+repr(counter)
                            if not _currentState['Component Connections'].has_key(connectionID):
                                break
                            counter = counter + 1
                    if _currentState['Component Connections'].has_key(connectionID):
                        raise AssertionError, " ... connectionID already in use"
                    usesPort_handle.connectPort(providesPort_ref, str(connectionID))
                    if _DEBUG == True:
                        print "Component:connect() calling connectPort() with connectionID " + str(connectionID) 

            except Exception, e:
                raise AssertionError, "Component:connect(): connect failed " + str(e)

            _currentState['Component Connections'][connectionID] = {}
            _currentState['Component Connections'][connectionID]['Uses Port Name'] = str(foundUsesPortName)
            _currentState['Component Connections'][connectionID]['Uses Port Interface'] = str(foundUsesPortInterface)
            _currentState['Component Connections'][connectionID]['Uses Component'] = self
            if foundProvidesPortName != None:
                _currentState['Component Connections'][connectionID]['Provides Port Name'] = foundProvidesPortName 
            elif foundUsesPortInterface.find("CF/Resource"):
                _currentState['Component Connections'][connectionID]['Provides Port Name'] = "CF:Resource"
            _currentState['Component Connections'][connectionID]['Provides Port Interface'] = str(foundProvidesPortInterface)
            _currentState['Component Connections'][connectionID]['Provides Component'] = providesComponent
            if _DEBUG == True:
                print "Component:connect() succeeded"
            return
        else:
            raise AssertionError, "Component:connect failed" 

    def disconnect(self,providesComponent):
        global _currentState
        if _DEBUG == True:
            print "Component: disconnect()"
        if providesComponent == None:
            return
        usesPort_ref = None
        usesPort_handle = None
        for id in _currentState['Component Connections'].keys():
            if _currentState['Component Connections'][id]['Uses Component']._refid == self._refid and \
               _currentState['Component Connections'][id]['Provides Component']._refid == providesComponent._refid:
                usesPortName = _currentState['Component Connections'][id]['Uses Port Name']
                usesPort_ref = self.getPort(str(usesPortName))
                if usesPort_ref != None:
                    usesPort_handle = usesPort_ref._narrow(_CF.Port)
                if usesPort_handle != None:
                    if _DEBUG == True:
                        print "Component:disconnect(): calling disconnectPort"
                    usesPort_handle.disconnectPort(id)
                    del _currentState['Component Connections'][id]

    def _populatePorts(self):
        """Add all port descriptions to the component instance"""
        if object.__getattribute__(self,'_spd') == '':
            print "Unable to create port list for " + object.__getattribute__(self,'_name') + " - profile unavailable"
            return
        if len(object.__getattribute__(self,'_ports')) != 0:
            return

        if self._fileManager != None:
            spdFile = self._fileManager.open(object.__getattribute__(self,'_profile'),True)
            spdContents = spdFile.read(spdFile.sizeOf())
            spdFile.close()
        else:
            spdFile = open(object.__getattribute__(self,'_profile'),'r')
            spdContents = spdFile.read()
            spdFile.close()
        doc_spd = _minidom.parseString(spdContents)
        localfiles = doc_spd.getElementsByTagName('localfile')
        for localfile in localfiles:
            if localfile.parentNode.tagName == 'propertyfile':
                self.__setattr__('_prf_path',object.__getattribute__(self,'_profile')[:object.__getattribute__(self,'_profile').rfind('/')+1]+str(localfile.getAttribute('name')))
            if localfile.parentNode.tagName == 'descriptor':
                self.__setattr__('_scd_path',object.__getattribute__(self,'_profile')[:object.__getattribute__(self,'_profile').rfind('/')+1]+str(localfile.getAttribute('name')))
        if self._fileManager != None:
            prfFile = self._fileManager.open(object.__getattribute__(self,'_prf_path'), True)
            prfContents = prfFile.read(prfFile.sizeOf())
            prfFile.close()
        else:
            prfFile = open(object.__getattribute__(self,'_prf_path'), 'r')
            prfContents = prfFile.read()
            prfFile.close()
        doc_prf = _minidom.parseString(prfContents)
        if self._fileManager != None:
            scdFile = self._fileManager.open(object.__getattribute__(self,'_scd_path'), True)
            scdContents = scdFile.read(scdFile.sizeOf())
            scdFile.close()
        else:
            scdFile = open(object.__getattribute__(self,'_scd_path'), 'r')
            scdContents = scdFile.read()
            scdFile.close()
        doc_scd = _minidom.parseString(scdContents)
    
        interface_modules = ['BULKIO', 'BULKIO__POA']
        
        int_list = {}

        if not globals()["_loadedInterfaceList"]:
            globals()["_interface_list"] = _importIDL.importStandardIdl(std_idl_path=_std_idl_path,std_idl_include_path=_std_idl_include_path)
            if globals()["_interface_list"]:
                globals()["_loadedInterfaceList"] = True

        for int_entry in globals()["_interface_list"]:
            int_list[int_entry.repoId]=int_entry
        for uses in doc_scd.getElementsByTagName('uses'):
            idl_repid = str(uses.getAttribute('repid'))
            if not int_list.has_key(idl_repid):
                if _DEBUG == True:
                    print "Invalid port descriptor in scd for " + idl_repid
                continue
            int_entry = int_list[idl_repid]

            new_port = _Port(str(uses.getAttribute('usesname')), interface=None, direction="Uses", using=int_entry)
            try:
                new_port.generic_ref = self.getPort(str(new_port._name))
                new_port.ref = new_port.generic_ref._narrow(_ExtendedCF.QueryablePort)
                if new_port.ref == None:
                    new_port.ref = new_port.generic_ref._narrow(_CF.Port)
                new_port.extendPort()
    
                idl_repid = new_port.ref._NP_RepositoryId
                if not int_list.has_key(idl_repid):
                    if _DEBUG == True:
                        print "Unable to find port description for " + idl_repid
                    continue
                int_entry = int_list[idl_repid]
                new_port._interface = int_entry

                object.__getattribute__(self,'_ports').append(new_port)
            except:
                if _DEBUG == True:
                    print "getPort failed for port name: ", str(new_port._name)
        for provides in doc_scd.getElementsByTagName('provides'):
            idl_repid = str(provides.getAttribute('repid'))
            if not int_list.has_key(idl_repid):
                if _DEBUG == True:
                    print "Invalid port descriptor in scd for " + idl_repid
                continue
            int_entry = int_list[idl_repid]
            new_port = _Port(str(provides.getAttribute('providesname')), interface=int_entry, direction="Provides")
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
                    object.__getattribute__(self,'_ports').append(new_port)
                except:
                    continue
            except:
                if _DEBUG == True:
                    print "getPort failed for port name: ", str(new_port._name)


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
