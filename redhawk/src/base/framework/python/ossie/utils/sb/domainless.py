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

Component, Device and Service proxy classes are used to manage executable softpackages:
  - The launch() function will kick off the executable and return the appropriate object type.
    - It can take an SPD XML file or a component name found in the defined SDRROOT.
  - SCD (XML) describes the interfaces for the instantiated component.
  - The destructor cleans up the launched component process.
  - releaseObject removes the object (and destroys the component instance)

Component and Device classes support additional functionality:
  - Properties are attributes of the instantiated object.
  - connect() is called on the component with the uses port and the component with the provides port is the argument
    - Ambiguities can be resolved with usesPortName and/or providesPortName.

Helpers are provided to move data into and out of component objects
  - StreamSource and StreamSink are used to push vectors from Python to components and back.
  - FileSource and FileSink are used to push data from a file into components and back.
  - MessageSource and MessageSink are used to push messages from Python into components and back.

Plotting (matplotlib/PyQt4-based):
  - LinePlot():
      Line (X/Y) plot of input signal(s)
  - LinePSD():
      Line (X/Y) plot of power spectral density (PSD) of input signal(s)
  - RasterPlot():
      Falling raster (2D image) plot of input signal
  - RasterPSD():
      Falling raster (2D image) plot of the power spectral density (PSD) of input signal

Sound:
  - SoundSink()

Plotting (REDHAWK IDE-based)
  - Plot():
      Provides a way to display data from a particular port.
      Requires:
        - Eclipse Redhawk IDE must be installed
        - Environment variable RH_IDE must be set defining the path to the main eclipse directory (/data/eclipse for example)

Examples of use:

    # List available components in SDRROOT
    catalog()

    # Show current list of components running and component connections
    show()

    # Show the interfaces and properties for a component that has not been launched
    api('TestComponent')

    # Launch component from component name
    a = launch('TestComponent')

    # Launch component passing in path to SPD XML file of component
    b = launch('/var/redhawk/sdr/dom/components/TestComponent/TestComponent.spd.xml')

    # Inspect interfaces and property for a given component
    a.api()

    # Provide file input to a component and plot output from a given output port
    a = FileSource('<input filename here>','<data type here>')
    b = Component('SomeComponent')
    c = LinePlot()
    a.connect(b)
    b.connect(c) # If component has more than one BULKIO provides port, usesPortName must be specified
    start() # calls start on all deployed components and helpers
    stop() # calls stop on all deployed components and helpers

    # Push data to a plot
    src = StreamSource()
    plot = LinePlot()
    src.connect(plot, usesPortName='shortOut')
    data = range(1000) * 4
    start()
    src.write(data)
    stop()
"""
import atexit
import copy
import os
import fnmatch
import sys
import logging
import string as _string
import io, pydoc
import warnings
import traceback
from omniORB import CORBA, any

from ossie import parsers
from ossie import properties as core_properties
from ossie.utils import prop_helpers
from ossie.utils.model import _DEBUG
from ossie.utils.model import *
from ossie.utils.model.connect import ConnectionManager
from ossie.utils.uuid import uuid4

import omniORB
import threading, time, subprocess

from ossie.utils.sandbox import LocalSandbox, IDESandbox
import ossie.utils.sandbox
import CosNaming as _CosNaming
from ossie.cf import CF__POA as _CF__POA
from ossie.cf import CF as _CF

import pdb

warnings.filterwarnings('once',category=DeprecationWarning)

# Limit exported symbols
__all__ = ('show', 'loadSADFile', 'attachApplication', 'launchLocalSADFile', 'IDELocation', 'connectedIDE', 'getIDE_REF',
           'start', 'getSDRROOT', 'setSDRROOT', 'Component', 'generateSADXML',
           'getDEBUG', 'setDEBUG', 'getComponent', 'IDE_REF', 'setIDE_REF',
           'stop', 'catalog', 'redirectSTDOUT', 'orb', 'reset', 'launch', 'api',
           'createEventChannel', 'getEventChannel', 'getService', 'browse',
           'release', 'started', '_get_started', '_loadStructMembers', 'overloadProperty',
           'convertToValue', 'convertStringToComplex', 'availableOrchestrationTypes')

# Set up logging
log = logging.getLogger(__name__)

connectedIDE = False
IDE_REF = None
def setIDE_REF(ref):
    global IDE_REF
    global connectedIDE
    IDE_REF = ref
    connectedIDE = True

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
        for component in list(_currentState['Components Running'].values()):
            for port in component._ports:
                if port._direction == "Uses" and hasattr(port,"_get_connections"):
                    for connection in port._get_connections():
                        connectionId = connection.connectionId
                        connectionProvidesPortObj = connection.port
                        _currentState['Component Connections'][connectionId] = {}
                        _currentState['Component Connections'][connectionId]['Uses Component'] = component
                        _currentState['Component Connections'][connectionId]['Uses Port Name'] = port._name
                        for entry in list(component._usesPortDict.values()):
                            if entry['Port Name'] == port._name:
                                _currentState['Component Connections'][connectionId]['Uses Port Interface'] = entry['Port Interface']
                                break
                        # Loop over all components to find a matching port
                        for providesComponent in list(_currentState['Components Running'].values()):
                            if not component.ref._is_equivalent(providesComponent.ref):
                                for providesPort in providesComponent._ports:
                                    if providesPort._direction == "Provides":
                                        if connectionProvidesPortObj._is_equivalent(providesPort.ref):
                                            _currentState['Component Connections'][connectionId]['Provides Component'] = providesComponent
                                            _currentState['Component Connections'][connectionId]['Provides Port Name'] = providesPort._name
                                            for entry in list(providesComponent._providesPortDict.values()):
                                                if entry['Port Name'] == providesPort._name:
                                                    _currentState['Component Connections'][connectionId]['Provides Port Interface'] = entry['Port Interface']
                                                    break

# Initialize globals
_currentState = {}
_currentState['Components Running'] = {}
_currentState['Component Connections']= {}

# Sandbox singleton
_sandbox = None
def _getSandbox():
    global _sandbox
    if not _sandbox:
        if connectedIDE:
            log.trace('Creating IDE sandbox')
            _sandbox = IDESandbox(IDE_REF)
        else:
            sdrRoot = os.environ.get('SDRROOT', None)
            log.trace("Creating local sandbox with SDRROOT '%s'", sdrRoot)
            if sdrRoot is None:
                sdrRoot = os.getcwd()
            _sandbox = LocalSandbox(sdrRoot)
    return _sandbox

# Prepare the ORB
orb = CORBA.ORB_init()
_obj_poa = orb.resolve_initial_references("RootPOA")
_poaManager = _obj_poa._get_the_POAManager()
_poaManager.activate()

_obj_inspoa = orb.resolve_initial_references("omniINSPOA")
_inspoaManager = _obj_inspoa._get_the_POAManager()
_inspoaManager.activate()
if 'ORBendPoint' in os.environ:
    os.environ.pop('ORBendPoint')

def _cleanUpLaunchedComponents():
    if not _sandbox:
        return
    _sandbox.shutdown()

atexit.register(_cleanUpLaunchedComponents)

def reset():
    '''
    Set all components to their initial running condition
    '''
    # Save connection state.
    connectionList = []
    for _connectionId, connection in ConnectionManager.instance().getConnections().items():
        connectionId, uses, provides = connection
        entry = { 'USES_REFID': uses.getRefid(),
                  'USES_PORT_NAME': uses.getPortName(),
                  'PROVIDES_REFID': provides.getRefid(),
                  'PROVIDES_PORT_NAME': provides.getPortName() }
        connectionList.append(entry)

    log.debug('Saved %d component connections', len(connectionList))

    # Reset all sandbox components.
    sandbox = _getSandbox()
    sandbox.reset()

    # Restore connections.
    for connection in connectionList:
        usesComp = sandbox.getComponentByRefid(connection['USES_REFID'])
        usesName = connection['USES_PORT_NAME']
        providesComp = sandbox.getComponentByRefid(connection['PROVIDES_REFID'])
        providesName = connection['PROVIDES_PORT_NAME']
        if usesComp and usesName and providesComp:
            usesComp.connect(providesComp, providesPortName=providesName, usesPortName=usesName)

def IDELocation(location=None):
    if location == None:
        if "RH_IDE" in os.environ:
            if _DEBUG:
                print("IDELocation(): RH_IDE environment variable is set to " + str(os.environ["RH_IDE"]))
            return str(os.environ["RH_IDE"])
        else:
            if _DEBUG:
                print("IDELocation(): WARNING - RH_IDE environment variable is not set so plotting will not work")
            return None
    else:
        foundIDE = False
        if os.path.exists(location) and os.path.isdir(location):
            for file in os.listdir(location):
                if file == "eclipse":
                    foundIDE = True
                    os.environ["RH_IDE"] = str(location)
                    if _DEBUG:
                         print("IDELocation(): setting RH_IDE environment variable " + str(location))
        if not foundIDE:
            print("IDELocation(): ERROR - invalid location passed in, must give absolute path " + str(location))
        if _DEBUG:
            print("IDELocation(): setting RH_IDE environment variable " + str(location))
            return str(location)
        else:
            if _DEBUG:
                print("IDELocation(): ERROR - invalid location passed in for RH_IDE environment variable")
            return None

def redirectSTDOUT(filename):
    if _DEBUG == True:
        print("redirectSTDOUT(): redirecting stdout/stderr to filename " + str(filename))
    if type(filename) == str:
        dirname = os.path.dirname(filename)
        if len(dirname) == 0 or \
           (len(dirname) > 0 and os.path.isdir(dirname)):
            try:
                f = open(filename,'w')
                # Send stdout and stderr to provided filename
                sys.stdout = f
                sys.stderr = f
            except Exception as e:
                print("redirectSTDOUT(): ERROR - Unable to open file " + str(filename) + " for writing stdout and stderr " + str(e))
    elif type(filename) == io.OutputType:
        sys.stdout = filename
        sys.stderr = filename
    else:
        print('redirectSTDOUT(): failed to redirect stdout/stderr to ' + str(filename))
        print('redirectSTDOUT(): argument must be: string filename, cStringIO.StringIO object')

def show():
    '''
    Show current list of components running and component connections
    '''
    sandbox = _getSandbox()
    print("Components Running:")
    print("------------------")
    for component in sandbox.getComponents():
        print(component._instanceName, component)
    print("\n")

    print("Services Running:")
    print("----------------")
    for service in sandbox.getServices():
        print(service._instanceName, service)
    print("\n")

    print("Component Connections:")
    print("---------------------")
    if connectedIDE:
        log.trace('Scan for new connections')
        ConnectionManager.instance().resetConnections()
        ConnectionManager.instance().refreshConnections(sandbox.getComponents())
    for connectionid, uses, provides in list(ConnectionManager.instance().getConnections().values()):
        print("%s [%s] -> %s [%s]" % (uses.getName(), uses.getInterface(),
                                      provides.getName(), provides.getInterface()))

    print("\n")
    print("Event Channels:")
    print("--------------")
    for channel in sandbox.getEventChannels():
        print('%s (%d supplier(s), %d consumer(s))' % (channel.name, channel.supplier_count,
                                                       channel.consumer_count))
    print("\n")

    print("SDRROOT:")
    print("-------")
    print(sandbox.getSdrRoot().getLocation())
    print("\n")

def getComponent(name):
    '''
    Retrieve a pointer to a running component instance. This function is being deprecated
    '''
    return _getSandbox().getComponent(name)

def retrieve(name):
    '''
    Retrieve a pointer to a running component instance (replaces getComponent)
    '''
    return _getSandbox().retrieve(name)

def getService(name):
    """
    Retrieve a reference to a running service instance
    """
    return _getSandbox().getService(name)

def generateSADXML(waveform_name):
    '''
    generate a SAD XML string describing the current sandbox
    '''
    import ossie.utils.redhawk.sad_template as sad_template
    if _DEBUG == True:
        print("generateSadFileString(): generating SAD XML string for given waveform name " + str(waveform_name))
    sandbox = _getSandbox()
    Sad_template = sad_template.sad()
    initial_file = Sad_template.template
    with_id = initial_file.replace('@__UUID__@', str(uuid4()))
    # Need waveform name
    with_name = with_id.replace('@__NAME__@',waveform_name)
    # Loop over components to define individual componentfile and componentplacement entries
    componentfiles = ''
    partitioning = ''
    assemblycontroller = ''
    spdFileIds = {}
    for component in sandbox.getComponents():
        # Exclude local input file , output file , and output array components from the sad file
        if not isinstance(component, ossie.utils.model.ComponentBase):
            continue
        relativePathIndex = component._profile.find("/components")
        if relativePathIndex != -1:
            relativePathFilename = component._profile[relativePathIndex:]
        else:
            relativePathFilename = component._profile
        if relativePathFilename in spdFileIds:
            spdFileId = spdFileIds[relativePathFilename]
        else:
            spdFileId = '%s_%s' % (component._spd.get_name(), str(uuid4()))
            spdFileIds[relativePathFilename] = spdFileId
            componentfiles += Sad_template.componentfile.replace('@__SPDFILE__@',relativePathFilename)
            componentfiles = componentfiles.replace('@__SPDFILEID__@', spdFileId)
        partitioning += Sad_template.componentplacement.replace('@__COMPONENTNAME__@',component._instanceName)
        partitioning = partitioning.replace('@__COMPONENTINSTANCE__@',component._refid)
        partitioning = partitioning.replace('@__SPDFILEID__@', spdFileId)

        # Set Assembly Controller to first component in list
        if not assemblycontroller:
            assemblycontroller = Sad_template.assemblycontroller.replace('@__COMPONENTINSTANCE__@',component._refid)
    with_componentfiles = with_name.replace('@__COMPONENTFILE__@',componentfiles)
    with_partitioning = with_componentfiles.replace('@__COMPONENTPLACEMENT__@',partitioning)
    with_ac = with_partitioning.replace('@__ASSEMBLYCONTROLLER__@', assemblycontroller)
    # Loop over connections
    connectinterface = ''
    #for connection in _currentState['Component Connections'].values():
    _connection_map = ConnectionManager.instance().getConnections()
    for _tmp_connection in _connection_map:
        connection_id = _connection_map[_tmp_connection][0]
        uses_side = _connection_map[_tmp_connection][1]
        uses_name = uses_side.getName()
        if len(uses_name.split('/')) != 2:
            continue
        uses_inst_name = uses_name.split('/')[0]
        uses_inst_id = None
        for component in sandbox.getComponents():
            if not isinstance(component, ossie.utils.model.ComponentBase):
                continue
            if component._instanceName == uses_inst_name:
                if component._refid[:3] == 'DCE':
                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                else:
                    comprefid = component._refid.split(':')[0]
                uses_inst_id = comprefid
                break
        if not uses_inst_id:
            continue
        usesport = Sad_template.usesport.replace('@__PORTNAME__@',uses_side.getPortName())
        usesport = usesport.replace('@__COMPONENTINSTANCE__@',uses_inst_id)
        provides_side = _connection_map[_tmp_connection][2]
        supported_interface = False
        provides_name = provides_side.getName()
        if len(provides_name.split('/')) == 1:
            supported_interface = True
        else:
            provides_name = provides_side.getName().split('/')[0]
        provides_inst_id = None
        for component in sandbox.getComponents():
            if not isinstance(component, ossie.utils.model.ComponentBase):
                continue
            if component._instanceName == provides_name:
                if component._refid[:3] == 'DCE':
                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                else:
                    comprefid = component._refid.split(':')[0]
                provides_inst_id = comprefid
                break
        if not provides_inst_id:
            continue
        if supported_interface:
            # component support interface
            providesport = Sad_template.componentsupportedinterface.replace('@__PORTINTERFACE__@','IDL:CF/Resource:1.0')
            providesport = providesport.replace('@__COMPONENTINSTANCE__@',provides_inst_id)
        else:
            providesport = Sad_template.providesport.replace('@__PORTNAME__@',provides_side.getPortName())
            providesport = providesport.replace('@__COMPONENTINSTANCE__@',provides_inst_id)
        connectinterface += Sad_template.connectinterface.replace('@__USESPORT__@',usesport)
        connectinterface = connectinterface.replace('@__PROVIDESPORT__@',providesport)
        connectinterface = connectinterface.replace('@__CONNECTID__@',connection_id)
    with_connections = with_ac.replace('@__CONNECTINTERFACE__@',connectinterface)
    # External ports are ignored
    with_connections = with_connections.replace('@__EXTERNALPORTS__@',"")
    sadString = with_connections
    if _DEBUG == True:
        print("generateSadFileString(): returning SAD XML string " + str(sadString))
    return sadString

class overloadContainer:
    def __init__(self, id, value=None, type=None):
        self.id = id
        self.value = value
        self.type = type

def convertStringToComplex(value, basetype):
    negative_imag = False
    _split = value.split('+')
    if value[1:].find('-') != -1:
        _split = value.split('-')
        if len(_split) == 3: # negative real, negative imaginary
            if _split[0] == '':
                _split.pop(0)
                _split[0] = '-'+_split[0]
        negative_imag = True
    real_idx = -1
    imag_idx = -1
    for idx in range(len(_split)):
        if 'j' in _split[idx]:
            if imag_idx != -1:
                raise Exception("the proposed overload (id="+id+") is not a complex number ("+value+")")
            imag_idx = idx
        else:
            if real_idx != -1:
                raise Exception("the proposed overload (id="+id+") is not a complex number ("+value+")")
            real_idx = idx
    _real = None
    _imag = None
    if real_idx != -1:
        _real = basetype(_split[real_idx])
    if imag_idx != -1:
        _imag_str = _split[imag_idx].replace('j', '')
        _imag = basetype(_imag_str)
        if negative_imag:
            _imag = _imag * -1

    if not _real and not _imag:
        return None
    if _real and not _imag:
        return complex(basetype(_real), 0)
    if not _real and _imag:
        return complex(0, basetype(_imag))
    return complex(basetype(_real), basetype(_imag))

def convertToValue(valuetype, value):
    if value == None:
        return None
    if valuetype == 'utctime':
        return ossie.utils.rhtime.helpers.convert(value)
    if valuetype == 'string' or valuetype == 'char':
        if type(value) == list:
            return [str(s) for s in value]
        else:
            return str(value)
    elif valuetype == 'boolean':
        def _tobool(v):
           if type(v) == str:
               if v.lower() in ('true', 'y', 'yes', 't', 'on', '1' ):
                   return True
               elif v.lower() in ('false', 'n', 'no', 'f', 'off', '0' ):
                   return False
               else:
                 raise ValueError("invalid boolean value {}".format(v))
           else:
                return bool(v)
        if type(value) == list:
            return [_tobool(s) for s in value]
        else:
            return _tobool(value)
    elif valuetype == 'complexBoolean':
        if type(value) == list:
            return [convertStringToComplex(s, bool) for s in value]
        else:
            return convertStringToComplex(value, bool)
    elif valuetype == 'ulong' or valuetype == 'short' or valuetype == 'octet' or \
            valuetype == 'ushort' or valuetype == 'long' or valuetype == 'longlong' or \
            valuetype == 'ulonglong':
        if type(value) == list:
            return [int(s) for s in value]
        else:
            return int(value)
    elif valuetype == 'complexULong' or valuetype == 'complexShort' or valuetype == 'complexOctet' or \
            valuetype == 'complexUShort' or valuetype == 'complexLong' or valuetype == 'complexLongLong' or \
            valuetype == 'complexULongLong':
        if type(value) == list:
            return [convertStringToComplex(s, int) for s in value]
        else:
            return convertStringToComplex(value, int)
    elif valuetype == 'float' or valuetype == 'double':
        if type(value) == list:
            return [float(s) for s in value]
        else:
            return float(value)
    elif valuetype == 'complexFloat' or valuetype == 'complexDouble':
        if type(value) == list:
            return [convertStringToComplex(s, float) for s in value]
        else:
            return convertStringToComplex(value, float)
    else:
        raise Exception('bad value type')

def overloadProperty(component, simples=None, simpleseq=None, struct=None, structseq=None):
    if len(component._properties) > 0:
        allProps = dict([(str(prop.id),prop) for prop in component._properties])
        for entry in component._properties:
            if entry.mode == "readonly":
                continue
            for overload in simples:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    try:
                        setattr(component, entry.clean_name, convertToValue(entry.valueType, overload.value))
                    except Exception as e:
                        print(e, "Problem overloading id="+entry.id)
            for overload in simpleseq:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    try:
                        new_list = []
                        for item in overload.value:
                            new_list.append(convertToValue(entry.valueType, item))
                        setattr(component, entry.clean_name, new_list)
                    except Exception as e:
                        print(e, "Problem overloading id="+entry.id)
            for overload in struct:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    structValue = {}

                    # create map of clean keys to overloaded keys
                    _keys = {}
                    translation = 48*"_"+_string.digits+7*"_"+_string.ascii_uppercase+6*"_"+_string.ascii_lowercase+133*"_"
                    for _key in overload.value:
                        _keys[_key.translate(translation)] = _key

                    # for each simple entry in the struct
                    for simple in entry.valueType:

                        # get id to use for override
                        if len(simple) == 3:
                            # no name specified in prf, use id value
                            st_clean = str(simple[0])
                        else:
                            st_clean = str(simple[3])

                        # find a matching key to provide the overloaded value
                        _ov_key=None
                        for kl in [ st_clean, simple[0] ]:
                            kl_clean = kl.translate(translation)
                            if kl in overload.value:
                                _ov_key = kl
                            else:
                                if kl_clean in _keys and _keys.get(kl_clean) in overload.value:
                                    _ov_key = _keys[kl_clean]
                                    break

                        if _ov_key == None or _ov_key not in overload.value:
                            if _DEBUG:
                                print("Struct::Simple:  id:", str(simple[0]), " cleaned id:", st_clean, "  Unable to match overloaded key: ", _ov_key)
                            continue

                        if _DEBUG:
                            print("Struct::Simple:  id:", str(simple[0]), " cleaned id:", st_clean, " Overloaded ID: ", overload.id, " value: ", overload.value, " key:", _ov_key)

                        # cleanup struct key if it has illegal characters...
                        st_clean = st_clean.translate(translation)
                        try:
                            structValue[st_clean] = convertToValue(simple[1], overload.value[_ov_key])
                        except Exception as e:
                            print(e, "Problem overloading id="+entry.id)

                    if _DEBUG:
                         print("setattr  ", component, " clean name ", entry.clean_name,  " struct ", structValue)
                    setattr(component, entry.clean_name, structValue)
            for overload in structseq:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    structSeqValue = []
                    for overloadedValue in overload.value:

                        structValue = {}
                        translation = 48*"_"+_string.digits+7*"_"+_string.ascii_uppercase+6*"_"+_string.ascii_lowercase+133*"_"
                        # create map of clean keys to overloaded keys
                        _keys = {}
                        for _key in overloadedValue:
                            _keys[_key.translate(translation)] = _key

                        # for each simple entry in the struct
                        for simple in entry.valueType:
                            if len(simple) == 3:
                                # no name specified, use id value
                                st_clean = str(simple[0])
                            else:
                                st_clean = str(simple[3])

                            # find a matching key to provide the overloaded value
                            _ov_key=None
                            for kl in [ st_clean, simple[0] ]:
                                kl_clean = kl.translate(translation)
                                if kl in overloadedValue:
                                    _ov_key = kl
                                else:
                                    if kl_clean in _keys and _keys.get(kl_clean) in overloadedValue:
                                        _ov_key = _keys[kl_clean]
                                        break

                            if _ov_key == None or _ov_key not in overloadedValue:
                                if _DEBUG:
                                    print("StructSeq::Struct::Simple:  id:",str(simple[0]), " cleaned id:", st_clean, "  Unable to match overloaded key: ", _ov_key)
                                continue

                            if _DEBUG:
                                print("StructSeq::Struct::Simple:  id:", str(simple[0]), " cleaned id:", st_clean, " Overloaded  value: ", overloadedValue, " key:", _ov_key)

                            # cleanup struct key if it has illegal characters...
                            st_clean = st_clean.translate(translation)
                            try:
                                structValue[st_clean] = convertToValue(simple[1], overloadedValue[_ov_key])
                            except Exception as e:
                                print(e, "Problem overloading id="+entry.id)

                        structSeqValue.append(structValue)
                    setattr(component, entry.clean_name, structSeqValue)
        for prop in allProps:
            dV = allProps[prop].defValue
            if dV == None:
                continue
            if allProps[prop].mode != "readonly" and 'configure' in allProps[prop].kinds:
                setattr(component, allProps[prop].clean_name, allProps[prop].defValue)

def _loadStructMembers(parent, structdef_type=None):
    simples = parent.get_simpleref()
    value = {}
    for simple in simples:
        _value = str(simple.value)
        try:
            if structdef_type:
                p = [ _p for _p in structdef_type.simple if _p.id_ == simple.refid ][0]
                _value = convertToValue( p.get_type(), _value)
        except:
            traceback.print_exc()
            pass
        value[str(simple.refid)] = _value
    simpleseqs = parent.get_simplesequenceref()
    for simpleseq in simpleseqs:
        _seq = []
        for seq_value in simpleseq.values.get_value():
            _seq.append(str(seq_value))
        value[str(simpleseq.refid)] = _seq
        try:
            if structdef_type:
                p = [ _p for _p in structdef_type.simplesequence if _p.id_ == simpleseq.refid ][0]
                _seq = convertToValue( p.get_type(), _seq )
        except:
            traceback.print_exc()
            pass
    return value

def _loadStructMembersFromDict(struct_dict, structdef_type):

    value = {}
    for k,v in struct_dict.items():
        try:
            p = [ _p for _p in structdef_type.simple if _p.id_ == k ][0]
            value[str(k)] = convertToValue( p.get_type(), str(v))
            continue
        except:
            pass
        try:
            p = [ _p for _p in structdef_type.simplesequence if _p.id_ ==  k ][0]
            _seq = []
            for seq_value in v :
                _seq.append(str(seq_value))
            _seq = convertToValue( p.get_type(), _seq )
            value[str(k)] = _seq
        except:
            pass

    return value


def createRequest(requestId, props, pools=[], devices=[], sourceId=''):
    return _CF.AllocationManager.AllocationRequestType(requestId, props, pools, devices, sourceId)

def logRotator(logrotate_cfg):
    while True:
        command,status = subprocess.getstatusoutput("/usr/sbin/logrotate "+logrotate_cfg)
        time.sleep(30)

def resolveFindby(xml_root_node, domain_ref, usesPortComponent, usesPortName, connectionID, noevent=False):
    domainfinder = xml_root_node.get_findby().get_domainfinder()
    providesInterface = None
    if domainfinder:
        if domainfinder.get_type() == 'eventchannel':
            if noevent:
                return None
            channelname = domainfinder.get_name()
            channel = None
            try:
                channel = domain_ref.domMgr.eventChannelMgr.get(channelname)
            except:
                pass
            if not channel:
                channel = domain_ref.domMgr.eventChannelMgr.create(channelname)
            _supplier = usesPortComponent.getPort(usesPortName)
            print('establishing connection to eventchannel', channel)
            _supplier.connectPort(channel, connectionID)
        elif domainfinder.get_type() == 'filemanager':
            providesInterface = domain_ref.domMgr.fileMgr
        elif domainfinder.get_type() == 'servicename':
            service_name = domainfinder.get_name()
            for dev_mgr in domain_ref.domMgr.deviceManagers:
                for serv in dev_mgr.registeredServices:
                    if service_name == serv.serviceName:
                        providesInterface = serv.serviceObject
                        break
                if providesInterface:
                    break
        elif domainfinder.get_type() == 'servicetype':
            service_type = domainfinder.get_name()
            for dev_mgr in domain_ref.domMgr.deviceManagers:
                for serv in dev_mgr.registeredServices:
                    if serv.serviceObject._is_a(service_type):
                        providesInterface = serv.serviceObject
                        break
                if providesInterface:
                    break
        elif domainfinder.get_type() == 'domainmanager':
            providesInterface = domain_ref.domMgr
        elif domainfinder.get_type() == 'application':
            applications = domain_ref.domMgr.applications
            application_id = domainfinder.get_name()
            for app in applications:
                if app.get_identifier() == application_id:
                    providesInterface = app
                    break
    return providesInterface


def identifySadFile( wf ) :
    if wf.startswith('/'):
        import os
        return wf, os.path.dirname(wf), os.path.basename(wf)

    sandbox = _getSandbox()
    sdrroot = sandbox.getSdrRoot()
    log.debug("Looking for SAD  file {} in sdrroot {} ".format(wf,sdrroot.domPath('')))
    apps=[]
    import fnmatch
    import os
    for root, dirs, fnames in os.walk(sdrroot.domPath('/waveforms')):
        for fname in fnmatch.filter(fnames, '*.sad.xml'):
            apps.append((os.path.join(root, fname), root, fname ))

    log.debug("Found Possible SAD Files: {}".format(apps))
    for _a in apps:
        log.debug("Checking etry {}".format(_a))
        if 'sad.xml' in wf:
            if _a[2] == wf or _a[0].endswith(wf):
               return _a[0], _a[1], _a[2]
        elif _a[1].endswith(wf):
            return _a[0], _a[1], _a[2]

    return None, None, None

#
# configure properties for launched component, following rh assembly controller is configured last..
# need to create configure packages and put assembly controller last element in the list
# class ComponentDeployment(object):
class ComponentDeployment(object):
    def __init__(self, instanceName, ac):
        self.instanceName=instanceName
        self.assemblyController=ac
        self.comp=None
        self.simple_vals=[]
        self.simpleseq_vals=[]
        self.struct_vals=[]
        self.structseq_vals=[]


def attachApplication(wf_ior):

    # Attach to local running waveform and populate as SandboxComponents
    sandbox=_getSandbox()
    sdrroot=sandbox.getSdrRoot()

    app=None
    try:

        # get all the registered components
        app_ref = orb.string_to_object(wf_ior)
        if 'corbaloc' in wf_ior:
            app_ref = app_ref._narrow(_CF.Application)
        for rcomp in app_ref.registeredComponents:
            c=sandbox.attachToResource(rcomp)

        # look for waveform file .. SDDRROOT
        profile = app_ref.profile
        wf_fq_path, wf_dir, wf_file = identifySadFile(profile.strip('/'))
        if wf_fq_path == None:
            raise RuntimeError('Could not find {} in SDR root'.format(profile))

        # open and process the SAD file..
        sadFile = open(wf_fq_path,'r')
        sad = parsers.sad.parseString(sadFile.read())

        app_id = app_ref.identifier
        instance_name = app_id
        if ':' in app_id:
            instance_name = app_id.split(':')[-1]

        app=LocalApplicationStub( app_ref.name,
                                  instance_name,
                                  sad,
                                  profile,
                                  identifier=app_ref.identifier,
                                  logger=log)

        # setup component list to reflect start order
        componentPlacements = sad.partitioning.get_componentplacement()
        for hostCollocation in sad.get_partitioning().get_hostcollocation():
            componentPlacements.extend(hostCollocation.get_componentplacement())

        log.debug("Creating start order for: {}".format(profile))
        startorder={}
        for c in componentPlacements:
            log.debug("COMPONENT PLACEMENT component instantiation id '%s'", c.get_componentinstantiation()[0].id_)
            seq = None
            try:
                seq = c.get_componentinstantiation()[0].get_startorder()
                seq = int(seq)
                log.debug("COMPONENT PLACEMENT component startorder '%s'", str(seq))
            except:
                seq=-1
            startorder.setdefault( seq, [])
            startorder[ seq ].append( c )

        # flatten dictionary to an ordered list
        ordered_placements=[]
        for k,v in startorder.items():
            ordered_placements = ordered_placements + v

        app.setStartOrder(ordered_placements)
        app.refreshComponents()

        return app
    except Exception as e:
        traceback.print_exc()
        log.error("Unable to attach to application {}".format(e))

def launchLocalSADFile(wf,
                       props={},
                       extra_props={},
                       app_name=None,
                       waveform_context_name=None,
                       use_inspoa=False,
                       domain_ref=None):
    '''
    Load the graph/configuration described in the SAD file onto the sandbox

    Parameters:
    ----------
    wf         - string
               Path to the sad.xml file, or just sad name without extension
    props    - dict
               Property dictionary of name/value pairs for Assembly Controller and External properties. { 'prop1' : value1 }

    extra_props - dict
                  keys by component instantion ref contains a dictionary of { prop : value }. Special
                  handling of key 'ENV' will be use to set process' environment variables.

    use_inspoa  - bind application to human readable name for use with corbaloc

    Returns:
    --------
    app  - Returns Application Object if deployed

    Exceptions:
    RuntimeError - An error occurred during execution

    Warning - The SAD file was deployed but not all property overloads were processed.
    '''

    # look for waveform file .. x.sad.xml in SDDRROOT
    wf_fq_path, wf_dir, wf_file = identifySadFile(wf)
    if wf_fq_path == None:
        raise RuntimeError('Could not find {} in SDR root'.format(wf))

    # create application name
    if app_name == None:
        if waveform_context_name == None:
            app_name = "app_{}".format(wf)
            log.debug("Setting Application Name name '%s'", app_name)

            waveform_context_name=app_name
            log.debug("Setting Waveform context name '%s'", waveform_context_name)
        else:
            app_name = waveform_context_name
            log.debug("Setting Application Name name '%s'", app_name)

    if not waveform_context_name :
        waveform_context_name=str(uuid4())
        log.debug("Setting Waveform context name '%s'", waveform_context_name)

    instanceName = waveform_context_name
    sandbox = _getSandbox()
    sdrroot = sandbox.getSdrRoot()
    log.debug("loading SAD file '%s'", wf_fq_path)
    launchedComponents = []
    naming_context_ref=None
    try:
        # try to see if we have access to a naming service context object.. if not then we fail if
        # one was provided
        if type(props) != dict:
            raise RuntimeError("props argument must be a dictionary")

        props=copy.deepcopy(props)

        # open and process the SAD file..
        sadFile = open(wf_fq_path,'r')
        sadFileString = sadFile.read()
        sad = parsers.sad.parseString(sadFileString)

        _sdrroot_path=os.getenv('SDRROOT')
        if _sdrroot_path in wf_fq_path:
            profile_on_sdrroot = wf_fq_path.replace(_sdrroot_path,'')
            if not (profile_on_sdrroot[0] == '/'):
                profile_on_sdrroot = '/' + profile_on_sdrroot
        else:
            profile_on_sdrroot = wf_file

        app = LocalApplicationStub(app_name, instanceName, sad, profile_on_sdrroot, logger=log, use_inspoa=use_inspoa)

        if hasattr(sandbox, "setReferences"):
            sandbox.setReferences(app._this(),None)
        log.debug("waveform ID '%s'", sad.get_id())
        log.debug("waveform name '%s'", sad.get_name())
        validRequestedComponents = {}
        # Loop over each <componentfile> entry to determine SPD filenames and which components are kickable
        for component in sad.componentfiles.get_componentfile():
            log.debug("COMPONENT FILE localfile '%s'", component.get_localfile().get_name())
            log.debug("COMPONENT FILE id '%s'", component.get_id())
            log.debug("COMPONENT FILE type '%s'", component.get_type())
            try:
                localfile = 'dom' + component.get_localfile().get_name()
                spdFilename = sdrroot.findProfile(localfile,objType="components")
                log.debug("Found softpkg '%s'", spdFilename)
                validRequestedComponents[component.get_id()] = spdFilename
            except Exception as e:
                log.warn("Could not find softpkg '%s'", component.get_localfile().get_name())

        # Need to determine which component is the assembly controller
        assemblyControllerRefid = None
        if sad.assemblycontroller:
            #assemblyControllerRefid = sad.assemblycontroller.get_componentinstantiationref().get_refid() + ':' + waveform_context_name
            assemblyControllerRefid = sad.assemblycontroller.get_componentinstantiationref().get_refid()
            log.debug("ASSEMBLY CONTROLLER component instantiation ref '%s'", assemblyControllerRefid)
        if not assemblyControllerRefid:
            log.warn('SAD file did not specify an assembly controller')

        #
        # Allow for external properties to be set
        #
        externprops=[]
        if sad.get_externalproperties():
            externprops=[ { 'comprefid' : x.comprefid, 'propid' : x.propid, 'externalpropid' : x.externalpropid } for x in sad.get_externalproperties().get_property() ]
            log.debug( "External Props: %s", externprops )

        # Loop over each <componentplacement> entry to determine actual instance name for component
        # NOTE: <componentplacement> can also occur within <hostcollocation> elements if that exists
        #       so that needs to be checked also
        componentPlacements = sad.partitioning.get_componentplacement()
        for hostCollocation in sad.get_partitioning().get_hostcollocation():
            componentPlacements.extend(hostCollocation.get_componentplacement())

        log.debug("Creating start order for: {}".format(wf))
        startorder={}
        for c in componentPlacements:
            log.debug("COMPONENT PLACEMENT component instantiation id '%s'", c.get_componentinstantiation()[0].id_)
            seq = None
            try:
                seq = c.get_componentinstantiation()[0].get_startorder()
                seq = int(seq)
                log.debug("COMPONENT PLACEMENT component startorder '%s'", str(seq))
            except:
                seq=-1
            startorder.setdefault( seq, [])
            startorder[ seq ].append( c )

        # flatten dictionary to an ordered list
        ordered_placements=[]
        for k,v in startorder.items():
            ordered_placements = ordered_placements + v

        comp_launches = []
        configurable = {}
        sandBox = _getSandbox()
        app.setStartOrder(ordered_placements)

        for component in ordered_placements:
            log.debug("COMPONENT PLACEMENT component spd file id '%s'", component.componentfileref.refid)
            log.debug("COMPONENT PLACEMENT component instantiation id '%s'", component.get_componentinstantiation()[0].id_)
            log.debug("COMPONENT PLACEMENT component startorder '%s'", component.get_componentinstantiation()[0].get_startorder())
            log.debug("COMPONENT PLACEMENT component name '%s'", component.get_componentinstantiation()[0].get_usagename())
            # If component has a valid SPD file (isKickable), launch it
            file_refid = component.componentfileref.refid
            if file_refid in validRequestedComponents:
                instanceName = component.get_componentinstantiation()[0].get_usagename()
                comprefid = component.get_componentinstantiation()[0].id_
                instanceID = component.get_componentinstantiation()[0].id_ + ':' + waveform_context_name
                is_assembly_controller =  comprefid == assemblyControllerRefid
                log.debug("launching component '%s'", instanceName)
                _properties=component.get_componentinstantiation()[0].get_componentproperties()
                #simples
                spd = validRequestedComponents[file_refid]
                parsed_spd = parsers.spd.parse(spd)
                componentName = parsed_spd.get_name()
                pathToComponentXML = os.path.dirname(spd)
                # holds all execparam/initprops  when we call launch
                launch_props = {}

                # add default execparams that we put on command line, via sad file..
                execprops = ['LOGGING_CONFIG_URI', 'DEBUG_LEVEL' ]
                initprops=[]
                configprops=[]
                configurable[instanceName] = ComponentDeployment(instanceName, is_assembly_controller)
                cdeploy=configurable[instanceName]
                cfgprop_ids=dict.fromkeys(['simple','simpleseq','struct','structseq'],[])
                if parsed_spd.get_propertyfile():
                    prfFilename = pathToComponentXML+'/'+parsed_spd.get_propertyfile().get_localfile().name
                    _prf = parsers.prf.parse(prfFilename)
                    for prop, stage in sandbox._getInitializationStages(_prf):
                        pid = str(prop.get_id())
                        if stage == 'commandline':
                            execprops.append(pid)
                        if stage == 'initialize':
                            initprops.append(pid)
                        if stage == 'configure':
                            configprops.append(pid)
                            cfgprop_ids['simple'] += [ p.id_ for p in _prf.get_simple() if p.id_ == pid ]
                            cfgprop_ids['simpleseq']+=[ p.id_ for p in _prf.get_simplesequence() if p.id_ == pid ]
                            cfgprop_ids['struct']+=[ p.id_ for p in _prf.get_struct() if p.id_ == pid ]
                            cfgprop_ids['structseq']+=[ p.id_ for p in _prf.get_structsequence() if p.id_ == pid ]

                if _properties != None :
                    simples = _properties.get_simpleref()
                    if simples != None:
                        for simple in _properties.get_simpleref():
                            overload_value = str(simple.value)

                            # for init props convert to correct value ( long, double, etc)
                            if simple.refid in initprops:
                                try:
                                    p = [ _p for _p in _prf.simple if _p.id_ == simple.refid ][0]
                                    overload_value = convertToValue( p.get_type(), overload_value)
                                except:
                                    traceback.print_exc()
                                    pass

                            if simple.refid in props and is_assembly_controller:
                                overload_value = props[simple.refid]
                                props.pop(simple.refid)

                            for ep in [ ep for ep in externprops if simple.refid == ep['propid'] and comprefid == ep['comprefid'] and ep['externalpropid'] in props]:
                                overload_value = props[ep['externalpropid']]
                                log.debug('overriding external simple property ' + str(ep) )
                                props.pop(ep['externalpropid'])

                            container = overloadContainer(str(simple.refid),overload_value)

                            if simple.refid in execprops or simple.refid in initprops or simple.refid.startswith('ORB'):
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.simple_vals.append(container)

                    #simple sequences
                    simpleseqs = _properties.get_simplesequenceref()
                    if simpleseqs != None:
                        for simpleseq in simpleseqs:
                            overload_vals = [str(value) for value in simpleseq.get_values().get_value()]

                            if simpleseq.refid in initprops:
                                try:
                                    p = [ _p for _p in _prf.simplesequence if _p.id_ == simpleseq.refid ][0]
                                    overload_vals = convertToValue( p.get_type(), overload_vals)
                                except:
                                    traceback.print_exc()

                            # process assembly controller overrides
                            if simpleseq.refid in props and is_assembly_controller:
                                overload_vals = props[simpleseq.refid]
                                props.pop(simpleseq.refid)

                            # process externprops overrides
                            for ep in [ ep for ep in externprops if simpleseq.refid == ep['propid'] and comprefid == ep['comprefid'] and ep['externalpropid'] in props]:
                                    overload_vals = props[ep['externalpropid']]
                                    log.debug('overriding external simpleseq property ' + str(ep) )
                                    props.pop(ep['externalpropid'])

                            container = overloadContainer(str(simpleseq.refid),overload_vals)
                            if simpleseq.refid in execprops or simpleseq.refid in initprops:
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.simpleseq_vals.append(container)

                    # simple structures
                    structs = _properties.get_structref()
                    if structs != None:
                        for struct in structs:
                            overload_value = {}
                            xx = [ _prop for _prop in _prf.struct if str(_prop.id_)  == str(struct.refid) ][0]
                            overload_value.update(_loadStructMembers(struct, xx ))

                            # process assembly controller overrides
                            if struct.refid in props and is_assembly_controller:
                                overload_value = props[struct.refid]
                                props.pop(struct.refid)

                            # process externprops overrides
                            for ep in [ ep for ep in externprops if struct.refid == ep['propid'] and comprefid == ep['comprefid'] and ep['externalpropid'] in props]:
                                    overload_value = props[ep['externalpropid']]
                                    log.debug('overriding external struct property ' + str(ep) )
                                    props.pop(ep['externalpropid'])

                            container = overloadContainer(str(struct.refid),overload_value)
                            if struct.refid in execprops or struct.refid in initprops:
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.struct_vals.append(container)

                    #struct sequences
                    structseqs = _properties.get_structsequenceref()
                    if structseqs != None:
                        for structseq in structseqs:
                            overload_vals = []
                            xx = [ _prop for _prop in _prf.structsequence if str(_prop.id_)  == str(structseq.refid) ][0]
                            for struct_value in structseq.get_structvalue():
                                value = {}
                                value.update(_loadStructMembers(struct_value,xx.struct))
                                overload_vals.append(value)

                            # process assembly controller overridse
                            if structseq.refid in props and is_assembly_controller:
                                overload_vals = props[structseq.refid]
                                props.pop(structseq.refid)

                            # process externprops overrides
                            for ep in [ ep for ep in externprops if structseq.refid == ep['propid'] and comprefid == ep['comprefid'] and ep['externalpropid'] in props]:
                                overload_vals = props[ep['externalpropid']]
                                log.debug('overriding external structseq property ' + str(ep) )
                                props.pop(ep['externalpropid'])

                            container = overloadContainer(str(structseq.refid), overload_vals)
                            if structseq.refid in execprops or structseq.refid in initprops:
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.structseq_vals.append(container)


                # process remaining property overrides
                for prop in props.keys():
                    if is_assembly_controller:
                        if prop in execprops or prop in initprops or prop in configprops:
                            overload_value = props[prop]
                            props.pop(prop)
                            container = overloadContainer(str(prop),overload_value)

                            if prop in execprops or prop in initprops:
                                launch_props[container.id] = container.value
                            else:
                                if prop in cfgprop_ids['simple']:
                                    cdeploy.simple_vals.append(container)
                                if prop in cfgprop_ids['simpleseq']:
                                    cdeploy.simpleseq_vals.append(container)
                                if prop in cfgprop_ids['struct']:
                                    cdeploy.struct_vals.append(container)
                                if prop in cfgprop_ids['structseq']:
                                    cdeploy.structseq_vals.append(container)

                    else:
                        # non-assembly controller match with externprop ids
                        try:
                            ep = [ ep for ep in externprops if comprefid == ep['comprefid'] and prop == ep['externalpropid'] ][0]
                            propid = ep['propid']
                            overload_value = props[ep['externalpropid']]
                            log.debug('overriding external property ' + str(ep) )
                            props.pop(ep['externalpropid'])
                            container = overloadContainer(str(propid),overload_value)

                            if propid in execprops or propid in initprops:
                                launch_props[container.id] = container.value
                            else:
                                if prop in cfgprop_ids['simple']:
                                    cdeploy.simple_vals.append(container)
                                if prop in cfgprop_ids['simpleseq']:
                                    cdeploy.simpleseq_vals.append(container)
                                if prop in cfgprop_ids['struct']:
                                    cdeploy.struct_vals.append(container)
                                if prop in cfgprop_ids['structseq']:
                                    cdeploy.structseq_vals.append(container)
                        except:
                            pass


                #
                # process extra properties dictionary
                #
                comp_env={}
                if comprefid in extra_props:
                    cprops=extra_props[comprefid]
                    if 'ENV' in cprops:
                        comp_env=cprops['ENV']

                    for k in cprops.keys():

                        if k == 'ENV':
                            continue

                        # try simple
                        try:
                            overload_value = str(cprops[k])

                            if k in initprops or k in configprops:
                                p = [ _p for _p in _prf.simple if _p.id_ == k ][0]
                                overload_value = convertToValue( p.get_type(), overload_value)

                            container = overloadContainer(str(k),overload_value)

                            if k in execprops or k in initprops or k.startswith('ORB'):
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.simple_vals.append(container)

                            continue
                        except:
                            #traceback.print_exc()
                            pass

                        # try simpleseq
                        try:
                            #  check if k is valid simpleseq
                            p = [ _p for _p in _prf.simplesequence if _p.id_ == k ][0]
                            overload_vals = [str(value) for value in cprops[k] ]
                            if k in initprops or k in configprops:
                                overload_vals = convertToValue( p.get_type(), overload_vals)

                            container = overloadContainer(str(k),overload_vals)

                            if k in execprops or k in initprops:
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.simpleseq_vals.append(container)

                            continue
                        except:
                            #traceback.print_exc()
                            pass

                        # try structs
                        try:
                            overload_value = {}
                            xx = [ _prop for _prop in _prf.struct if str(_prop.id_)  == str(k) ][0]
                            overload_value.update(_loadStructMembersFromDict(cprops[k], xx ))

                            container = overloadContainer(str(k),overload_value)

                            if k in execprops or k in initprops:
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.struct_vals.append(container)
                            continue
                        except:
                            traceback.print_exc()
                            pass

                        # try structseq
                        try:
                            # lookup if in structsequence list
                            xx = [ _prop for _prop in _prf.structsequence if str(_prop.id_)  == str(k) ][0]
                            overload_vals= []
                            for struct_value in cprops[k]:
                                value={}
                                value.update(_loadStructMembersFromDict(struct_value, xx.struct ))
                                overload_vals.append(value)

                            container = overloadContainer(str(k),overload_vals)

                            if k in execprops or k in initprops:
                                launch_props[container.id] = container.value
                            else:
                                cdeploy.structseq_vals.append(container)
                        except:
                            #traceback.print_exc()
                            pass

                # save off component launch, info
                comp_launches.append( (componentName, instanceName, instanceID, False, launch_props, comp_env, "components", is_assembly_controller, cdeploy))

        component_output_log = None
        try:
            comp_out_log_fp = open('/var/redhawk/sdr/output_log.cfg','r')
            contents=comp_out_log_fp.readline()
            contents = contents[:-1]
            if len(contents) > 0:
                component_output_log = contents
        except:
            component_output_log = None

        root_context=None
        try:
            ns_obj = orb.resolve_initial_references("NameService")
            root_context = ns_obj._narrow(_CosNaming.NamingContext)
        except:
            pass

        base_log_location = None
        for comp_launch in comp_launches:
            base_log_location = None
            instance_logging_config_uri = None
            actual_file=''
            try:
                # NB: Explicitly request no configure call is made on the component
                componentName = comp_launch[0]
                instanceName = comp_launch[1]
                instanceID = comp_launch[2]
                launch_properties = comp_launch[4]
                env = comp_launch[5]
                objType=comp_launch[6]
                configure_properties = comp_launch[3]
                is_assemblycontroller = comp_launch[7]
                cdeploy=comp_launch[8]

                if component_output_log:
                    try:
                        os.makedirs(component_output_log+'/'+instanceName)

                        base_log_location = component_output_log+'/'+instanceName
                        logrotate_filename = component_output_log+'/'+instanceName+"/logrotate.cfg"
                        # create logrotate configuration file
                        logrotate_cfg = base_log_location+"/*.log {\n"
                        logrotate_cfg += "    size 10M\n"
                        logrotate_cfg += "    copytruncate\n"
                        logrotate_cfg += "    rotate 4\n"
                        logrotate_cfg += "}\n"
                        logrotate_fp = open(logrotate_filename, "w")
                        lograotate_fp.write(logrotate_cfg)
                        logrotate_fp.close()
                    except:
                        pass

                component_log_config_env_var = os.getenv('COMPONENT_LOG_CONFIG')
                tmp_domain_log_config_file=""
                if component_log_config_env_var:
                    tmp_domain_log_config_file = component_log_config_env_var
                else:
                    if domain_ref:
                        domain_log_config = domain_ref.domMgr.getLogConfig()
                        tmp_domain_log_config_file = '/tmp/'+instanceName+'_default_log_config.cfg'
                        dom_cfg_file = open(tmp_domain_log_config_file, 'w')
                        dom_cfg_file.write(domain_log_config)
                        dom_cfg_file.close()
                    else:
                        tmp_domain_log_config_file = '/tmp/{}_default_log_config.cfg'.format(instanceName)
                        print("instance {} log file {} ".format(instanceName, tmp_domain_log_config_file))
                        domain_log_config = "log4j.rootLogger=INFO,STDOUT\n" + \
                                            "# Direct log messages to STDOUT\n" + \
                                            "log4j.appender.STDOUT=org.apache.log4j.ConsoleAppender\n" +  \
                                            "log4j.appender.STDOUT.layout=org.apache.log4j.PatternLayout\n" +  \
                                            "log4j.appender.STDOUT.layout.ConversionPattern=%d{yyyy-MM-dd HH:mm:ss} %-5p %c:%L - %m%n\n"
                        dom_cfg_file = open(tmp_domain_log_config_file, 'w')
                        dom_cfg_file.write(domain_log_config)
                        dom_cfg_file.close()

                if tmp_domain_log_config_file:
                    launch_properties['LOGGING_CONFIG_URI'] =  'file://'+tmp_domain_log_config_file

                component_fp = None
                if base_log_location:
                    actual_file= '{}/{}.log'.format(base_log_location,instanceName)
                    component_fp=open( actual_file, 'a')
                print('Launching {} instanceName {} id {} with props {} '.format(componentName,
                                                                                 instanceName,
                                                                                 instanceID,
                                                                                 launch_properties))
                if domain_ref:
                    newComponent = launch(componentName, instanceName, instanceID, configure=configure_properties,  properties=launch_properties, env=env, objType=objType, stdout=component_fp, app_object=app._this(), domain_object=domain_ref.domMgr)
                    comp_ns_name = _CosNaming.NameComponent(instanceName,"")
                    naming_context_ref.rebind([comp_ns_name], newComponent.ref)
                    newComponent.waveform_context_name = [_CosNaming.NameComponent(domain_ref.domMgr.name,""), _CosNaming.NameComponent(waveform_context_name,"")]
                else:
                    newComponent = launch(componentName, instanceName, instanceID,
                                          properties=launch_properties,
                                          env=env,
                                          objType=objType,
                                          stdout=component_fp,
                                          app_object=app._this())
                    comp_ns_name = _CosNaming.NameComponent(instanceName,"")
                    newComponent.waveform_context_name = [_CosNaming.NameComponent('SB',""), _CosNaming.NameComponent(waveform_context_name,"")]

                newComponent.naming_context_ref = naming_context_ref
                newComponent.comp_ns_name = comp_ns_name
                newComponent.root_context = root_context
                newComponent.is_assemblycontroller = is_assemblycontroller
                cdeploy.comp = newComponent
                if base_log_location:
                    newComponent._stdout_filename = actual_file
                launchedComponents.append(newComponent)
            except Exception as e:
                msg = "Failed to launch component '%s', REASON: %s" %  (instanceName, str(e))
                print(msg)
                raise RuntimeError(msg)

        # start logrotate thread
        if base_log_location:
            log_rotator = threading.Thread(name=instanceName+'_log', target=logRotator, args=(logrotate_filename,))
            log_rotator.daemon = True
            log_rotator.start()

        usesdevs = {}
        # establish uses device relationship from the spd file
        for component in ordered_placements:
            # If component has a valid SPD file (isKickable), launch it
            refid = component.componentfileref.refid

            live_component = None
            for comp in launchedComponents:
                if instanceID == comp._refid:
                    live_component = comp

            if refid in validRequestedComponents:
                instanceName = component.get_componentinstantiation()[0].get_usagename()
                comprefid  = component.get_componentinstantiation()[0].id_
                instanceID = component.get_componentinstantiation()[0].id_ + ':' + waveform_context_name
                properties=component.get_componentinstantiation()[0].get_componentproperties()
                #simples
                spd = validRequestedComponents[refid]
                parsed_spd = parsers.spd.parse(spd)

                spd_usesdevs = parsed_spd.get_implementation()[0].get_usesdevice()
                for spd_usesdev in spd_usesdevs:
                    usesdevs[spd_usesdev.get_id()] = {}
                    for prop in spd_usesdev.propertyref:
                        tmp_value = prop.get_value()
                        if '__MATH__' in tmp_value:
                            # ignore MATH at this stage
                            pass
                        usesdevs[spd_usesdev.get_id()][prop.get_refid()] = tmp_value
                    for structprop in spd_usesdev.structref:
                        tmp_struct = {}
                        for simpleelem in structprop.simpleref:
                            tmp_value = simpleelem.get_value()
                            if '__MATH__' in tmp_value:
                                # ignore MATH at this stage
                                pass
                            tmp_struct[simpleelem.get_refid()] = tmp_value
                        for simpleseqelem in structprop.simplesequenceref:
                            tmp_values = simpleseqelem.get_values()
                            tmp_seq = []
                            for tmp_value in tmp_values.get_value():
                                tmp_seq.append(tmp_value)
                            tmp_struct[simpleseqelem.get_refid()] = tmp_seq
                        usesdevs[spd_usesdev.get_id()][structprop.get_refid()] = tmp_struct

        # establish uses device relationship from the sad file
        for usedevs in sad.usesdevicedependencies:
            for usedev in usedevs.usesdevice:
                usesdevs[usedev.get_id()] = {}
                for prop in usedev.propertyref:
                    usesdevs[usedev.get_id()][prop.get_refid()] = prop.get_value()

        log.debug( 'establish usesdevice')
        intial_responses = {}
        for usesdevid in usesdevs:
            has_math = False
            for prop_id in usesdevs[usesdevid]:
                print(usesdevs[usesdevid][prop_id])
                if 'MATH' in str(usesdevs[usesdevid][prop_id]):
                    has_math = True
                    break
                if type(usesdevs[usesdevid][prop_id]) == dict:
                    for entry in usesdevs[usesdevid][prop_id]:
                        if 'MATH' in str(usesdevs[usesdevid][prop_id][entry]):
                            has_math = True
                            break
                if has_math:
                    break
            if has_math:
                continue

            if domain_ref:
                request = [createRequest(usesdevid, core_properties.props_from_dict(usesdevs[usesdevid]))]
                response = domain_ref.domMgr._get_allocationMgr().allocate(request)
                if len(response) == 1:
                    intial_responses[usesdevid]=response[0]
                else:
                    for resp in intial_responses:
                        domain_ref.domMgr._get_allocationMgr().deallocate(intial_responses[resp].allocationID)
                    msg=" ERROR - Failed to allocate: " + str(usesdevs[usesdevid])
                    raise RuntimeError(msg)

        # Set up component connections
        log.debug( 'check connections')
        if sad.connections:
            for connection in sad.connections.get_connectinterface():
                log.debug('evaluating connection {}'.format(connection.get_id()))
                if connection != None:
                    connectionID = None
                    if connection.get_id():
                        connectionID = connection.get_id()
                    log.debug("CONNECTION INTERFACE: connection ID {}'".format(connection.get_id()))
                    usesPortComponent = None
                    usesPortName = None
                    providesPortComponent = None
                    providesPortName = None
                    providesPortComponentRefid = None
                    usesPortComponentRefid = None
                    usesDeviceUsedByApplication = None
                    providesDeviceUsedByApplication = None
                    providesInterface = None
                    # Check for uses port
                    if connection.get_usesport() != None:
                        usesPortName = connection.get_usesport().get_usesidentifier()
                        if connection.get_usesport().get_componentinstantiationref() != None:
                            usesPortComponentRefid = connection.get_usesport().get_componentinstantiationref().get_refid()
                        elif connection.get_usesport().get_deviceusedbythiscomponentref() != None:
                            usesDeviceUsedByApplication = connection.get_usesport().get_deviceusedbythiscomponentref().get_usesrefid()
                        elif connection.get_usesport().get_deviceusedbyapplication() != None:
                            usesDeviceUsedByApplication = connection.get_usesport().get_deviceusedbyapplication().get_usesrefid()
                        else:
                            log.warn("Unable to create connection for '%s'",connection.get_usesport().get_usesidentifier())
                            continue
                        log.debug("CONNECTION INTERFACE: uses port name '%s'", usesPortName)
                        log.debug("CONNECTION INTERFACE: uses port component ref '%s'", usesPortComponentRefid)
                        # Loop through launched components to find one containing the uses port to be connected
                        for component in launchedComponents:
                            if component._refid[:3] == 'DCE':
                                comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                            else:
                                comprefid = component._refid.split(':')[0]
                            if comprefid == usesPortComponentRefid:
                                usesPortComponent = component
                                break

                        # Look for end point of the connection
                        # Check for provides port
                        if connection.get_providesport() != None:
                            providesPortName = connection.get_providesport().get_providesidentifier()
                            if connection.get_providesport().get_componentinstantiationref() != None:
                                providesPortComponentRefid = connection.get_providesport().get_componentinstantiationref().get_refid()
                            elif connection.get_providesport().get_deviceusedbythiscomponentref() != None and domain_ref:
                                providesDeviceUsedByApplication = connection.get_providesport().get_deviceusedbythiscomponentref().get_usesrefid()
                            elif connection.get_providesport().get_deviceusedbyapplication() != None and domain_ref:
                                providesDeviceUsedByApplication = connection.get_providesport().get_deviceusedbyapplication().get_usesrefid()
                            else:
                                log.warn("Unable to create connection for '%s'",connection.get_providesport().get_providesidentifier())
                                continue
                            log.debug("CONNECTION INTERFACE: provides port name '%s'", providesPortName)
                            log.debug("CONNECTION INTERFACE: provides port component ref '%s'", providesPortComponentRefid)
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid[:3] == 'DCE':
                                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                                else:
                                    comprefid = component._refid.split(':')[0]
                                if comprefid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    break
                        elif connection.get_componentsupportedinterface() != None:
                            if connection.get_componentsupportedinterface().get_componentinstantiationref() != None:
                                providesPortComponentRefid = connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid()
                            elif connection.get_componentsupportedinterface().get_deviceusedbythiscomponentref() != None and domain_ref:
                                providesDeviceUsedByApplication = connection.get_componentsupportedinterface().get_deviceusedbythiscomponentref().get_usesrefid()
                            elif connection.get_componentsupportedinterface().get_deviceusedbyapplication() != None and domain_ref:
                                providesDeviceUsedByApplication = connection.get_componentsupportedinterface().get_deviceusedbyapplication().get_usesrefid()
                            elif domain_ref and connection.get_componentsupportedinterface().get_findby() != None and domain_ref:
                                providesInterface = resolveFindby(connection.get_componentsupportedinterface(), domain_ref, usesPortComponent, usesPortName, connectionID)
                                # event channel connection
                                if providesInterface == None:
                                    continue
                            else:
                                log.warn("Unable to create connection")
                                continue
                            if _DEBUG == True:
                                print("loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port interface " + str(connection.get_componentsupportedinterface().get_supportedidentifier()))
                                print("loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port component ref " + str(connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid()))
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid[:3] == 'DCE':
                                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                                else:
                                    comprefid = component._refid.split(':')[0]
                                if comprefid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    providesInterface = component.ref
                                    break
                        elif domain_ref and connection.get_findby():
                            providesInterface = resolveFindby(connection, domain_ref, usesPortComponent, usesPortName, connectionID)
                            # event channel connection
                            if providesInterface == None:
                                continue

                        # Make connection
                        if usesPortComponent and providesPortComponent:
                            log.debug("calling connect on usesPortComponent")
                            if providesPortName:
                                log.debug("passing in to connect() providesPortName '%s'", providesPortName)
                            if usesPortName:
                                log.debug("passing in to connect() usesPortName '%s'", usesPortName)
                            if connectionID:
                                log.debug("passing in to connect() connectionID '%s'", connectionID)
                            # NB: None values will fall back to connect() defaults.
                            log.debug('creating connection component {}/{}  component {}/{} '.format(usesPortComponent, usesPortName, providesPortComponent, providesPortName))
                            usesPortComponent.connect(providesPortComponent, providesPortName, usesPortName, connectionID)
                        elif usesPortComponent and providesDeviceUsedByApplication:
                            if not connectionID:
                                connectionID = str(uuid4())
                            if providesPortName:
                                if providesDeviceUsedByApplication in intial_responses:
                                    providesport = intial_responses[providesDeviceUsedByApplication].allocatedDevice.getPort(providesPortName)
                                    log.debug('creating connection component {}/{} to providesDeviceUsedByApplication (1) {}/{}'.format( usesPortComponent, usesPortName, providesDeviceUsedByApplication, providesPortName ))
                                    usesPortComponent.getPort(usesPortName).connectPort(providesport, connectionID)
                            else:
                                if providesDeviceUsedByApplication in intial_responses:
                                    log.debug('creating connection component {}{} to providesDeviceUsedByApplication (2) {}'.format(usesPortComponent, usesPortName, providesDeviceUsedByApplication))
                                    usesPortComponent.getPort(usesPortName).connectPort(intial_responses[providesDeviceUsedByApplication].allocatedDevice, connectionID)
                        elif providesPortComponent and usesDeviceUsedByApplication:
                            if not connectionID:
                                connectionID = str(uuid4())
                            if usesDeviceUsedByApplication in intial_responses:
                                usesport = intial_responses[usesDeviceUsedByApplication].allocatedDevice.getPort(usesPortName)
                                if providesPortName:
                                    log.debug('creating connection usesDeviceUsedByApplication {}/{} to component (1) {}/{}'.format( usesDeviceUsedByApplication, usesPortName, providesPortComponent, providesPortName))
                                    usesport.connectPort(providesPortComponent.getPort(providesPortName), connectionID)
                                else:
                                    log.debug('creating connection usesDeviceUsedByApplication {}/{} to component (2) {} '.format(usesDeviceUsedByApplication, usesPortName, providesPortComponent))
                                    usesport.connectPort(intial_responses[usesDeviceUsedByApplication], connectionID)
                        elif usesPortComponent and providesInterface:
                            if not connectionID:
                                connectionID = str(uuid4())
                            usesport = usesPortComponent.getPort(usesPortName)
                            log.debug('creating connection component {}/{} to providesInterface {}'.format( usesPortComponent, usesPortName, providesInterface))
                            usesport.connectPort(providesInterface, connectionID)
                    else:
                        if connection.get_providesport() != None and connection.get_findby() != None:

                            providesPortComponent = None
                            providesPortName = connection.get_providesport().get_providesidentifier()
                            if connection.get_providesport().get_componentinstantiationref() != None:
                                providesPortComponentRefid = connection.get_providesport().get_componentinstantiationref().get_refid()
                            else:
                                log.warn("Unable to create connection for '%s'",connection.get_providesport().get_providesidentifier())
                                continue
                            log.debug("CONNECTION INTERFACE: provides port name '%s'", providesPortName)
                            log.debug("CONNECTION INTERFACE: provides port component ref '%s'", providesPortComponentRefid)
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid[:3] == 'DCE':
                                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                                else:
                                    comprefid = component._refid.split(':')[0]
                                if comprefid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    break
                            if providesPortComponent and domain_ref :
                                domainfinder = connection.get_findby().get_domainfinder()
                                if domainfinder :
                                    if domainfinder.get_type() == 'eventchannel':
                                        channelname = domainfinder.get_name()
                                        channel = None
                                        try:
                                            channel = domain_ref.domMgr.eventChannelMgr.get(channelname)
                                        except:
                                            pass
                                        if not channel:
                                            channel = domain_ref.domMgr.eventChannelMgr.create(channelname)
                                        consumer_admin = channel.for_consumers()
                                        _proxy_supplier = consumer_admin.obtain_push_supplier()
                                        log.debug('creating connection from eventchannel {}'.format(channelname))
                                        _consumer = providesPortComponent.getPort(providesPortName)
                                        _proxy_supplier.connect_push_consumer(_consumer)

        log.debug('overload properties')
        assemblyComponent=None
        for component in ordered_placements:
            refid = component.componentfileref.refid
            instanceID = component.get_componentinstantiation()[0].id_ + ':' + waveform_context_name
            comprefid = component.get_componentinstantiation()[0].id_
            sandboxComponent = None
            if refid in validRequestedComponents:
                for comp in launchedComponents:
                    if instanceID == comp._refid:
                        sandboxComponent = comp
                        # skip assembly controller for last..
                        if comprefid == assemblyControllerRefid:
                            assemblyComponent=comp
                            sandboxComponent=None
                        break

            if sandboxComponent != None:
                log.debug( 'Configuring placement: {}'.format(sandboxComponent.comp_ns_name.id))
                # get deployment
                cdeploy=None
                if sandboxComponent._instanceName in configurable:
                    cdeploy=configurable[sandboxComponent._instanceName ]
                    if cdeploy:
                        log.debug('configuring context: {} {} {} {} {}'.format(sandboxComponent.comp_ns_name.id,
                                                                       cdeploy.simple_vals,
                                                                       cdeploy.simpleseq_vals,
                                                                       cdeploy.struct_vals,
                                                                       cdeploy.structseq_vals))
                        overloadProperty(sandboxComponent,
                                         cdeploy.simple_vals,
                                         cdeploy.simpleseq_vals,
                                         cdeploy.struct_vals,
                                         cdeploy.structseq_vals)

        if assemblyComponent:
            cdeploy=None
            if assemblyComponent._instanceName in configurable:
                cdeploy=configurable[assemblyComponent._instanceName ]
                log.debug('configuring AssemblyController: {} {} {} {} {}'.format(assemblyComponent.comp_ns_name.id,
                                                                   cdeploy.simple_vals,
                                                                   cdeploy.simpleseq_vals,
                                                                   cdeploy.struct_vals,
                                                                   cdeploy.structseq_vals))
                overloadProperty(assemblyComponent,
                                 cdeploy.simple_vals,
                                 cdeploy.simpleseq_vals,
                                 cdeploy.struct_vals,
                                 cdeploy.structseq_vals)


        # establish uses device relationship from the spd file with overloaded values
        for component in ordered_placements:
            # If component has a valid SPD file (isKickable), launch it
            refid = component.componentfileref.refid

            live_component = None
            for comp in launchedComponents:
                if instanceID == comp._refid:
                    live_component = comp

            if refid in validRequestedComponents:
                instanceName = component.get_componentinstantiation()[0].get_usagename()
                comprefid = component.get_componentinstantiation()[0].id_
                instanceID = component.get_componentinstantiation()[0].id_ + ':' + waveform_context_name
                properties=component.get_componentinstantiation()[0].get_componentproperties()
                #simples
                spd = validRequestedComponents[refid]
                parsed_spd = parsers.spd.parse(spd)

                spd_usesdevs = parsed_spd.get_implementation()[0].get_usesdevice()
                for spd_usesdev in spd_usesdevs:
                    usesdevs[spd_usesdev.get_id()] = {}
                    for prop in spd_usesdev.propertyref:
                        tmp_value = prop.get_value()
                        if '__MATH__' in tmp_value:
                            # resolve math issue
                            pass
                        usesdevs[spd_usesdev.get_id()][prop.get_refid()] = tmp_value
                    for structprop in spd_usesdev.structref:
                        tmp_struct = {}
                        for simpleelem in structprop.simpleref:
                            tmp_value = simpleelem.get_value()
                            if '__MATH__' in tmp_value:
                                # resolve math issue
                                split_command = tmp_value.split(',')
                                operation = split_command[2][0]
                                operand = float(split_command[0].split('(')[1])
                                target_prop = split_command[1]
                                target_value = None
                                for entry in live_component._properties:
                                    print(entry.clean_name, entry.queryValue())
                                    if entry.clean_name == target_prop or entry.id == target_prop:
                                        target_value = entry.queryValue()
                                        break
                                    if type(entry.queryValue()) == dict:
                                        for subentry in entry.queryValue():
                                            if subentry == target_prop:
                                                target_value = entry.queryValue()[subentry]
                                                break
                                if operation == '=':
                                    tmp_value = target_value
                                elif operation == '*':
                                    tmp_value = float(target_value) * float(operand)
                                elif operation == '+':
                                    tmp_value = float(target_value) + float(operand)
                                elif operation == '-':
                                    tmp_value = float(target_value) - float(operand)
                                elif operation == '/':
                                    tmp_value = float(target_value) / float(operand)
                            tmp_struct[simpleelem.get_refid()] = tmp_value
                        for simpleseqelem in structprop.simplesequenceref:
                            tmp_values = simpleseqelem.get_values()
                            tmp_seq = []
                            for tmp_value in tmp_values.get_value():
                                tmp_seq.append(tmp_value)
                            tmp_struct[simpleseqelem.get_refid()] = tmp_seq
                        usesdevs[spd_usesdev.get_id()][structprop.get_refid()] = tmp_struct

        log.debug('usesdevice from MATH')
        # establish uses device relationship from the sad file
        for usedevs in sad.usesdevicedependencies:
            for usedev in usedevs.usesdevice:
                usesdevs[usedev.get_id()] = {}
                for prop in usedev.propertyref:
                    usesdevs[usedev.get_id()][prop.get_refid()] = prop.get_value()

        responses = {}
        for usesdevid in usesdevs:
            if usesdevid in intial_responses:
                continue
            if domain_ref:
                request = [createRequest(usesdevid, core_properties.props_from_dict(usesdevs[usesdevid]))]
                log.debug( 'request: {}'.format(request))
                response = domain_ref.domMgr._get_allocationMgr().allocate(request)
                if len(response) == 1:
                    responses[usesdevid]=response[0]
                else:
                    for resp in responses:
                        domain_ref.domMgr._get_allocationMgr().deallocate(responses[resp].allocationID)
                    msg=" ERROR - Failed to allocate: " + str(usesdevs[usesdevid])
                    raise RuntimeError(msg)

        for component in componentPlacements:
            refid = component.componentfileref.refid
            assemblyController = False
            sandboxComponent = None
            if refid in validRequestedComponents:
                instanceID = component.get_componentinstantiation()[0].id_ + ':' + waveform_context_name
                componentProps = None
                if len(launchedComponents) > 0:
                    for comp in launchedComponents:
                        if instanceID == comp._refid:
                            sandboxComponent = comp
                    try:
                        destfile = cStringIO.StringIO()
                        sandboxComponent.api(destfile)
                        log.debug('--- component {} api {}'.format(sandboxComponent.comp_ns_name.id, destfile.getvalue()))
                    except:
                        pass

        # Set up component connections
        if sad.connections:
            for connection in sad.connections.get_connectinterface():
                print('evaluating connection', connection.get_id())
                if connection != None:
                    connectionID = None
                    if connection.get_id():
                        connectionID = connection.get_id()
                    log.debug("CONNECTION INTERFACE: connection ID '%s'", connection.get_id())
                    usesPortComponent = None
                    usesPortName = None
                    providesPortComponent = None
                    providesPortName = None
                    providesPortComponentRefid = None
                    usesPortComponentRefid = None
                    usesDeviceUsedByApplication = None
                    providesDeviceUsedByApplication = None
                    providesInterface = None
                    # Check for uses port
                    if connection.get_usesport() != None:
                        usesPortName = connection.get_usesport().get_usesidentifier()
                        if connection.get_usesport().get_componentinstantiationref() != None:
                            usesPortComponentRefid = connection.get_usesport().get_componentinstantiationref().get_refid()
                        elif connection.get_usesport().get_deviceusedbythiscomponentref() != None:
                            usesDeviceUsedByApplication = connection.get_usesport().get_deviceusedbythiscomponentref().get_usesrefid()
                        elif connection.get_usesport().get_deviceusedbyapplication() != None:
                            usesDeviceUsedByApplication = connection.get_usesport().get_deviceusedbyapplication().get_usesrefid()
                        else:
                            log.warn("Unable to create connection for '%s'",connection.get_usesport().get_usesidentifier())
                            continue
                        log.debug("CONNECTION INTERFACE: uses port name '%s'", usesPortName)
                        log.debug("CONNECTION INTERFACE: uses port component ref '%s'", usesPortComponentRefid)
                        # Loop through launched components to find one containing the uses port to be connected
                        for component in launchedComponents:
                            if component._refid[:3] == 'DCE':
                                comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                            else:
                                comprefid = component._refid.split(':')[0]
                            if comprefid == usesPortComponentRefid:
                                usesPortComponent = component
                                break

                        # Look for end point of the connection
                        # Check for provides port
                        if connection.get_providesport() != None:
                            providesPortName = connection.get_providesport().get_providesidentifier()
                            if connection.get_providesport().get_componentinstantiationref() != None:
                                providesPortComponentRefid = connection.get_providesport().get_componentinstantiationref().get_refid()
                            elif connection.get_providesport().get_deviceusedbythiscomponentref() != None:
                                providesDeviceUsedByApplication = connection.get_providesport().get_deviceusedbythiscomponentref().get_usesrefid()
                            elif connection.get_providesport().get_deviceusedbyapplication() != None:
                                providesDeviceUsedByApplication = connection.get_providesport().get_deviceusedbyapplication().get_usesrefid()
                            else:
                                log.warn("Unable to create connection for '%s'",connection.get_providesport().get_providesidentifier())
                                continue
                            log.debug("CONNECTION INTERFACE: provides port name '%s'", providesPortName)
                            log.debug("CONNECTION INTERFACE: provides port component ref '%s'", providesPortComponentRefid)
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid[:3] == 'DCE':
                                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                                else:
                                    comprefid = component._refid.split(':')[0]
                                if comprefid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    break
                        elif connection.get_componentsupportedinterface() != None:
                            if connection.get_componentsupportedinterface().get_componentinstantiationref() != None:
                                providesPortComponentRefid = connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid()
                            elif connection.get_componentsupportedinterface().get_deviceusedbythiscomponentref() != None:
                                providesDeviceUsedByApplication = connection.get_componentsupportedinterface().get_deviceusedbythiscomponentref().get_usesrefid()
                            elif connection.get_componentsupportedinterface().get_deviceusedbyapplication() != None:
                                providesDeviceUsedByApplication = connection.get_componentsupportedinterface().get_deviceusedbyapplication().get_usesrefid()
                            elif domain_ref and connection.get_componentsupportedinterface().get_findby() != None:
                                providesInterface = resolveFindby(connection.get_componentsupportedinterface(), domain_ref, usesPortComponent, usesPortName, connectionID, True)
                                # event channel connection
                                if providesInterface == None:
                                    continue
                            else:
                                log.warn("Unable to create connection")
                                continue
                            if _DEBUG == True:
                                print("loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port interface " + str(connection.get_componentsupportedinterface().get_supportedidentifier()))
                                print("loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port component ref " + str(connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid()))
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid[:3] == 'DCE':
                                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                                else:
                                    comprefid = component._refid.split(':')[0]
                                if comprefid == providesPortComponentRefid:
                                    providesInterface = component.ref
                                    break
                        elif domain_ref and connection.get_findby():
                            providesInterface = resolveFindby(connection, domain_ref, usesPortComponent, usesPortName, connectionID, True)
                            # event channel connection
                            if providesInterface == None:
                                continue

                        # Make connection
                        if usesPortComponent and providesDeviceUsedByApplication:
                            if not connectionID:
                                connectionID = str(uuid4())
                            if providesPortName:
                                if providesDeviceUsedByApplication in responses:
                                    providesport = responses[providesDeviceUsedByApplication].allocatedDevice.getPort(providesPortName)
                                    print('creating connection component to providesDeviceUsedByApplication (1)', usesPortComponent, providesDeviceUsedByApplication, providesPortName, usesPortName)
                                    usesPortComponent.getPort(usesPortName).connectPort(providesport, connectionID)
                                    print('done creating connection')
                            else:
                                if providesDeviceUsedByApplication in responses:
                                    print('creating connection component to providesDeviceUsedByApplication (2)', usesPortComponent, providesDeviceUsedByApplication, usesPortName)
                                    usesPortComponent.getPort(usesPortName).connectPort(responses[providesDeviceUsedByApplication].allocatedDevice, connectionID)
                                    print('done creating connection')
                        elif providesPortComponent and usesDeviceUsedByApplication:
                            if not connectionID:
                                connectionID = str(uuid4())
                            if usesDeviceUsedByApplication in responses:
                                usesport = responses[usesDeviceUsedByApplication].allocatedDevice.getPort(usesPortName)
                                if providesPortName:
                                    print('creating connection usesDeviceUsedByApplication to component (1)', usesDeviceUsedByApplication, providesPortComponent, providesPortName, usesPortName)
                                    usesport.connectPort(providesPortComponent.getPort(providesPortName), connectionID)
                                    print('done creating connection')
                                else:
                                    print('creating connection usesDeviceUsedByApplication to component (2)', usesDeviceUsedByApplication, providesPortComponent, usesPortName)
                                    usesport.connectPort(responses[usesDeviceUsedByApplication], connectionID)
                                    print('done creating connection')

        responses.update(intial_responses)
        app.alloc_responses = responses

        launchedComponents = []
    except Exception as e:
        traceback.print_exc()
        msg=" ERROR - Failed to load sad file: " + str(wf) + " REASON: " + str(e)
        for lc in launchedComponents:
            lc.releaseObject()
        raise RuntimeError(msg)

    if len(props) != 0:
        pnames=""
        for prop in props:
            pnames += ", " + prop
            print("Overload property '"+prop+"' was ignored because it is not on the Assembly Controller")

    app.refreshComponents()
    try:
        if domain_ref:
            domain_ref.registerComponent(instanceName, app._this())
    except Exception as e:
        print('failed to register the application', instanceName, 'with exception', str(e))
    app_ior = orb.object_to_string(app._this())
    cluster = None

    return app

def loadSADFile(filename, props={}):
    '''
    Load the graph/configuration described in the SAD file onto the sandbox
    
    Parameters:
    ----------
    filename - string
               Path to the sad.xml file
    props    - dict
               Property dictionary of name/value pairs for Assembly Controller and External properties. { 'prop1' : value1 }
    
    Returns:
    --------
    True  - If the SAD file was loaded and deployed.

    Exceptions:
    RuntimeError - An error occurred during execution

    Warning - The SAD file was deployed but not all property overloads were processed.
    '''
    log.debug("loading SAD file '%s'", filename)
    launchedComponents = []
    try:
        sandbox = _getSandbox()
        sdrroot = sandbox.getSdrRoot()
        if type(props) != dict:
            print("loadSADFile(): props argument must be a dictionary. Ignoring overload")
            props = {}
        sadFile = open(filename,'r')
        sadFileString = sadFile.read()
        sad = parsers.sad.parseString(sadFileString)
        log.debug("waveform ID '%s'", sad.get_id())
        log.debug("waveform name '%s'", sad.get_name())
        waveform_modifier = ':'+sad.get_name()
        validRequestedComponents = {}
        # Loop over each <componentfile> entry to determine SPD filenames and which components are kickable
        for component in sad.componentfiles.get_componentfile():
            log.debug("COMPONENT FILE localfile '%s'", component.get_localfile().get_name())
            log.debug("COMPONENT FILE id '%s'", component.get_id())
            log.debug("COMPONENT FILE type '%s'", component.get_type())
            try:
                localfile = 'dom' + component.get_localfile().get_name()
                spdFilename = sdrroot.findProfile(localfile,objType="components")
                log.debug("Found softpkg '%s'", spdFilename)
                validRequestedComponents[component.get_id()] = spdFilename
            except:
                log.warning("Could not find softpkg '%s'", component.get_localfile().get_name())

        # Need to determine which component is the assembly controller
        assemblyControllerRefid = None
        if sad.assemblycontroller:
            assemblyControllerRefid = sad.assemblycontroller.get_componentinstantiationref().get_refid() + waveform_modifier
            log.debug("ASSEMBLY CONTROLLER component instantiation ref '%s'", assemblyControllerRefid)
        if not assemblyControllerRefid:
            log.warning('SAD file did not specify an assembly controller')

        #
        # Allow for external properties to be set
        # 
        externprops=[]
        if sad.get_externalproperties():
            externprops=[ { 'comprefid' : x.comprefid + waveform_modifier, 'propid' : x.propid, 'externalpropid' : x.externalpropid } for x in sad.get_externalproperties().get_property() ]
            log.debug( "External Props: %s", externprops )

        # Loop over each <componentplacement> entry to determine actual instance name for component
        # NOTE: <componentplacement> can also occur within <hostcollocation> elements if that exists
        #       so that needs to be checked also
        componentPlacements = sad.partitioning.get_componentplacement()
        for hostCollocation in sad.get_partitioning().get_hostcollocation():
            componentPlacements.extend(hostCollocation.get_componentplacement())

        log.debug("Creating start order for: %s", filename )
        startorder={}
        for c in componentPlacements:
            log.debug("COMPONENT PLACEMENT component instantiation id '%s'", c.get_componentinstantiation()[0].id_)
            seq = None
            try:
                seq = c.get_componentinstantiation()[0].get_startorder()
                seq = int(seq)
                log.debug("COMPONENT PLACEMENT component startorder '%s'", str(seq))
            except:
                seq=-1
            startorder.setdefault( seq, [])
            startorder[ seq ].append( c )

        # flatten dictionary to an ordered list
        ordered_placements=[]
        for k,v in list(startorder.items()):
            ordered_placements = ordered_placements + v

        configurable = {}
        for component in ordered_placements:
            log.debug("COMPONENT PLACEMENT component spd file id '%s'", component.componentfileref.refid)
            log.debug("COMPONENT PLACEMENT component instantiation id '%s'", component.get_componentinstantiation()[0].id_)
            log.debug("COMPONENT PLACEMENT component startorder '%s'", component.get_componentinstantiation()[0].get_startorder())
            log.debug("COMPONENT PLACEMENT component name '%s'", component.get_componentinstantiation()[0].get_usagename())
            # If component has a valid SPD file (isKickable), launch it
            refid = component.componentfileref.refid
            if refid in validRequestedComponents:
                instanceName = component.get_componentinstantiation()[0].get_usagename()
                instanceID = component.get_componentinstantiation()[0].id_ + waveform_modifier
                log.debug("launching component '%s'", instanceName)
                properties=component.get_componentinstantiation()[0].get_componentproperties()
                #simples
                spd = validRequestedComponents[refid]
                parsed_spd = parsers.spd.parse(spd)
                componentName = parsed_spd.get_name()
                pathToComponentXML = os.path.dirname(spd)
                prfFilename = pathToComponentXML+'/'+parsed_spd.get_propertyfile().get_localfile().name
                _prf = parsers.prf.parse(prfFilename)
                execprops = []
                configurable[instanceName] = []
                for prop_check in _prf.get_simple():
                    if prop_check.get_kind()[0].get_kindtype() == 'execparam':
                        execprops.append(str(prop_check.get_id()))
                    if prop_check.get_kind()[0].get_kindtype() == 'configure':
                        if prop_check.get_mode() == 'readwrite' or prop_check.get_mode() == 'writeonly':
                            configurable[instanceName].append(str(prop_check.get_id()))
                    if prop_check.get_kind()[0].get_kindtype() == 'property':
                        if prop_check.get_commandline() == 'true':
                            execprops.append(str(prop_check.get_id()))
                        if prop_check.get_mode() == 'readwrite' or prop_check.get_mode() == 'writeonly':
                            configurable[instanceName].append(str(prop_check.get_id()))
                for prop_check in _prf.get_simplesequence():
                    if prop_check.get_kind()[0].get_kindtype() == 'configure':
                        if prop_check.get_mode() == 'readwrite' or prop_check.get_mode() == 'writeonly':
                            configurable[instanceName].append(str(prop_check.get_id()))
                for prop_check in _prf.get_simplesequence():
                    if prop_check.get_kind()[0].get_kindtype() == 'property':
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
                    if simple.refid in props and instanceID == assemblyControllerRefid:
                        overload_value = props[simple.refid]
                        props.pop(simple.refid)
                    container = overloadContainer(str(simple.refid),overload_value)
                    simple_exec_vals[container.id] = container.value
                for ep in externprops:
                    if ep['comprefid'] != instanceID:
                        continue
                    if ep['propid'] in execprops and ep['externalpropid'] in props:
                        overload_value = props[ep['externalpropid']]
                        props.pop(ep['externalpropid'])
                        container = overloadContainer(str(ep['propid']), overload_value)
                        simple_exec_vals[container.id] = container.value
                # If AC execparam property is overriden in props but not SAD file, update value
                for prop in list(props):
                    if prop in execprops and instanceID == assemblyControllerRefid:
                        overload_value = props[prop]
                        props.pop(prop)
                        container = overloadContainer(str(prop),overload_value)
                        simple_exec_vals[container.id] = container.value
                try:
                    # NB: Explicitly request no configure call is made on the component
                    newComponent = launch(componentName, instanceName, instanceID, configure=False, properties=simple_exec_vals, objType="components")
                    newComponent.is_assemblycontroller = instanceID==assemblyControllerRefid
                    launchedComponents.append(newComponent)
                except Exception as e:
                    msg = "Failed to launch component '%s', REASON: %s" %  (instanceName, str(e))
                    print(msg)
                    raise RuntimeError(msg)

        # Set up component connections
        if sad.connections:
            for connection in sad.connections.get_connectinterface():
                if connection != None:
                    connectionID = None
                    if connection.get_id():
                        connectionID = connection.get_id()
                    log.debug("CONNECTION INTERFACE: connection ID '%s'", connection.get_id())
                    usesPortComponent = None
                    usesPortName = None
                    providesPortComponent = None
                    providesPortName = None
                    providesPortComponentRefid = None
                    usesPortComponentRefid = None
                    # Check for uses port
                    if connection.get_usesport() != None:
                        usesPortName = connection.get_usesport().get_usesidentifier()
                        if connection.get_usesport().get_componentinstantiationref() != None:
                            usesPortComponentRefid = connection.get_usesport().get_componentinstantiationref().get_refid()
                        else:
                            log.warning("Unable to create connection for '%s'",connection.get_usesport().get_usesidentifier())
                            continue
                        log.debug("CONNECTION INTERFACE: uses port name '%s'", usesPortName)
                        log.debug("CONNECTION INTERFACE: uses port component ref '%s'", usesPortComponentRefid)
                        # Loop through launched components to find one containing the uses port to be connected
                        for component in launchedComponents:
                            if component._refid[:3] == 'DCE':
                                comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                            else:
                                comprefid = component._refid.split(':')[0]
                            if comprefid == usesPortComponentRefid:
                                usesPortComponent = component
                                break

                        # Look for end point of the connection
                        # Check for provides port 
                        if connection.get_providesport() != None:
                            providesPortName = connection.get_providesport().get_providesidentifier()
                            if connection.get_providesport().get_componentinstantiationref() != None:
                                providesPortComponentRefid = connection.get_providesport().get_componentinstantiationref().get_refid()
                            else:
                                log.warning("Unable to create connection for '%s'",connection.get_providesport().get_providesidentifier())
                                continue
                            log.debug("CONNECTION INTERFACE: provides port name '%s'", providesPortName)
                            log.debug("CONNECTION INTERFACE: provides port component ref '%s'", providesPortComponentRefid)
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid[:3] == 'DCE':
                                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                                else:
                                    comprefid = component._refid.split(':')[0]
                                if comprefid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    break
                        elif connection.get_componentsupportedinterface() != None:
                            if connection.get_componentsupportedinterface().get_componentinstantiationref() != None:
                                providesPortComponentRefid = connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid()
                            else:
                                log.warning("Unable to create connection")
                                continue
                            if _DEBUG == True:
                                print("loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port interface " + str(connection.get_componentsupportedinterface().get_supportedidentifier()))
                                print("loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port component ref " + str(connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid()))
                            # Loop through launched components to find one containing the provides port to be connected
                            for component in launchedComponents:
                                if component._refid[:3] == 'DCE':
                                    comprefid = component._refid.split(':')[0]+':'+component._refid.split(':')[1]
                                else:
                                    comprefid = component._refid.split(':')[0]
                                if comprefid == providesPortComponentRefid:
                                    providesPortComponent = component
                                    break
                        elif connection.get_findby():
                            log.debug("CONNECTION INTERFACE: findby '%s'", connection.get_componentsupportedinterface())

                        # Make connection
                        if usesPortComponent and providesPortComponent:
                            log.debug("calling connect on usesPortComponent")
                            if providesPortName:
                                log.debug("passing in to connect() providesPortName '%s'", providesPortName)
                            if usesPortName:
                                log.debug("passing in to connect() usesPortName '%s'", usesPortName)
                            if connectionID:
                                log.debug("passing in to connect() connectionID '%s'", connectionID)
                            # NB: None values will fall back to connect() defaults.
                            usesPortComponent.connect(providesPortComponent, providesPortName, usesPortName, connectionID)

        # configure properties for launched component
        for component in componentPlacements:
            refid = component.componentfileref.refid
            assemblyController = False
            sandboxComponent = None
            if refid in validRequestedComponents:
                instanceID = component.get_componentinstantiation()[0].id_ + waveform_modifier
                componentProps = None
                if len(launchedComponents) > 0:
                    for comp in launchedComponents:
                        if instanceID == comp._refid:
                            componentProps = comp._getInitialConfigureProperties()
                            if instanceID == assemblyControllerRefid:
                                assemblyController = True
                            sandboxComponent = comp
                            break

            if sandboxComponent != None and componentProps != None:
                properties=component.get_componentinstantiation()[0].get_componentproperties()
                simple_vals=[]
                simpleseq_vals=[]
                struct_vals=[]
                structseq_vals = []
                if properties != None:
                    #simples
                    simples = properties.get_simpleref()
                    for simple in simples:
                        if not (simple.refid in configurable[sandboxComponent._instanceName]):
                            continue
                        overload_value = str(simple.value)
                        if simple.refid in props and assemblyController:
                            overload_value = props[simple.refid]
                            props.pop(simple.refid)
                        for ep in externprops:
                            if simple.refid == ep['propid'] and instanceID == ep['comprefid']:
                                if ep['externalpropid'] in props:
                                    overload_value = props[ep['externalpropid']]
                                    log.debug('overriding external simple property ' + str(ep) )
                                    props.pop(ep['externalpropid'])
                        simple_vals.append(overloadContainer(str(simple.refid),overload_value))
                    #simple sequences
                    simpleseqs = properties.get_simplesequenceref()
                    if simpleseqs != None:
                        for simpleseq in simpleseqs:
                            if not (simpleseq.refid in configurable[sandboxComponent._instanceName]):
                                continue
                            values_vals = [str(value) for value in simpleseq.get_values().get_value()]
                            if simpleseq.refid in props and assemblyController:
                                values_vals = props[simpleseq.refid]
                                props.pop(simpleseq.refid)
                            for ep in externprops:
                                if simpleseq.refid == ep['propid'] and instanceID == ep['comprefid']:
                                    if ep['externalpropid'] in props:
                                        overload_value = props[ep['externalpropid']]
                                        log.debug('overriding external simple sequence property ' + str(ep) )
                                        props.pop(ep['externalpropid'])

                            simpleseq_vals.append(overloadContainer(str(simpleseq.refid),values_vals))
                    #structs
                    structs = properties.get_structref()
                    if structs != None:
                        for struct in structs:
                            if not (struct.refid in configurable[sandboxComponent._instanceName]):
                                continue
                            value = {}
                            value.update(_loadStructMembers(struct))
                            if struct.refid in props and assemblyController:
                                value = props[struct.refid]
                                props.pop(struct.refid)
                            for ep in externprops:
                                if struct.refid == ep['propid'] and instanceID == ep['comprefid']:
                                    if ep['externalpropid'] in props:
                                        overload_value = props[ep['externalpropid']]
                                        log.debug('overriding external structure property ' + str(ep) )
                                        props.pop(ep['externalpropid'])
                            struct_vals.append(overloadContainer(str(struct.refid),value))
                    #struct sequences
                    structseqs = properties.get_structsequenceref()
                    if structseqs != None:
                        for structseq in structseqs:
                            if not (structseq.refid in configurable[sandboxComponent._instanceName]):
                                continue
                            values_vals = []
                            for struct_template in structseq.get_structvalue():
                                value = {}
                                value.update(_loadStructMembers(struct_template))
                                values_vals.append(value)
                            if structseq.refid in props and assemblyController:
                                values_vals = props[structseq.refid]
                                props.pop(structseq.refid)
                            for ep in externprops:
                                if structseq.refid == ep['propid'] and instanceID == ep['comprefid']:
                                    if ep['externalpropid'] in props:
                                        overload_value = props[ep['externalpropid']]
                                        log.debug('overriding external structured sequence property ' + str(ep) )
                                        props.pop(ep['externalpropid'])
                            structseq_vals.append(overloadContainer(str(structseq.refid),values_vals))

                # for external properties that do not have sad file overrides, process them here
                if len(props) > 0 :
                    prop_types = { 'simple' : ([],[]), 'simpleseq' : ([],[]), 'struct' : ([],[]), 'structseq' : ([],[]) }
                    prop_set = sandboxComponent._prf.get_simple()
                    prop_types['simple'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                    prop_set = sandboxComponent._prf.get_simplesequence()
                    prop_types['simpleseq'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                    prop_set = sandboxComponent._prf.get_struct()
                    prop_types['struct'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                    prop_set = sandboxComponent._prf.get_structsequence()
                    prop_types['structseq'] = ([str(prop_iter.get_id()) for prop_iter in prop_set], [])
                    for ep in externprops:
                        prop = ep['propid']
                        ext_prop_id=ep['externalpropid']
                        if instanceID != ep['comprefid'] or ext_prop_id not in props:
                            continue

                        for prop_type in prop_types:
                            if prop in prop_types[prop_type][0] and ext_prop_id in props:
                                prop_types[prop_type][1].append(overloadContainer(str(prop), props[ext_prop_id]))
                                props.pop( ext_prop_id )

                    simple_vals += prop_types['simple'][1]
                    simpleseq_vals += prop_types['simpleseq'][1]
                    struct_vals += prop_types['struct'][1]
                    structseq_vals += prop_types['structseq'][1]

                if len(sandboxComponent._properties) > 0:
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
                    for prop_check in sandboxComponent._properties:
                        if prop_check.id == prop:
                            for prop_type_check in prop_types:
                                if prop in prop_types[prop_type_check][0]:
                                    prop_types[prop_type_check][1].append(overloadContainer(str(prop), props[prop]))
                                    prop_to_pop.append(str(prop))
                for prop_to_pop_iter in prop_to_pop:
                    props.pop(prop_to_pop_iter)
                if _DEBUG:
                    print("OverLoad Assembly Controller ", (sandboxComponent, prop_types['simple'][1], prop_types['simpleseq'][1], prop_types['struct'][1], prop_types['structseq'][1]))
                overloadProperty(sandboxComponent, prop_types['simple'][1], prop_types['simpleseq'][1], prop_types['struct'][1], prop_types['structseq'][1])

        launchedComponents = []
    except Exception as e:
        msg=" ERROR - Failed to load sad file: " + str(filename) + " REASON: " + str(e)
        for lc in launchedComponents:
            lc.releaseObject()
        raise RuntimeError(msg)

    if len(props) != 0:
        pnames=""
        for prop in props:
            pnames += ", " + prop
            print("Overload property '"+prop+"' was ignored because it is not on the Assembly Controller")
        raise Warning('Requested property overloads not assigned' + pnames )
    return True

def createRequest(requestId, props, pools=[], devices=[], sourceId=''):
    return _CF.AllocationManager.AllocationRequestType(requestId, props, pools, devices, sourceId)

def logRotator(logrotate_cfg):
    while True:
        command,status = subprocess.getstatusoutput("/usr/sbin/logrotate "+logrotate_cfg)
        time.sleep(30)

def resolveFindby(xml_root_node, domain_ref, usesPortComponent, usesPortName, connectionID, noevent=False):
    domainfinder = xml_root_node.get_findby().get_domainfinder()
    providesInterface = None
    if domainfinder:
        if domainfinder.get_type() == 'eventchannel':
            if noevent:
                return None
            channelname = domainfinder.get_name()
            channel = None
            try:
                channel = domain_ref.domMgr.eventChannelMgr.get(channelname)
            except:
                pass
            if not channel:
                channel = domain_ref.domMgr.eventChannelMgr.create(channelname)
            _supplier = usesPortComponent.getPort(usesPortName)
            print('establishing connection to eventchannel', channel)
            _supplier.connectPort(channel, connectionID)
        elif domainfinder.get_type() == 'filemanager':
            providesInterface = domain_ref.domMgr.fileMgr
        elif domainfinder.get_type() == 'servicename':
            service_name = domainfinder.get_name()
            for dev_mgr in domain_ref.domMgr.deviceManagers:
                for serv in dev_mgr.registeredServices:
                    if service_name == serv.serviceName:
                        providesInterface = serv.serviceObject
                        break
                if providesInterface:
                    break
        elif domainfinder.get_type() == 'servicetype':
            service_type = domainfinder.get_name()
            for dev_mgr in domain_ref.domMgr.deviceManagers:
                for serv in dev_mgr.registeredServices:
                    if serv.serviceObject._is_a(service_type):
                        providesInterface = serv.serviceObject
                        break
                if providesInterface:
                    break
        elif domainfinder.get_type() == 'domainmanager':
            providesInterface = domain_ref.domMgr
        elif domainfinder.get_type() == 'application':
            applications = domain_ref.domMgr.applications
            application_id = domainfinder.get_name()
            for app in applications:
                if app.get_identifier() == application_id:
                    providesInterface = app
                    break
    return providesInterface

class LocalApplicationStub(_CF__POA.Application):
    def __init__(self,
                 name="",
                 instanceName=None,
                 sad=None,
                 profile='',
                 identifier=None,
                 logger=None,
                 domain=None,
                 use_inspoa=False):
        # __initialized needs to be set first to prevent __setattr__ from entering an error state
        self.log = logger
        self.__initialized = False
        self.name = name
        self._instanceName = instanceName
        if not self._instanceName:
            if name == "" or not name:
                name = str(uuid4())
            self._instanceName = name
        self.sandbox = _getSandbox()
        self.__components=[]
        self._profile = profile
        self.ports = []
        self._portsUpdated = False
        self._domain = domain
        self._ns_name =self._instanceName
        if domain:
            self._ns_name = domain.name+'/'+self._instanceName
        self._sad = sad
        self._externalProps = self._getExternalProperties()
        self.adhocConnections = []
        self._connectioncount = 0
        self.assemblyController = None
        self._acRef = None
        self._propertyChangeRegistrations = {}
        self._stoptimeout = 0
        self.initial_props = None
        self._start_order = None
        try:
            if not identifier:
                self._id = str(self._sad.get_id()) + ":" + str(self._instanceName)
            else:
                self._id = identifier
        except:
            pass
        self.__initialized = True
        self.alloc_responses = {}
        self.refreshComponents()
        if use_inspoa:
            try:
                _obj_inspoa.activate_object_with_id(self._ns_name,self)
            except:
                 traceback.print_exc()
                 raise

    @property
    def comps(self):
        if len(self.__components) == 0:
            self.refreshComponents()
        return self.__components

    def setStartOrder(self, startOrder ):
        self._start_order=[]
        for c in startOrder:
            # this is fragile if the caller changes the naming pattern..
            self._start_order.append( c.get_componentinstantiation()[0].id_ + ':' + self._instanceName )

    def refreshComponents(self):
        tmp_comps = []
        sb_comps= self.sandbox.getComponents()
        if self._start_order:
            for cid in self._start_order:
                try:
                    c=[ c for c in sb_comps if hasattr(c,'identifier') and cid == c.identifier ][0]
                    tmp_comps.append(c)
                except:
                    pass
        else:
            assem_refid=None
            try:
                assem_refid=self._sad.assemblycontroller.componentinstantiationref.refid
            except:
                pass
            for comp in sb_comps:
                # check if component is part of this application
                if hasattr(comp,'identifier') and self._instanceName in comp.identifier:

                    comp.is_assemblycontroller=False
                    comp.root_context = None
                    comp.naming_context_ref = None

                    # determine assembly controller
                    if hasattr(comp,'instanceName'):
                        comp.comp_ns_name = _CosNaming.NameComponent(comp.instanceName,"")
                        try:
                            if comp.instanceName == assem_refid:
                                self.assemblyController=comp
                                comp.is_assemblycontroller=True
                        except:
                            pass
                    tmp_comps.append(comp)

        self.__components = tmp_comps
        self._externalProps = self._getExternalProperties()
        x=self.getPortSet()

    def _getExternalProperties(self):
        # Return a map with all external properties
        extProps = {}
        if self._sad:
            if self._sad.get_externalproperties():
                for prop in self._sad.get_externalproperties().get_property():
                    propId = prop.get_externalpropid()
                    if not propId:
                        propId = prop.get_propid()
                    extProps[propId] = (prop.get_propid(), prop.get_comprefid())
        return extProps

    ########################################
    # External Application API
    def _get_profile(self):
        return self._profile

    def _get_name(self):
        return self.name

    def _get_stopTimeout(self):
        return self._stoptimeout

    def _set_stopTimeout(self, timeout):
        self._stoptimeout = timeout

    def _get_registeredComponents(self):
        retval = []
        tmp_comps = []
        self.refreshComponents()
        for comp in self.__components:
            ports=[]
            tmp_profile = comp.softwareProfile.replace('/var/redhawk/sdr/dom','')
            c = _CF.ComponentType(comp._id, tmp_profile, _CF.APPLICATION_COMPONENT, comp.ref, ports)
            retval.append(c)
        return retval

    def _get_componentNamingContexts(self):
        retval = []
        for comp in self.__components:
            c = _CF.Application.ComponentElementType(comp._id, self._ns_name+'/'+comp.comp_ns_name.id)
            retval.append(c)
        return retval

    def _get_componentProcessIds(self):
        pids = []
        for comp in self.__components:
            pids.append(_CF.Application.ComponentProcessIdType(comp._id, long(comp._pid)))
        return pids

    def _get_componentDevices(self):
        retval = []
        namespace_fp = open('/var/run/secrets/kubernetes.io/serviceaccount/namespace','r')
        namespace = namespace_fp.read()
        namespace_fp.close()
        for comp in self.__components:
            c = _CF.DeviceAssignmentType(comp._id, os.getenv('MY_NODE_NAME')+'/'+namespace+'.'+os.getenv('HOSTNAME'))
            retval.append(c)
        return retval

    def _get_componentImplementations(self):
        retval = []
        return retval

    def _get_aware(self):
        retval = False
        return retval

    def metrics(self, components, attributes):
        retval = None
        return retval

    @property
    def aware(self):
        return self._get_aware()

    @property
    def componentDevices(self):
        return self._get_componentDevices()

    @property
    def componentImplementations(self):
        return self._get_componentImplementations()

    @property
    def stopTimeout(self):
        return self._get_stopTimeout()

    @property
    def registeredComponents(self):
        return self._get_registeredComponents()

    @property
    def componentNamingContexts(self):
        return self._get_componentNamingContexts()

    @property
    def componentProcessIds(self):
        return self._get_componentProcessIds()

    @property
    def profile(self):
        return self._get_profile()

    def start(self):
        self.refreshComponents()
        if self._start_order:
            for _cid in self._start_order:
                for c in self.__components:
                    if c.identifier == _cid:
                       c.start()
        else:
            for comp in self.__components:
                comp.start()
        #self.sandbox.start()

    def stop(self):
        # RESOLVE - reverse start order
        omniORB.setClientCallTimeout(self._stoptimeout)
        self.refreshComponents()
        sb_comps=[]
        if self._start_order:
            for _cid in reversed(self._start_order):
                for c in self.__components:
                    if c.identifier == _cid:
                        sb_comps.append(c)
        else:
            sb_comps =  reversed(self.__components)

        for comp in sb_comps:
            try:
                comp.stop()
            except:
                pass
        omniORB.setClientCallTimeout(0)


    def _get_identifier(self):
        return self._id

    @property
    def started(self):
        return self._get_started()

    def _get_started(self):
        self.refreshComponents()
        if len(self.__components) != 0:
            _started=True
            for comp in self.__components:
                _started &= self.__components[0].started
            return _started

        return self.sandbox.started

    def _get_softwareProfile(self):
        return self._profile

    #########################################
    # CF::LifeCycle
    def initialize(self):
        pass

    def constructor(self):
        pass

    def releaseObject(self):

        self._stoptimeout=5
        self.stop()
        filenames = []
        directory_name = None
        waveform_context_name =  None
        root_context = None

        for resp in self.alloc_responses:
            if self._domain:
                self._domain._get_allocationMgr().deallocate([self.alloc_responses[resp].allocationID])

        self.refreshComponents()
        for component in self.__components:
            try:
                naming_context_ref = component.naming_context_ref
                root_context = component.root_context
                waveform_context_name = component.waveform_context_name
                comp_ns_name = component.comp_ns_name
                naming_context_ref.unbind([comp_ns_name])
            except:
                pass

            try:
                filenames.append(component._stdout_filename)
                directory_name = filenames[-1][:filenames[-1].rfind('/')]
            except:
                pass

        if waveform_context_name:
            try:
                root_context.unbind(waveform_context_name)
            except:
                pass

        # terminate components for the waveform
        sb_comps=[]
        if self._start_order:
            for _cid in reversed(self._start_order):
                for c in self.__components:
                    if c.identifier == _cid:
                        sb_comps.append(c)
        else:
            sb_comps=self.__components

        omniORB.setClientCallTimeout(self._stoptimeout)
        for comp in sb_comps:
            try:
                comp.releaseObject()
            except:
                pass
        omniORB.setClientCallTimeout(0)

        self.no_under_upper_app_name = self._instanceName.replace('_','-').replace('.','-').replace(':','-')
        self.no_under_name = self.no_under_upper_app_name.lower()

        if directory_name:
            for filename in os.listdir(directory_name):
                try:
                    os.remove(directory_name+'/'+filename)
                except:
                    pass
            try:
                os.rmdir(directory_name)
            except:
                pass

        try:
            tmp_domain_log_config_file = '/tmp/'+self._instanceName+'_default_log_config.cfg'
            os.remove(tmp_domain_log_config_file)
        except:
            pass


    #########################################
    # CF::PortSupplier
    def getPort(self, name):
        if not self._sad.get_externalports():
            # Nothing to do
            raise _CF.PortSupplier.UnknownPort()

        for externalport in self._sad.get_externalports().get_port():
            externalName = None
            portName = None
            instanceId = None
            internalPortName = None
            externalPortName = None
            supportedInterface = False

            if externalport.get_externalname():
                externalPortName = externalport.get_externalname()

            if externalport.get_usesidentifier():
                internalPortName = str(externalport.get_usesidentifier())
            elif externalport.get_providesidentifier():
                internalPortName = str(externalport.get_providesidentifier())
            elif externalport.get_supportedidentifier():
                supportedInterface = True
            else:
                # Invalid XML; this should never occur.
                continue
            portName = internalPortName
            if externalPortName:
                portName = externalPortName
            if portName != name:
                continue
            instanceId = str(externalport.get_componentinstantiationref().get_refid())
            if supportedInterface:
                return comp.ref
            for comp in self.__components:
                if comp.instanceName == instanceId:
                    return comp.getPort(internalPortName)
        raise _CF.PortSupplier.UnknownPort()

    def getPortSet(self):
        portList = []
        if self._sad.get_externalports() == None:
            return portList
        for externalport in self._sad.get_externalports().get_port():
            externalName = None
            portName = None
            instanceId = None
            internalPortName = None
            externalPortName = None
            direction = ''
            repid = ''
            description = ''
            supportedInterface = False

            if externalport.get_externalname():
                externalPortName = externalport.get_externalname()

            if externalport.get_usesidentifier():
                internalPortName = str(externalport.get_usesidentifier())
                direction = _CF.PortSet.DIRECTION_USES
            elif externalport.get_providesidentifier():
                internalPortName = str(externalport.get_providesidentifier())
                direction = _CF.PortSet.DIRECTION_PROVIDES
            elif externalport.get_supportedidentifier():
                supportedInterface = True
            else:
                # invalid XML; this should never occur.
                continue

            portName = internalPortName
            if externalPortName:
                portName = externalPortName

            instanceId = str(externalport.get_componentinstantiationref().get_refid())
            for comp in self.__components:
                if comp.instanceName == instanceId:
                    if supportedInterface:
                        obj_ptr = comp.ref
                        repid = obj_ptr._NP_RepositoryId
                        info = _CF.PortSet.PortInfoType(obj_ptr, portName, repid, description, direction)
                        portList.append(info)

                        # add to ports...
                        try:
                            for p in comp.ports:
                                if portName == p.name:
                                     if p not in self.ports:
                                         self.ports.append(p)
                        except:
                            pass
                        break
                    comp_port_set = comp.getPortSet()
                    found_port = None
                    for port in comp_port_set:
                        if port.name == internalPortName:
                            found_port = port
                            # add to ports...
                            try:
                                for p in comp.ports:
                                    if portName == p.name:
                                         if p not in self.ports:
                                             self.ports.append(p)
                            except:
                                pass
                            break
                    if not found_port:
                        #print 'could not find port', internalPortName, ' in ', comp_port_set
                        continue
                    found_port.name = portName
                    portList.append(found_port)
                    break
        return portList

    def __loadPorts(self):
        self.__ports = {}
        for name in dir(type(self)):
            attr = getattr(type(self), name)
            if isinstance(attr, _port):
                self.__ports[attr.name] = attr

    #########################################
    #  Common resource logging API

    # return logger assigned to resource
    def getLogger(self, logid=None):
        return self._log

    # return a named logger
    def getNamedLogger(self, logid, assignToResource=False):
        for component in self.__components:
            try:
                comp_logger = component.getNamedLogger(logid)
            except:
                continue
            if comp_logger:
                return comp_logger
        newLogger=logging.getLogger(logid)
        return newLogger

    # Apply a resource context to the set of logging Macros
    def setLoggingMacros(self, newTbl, applyCtx=False ):
        pass

    # Apply a resource context to the set of logging Macros
    def setResourceContext(self, rscCtx ):
        pass

    def setLoggingContext(self, logcfg_url, oldstyle_loglevel, rscCtx ):
        pass

    def saveLoggingContext(self, logcfg_url, oldstyle_loglevel, rscCtx ):
        pass

    def setLogListenerCallback(self, logListenerCB ):
        self.logListenerCallback=logListenerCB

    def addPropertyChangeListener(self, id, callback):
        # self._props.addChangeListener(callback, id)
        pass

    #########################################
    # CF::LogConfiguration
    def _get_log_level(self):
        return self._logLevel

    def _set_log_level(self, newLogLevel ):
        self.log_level( newLogLevel )

    def log_level(self, newLogLevel=None ):
        return self._logLevel

    def setLogLevel(self, logid, newLogLevel ):
        for component in self.__components:
            try:
                component.setLogLevel(logid, newLogLevel)
                return
            except:
                continue
        raise _CF.UnknownIdentifier()

    def getLogLevel(self, logid ):
        for component in self.__components:
            try:
                return component.getLogLevel(logid)
            except:
                continue
        raise _CF.UnknownIdentifier()

    def getNamedLoggers(self):
        retval = []
        for component in self.__components:
            comp_loggers = component.getNamedLoggers()
            for comp_logger in comp_loggers:
                if not (comp_logger in retval):
                    retval.append(comp_logger)
        return retval

    def resetLog(self):
        for component in self.__components:
            try:
                return component.resetLog()
            except:
                continue

    def getLogConfig(self):
        return self.logConfig

    def setLogConfig(self, new_log_config):
        pass

    def setLogConfigURL(self, log_config_url):
        pass

    #########################################
    # CF::LogEventConsumer
    def retrieve_records(self, howMany, startingRecord ):
        howMany=0
        return None

    def retrieve_records_by_date(self, howMany, to_timetStamp ):
        howMany=0
        return None

    def retrieve_records_from_date(self, howMany, from_timetStamp ):
        howMany=0
        return None

    #########################################
    # CF::TestableObject
    def runTest(self, testid, properties):
        raise _CF.TestableObject.UnknownTest()

    _propertyQueryTimestamp = 'QUERY_TIMESTAMP'

    #########################################
    # CF::PropertySet
    def query(self, configProperties):
        retval = []
        extProps = {}
        if len(configProperties) == 0:
            for comp in self.__components:
                if comp.is_assemblycontroller:
                    retval = comp.query([])
            if self._sad.get_externalproperties():
                for prop in self._sad.get_externalproperties().get_property():
                    propId = str(prop.get_propid())
                    external_propId = propId
                    if prop.get_externalpropid():
                        external_propId = str(prop.get_externalpropid())
                    for comp in self.__components:
                        if comp.instanceName == str(prop.get_comprefid()):
                            retval.append(comp.query([_CF.DataType(id=propId, value=any.to_any(None))])[0])
                            retval[-1].id = external_propId
        else:
            if self._sad.get_externalproperties():
                for prop in self._sad.get_externalproperties().get_property():
                    propId = str(prop.get_propid())
                    external_propId = propId
                    if prop.get_externalpropid():
                        external_propId = str(prop.get_externalpropid())
                    for comp in self.__components:
                        if comp.instanceName == prop.get_comprefid():
                            extProps[external_propId] = (comp, propId)
                            break
            for comp in self.__components:
                if comp.is_assemblycontroller:
                    assembly_comp_props = comp.query([])
                    for prop in assembly_comp_props:
                        extProps[prop.id] = (comp, prop.id)
            for cProp in configProperties:
                if not extProps.get(cProp.id):
                    raise _CF.UnknownProperties(configProperties)
                compref = extProps[cProp.id][0]
                comp_propid = extProps[cProp.id][1]
                retval.append(compref.query([_CF.DataType(id=comp_propid, value=any.to_any(None))])[0])
        return retval

    def initializeProperties(self, ctorProps):
        self.initial_props = core_properties.props_to_dict(ctorProps)

    def configure(self, configProperties):
        extProps = {}
        if self._sad.get_externalproperties():
            for prop in self._sad.get_externalproperties().get_property():
                propId = str(prop.get_propid())
                external_propId = propId
                if prop.get_externalpropid():
                    external_propId = str(prop.get_externalpropid())
                for comp in self.__components:
                    if comp.instanceName == str(prop.get_comprefid()):
                        extProps[external_propId] = (comp, propId)
                        break
        assembly_comp = None
        for comp in self.__components:
            if comp.is_assemblycontroller:
                assembly_comp = comp
                break
        for cProp in configProperties:
            if not extProps.get(cProp.id):
                if assembly_comp:
                    assembly_comp.configure([cProp])
            else:
                compref = extProps[cProp.id][0]
                comp_propid = extProps[cProp.id][1]
                send_prop = _CF.DataType(id=str(comp_propid), value=cProp.value)
                compref.configure([send_prop])

    def registerPropertyListener(self, listener, prop_ids, interval):
        assembly_comp = None
        for comp in self.__components:
            if comp.is_assemblycontroller:
                assembly_comp = comp
                break
        current_registrations = {}
        for prop_id in prop_ids:
            registered = False
            if self._sad.get_externalproperties():
                for prop in self._sad.get_externalproperties().get_property():
                    propId = str(prop.get_propid())
                    external_propId = propId
                    if prop.get_externalpropid():
                        external_propId = str(prop.get_externalpropid())
                    if prop_id == external_propId:
                        for comp in self.__components:
                            if comp.instanceName == str(prop.get_comprefid()):
                                retval = comp.registerPropertyListener(listener, [prop_id], interval)
                                current_registrations[retval] = comp
                                registered = True
                                break
                    if registered:
                        break
            if registered:
                continue
            retval = assembly_comp.registerPropertyListener(listener, [prop_id], interval)
            current_registrations[retval] = comp
        registration_id = str(uuid4())
        self._propertyChangeRegistrations[registration_id] = current_registrations
        return registration_id

    def unregisterPropertyListener(self, reg_id ):
        registration_list = self._propertyChangeRegistrations.get(reg_id)
        if registration_list:
            for reg in registration_list:
                registration_list[reg].unregisterPropertyListener(reg)
            self._propertyChangeRegistrations.pop(reg_id)

    def startPropertyChangeMonitor(self):
        pass

    def stopPropertyChangeMonitor(self):
        pass

def getSDRROOT():
    '''
    Get the current SDRROOT location in use by the sandbox.
    '''
    return _getSandbox().getSdrRoot().getLocation()

def setSDRROOT(newRoot):
    '''
    Change the SDRROOT location where sandbox looks for components and devices
    '''
    try:
        _getSandbox().setSdrRoot(newRoot)
    except RuntimeError as e:
        # Turn RuntimeErrors into AssertionErrors to match legacy expectation.
        raise AssertionError("Cannot set SDRROOT: '%s'" % e)

def catalog(searchPath=None, printResults=False, returnSPDs=False, objType="components"):
    '''
    Lists all available types in $SDRROOT
    Arguments
     searchPath     - specify the directory to search
     printResults   - prints results on seperate lines
     returnSPDs     - prints the name of each spd file
     objType        - specify the object type to list. Default is components.
                      devices and services can also be requested
    '''
    profiles = _getSandbox().catalog(searchPath, objType)
    componentNames = list(profiles.keys())
    componentNames.sort()
    spdFilesWithFullPath = list(profiles.values())

    if printResults == True:
        print("catalog(): available components found in search path " + str(searchPath) + " =====================")
        count = 0
        for component in componentNames:
            print(componentNames[count])
            count = count + 1

    if returnSPDs:
        return spdFilesWithFullPath
    else:
        return componentNames

def browse(searchPath=None, objType=None, withDescription=False):
    _getSandbox().browse(searchPath, objType, withDescription)

class Component(object):
    """
    DEPRECATED. Use launch() method to launch components instead.

    This representation provides a proxy to a running component. The CF::Resource api can be accessed
    directly by accessing the members of the class
    
      componentDescriptor can be either the name of the component or the absolute file path
    
    A simplified access to properties is available through:
        Component.<property id> provides read/write access to component properties
    
    """
    def __new__(self,
                 componentDescriptor = None,
                 instanceName        = None,
                 refid               = None,
                 autoKick            = True,
                 componentObj        = None,
                 impl                = None,
                 launchedFromSADFile = False,
                 debugger            = None,
                 execparams          = {},
                 objType             = None,
                 *args,
                 **kwargs):
        warnings.warn('Component class is deprecated. Use launch() method instead.', DeprecationWarning)
        try:
            if launchedFromSADFile:
                configure = None
            else:
                configure = kwargs
            return launch(componentDescriptor, instanceName, refid, impl, debugger, execparams=execparams, configure=configure, objType=objType)
        except RuntimeError as e:
            # Turn RuntimeErrors into AssertionErrors to match legacy expectation.
            raise AssertionError("Unable to launch component: '%s'" % e)
def api(descriptor, objType=None, destfile=None):
    localdef_dest = False
    if destfile == None:
        localdef_dest = True
        destfile = io.StringIO()

    sdrRoot = _getSandbox().getSdrRoot()
    profile = sdrRoot.findProfile(descriptor, objType=objType)
    spd, scd, prf = sdrRoot.readProfile(profile)
    #spd,scd,prf = _getSandbox().getSdrRoot().readProfile(descriptor)
    if spd:
        if spd.description == None:
            if spd.get_implementation()[0].description == None or \
               spd.get_implementation()[0].description == "The implementation contains descriptive information about the template for a software component.":
                description = None
            else:
                description = spd.get_implementation()[0].description
        else:
            description = spd.description
    if description:
        print('\nDescription ======================\n', file=destfile)
        print(description, file=destfile)
    print('\nPorts ======================', file=destfile)
    print('\nUses (output)', file=destfile)
    table = TablePrinter('Port Name', 'Port Interface')
    for uses in scd.get_componentfeatures().get_ports().get_uses():
        table.append(uses.get_usesname(), uses.get_repid())
    table.write(f=destfile)
    print('\nProvides (input)', file=destfile)
    table = TablePrinter('Port Name', 'Port Interface')
    for provides in scd.get_componentfeatures().get_ports().get_provides():
        table.append(provides.get_providesname(), provides.get_repid())
    table.write(f=destfile)

    print('\nProperties ======================\n', file=destfile)
    table = TablePrinter('id', 'type')
    if prf != None:
        for simple in prf.simple:
            table.append(simple.get_id(),simple.get_type())
        for simpleseq in prf.simplesequence:
            table.append(simpleseq.get_id(),simpleseq.get_type())
        for struct in prf.struct:
            _id = struct.get_id()
            kinds = []
            mode = struct.get_mode()
            table.append(_id, 'struct')
            for prop in struct.get_simple():
                table.append('  '+prop.get_id(),prop.get_type())
            for prop in struct.get_simplesequence():
                table.append('  '+prop.get_id(),prop.get_type())
        for struct in prf.structsequence:
            _id = struct.get_id()
            kinds = []
            mode = struct.get_mode()
            table.append(_id, 'struct sequence')
            for prop in struct.get_struct().get_simple():
                table.append('  '+prop.get_id(),prop.get_type())
            for prop in struct.get_struct().get_simplesequence():
                table.append('  '+prop.get_id(),prop.get_type())
        table.write(f=destfile)

    if localdef_dest:
        pydoc.pager(destfile.getvalue())
        destfile.close()

def _get_started():
    warnings.warn('_get_started() is deprecated, use started()', DeprecationWarning)
    return started()

def started():
    return _getSandbox().started

def start():
    _getSandbox().start()

def stop():
    _getSandbox().stop()

def release():
    """
    Releases and cleans up all launched components, devices, services and
    helpers.
    """
    _getSandbox().shutdown()

def launch(descriptor, instanceName=None, refid=None, impl=None,
           debugger=None, window=None, env={},execparams={}, configure=True,
           initialize=True, timeout=None, objType=None, properties={},
           shared=True, stdout=None, orchestrationType=None, app_object=None, domain_object=None):
    """
    Execute a softpkg, returning a proxy object. This is a factory function
    that may return a component, device or service depending on the SPD.

    When run standalone, returns a local instance, with the executable running
    as a child process.

    When run inside of the REDHAWK IDE, returns a proxy to a component running
    in the IDE.

    Arguments:
      descriptor   - An absolute path to an SPD file, or the name of a softpkg
                     in SDRROOT.
      instanceName - Unique name of this softpackage instance. If not given,
                     one will be generated based on the SPD name.
      refid        - Unique ID of this softpackage instance. If not given, a
                     UUID will be generated.
      impl         - Implementation ID to execute. If not given, the first
                     implementation whose entry point exists will be used.
      debugger     - Debugger to attach to the executable (default: None).
                     Options: gdb (C++), jdb (Java), pdb (Python)
      window       - Terminal to receive command input/output. If not given,
                     output will be directed to stdout, and component will not
                     receive input.
      env           - Dictionary of key/value pairs to set environment variables at runtime
      properties   - Dictionary of key/value pairs to override the initial
                     property values of the component.
      configure    - If true, call configure() with the default values for
                     properties of kind 'configure' after launching the
                     component.
                     If false, defer configuration to the caller (generally
                     used by loadSADFile).
                     DEPRECATED: If a dictionary, a set of key/value pairs
                     to override the initial configuration values of the
                     component. All property kinds should be included in the
                     'properties' argument.
      initialize   - If true, call initialize() after launching the component.
                     If false, defer initialization to the caller.
      timeout      - Time, in seconds, to wait for launch to complete. If not
                     given, the default is 10 seconds, except when running with
                     a debugger, in which case the default is 60 seconds.
      objType      - The type that you would like to launch. Options are
                     component, device, or service.  If not given, all 
                     types will be searched for with the descriptor given.
      shared       - Launch this component into a shared address space, if
                     possible.
      stdout       - File object to send stdout/stderr to.
      orchestrationType - If code type of a component is container, this is 
                          the type of the container orchestrater. 
                          Options: Docker, EksKube

    Deprecated arguments:
      execparams   - Execparams to override on component execution. All property
                     kinds should included in the 'properties' argument.
    """
    # Check for deprecation conditions
    if isinstance(configure, dict) or execparams:
        if properties:
            raise ValueError("'properties' argument cannot be mixed with 'configure' overrides or 'execparams'")
        # Combine the overrides from configure and execparams into a single
        # properties dictionary, with the latter having precedence
        properties = {}
        if isinstance(configure, dict) and len(configure) != 0:
            warnings.warn("'configure' argument is deprecated for property overrides; use 'properties'.", DeprecationWarning)
            properties.update(configure)
            configure = True
        if execparams and len(execparams) != 0:
            warnings.warn("'execparams' argument is deprecated; use 'properties'.", DeprecationWarning)
            properties.update(execparams)

    launch_sb = _getSandbox()
    if hasattr(launch_sb, "setReferences"):
        launch_sb.setReferences(app_object, domain_object)
    return _getSandbox().launch(descriptor=descriptor, instanceName=instanceName, refid=refid,
                                impl=impl, debugger=debugger, window=window, env=env, properties=properties,
                                initialize=initialize, configure=configure, timeout=timeout,
                                objType=objType, shared=shared, stdout=stdout, orchestrationType=orchestrationType)

def createEventChannel(name, exclusive=False):
    """
    Create a sandbox event channel 'name'. If the channel already exists,
    return a handle to the existing channel when 'exclusive' is False, or 
    raise a NameError otherwise.
    """
    return _getSandbox().createEventChannel(name, exclusive)

def getEventChannel(name):
    """
    Get the sandbox event channel 'name'. If it does not exist,
    throw a NameError.
    """
    return _getSandbox().getEventChannel(name)

def availableOrchestrationTypes():
    print("Provided Orchestration Types:")
    print("\tDocker - local docker")
    print("\tDockerSwarm - remote docker swarm cluster")
    print("\tEksKube - remote EKS k8s cluster")
