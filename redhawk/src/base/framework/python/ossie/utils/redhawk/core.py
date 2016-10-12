#! /usr/local/bin/python
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


import atexit as _atexit
import commands as _commands
import os as _os
import xml.dom.minidom as _minidom
from ossie.cf import CF as _CF
from ossie.cf import CF__POA as _CF__POA
from ossie.cf import ExtendedCF as _ExtendedCF
from omniORB import CORBA as _CORBA
import CosNaming as _CosNaming
import sys as _sys
import copy as _copy
import time as _time
import ossie.parsers as _parsers
import string as _string
import datetime as _datetime
from ossie.properties import __TYPE_MAP
from ossie.utils.sca import importIDL as _importIDL
from ossie.utils import prop_helpers as _prop_helpers
from omniORB import any as _any

_ossiehome = _os.getenv('OSSIEHOME')

if _ossiehome == None:
    _ossiehome = ''

_interface_list = []
_loadedInterfaceList = False
_interfaces = {}

__MIDAS_TYPE_MAP = {'char'  : ('/SB/8t'),
                    'octet' : ('/SO/8o'),
                    'short' : ('/SI/16t'),
                    'long'  : ('/SL/32t'),
                    'float' : ('/SF/32f'),
                    'double': ('/SD/64f')}

_DEBUG = False 
_launchedApps = []

def _uuidgen():
    return _commands.getoutput('uuidgen')

def getCurrentDateTimeString():
    # return a string representing current day and time
    # format is DDD_HHMMSSmmm where DDD represents day of year (1-365) and mmm represents number of milliseconds
    dt = _datetime.datetime.now()
    time_str = str(dt)
    # If time is on an even second (no microseconds) then don't do split to avoid exception
    if len(time_str.split('.')) == 1:
        microseconds_str = "000000"
    else:
        time_str, microseconds_str = time_str.split('.')
    return _time.strftime("%j_%H%M%S",dt.timetuple()) + microseconds_str[:3]

def setDEBUG(debug=False):
    global _DEBUG
    _DEBUG = debug

def getDEBUG():
    return _DEBUG

def _cleanUpLaunchedApps():
    for app in _launchedApps[:]:
        app.releaseObject()

_atexit.register(_cleanUpLaunchedApps)


class _componentBase(object):
    '''
      componentDescriptor can be either the name of the component or the absolute file path
    '''
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
    def __init__(self,componentDescriptor=None,instanceName=None,refid=None,componentObj=None,impl=None,domainMgr=None,int_list=None,*args,**kwargs):
        self._profile = ''
        self._spd = None
        self._scd = None
        self._prf = None
        self._spdContents = None
        self._scdContents = None
        self._prfContents = None
        self._impl = impl
        self.ref = componentObj
        self.ports = []
        self._providesPortDict = {}
        self._configureTable = {}
        self._usesPortDict = {}
        self._autoKick = False
        self._fileManager = None
        self.name = None
        self._interface_list = int_list
        if domainMgr != None:
            self._fileManager = domainMgr.ref._get_fileMgr()

        if refid == None:
            self._refid = _uuidgen()
        else:
            self._refid = refid

        try:
            if componentDescriptor != None:
                self._profile = componentDescriptor
                self.name = _os.path.basename(componentDescriptor).split('.')[0]
                self._parseComponentXMLFiles()
                self._buildAPI()
                if self.ref != None:
                    self._populatePorts(fs=self._fileManager)
            else:
                raise AssertionError, "Component:__init__() ERROR - Failed to instantiate invalid component % " % (str(self.name))

        except Exception, e:
            print "Component:__init__() ERROR - Failed to instantiate component " + str(self.name) + " with exception " + str(e)

    def _isMatch(self, prop, modes, kinds, actions):
        if prop.get_mode() == None:
            m = "readwrite"
        else:
            m = prop.get_mode()
        matchMode = (m in modes)

        if prop.__class__ in (_parsers.PRFParser.simple, _parsers.PRFParser.simpleSequence):
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

        elif prop.__class__ in (_parsers.PRFParser.struct, _parsers.PRFParser.structSequence):
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
                p.clean_name = _prop_helpers.addCleanName(id_clean, prop.get_id(), self._get_identifier())
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
                if str(values) != str(None):
                    defValue = []
                    defValues = values.get_value()
                    for val in defValues:
                        if propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
                            # If value contains an x, it is a hex value (base 16) 
                            if val.find('x') != -1:
                                defValue.append(int(val,16))
                            else:
                                defValue.append(int(val))
                        elif propType in ['double', 'float']:
                            defValue.append(float(val))
                        elif propType in ['char', 'string']:
                            defValue.append(str(val))
                        elif propType == 'boolean':
                            defValue.append({"TRUE": True, "FALSE": False}[val.strip().upper()])
                p = _prop_helpers.sequenceProperty(id=prop.get_id(), valueType=propType, compRef=self, defValue=defValue, mode=prop.get_mode())
                prop_id = prop.get_name()
                if prop_id == None:
                    prop_id = prop.get_id()
                id_clean = str(prop_id).translate(translation)
                p.clean_name = id_clean
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
            print "Component: _getPropertySet() propertySet " + str(propertySet)
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
        if self.ref:
            try:
                self.ref.releaseObject()
            except:
                raise
    
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
            self.api()

    def api(self, showComponentName=True, showInterfaces=True, showProperties=True):
        '''
        Inspect interfaces and properties for the component
        '''
        if _DEBUG == True:
            print "Component:api()"
        if showComponentName == True:
            print "Component [" + str(self.name) + "]:"
        if showInterfaces == True:
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
        
        if showProperties == True and self._propertySet != None:
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
                valueType = _copy.deepcopy(prop.valueType)
            
                if mode != "writeonly":
                    if propType == 'struct':
                        currentValue = prop.members
                    else:
                        currentValue = _copy.deepcopy(prop.queryValue())

                scaType = str(propType)
                if midasTypeMap.has_key(scaType):
                    scaType = scaType + midasTypeMap[scaType]

                propList.append([clean_name,scaType,defValue,currentValue,valueType]) 
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
                for clean_name, propType, defValue, currentValue, valueType in propList:
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
                        item_count = -1
                        for item in currentValue:
                            item_count += 1
                            for member in valueType:
                                _id = _copy.deepcopy(member[0])
                                _propType = _copy.deepcopy(member[1])
                                _defValue = _copy.deepcopy(member[2])
                                _currentValue = _copy.deepcopy(item[member[0]])
                        
                                name = str(_id)
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
                            
                                print ' ['+str(item_count)+'] ' + name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal  + " "*(maxDefaultValueLen-len(defVal)) + "\t\t" + currVal
                    elif scaType == 'struct':
                        currVal = ''
                        defVal = ''
                        print name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal + " "*(maxDefaultValueLen-len(defVal)) + "\t" + currVal
                    
                        for member in currentValue.values():
                            _id = _copy.deepcopy(member.id)
                            _propType = _copy.deepcopy(member.type)
                            _defValue = _copy.deepcopy(member.defValue)
                            _currentValue = _copy.deepcopy(member.queryValue())
                        
                            name = str(_id)
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
                            
                            print '  ' + name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal  + " "*(maxDefaultValueLen-len(defVal)) + "\t\t" + currVal
                    else:     
                        defVal = str(defValue)    
                        if defVal != None and len(defVal) > maxDefaultValueLen:
                            defVal = defVal[0:maxDefaultValueLen-3] + '...'
                           
                        currVal = str(currentValue)
                        if currVal != None and len(currVal) > maxDefaultValueLen:
                            currVal = currVal[0:maxDefaultValueLen-3] + '...'
                
                        print name + " "*(maxNameLen-len(name)) + "\t(" + scaType + ")" + " "*(maxSCATypeLen-len(scaType)) + "\t\t" + defVal  + " "*(maxDefaultValueLen-len(defVal)) + "\t\t" + currVal
            
    def _parseComponentXMLFiles(self):
        if self._fileManager != None and len(self._profile) > 0:
            # SPD File
            spdFile = self._fileManager.open(self._profile,True)
            self._spdContents = spdFile.read(spdFile.sizeOf())
            spdFile.close()
            if self._spdContents != None:
                self._spd = _parsers.SPDParser.parseString(self._spdContents)

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
                self._prf = _parsers.PRFParser.parseString(self._prfContents)

            # SCD File
            scdFile = self._fileManager.open(object.__getattribute__(self,'_scd_path'), True)
            self._scdContents = scdFile.read(scdFile.sizeOf())
            scdFile.close()
            if self._scdContents != None:
                self._scd = _parsers.SCDParser.parseString(self._scdContents)

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
                        providesComponent.setup(portIOR, dataType=dataType, componentName=self.name, usesPortName=usesPortName)
                        return
                    raise AssertionError, "Component:connect() failed because usesPortName given was not a valid port for providesComponent"
                else:
                    raise AssertionError, "Component:connect() failed because usesPortName given was not a valid port for providesComponent"
            else:
                raise AssertionError, "Component:connect() failed because usesPortName was not specified ... providesComponent being connect requires it for connection"

        if isinstance(providesComponent,Component) == False and \
           isinstance(providesComponent, _io_helpers.OutputFile) == False and \
           isinstance(providesComponent, _io_helpers.OutputData) == False:
            raise AssertionError,"Component:connect() ERROR - connect failed because providesComponent passed in is not of valid type ... valid types include instance of _OutputBase,OutputData,OutputFile, or Component classes"

        # Perform connect between this Component instance and a given Component instance called providesComponent 
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
                            counter = counter + 1
                    usesPort_handle.connectPort(providesPort_ref, str(connectionID))
                    if _DEBUG == True:
                        print "Component:connect() calling connectPort() with connectionID " + str(connectionID) 

            except Exception, e:
                print "Component:connect(): connect failed " + str(e)
                return

            if _DEBUG == True:
                print "Component:connect() succeeded"
            return
        else:
            raise AssertionError, "Component:connect failed" 

    def disconnect(self,providesComponent):
        if _DEBUG == True:
            print "Component: disconnect()"
        usesPort_ref = None
        usesPort_handle = None

    def _populatePorts(self, fs=None):
        """Add all port descriptions to the component instance"""
        cname=object.__getattribute__(self,'name')
        if _DEBUG == True:
            print ' Populating Ports For:' + str(cname)
        if object.__getattribute__(self,'_spd') == '':
            print "Unable to create port list for " + object.__getattribute__(self,'name') + " - profile unavailable"
            return
        if len(object.__getattribute__(self,'ports')) != 0:
            return
    
        if fs != None:
            spdFile = fs.open(object.__getattribute__(self,'_profile'),True)
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
        if fs != None:
          prfFile = fs.open(object.__getattribute__(self,'_prf_path'), True)
          prfContents = prfFile.read(prfFile.sizeOf())
          prfFile.close()
        else:
          prfFile = open(object.__getattribute__(self,'_prf_path'), 'r')
          prfContents = prfFile.read()
          prfFile.close()
        doc_prf = _minidom.parseString(prfContents)
        if fs != None:
            scdFile = fs.open(object.__getattribute__(self,'_scd_path'), True)
            scdContents = scdFile.read(scdFile.sizeOf())
            scdFile.close()
        else:
            scdFile = open(object.__getattribute__(self,'_scd_path'), 'r')
            scdContents = scdFile.read()
            scdFile.close()
        doc_scd = _minidom.parseString(scdContents)
    
        interface_modules = ['BULKIO', 'BULKIO__POA']
        
        int_list = {}

        if self._interface_list == None:
            if not globals()["_loadedInterfaceList"]:
                globals()["_interface_list"] = _importIDL.importStandardIdl()
                globals()["_loadedInterfaceList"] = True
            self._interface_list = globals()["_interface_list"]

        for int_entry in self._interface_list:
            int_list[int_entry.repoId]=int_entry
        for uses in doc_scd.getElementsByTagName('uses'):
            idl_repid = str(uses.getAttribute('repid'))
            if not int_list.has_key(idl_repid):
                if _DEBUG == True:
                    print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
                continue
            int_entry = int_list[idl_repid]

            if _DEBUG == True:
                print '---> UsesPort ' + str(uses.getAttribute('usesname'))
            new_port = _Port(str(uses.getAttribute('usesname')), interface=None, direction="Uses", using=int_entry)
            try:
                new_port.generic_ref = self.getPort(str(new_port.name))
                new_port.ref = new_port.generic_ref._narrow(_ExtendedCF.QueryablePort)
                if new_port.ref == None:
                    new_port.ref = new_port.generic_ref._narrow(_CF.Port)
                new_port.extendPort()
    
                idl_repid = new_port.ref._NP_RepositoryId
                if not int_list.has_key(idl_repid):
                    if _DEBUG == True:
                        print "Unable to find port description for " + self.name + " for " + idl_repid
                    continue
                int_entry = int_list[idl_repid]
                new_port._interface = int_entry
                if _DEBUG == True:
                   print 'Component Adding USES Port:' + str(new_port.name)
                object.__getattribute__(self,'ports').append(new_port)
            except:
                if _DEBUG == True:
                    print "getPort failed for port name: ", str(new_port.name)
        for provides in doc_scd.getElementsByTagName('provides'):
            idl_repid = str(provides.getAttribute('repid'))
            if not int_list.has_key(idl_repid):
                if _DEBUG == True:
                    print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
                continue
            int_entry = int_list[idl_repid]
            if _DEBUG == True:
                print '---> ProvidesPort ' + str(provides.getAttribute('providesname'))
            new_port = _Port(str(provides.getAttribute('providesname')), interface=int_entry, direction="Provides")
            try:
                new_port.generic_ref = object.__getattribute__(self,'ref').getPort(str(new_port.name))
                
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
                    if _DEBUG == True:
                        print 'Component Adding PROVDES Port:' + str(new_port.name)
                    object.__getattribute__(self,'ports').append(new_port)
                except:
                    if _DEBUG == True:
                        print 'ERROR, Provides NARROW failed: ' + exec_string
                    continue
            except:
                if _DEBUG == True:
                    print "getPort failed for port name: ", str(new_port.name)

class App(_CF__POA.Application, object):
    """This is the basic descriptor for a waveform (collection of inter-connected Components)
       
       App overview:
       
       A waveform is defined by an XML file (<waveform name>.sad.xml) that resides in a waveform
       directory, usually $SDRROOT/dom/waveforms. This XML file lists a series of
       components, a variety of default values for each of these components, and a set of connections
       between different components' input and output ports. Input ports are referred to as 'Provides'
       and output ports are referred to as 'Uses'.
       
       A waveform can follow any type of design, but may look something like this:
       
                                  _________ 
                                  |        |
       _________    _________   ->| Comp 3 |
       |        |   |        | /  |        |
       | Comp 1 |-->| Comp 2 |/   ----------
       |        |   |        |\   _________ 
       ----------   ---------- \  |        |
                                ->| Comp 4 |
                                  |        |
                                  ----------
       
    """
    def __init__(self, name="", int_list=None, domain=None, sad=None):
        # _componentsUpdated needs to be set first to prevent __setattr__ from entering an error state
        self._componentsUpdated = False
        self.name = name
        self.comps = []
        self.ports = []
        self._portsUpdated = False
        self.ns_name = ''
        self.ref = None
        self._interface_list = int_list
        self._domain = domain
        self._sad = sad
        self.adhocConnections = []
        self.connectioncount = 0
        self.assemblyController = None

        if self._domain == None:
            orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
            obj = orb.resolve_initial_references("NameService")
            self.rootContext = obj._narrow(_CosNaming.NamingContext)
        else:
            self.rootContext = self._domain.rootContext

        if self._interface_list == None:
            if not globals()["_loadedInterfaceList"]:
                globals()["_interface_list"] = _importIDL.importStandardIdl()
                globals()["_loadedInterfaceList"] = True
            self._interface_list = globals()["_interface_list"]

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
    
    def _get_profile(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_profile()
            except:
                pass
        return retval
    
    def _get_name(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_name()
            except:
                pass
        return retval
    
    def _get_componentNamingContexts(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentNamingContexts()
            except:
                pass
        return retval
    
    def _get_componentProcessIds(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentProcessIds()
            except:
                pass
        return retval
    
    def _get_componentDevices(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentDevices()
            except:
                pass
        return retval
    
    def _get_componentImplementations(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_componentImplementations()
            except:
                pass
        return retval
    
    def start(self):
        if self.ref:
            try:
                self.ref.start()
            except:
                raise
    
    def stop(self):
        if self.ref:
            try:
                self.ref.stop()
            except:
                raise
    
    def initialize(self):
        if self.ref:
            try:
                self.ref.initialize()
            except:
                raise
    
    def releaseObject(self):
        if self.ref:
            try:
                self._domain.removeApplication(self)
            except:
                raise
    
    def getPort(self, name):
        retval = None
        if self.ref:
            try:
                retval = self.ref.getPort(name)
            except:
                raise
        return retval
    
    def configure(self, props):
        if self.ref:
            try:
                self.ref.configure(props)
            except:
                raise
    
    def query(self, props):
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

    # End external Resource API
    ########################################
    
    def __getattribute__(self, name):
        try:
            if name == 'ports':
                if not object.__getattribute__(self,'_portsUpdated'):
                    self._populatePorts()
            if name == 'comps':
                if not object.__getattribute__(self,'_componentsUpdated'):
                    self.update()

            return object.__getattribute__(self,name)
        except AttributeError:
            # Check if current request value is a member Components Property
            for curr_comp in self.comps:
                if curr_comp._get_identifier().find(self.assemblyController) != -1:
                    try:
                        return curr_comp.__getattribute__(name)
                    except AttributeError:
                        continue
            raise AttributeError('App object has no attribute ' + str(name))
    
    def __setattr__(self, name, value):
        if name == '_componentsUpdated' or self._componentsUpdated == False:
            return object.__setattr__(self, name, value)
        else:
            # Check if current value to be set is a member Components property
            for curr_comp in self.comps:
                if curr_comp._get_identifier().find(self.assemblyController) != -1:
                    try:
                        return curr_comp.__setattr__(name, value)
                    except AttributeError:
                        continue
           
        return object.__setattr__(self, name, value)
    
    def update(self):
        self.__setattr__('_componentsUpdated', True)
        comp_list = self.ref._get_registeredComponents()
        app_name = self.ref._get_name()
        self._populateComponents(comp_list)

    def _populateComponents(self, component_list):
        """component_list is a list of component names
           in_doc_sad is a parsed version of the SAD (using xml.dom.minidom)"""

        spd_list = {}
        for compfile in self._sad.getElementsByTagName('componentfile'):
            spd_list[str(compfile.getAttribute('id'))] = str(compfile.getElementsByTagName('localfile')[0].getAttribute('name'))

        dce_list = {}
        for compplac in self._sad.getElementsByTagName('componentplacement'):
            dce_list[str(compplac.getElementsByTagName('componentinstantiation')[0].getAttribute('id'))] = str(compplac.getElementsByTagName('componentfileref')[0].getAttribute('refid'))

        for compfile in self._sad.getElementsByTagName('assemblycontroller'):
             self.assemblyController = compfile.getElementsByTagName('componentinstantiationref')[0].getAttribute('refid')

        for comp_entry in component_list:
            new_comp = Component(
                componentDescriptor=comp_entry.softwareProfile,
                componentObj=comp_entry.componentObject,
                domainMgr=self._domain,
                int_list=self._interface_list)

            foundObject = False
            try:
                new_comp._id = new_comp.ref._get_identifier()
                foundObject = True
            except:
                pass

            if not foundObject:
                print "Unable to get the pointer to Component "+comp_entry.identifier+", it is probably not running"

            self.comps.append(new_comp)

    def api(self):
        # Display components, their properties, and external ports
        print "Waveform [" + self.ns_name + "]"
        print "---------------------------------------------------"
        # Determine the maximum length of port names for print formatting
        print "External Ports =============="
        maxNameLen = 0
        hasProvidesPort = False
        for port in self.ports:
            if len(port.name) > maxNameLen:
                maxNameLen = len(port.name)
        print "Provides (Input) Ports =============="
        print "Port Name" + " "*(maxNameLen-len("Port Name")) + "\tPort Interface"
        print "---------" + " "*(maxNameLen-len("---------")) + "\t--------------"
        for port in self.ports:
            if port._direction == "Provides":
                hasProvidesPort = True
                print str(port.name) + " "*(maxNameLen-len(str(port.name))) + "\t" + "IDL:" + str(port._interface.nameSpace + "/" + port._interface.name)
        if hasProvidesPort == False:
            print "None"
        print "\n"

        maxNameLen = 0
        hasUsesPort = False
        # Determine the maximum length of port names for print formatting
        for port in self.ports:
            if len(port.name) > maxNameLen:
                maxNameLen = len(port.name)
        print "Uses (Output) Ports =============="
        print "Port Name" + " "*(maxNameLen-len("Port Name")) + "\tPort Interface"
        print "---------" + " "*(maxNameLen-len("---------")) + "\t--------------"
        for port in self.ports:
            if port._direction == "Uses":
                hasUsesPort = True
                print str(port.name) + " "*(maxNameLen-len(str(port.name))) + "\t" + "IDL:" + str(port._interface.nameSpace + "/" + port._interface.name)
        if hasUsesPort == False:
            print "None"
        print "\n"
    

        print "Components =============="
        count = 1 
        for comp_entry in self.comps:
            if comp_entry._get_identifier().find(self.assemblyController) != -1:
                print str(count) + ". " + comp_entry.name + " (Assembly Controller)"
            else:
                print str(count) + ". " + comp_entry.name
            count += 1
        print "\n"

        for comp_entry in self.comps:
            if comp_entry._get_identifier().find(self.assemblyController) != -1:
                comp_entry.api(showComponentName=False,showInterfaces=False,showProperties=True)
        print "\n"

    def _populatePorts(self, fs=None):
        self.__setattr__('_portsUpdated', True)
        """Add all port descriptions to the component instance"""
        if object.__getattribute__(self,'_sad') == '':
            print "Unable to create port list for " + object.__getattribute__(self,'name') + " - sad file unavailable"
            return
        if len(object.__getattribute__(self,'ports')) != 0:
            return
    
        if fs==None:
            fs = object.__getattribute__(self,'_domain').fileManager
    
        interface_modules = ['BULKIO', 'BULKIO__POA']
        
        int_list = {}
        for int_entry in object.__getattribute__(self,'_interface_list'):
            int_list[int_entry.repoId]=int_entry
        
        externalports = object.__getattribute__(self,'_sad').getElementsByTagName('port')
        for externalport in externalports:
            portName = None
            instanceId = None
            for child in externalport.childNodes:
                if str(child.nodeName) == 'usesidentifier' or str(child.nodeName) == 'providesidentifier':
                    portName = str(child.childNodes[0].data)
                if str(child.nodeName) == 'componentinstantiationref':
                    instanceId = str(child._attrs['refid'].value)
            if portName == None or instanceId == None:
                continue
            placements = object.__getattribute__(self,'_sad').getElementsByTagName('componentplacement')
            componentfileref = None
            foundinstance = False
            for placement in placements:
                for child in placement.childNodes:
                    if str(child.nodeName) == 'componentinstantiation':
                        if str(child._attrs['id'].value) == instanceId:
                            foundinstance = True
                    if str(child.nodeName) == 'componentfileref':
                        componentfileref = str(child._attrs['refid'].value)
                if foundinstance:
                    break
            if not foundinstance and componentfileref == None:
                continue
            componentfiles = object.__getattribute__(self,'_sad').getElementsByTagName('componentfile')
            spd_file = None
            foundinstance = False
            for componentfile in componentfiles:
                if str(componentfile._attrs['id'].value) != componentfileref:
                    continue
                else:
                    foundinstance = True
                for child in componentfile.childNodes:
                    if str(child.nodeName) == 'localfile':
                        spd_file = str(child._attrs['name'].value)
            if not foundinstance and spd_file == None:
                continue
            
            spdFile = fs.open(spd_file, True)
            spdContents = spdFile.read(spdFile.sizeOf())
            spdFile.close()
            doc_spd = _minidom.parseString(spdContents)
            localfiles = doc_spd.getElementsByTagName('localfile')
            for localfile in localfiles:
                if localfile.parentNode.tagName == 'descriptor':
                    scdpath = spd_file[:spd_file.rfind('/')+1]+str(localfile.getAttribute('name'))
            scdFile = fs.open(scdpath, True)
            scdContents = scdFile.read(scdFile.sizeOf())
            scdFile.close()
            doc_scd = _minidom.parseString(scdContents)
            
            for uses in doc_scd.getElementsByTagName('uses'):
                if str(uses.getAttribute('usesname')) != portName:
                    continue
                idl_repid = str(uses.getAttribute('repid'))
                if not int_list.has_key(idl_repid):
                    print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
                    continue
                int_entry = int_list[idl_repid]
    
                new_port = _Port(str(uses.getAttribute('usesname')), interface=None, direction="Uses", using=int_entry)
                new_port.generic_ref = self.ref.getPort(str(new_port.name))
                new_port.ref = new_port.generic_ref._narrow(_ExtendedCF.QueryablePort)
                if new_port.ref == None:
                    new_port.ref = new_port.generic_ref._narrow(_CF.Port)
                new_port.extendPort()
    
                idl_repid = new_port.ref._NP_RepositoryId
                if not int_list.has_key(idl_repid):
                    print "Unable to find port description for " + self.name + " for " + idl_repid
                    continue
                int_entry = int_list[idl_repid]
                new_port._interface = int_entry
    
                object.__getattribute__(self,'ports').append(new_port)
            
            for provides in doc_scd.getElementsByTagName('provides'):
                if str(provides.getAttribute('providesname')) != portName:
                    continue
                idl_repid = str(provides.getAttribute('repid'))
                if not int_list.has_key(idl_repid):
                    print "Invalid port descriptor in scd for " + self.name + " for " + idl_repid
                    continue
                int_entry = int_list[idl_repid]
                new_port = _Port(str(provides.getAttribute('providesname')), interface=int_entry, direction="Provides")
                new_port.generic_ref = self.ref.getPort(str(new_port.name))
                
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
                    object.__getattribute__(self,'ports').append(new_port)
                except:
                    continue
    
    def connect(self, provides, usesName=None, providesName=None):
        for port in self.ports:
            if port._direction == 'Uses':
                for provide_port in provides.ports:
                    if provide_port._direction == 'Provides' and port._using.name == provide_port._interface.name:
                        self.connectioncount += 1
                        connectionid = 'adhoc_connection_'+str(self.connectioncount)
                        port.connectPort(port.ref, connectionid)
                        self.adhocConnections.append((provides, port, connectionid))
    
    def disconnect(self, provides):
        done = False
        while not done:
            for connection_idx in range(len(self.adhocConnections)):
                if self.adhocConnections[connection_idx][0] == provides:
                    self.adhocConnections[connection_idx][1].disconnectPort(self.adhocConnections[connection_idx][2])
                    tmp = self.adhocConnections.pop(connection_idx)
                    break
            if connection_idx == len(self.adhocConnections)-1 or len(self.adhocConnections) == 0:
                done = True
    
    def __getitem__(self,i):
        """Return the component with the given index (obsolete)"""
        return self.comps[i]

class Device(Component):
    def __init__(self,componentDescriptor=None,instanceName=None,refid=None,componentObj=None,impl=None,domainMgr=None,int_list=None,*args,**kwargs):
        self._profile = ''
        self._spd = None
        self._scd = None
        self._prf = None
        self._spdContents = None
        self._scdContents = None
        self._prfContents = None
        self._impl = impl
        self.ref = componentObj
        self.ports = []
        self._providesPortDict = {}
        self._configureTable = {}
        self._usesPortDict = {}
        self._autoKick = False
        self._fileManager = None
        self.name = None
        self._interface_list = int_list
        self._propertySet = None
        if self._interface_list == None:
            if not globals()["_loadedInterfaceList"]:
                globals()["_interface_list"] = _importIDL.importStandardIdl()
                globals()["_loadedInterfaceList"] = True
            self._interface_list = globals()["_interface_list"]

        if refid == None:
            self._refid = _uuidgen()
        else:
            self._refid = refid

        if domainMgr != None:
            self._fileManager = domainMgr.ref._get_fileMgr()

        try:
            if componentDescriptor != None:
                self._profile = componentDescriptor
                self.name = _os.path.basename(componentDescriptor).split('.')[0]
                self._parseComponentXMLFiles()
                self._buildAPI()
                if self.ref != None:
                    self._populatePorts(fs=self._fileManager)
            else:
                raise AssertionError, "Component:__init__() ERROR - Failed to instantiate invalid component % " % (str(self.name))

        except Exception, e:
            print "Component:__init__() ERROR - Failed to instantiate component " + str(self.name) + " with exception " + str(e)

    def updateReferences(self):
        self.device_ref = self.ref
        self.loadabledevice_ref = self.ref._narrow(_CF.LoadableDevice)
        self.executabledevice_ref = self.ref._narrow(_CF.ExecutableDevice)
        self.aggregatedevice_ref = self.ref._narrow(_CF.AggregateDevice)

    def _get_usageState(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_usageState()
            except:
                pass
        return retval

    def _get_adminState(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_adminState()
            except:
                pass
        return retval

    def _get_operationalState(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_operationalState()
            except:
                pass
        return retval

    def _get_softwareProfile(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_softwareProfile()
            except:
                pass
        return retval

    def _get_label(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_label()
            except:
                pass
        return retval

    def _get_compositeDevice(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_compositeDevice()
            except:
                pass
        return retval

    def allocateCapacity(self, props):
        retval = None
        if self.ref:
            try:
                retval = self.ref.allocateCapacity(props)
            except:
                raise
        return retval

    def deallocateCapacity(self, props):
        if self.ref:
            try:
                self.ref.deallocateCapacity(props)
            except:
                raise

    def load(self, fs, fileName, loadKind):
        if self.loadabledevice_ref:
            try:
                self.loadabledevice_ref.load(fs, fileName, loadKind)
            except:
                raise

    def unload(self, fileName):
        if self.loadabledevice_ref:
            try:
                self.loadabledevice_ref.unload(fileName)
            except:
                raise

    def execute(self, name, options, parameters):
        retval = None
        if self.executabledevice_ref:
            try:
                retval = self.executabledevice_ref.execute(name, options, parameters)
            except:
                raise
        return retval

    def terminate(self, processID):
        if self.executabledevice_ref:
            try:
                self.executabledevice_ref.terminate(processID)
            except:
                raise

    def _get_devices(self):
        retval = None
        if self.aggregatedevice_ref:
            try:
                retval = self.aggregatedevice_ref._get_devices()
            except:
                pass
        return retval

    def addDevice(self, associatedDevice):
        if self.aggregatedevice_ref:
            try:
                self.aggregatedevice_ref.addDevice(associatedDevice)
            except:
                raise

    def removeDevice(self, associatedDevice):
        if self.aggregatedevice_ref:
            try:
                self.aggregatedevice_ref.removeDevice(associatedDevice)
            except:
                raise
        
class DeviceManager(_CF__POA.DeviceManager, object):
    """The DeviceManager is a descriptor for an logical grouping of devices.

       Relevant member data:
       name - Node's name

    """
    def __init__(self, name="", devMgr=None, int_list=None, dcd=None, domain=None):
        self.name = name
        self.ref = devMgr
        self.devs = []
        self.id = ""
        self._interface_list = int_list
        self._domain = domain
        self._dcd = dcd
        self.fs = self.ref._get_fileSys()
        self._deviceManagerUpdated = False

        if self._domain == None:
            orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
            obj = orb.resolve_initial_references("NameService")
            self.rootContext = obj._narrow(_CosNaming.NamingContext)
        else:
            self.rootContext = self._domain.rootContext

        if self._interface_list == None:
            if not globals()["_loadedInterfaceList"]:
                globals()["_interface_list"] = _importIDL.importStandardIdl()
                globals()["_loadedInterfaceList"] = True
                self._interface_list = globals()["_interface_list"]
    
    ########################################
    # Begin external Device Manager API
    
    def _get_deviceConfigurationProfile(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_deviceConfigurationProfile()
            except:
                pass
        return retval
    
    def _get_fileSys(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_fileSys()
            except:
                pass
        return retval
    
    def _get_identifier(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_identifier()
            except:
                pass
        return retval
    
    def _get_label(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_label()
            except:
                pass
        return retval
    
    def _get_registeredDevices(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_registeredDevices()
            except:
                pass
        return retval
    
    def _get_registeredServices(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_registeredServices()
            except:
                pass
        return retval
    
    def registerDevice(self, registeringDevice):
        if self.ref:
            try:
                self.ref.registerDevice(registeringDevice)
            except:
                raise
    
    def unregisterDevice(self, registeredDevice):
        if self.ref:
            try:
                self.ref.unregisterDevice(registeredDevice)
            except:
                raise
    
    def shutdown(self):
        if self.ref:
            try:
                self.ref.shutdown()
            except:
                raise
    
    def registerService(self, registeringService):
        if self.ref:
            try:
                self.ref.registerService(registeringService)
            except:
                raise
    
    def unregisterService(self, registeredService):
        if self.ref:
            try:
                self.ref.unregisterService(registeredService)
            except:
                raise
    
    def getComponentImplementationId(self, componentInstantiationId):
        retval = None
        if self.ref:
            try:
                retval = self.ref.getComponentImplementationId(componentInstantiationId)
            except:
                raise
        return retval
    
    def getPort(self, name):
        retval = None
        if self.ref:
            try:
                retval = self.ref.getPort(name)
            except:
                raise
        return retval
    
    def configure(self, props):
        if self.ref:
            try:
                self.ref.configure(props)
            except:
                raise
    
    def query(self, props):
        retval = None
        if self.ref:
            try:
                retval = self.ref.query(props)
            except:
                raise
        return retval
    
    # End external Device Manager API
    ########################################
        
    def _populateDevices(self):
        """dev_list is a list of device names
           in_doc_sad is a parsed version of the SAD (using xml.dom.minidom)"""
        try:
            dev_list = self.ref._get_registeredDevices()
        except:
            return
        self.__setattr__('_deviceManagerUpdated', True)

        spd_list = {}
        for compfile in self._dcd.getElementsByTagName('componentfile'):
            spd_list[str(compfile.getAttribute('id'))] = str(compfile.getElementsByTagName('localfile')[0].getAttribute('name'))

        dce_list = {}
        for compplac in self._dcd.getElementsByTagName('componentplacement'):
            dce_list[str(compplac.getElementsByTagName('componentinstantiation')[0].getAttribute('id'))] = str(compplac.getElementsByTagName('componentfileref')[0].getAttribute('refid'))

        for comp_entry in dev_list:
            new_comp = Device(
                componentDescriptor="/"+self.name+comp_entry._get_softwareProfile(),
                componentObj=comp_entry._narrow(_CF.Device), 
                domainMgr=self._domain,
                int_list=self._interface_list)
            new_comp.updateReferences()

            foundObject = False
            try:
                new_comp._id = comp_entry._get_identifier()
                foundObject = True
            except:
                pass

            if not foundObject:
                print "Unable to get the pointer to Device "+comp_entry.identifier+", it is probably not running"

            self.devs.append(new_comp)
    
    def __getattribute__(self, name):
        try:
            if name == 'devs':
                if not object.__getattribute__(self,'_deviceManagerUpdated'):
                    self._populateDevices()

            return object.__getattribute__(self,name)
        except AttributeError:
            raise

class Domain(_CF__POA.DomainManager, object):
    """The Domain is a descriptor for a Domain Manager.
    
        The main functionality that can be exercised by this class is:
        - terminate - uninstalls all running waveforms and terminates the node
        - waveform management:
            - createApplication - install/create a particular waveform application
            - releaseApplication - release a particular waveform application
    """
    def __init__(self, name="DomainName1", int_list=None, location=None):
        self.name = name
        self._sads = []
        self.ref = None
        self.NodeAlive = True
        self._waveformsUpdated = False
        self._deviceManagersUpdated = False
        self.devMgrs = []
        self.location = location
        
        # create orb reference
        self.orb = _CORBA.ORB_init(_sys.argv, _CORBA.ORB_ID)
        if location:
            obj = self.orb.string_to_object('corbaname::'+location)
        else:
            obj = self.orb.resolve_initial_references("NameService")
        try:
            self.rootContext = obj._narrow(_CosNaming.NamingContext)
        except:
            raise RuntimeError('NameService not found')

        # get DomainManager reference
        dm_name = [_CosNaming.NameComponent(self.name,""),_CosNaming.NameComponent(self.name,"")]
        found_domain = False
        
        domain_find_attempts = 0
        
        self.poa = self.orb.resolve_initial_references("RootPOA")
        self.poaManager = self.poa._get_the_POAManager()
        self.poaManager.activate()
        
        while not found_domain and domain_find_attempts < 30:
            try:
                obj = self.rootContext.resolve(dm_name)
                found_domain = True
            except:
                _time.sleep(0.1)
                domain_find_attempts += 1
        
        if domain_find_attempts == 30:
            raise StandardError, "Did not find domain "+name
                
        self.ref = obj._narrow(_CF.DomainManager)
        self.fileManager = self.ref._get_fileMgr()
        
        if int_list == None:
            self._interface_list = _importIDL.importStandardIdl()
        else:
            self._interface_list = int_list
        
        for int_entry in self._interface_list:
            _interfaces[int_entry.repoId]=int_entry
    
    def _populateDeviceManagers(self):
        try:
            devmgr_seq = object.__getattribute__(self, 'ref')._get_deviceManagers()
        except:
            return None

        self.__setattr__('_deviceManagersUpdated', True)

        for devMgr in devmgr_seq:
            try:
                dcdPath = devMgr._get_deviceConfigurationProfile()
                devMgrFileSys = devMgr._get_fileSys()
                dcdFile = devMgrFileSys.open(dcdPath, True)
            except:
                print "Unable to open $SDRROOT/dev"+dcdPath+". Unable to add Device Manager '"+devMgr._get_label()+"'"
                continue
            dcdContents = dcdFile.read(dcdFile.sizeOf())
            dcdFile.close()

            parsed_dcd=_minidom.parseString(dcdContents)

            self._addDeviceManager(DeviceManager(name=devMgr._get_label(), 
                                                 devMgr=devMgr, 
                                                 int_list=object.__getattribute__(self, '_interface_list'), 
                                                 dcd=parsed_dcd, domain=self))
            
    def _populateApps(self):
        self.__setattr__('_waveformsUpdated', True)
        self._updateListAvailableSads()
    
    @property
    def apps(self):
        apps = []
        # Gets current list of apps from the domain manager
        try:
            app_list = self.ref._get_applications()
        except:
            return
    
        app_name_list = []
    
        for app in app_list:
            prof_path = app._get_profile()
            
            sadFile = self.fileManager.open(prof_path, True)
            sadContents = sadFile.read(sadFile.sizeOf())
            sadFile.close()
            doc_sad = _minidom.parseString(sadContents)
            comp_list = app._get_componentNamingContexts()
            waveform_ns_name = ''
            if len(comp_list) > 0:
                comp_ns_name = comp_list[0].elementId
                waveform_ns_name = comp_ns_name.split('/')[1]
    
            app_name_list.append(waveform_ns_name)
    
            app_name = app._get_name()
            if app_name[:7]=='OSSIE::':
                waveform_name = app_name[7:]
            else:
                waveform_name = app_name
            waveform_entry = App(name=waveform_name, int_list=self._interface_list, domain=self, sad=doc_sad)
            waveform_entry.ref = app
            waveform_entry.ns_name = waveform_ns_name
            waveform_entry.update()
    
            apps.append(waveform_entry)        
        return apps       
    
    def __getattribute__(self, name):
        try:
            if name == 'devMgrs':
                if not object.__getattribute__(self,'_deviceManagersUpdated'):
                    self._populateDeviceManagers()

            return object.__getattribute__(self,name)
        except AttributeError:
            raise
    
    def __del__(self):
        """
            Destructor
        """
        pass
    
    ########################################
    # External Domain Manager API
    
    def _get_identifier(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_identifier()
            except:
                pass
        return retval
    
    def _get_deviceManagers(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_deviceManagers()
            except:
                pass
        return retval
    
    def _get_applications(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_applications()
            except:
                pass
        return retval
    
    def _get_applicationFactories(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_applicationFactories()
            except:
                pass
        return retval
    
    def _get_fileMgr(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_fileMgr()
            except:
                pass
        return retval
    
    def _get_domainManagerProfile(self):
        retval = None
        if self.ref:
            try:
                retval = self.ref._get_domainManagerProfile()
            except:
                pass
        return retval
    
    def configure(self, props):
        if self.ref:
            try:
                self.ref.configure(props)
            except:
                raise
    
    def query(self, props):
        retval = None
        if self.ref:
            try:
                retval = self.ref.query(props)
            except:
                raise
        return retval
    
    def registerDevice(self, device, deviceManager):
        if self.ref:
            try:
                self.ref.registerDevice(device, deviceManager)
            except:
                raise
    
    def registerDeviceManager(self, deviceManager):
        if self.ref:
            try:
                self.ref.registerDeviceManager(deviceManager)
            except:
                raise
    
    def unregisterDevice(self, device):
        if self.ref:
            try:
                self.ref.unregisterDevice(device)
            except:
                raise
    
    def unregisterDeviceManager(self, deviceManager):
        if self.ref:
            try:
                self.ref.unregisterDeviceManager(deviceManager)
            except:
                raise
    
    def installApplication(self, profile):
        if self.ref:
            try:
                self.ref.installApplication(profile)
            except:
                raise
    
    def uninstallApplication(self, appid):
        if self.ref:
            try:
                self.ref.uninstallApplication(appid)
            except:
                raise
    
    def registerService(self, service, deviceManager, name):
        if self.ref:
            try:
                self.ref.registerService(service, deviceManager, name)
            except:
                raise
    
    def unregisterService(self, service, name):
        if self.ref:
            try:
                self.ref.unregisterService(service, name)
            except:
                raise
    
    def registerWithEventChannel(self, registeringObject, registeringId, eventChannelName):
        if self.ref:
            try:
                self.ref.registerWithEventChannel(registeringObject, registeringId, eventChannelName)
            except:
                raise
    
    def unregisterFromEventChannel(self, unregisteringId, eventChannelName):
        if self.ref:
            try:
                self.ref.unregisterFromEventChannel(unregisteringId, eventChannelName)
            except:
                raise
    
    # End external Domain Manager API
    ########################################
    
    def _addDeviceManager(self, in_node=None):
        if in_node != None:
            self.devMgrs.append(in_node)
    
    def _searchFilePattern(self, starting_point, pattern):
        file_list = self.fileManager.list(starting_point+'/*')
        filesFound = []
        for entry in file_list:
            if entry.kind == _CF.FileSystem.DIRECTORY:
                if starting_point == '/':
                    filesFound.extend(self._searchFilePattern(starting_point+entry.name, pattern))
                else:
                    filesFound.extend(self._searchFilePattern(starting_point+'/'+entry.name, pattern))
            else:
                if pattern in entry.name:
                    filesFound.append(starting_point+'/'+entry.name)
        return filesFound
    
    def _updateListAvailableSads(self):
        """
            Update available waveforms list.
        """
        sadList = self._searchFilePattern('/waveforms', 'sad.xml')
        for entry in range(len(sadList)):
            self._sads.append(sadList[entry].split('/')[-2])
    
    def terminate(self):
        # Kills waveforms (in reverse order since removeApplication() pops items from list) 
        for app in reversed(self.apps):
            self.removeApplication(app)
        
        # Kills nodes
        for node in self.devMgrs:
            node.shutdown()
    
    def removeApplication(self, app_obj=None):
        if app_obj == None:
            return
        app_idx = 0
        for app_idx in range(len(self.apps)):
            if self.apps[app_idx].ref._is_equivalent(app_obj.ref):
                break

        if app_idx == len(self.apps):
            return
        
        appId = self.apps[app_idx]._get_identifier()
        for app in _launchedApps:
            if app._get_identifier() == appId:
                _launchedApps.remove(app)
                break
            
        app_obj.ref.releaseObject()

    def createApplication(self, application_sad=''):
        """Install and create a particular waveform. This function returns
            a pointer to the instantiated waveform"""
        uninstallAppWhenDone = True
        # If only an application name is given, format it properly
        if application_sad[0] != "/" and not ".sad.xml" in application_sad:
            application_sad = "/waveforms/" + application_sad + "/" + application_sad + ".sad.xml"
        
        try:
            self.ref.installApplication(application_sad)
        except _CF.DomainManager.ApplicationAlreadyInstalled:
            uninstallAppWhenDone = False
    
        sadFile = self.fileManager.open(application_sad, True)
        sadContents = sadFile.read(sadFile.sizeOf())
        sadFile.close()
    
        doc_sad = _minidom.parseString(sadContents)
    
        # get a list of the application factories in the Domain
        _applicationFactories = self.ref._get_applicationFactories()
    
        # find the application factory that is needed
        app_name = str(doc_sad.getElementsByTagName('softwareassembly')[0].getAttribute('name'))
        app_factory_num = -1
        for app_num in range(len(_applicationFactories)):
            if _applicationFactories[app_num]._get_name()==app_name:
                app_factory_num = app_num
                break
    
        if app_factory_num == -1:
            raise AssertionError("Application factory not found")
    
        _appFacProps = []
    
        try:
            app = _applicationFactories[app_factory_num].create(_applicationFactories[app_factory_num]._get_name()+"_"+getCurrentDateTimeString(),_appFacProps,[])
        except:
            if uninstallAppWhenDone:
                self.ref.uninstallApplication(_applicationFactories[app_factory_num]._get_identifier())
            print "Unable to create application - make sure that all appropriate nodes are installed"
            return
        
        comp_list_1 = app._get_componentNamingContexts()
        waveform_ns_name = ''
        if len(comp_list_1) > 0:
            comp_ns_name = comp_list_1[0].elementId
            waveform_ns_name = comp_ns_name.split('/')[1]
        
        waveform_entry = App(name=app._get_name(), int_list=self._interface_list, domain=self, sad=doc_sad)
        waveform_entry.ref = app
        waveform_entry.ns_name = waveform_ns_name
        _launchedApps.append(waveform_entry)
        waveform_entry.update()
        
        if uninstallAppWhenDone:
            self.ref.uninstallApplication(_applicationFactories[app_factory_num]._get_identifier())
    
        return waveform_entry
    
    def _updateRunningApps(self):
        """Makes sure that the dictionary of waveforms is up-to-date"""
        print "WARNING: _updateRunningApps() is deprecated.  Running apps are automatically updated on access."


class _Port(object):
    """The Port is the gateway into and out of a particular component. A Port has a string name that is unique
        to that port within the context of a particular component.
        
        There are two types of Ports: Uses (output) and Provides (input).
    """
    def __init__(self, name, interface=None, direction="Uses",portType="data",using=None):
        self.extendedFunctionList = []
        self.name = name
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
