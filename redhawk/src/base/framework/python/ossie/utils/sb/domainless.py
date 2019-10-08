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
import cStringIO, pydoc
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

from ossie.utils.sandbox import LocalSandbox, IDESandbox
import ossie.utils.sandbox

warnings.filterwarnings('once',category=DeprecationWarning)

# Limit exported symbols
__all__ = ('show', 'loadSADFile', 'IDELocation', 'connectedIDE', 'getIDE_REF',
           'start', 'getSDRROOT', 'setSDRROOT', 'Component', 'generateSADXML',
           'getDEBUG', 'setDEBUG', 'getComponent', 'IDE_REF', 'setIDE_REF',
           'stop', 'catalog', 'redirectSTDOUT', 'orb', 'reset', 'launch', 'api',
           'createEventChannel', 'getEventChannel', 'getService', 'browse',
           'release', 'started', '_get_started')

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
    for _connectionId, connection in ConnectionManager.instance().getConnections().iteritems():
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
        if os.environ.has_key("RH_IDE"):
            if _DEBUG:
                print "IDELocation(): RH_IDE environment variable is set to " + str(os.environ["RH_IDE"])
            return str(os.environ["RH_IDE"])
        else:
            if _DEBUG:
                print "IDELocation(): WARNING - RH_IDE environment variable is not set so plotting will not work"
            return None
    else:
        foundIDE = False
        if os.path.exists(location) and os.path.isdir(location):
            for file in os.listdir(location):
                if file == "eclipse":
                    foundIDE = True
                    os.environ["RH_IDE"] = str(location)
                    if _DEBUG:
                         print "IDELocation(): setting RH_IDE environment variable " + str(location)
        if not foundIDE:
            print "IDELocation(): ERROR - invalid location passed in, must give absolute path " + str(location)
        if _DEBUG:
            print "IDELocation(): setting RH_IDE environment variable " + str(location)
            return str(location)
        else:
            if _DEBUG:
                print "IDELocation(): ERROR - invalid location passed in for RH_IDE environment variable"
            return None

def redirectSTDOUT(filename):
    if _DEBUG == True:
        print "redirectSTDOUT(): redirecting stdout/stderr to filename " + str(filename)
    if type(filename) == str:
        dirname = os.path.dirname(filename)
        if len(dirname) == 0 or \
           (len(dirname) > 0 and os.path.isdir(dirname)):
            try:
                f = open(filename,'w')
                # Send stdout and stderr to provided filename
                sys.stdout = f
                sys.stderr = f
            except Exception, e:
                print "redirectSTDOUT(): ERROR - Unable to open file " + str(filename) + " for writing stdout and stderr " + str(e)
    elif type(filename) == cStringIO.OutputType:
        sys.stdout = filename
        sys.stderr = filename
    else:
        print 'redirectSTDOUT(): failed to redirect stdout/stderr to ' + str(filename)
        print 'redirectSTDOUT(): argument must be: string filename, cStringIO.StringIO object'

def show():
    '''
    Show current list of components running and component connections
    '''
    sandbox = _getSandbox()
    print "Components Running:"
    print "------------------"
    for component in sandbox.getComponents():
        print component._instanceName, component
    print "\n"

    print "Services Running:"
    print "----------------"
    for service in sandbox.getServices():
        print service._instanceName, service
    print "\n"

    print "Component Connections:"
    print "---------------------"
    if connectedIDE:
        log.trace('Scan for new connections')
        ConnectionManager.instance().resetConnections()
        ConnectionManager.instance().refreshConnections(sandbox.getComponents())
    for connectionid, uses, provides in ConnectionManager.instance().getConnections().values():
        print "%s [%s] -> %s [%s]" % (uses.getName(), uses.getInterface(),
                                      provides.getName(), provides.getInterface())

    print "\n"
    print "Event Channels:"
    print "--------------"
    for channel in sandbox.getEventChannels():
        print '%s (%d supplier(s), %d consumer(s))' % (channel.name, channel.supplier_count,
                                                       channel.consumer_count)
    print "\n"

    print "SDRROOT:"
    print "-------"
    print sandbox.getSdrRoot().getLocation()
    print "\n"

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
        print "generateSadFileString(): generating SAD XML string for given waveform name " + str(waveform_name)
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
        print "generateSadFileString(): returning SAD XML string " + str(sadString)
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
    if valuetype == 'string' or valuetype == 'char':
        return value
    elif valuetype == 'boolean':
        if type(value) == list:
            return [bool(s) for s in value]
        else:
            return bool(value)
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
                    except Exception, e:
                        print e, "Problem overloading id="+entry.id
            for overload in simpleseq:
                if overload.id == entry.id:
                    allProps.pop(overload.id)
                    try:
                        setattr(component, entry.clean_name, convertToValue(entry.valueType, overload.value))
                    except Exception, e:
                        print e, "Problem overloading id="+entry.id
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
                            if overload.value.has_key(kl):
                                _ov_key = kl
                            else:
                                if _keys.has_key(kl_clean) and overload.value.has_key(_keys.get(kl_clean)):
                                    _ov_key = _keys[kl_clean]
                                    break

                        if _ov_key == None or not overload.value.has_key(_ov_key):
                            if _DEBUG:
                                print "Struct::Simple:  id:", str(simple[0]), " cleaned id:", st_clean, "  Unable to match overloaded key: ", _ov_key
                            continue

                        if _DEBUG:
                            print "Struct::Simple:  id:", str(simple[0]), " cleaned id:", st_clean, " Overloaded ID: ", overload.id, " value: ", overload.value, " key:", _ov_key

                        # cleanup struct key if it has illegal characters...
                        st_clean = st_clean.translate(translation)
                        try:
                            structValue[st_clean] = convertToValue(simple[1], overload.value[_ov_key])
                        except Exception, e:
                            print e, "Problem overloading id="+entry.id

                    if _DEBUG:
                         print "setattr  ", component, " clean name ", entry.clean_name,  " struct ", structValue
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
                                if overloadedValue.has_key(kl):
                                    _ov_key = kl
                                else:
                                    if _keys.has_key(kl_clean) and overloadedValue.has_key(_keys.get(kl_clean)):
                                        _ov_key = _keys[kl_clean]
                                        break

                            if _ov_key == None or not overloadedValue.has_key(_ov_key):
                                if _DEBUG:
                                    print "StructSeq::Struct::Simple:  id:",str(simple[0]), " cleaned id:", st_clean, "  Unable to match overloaded key: ", _ov_key
                                continue

                            if _DEBUG:
                                print "StructSeq::Struct::Simple:  id:", str(simple[0]), " cleaned id:", st_clean, " Overloaded  value: ", overloadedValue, " key:", _ov_key

                            # cleanup struct key if it has illegal characters...
                            st_clean = st_clean.translate(translation)
                            try:
                                structValue[st_clean] = convertToValue(simple[1], overloadedValue[_ov_key])
                            except Exception, e:
                                print e, "Problem overloading id="+entry.id

                        structSeqValue.append(structValue)
                    setattr(component, entry.clean_name, structSeqValue)
        for prop in allProps:
            dV = allProps[prop].defValue
            if dV == None:
                continue
            if allProps[prop].mode != "readonly" and 'configure' in allProps[prop].kinds:
                setattr(component, allProps[prop].clean_name, allProps[prop].defValue)

def _loadStructMembers(parent):
    simples = parent.get_simpleref()
    value = {}
    for simple in simples:
        value[str(simple.refid)] = str(simple.value)
    simpleseqs = parent.get_simplesequenceref()
    for simpleseq in simpleseqs:
        _seq = []
        for seq_value in simpleseq.values.get_value():
            _seq.append(str(seq_value))
        value[str(simpleseq.refid)] = _seq
    return value

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
            print "loadSADFile(): props argument must be a dictionary. Ignoring overload"
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
                log.warn("Could not find softpkg '%s'", component.get_localfile().get_name())

        # Need to determine which component is the assembly controller
        assemblyControllerRefid = None
        if sad.assemblycontroller:
            assemblyControllerRefid = sad.assemblycontroller.get_componentinstantiationref().get_refid() + waveform_modifier
            log.debug("ASSEMBLY CONTROLLER component instantiation ref '%s'", assemblyControllerRefid)
        if not assemblyControllerRefid:
            log.warn('SAD file did not specify an assembly controller')

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
        for k,v in startorder.items():
            ordered_placements = ordered_placements + v

        configurable = {}
        for component in ordered_placements:
            log.debug("COMPONENT PLACEMENT component spd file id '%s'", component.componentfileref.refid)
            log.debug("COMPONENT PLACEMENT component instantiation id '%s'", component.get_componentinstantiation()[0].id_)
            log.debug("COMPONENT PLACEMENT component startorder '%s'", component.get_componentinstantiation()[0].get_startorder())
            log.debug("COMPONENT PLACEMENT component name '%s'", component.get_componentinstantiation()[0].get_usagename())
            # If component has a valid SPD file (isKickable), launch it
            refid = component.componentfileref.refid
            if validRequestedComponents.has_key(refid):
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
                    launchedComponents.append(newComponent)
                except Exception as e:
                    msg = "Failed to launch component '%s', REASON: %s" %  (instanceName, str(e))
                    print msg
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
                    # Check for uses port
                    if connection.get_usesport() != None:
                        usesPortName = connection.get_usesport().get_usesidentifier()
                        if connection.get_usesport().get_componentinstantiationref() != None:
                            usesPortComponentRefid = connection.get_usesport().get_componentinstantiationref().get_refid()
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
                            else:
                                log.warn("Unable to create connection")
                                continue
                            if _DEBUG == True:
                                print "loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port interface " + str(connection.get_componentsupportedinterface().get_supportedidentifier())
                                print "loadSADFile(): CONNECTION INTERFACE: componentsupportedinterface port component ref " + str(connection.get_componentsupportedinterface().get_componentinstantiationref().get_refid())
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
            if validRequestedComponents.has_key(refid):
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
                    print "OverLoad Assembly Controller ", (sandboxComponent, prop_types['simple'][1], prop_types['simpleseq'][1], prop_types['struct'][1], prop_types['structseq'][1])
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
            print "Overload property '"+prop+"' was ignored because it is not on the Assembly Controller"
        raise Warning('Requested property overloads not assigned' + pnames )
    return True


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
    except RuntimeError, e:
        # Turn RuntimeErrors into AssertionErrors to match legacy expectation.
        raise AssertionError, "Cannot set SDRROOT: '%s'" % e

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
    componentNames = profiles.keys()
    componentNames.sort()
    spdFilesWithFullPath = profiles.values()

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
        except RuntimeError, e:
            # Turn RuntimeErrors into AssertionErrors to match legacy expectation.
            raise AssertionError, "Unable to launch component: '%s'" % e
def api(descriptor, objType=None, destfile=None):
    localdef_dest = False
    if destfile == None:
        localdef_dest = True
        destfile = cStringIO.StringIO()

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
        print >>destfile, '\nDescription ======================\n'
        print >>destfile, description
    print >>destfile, '\nPorts ======================'
    print >>destfile, '\nUses (output)'
    table = TablePrinter('Port Name', 'Port Interface')
    for uses in scd.get_componentfeatures().get_ports().get_uses():
        table.append(uses.get_usesname(), uses.get_repid())
    table.write(f=destfile)
    print >>destfile, '\nProvides (input)'
    table = TablePrinter('Port Name', 'Port Interface')
    for provides in scd.get_componentfeatures().get_ports().get_provides():
        table.append(provides.get_providesname(), provides.get_repid())
    table.write(f=destfile)

    print >>destfile, '\nProperties ======================\n'
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
           debugger=None, window=None, execparams={}, configure=True,
           initialize=True, timeout=None, objType=None, properties={},
           shared=True, stdout=None):
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

    return _getSandbox().launch(descriptor=descriptor, instanceName=instanceName, refid=refid,
                                impl=impl, debugger=debugger, window=window, properties=properties,
                                initialize=initialize, configure=configure, timeout=timeout,
                                objType=objType, shared=shared, stdout=stdout)

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
