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

from ossie.properties import __TYPE_MAP, _SCA_TYPES
from ossie.utils import type_helpers as _type_helpers
from ossie.cf import CF as _CF
from ossie.cf import PortTypes as _PortTypes
from omniORB import any as _any
from omniORB import CORBA as _CORBA
from omniORB import tcInternal as _tcInternal
import copy as _copy
import struct as _struct
from ossie.utils.type_helpers import OutOfRangeException

SCA_TYPES = globals()['_SCA_TYPES']
TYPE_MAP = globals()['__TYPE_MAP']

#helper function for configuring simple and sequence properties
def configureProp(compRef, propName, propValue):
    propRef = _CF.DataType(id=str(str(propName)), value=_any.to_any(None))
    
    #try the query to see if we can get the valid type
    results = compRef.query([propRef])
    if results != None:
        if results[0].value._t != _CORBA.TC_null:
            propRef.value._t = esults[0].value._t
            propRef.value._v = propValue
            compRef.configure([propRef])
            return
    
    #if we couldn't find the type of the property, then we need
    #to try ones that might fit until one works
    applicableTypes = None
    if type(propValue) == list:
        strTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.StringSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.CharSeq)]
        boolTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.BooleanSeq)]
        longTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.ULongSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ShortSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.OctetSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.UShortSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.LongSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.LongLongSequence),
                       _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.UlongLongSequence)]
        floatTypes = [_tcInternal.typeCodeFromClassOrRepoId(_CORBA.FloatSeq),
                       _tcInternal.typeCodeFromClassOrRepoId(_CORBA.DoubleSeq)]
        
        if len(propValue) > 0:
            valueType = type(propValue[0])
        else:
            applicableTypes = strListTypes + boolListTypes + longListTypes + floatListTypes
    else:
        strTypes = [TYPE_MAP['string'][1], TYPE_MAP['char'][1]]
        boolTypes = [TYPE_MAP['boolean'][1]]
        longTypes = [TYPE_MAP['ulong'][1], TYPE_MAP['short'][1], TYPE_MAP['octet'][1], TYPE_MAP['ushort'][1],
                     TYPE_MAP['long'][1], TYPE_MAP['longlong'][1], TYPE_MAP['ulonglong'][1]]
        floatTypes = [TYPE_MAP['float'][1], TYPE_MAP['double'][1]]
        
        valueType = type(propValue)
        
    if not applicableTypes:
        if valueType == str:
            applicableTypes = strTypes
        elif valueType == bool:
            applicableTypes = boolTypes
        elif valueType == long:
            applicableTypes = longTypes
        elif valueType == float:
            applicableTypes = floatTypes
        else:
            raise Exception, 'Could not match "'+str(valueType)+'" to a valid CORBA type'
    
    passConfigure = False
    for propType in applicableTypes:
        propRef.value._t = propType
        propRef.value._v = propValue
        try:
            compRef.configure([propRef])
            passConfigure = True
            break
        except:
            pass
    
    if not passConfigure:
        msg = 'Was not able to configure property: "'+str(propName)+'", trying the following types:\n'
        for propType in applicableTypes:
            msg += ('  ' + str(propType) + '\n')
        raise Exception, msg

def getPropNameDict(prf):
    #
    # make a dictionary of all of the IDs indexed by name
    #
    nameDict = {}

    # do it for all 'simple' and 'simplesequence' properties
    for prop in prf.get_simple() + prf.get_simplesequence():
        name = prop.get_name()
        if prop.get_name() == None:
            name = prop.get_id()
        else:
            name = prop.get_name()
        if nameDict.has_key( str(name)):
            print "WARN: property with non-unique name %s" % name
            continue
      
        nameDict[str(name)] = str(prop.get_id())

    # do it for all 'struct' properties
    for struct in prf.get_struct():
        if struct.get_name() == None:
            tagbase = str(struct.get_id())
        else:
            tagbase = str(struct.get_name())
        if nameDict.has_key( tagbase ):
            print "WARN: struct with duplicate name %s" % tagbase
            continue
      
        nameDict[str(tagbase)] = str(struct.get_id())
        tagbase = tagbase + "."
        for simple in struct.get_simple():
            if simple.get_name() == None:
                name = simple.get_id()
            else:
                name = simple.get_name()
            if nameDict.has_key( tagbase + str(name) ):
                print "WARN: struct element with duplicate name %s" % tagbase
                continue

            nameDict[str(tagbase + str(name))] = str(simple.get_id())

    # do it for all 'struct' sequences
    tagbase = ""
    for structSequence in prf.get_structsequence():
        if structSequence.get_name() == None:
            tagbase = str(structSequence.get_id())
        else:
            tagbase = str(structSequence.get_name())
        # make sure this structSequence name is unique
        if nameDict.has_key( str(tagbase) ):
            print "WARN: property with non-unique name %s" % tagbase
            continue

        nameDict[str(tagbase)] = str(structSequence.get_id())
        tagbase = tagbase + "."
        struct = structSequence.get_struct()

        # pull out all of the elements of the struct
        for simple in struct.get_simple():
            # make sure this struct simple element's name is unique
            if simple.get_name() == None:
                name = simple.get_id()
            else:
                name = simple.get_name()
            if nameDict.has_key( tagbase + str(name)):
                print "WARN: property with non-unique name %s" % name
                continue

            nameDict[str(tagbase + str(name))] = str(simple.get_id())
          
    return nameDict


_enums = {}
_displayNames = {}
_duplicateNames = {}

'''
-Maps a properties clean, display and access name to its ID
-Prevents duplicate entries within a component
-Allows for get/set on components with invalid chars in ID
'''
def addCleanName(cleanName, id, compRefId):
    if not _displayNames.has_key(compRefId):
        _displayNames[compRefId] = {}
        _duplicateNames[compRefId] = {}
        
    if not _displayNames[compRefId].has_key(cleanName):
        _displayNames[compRefId][cleanName] = id
        _duplicateNames[compRefId][cleanName] = 0
        return cleanName
    elif _displayNames[compRefId][cleanName] == id:
        return cleanName
    else:
        count = _duplicateNames[compRefId][cleanName] + 1
        _displayNames[compRefId][cleanName + str(count)] = id
        _duplicateNames[compRefId][cleanName] = count
        return cleanName + str(count)
    
'''
-Adds a property to the enumerated properties map
-This allows enforcement of enumerations as the property
values are configured
'''
def _addEnumerations(prop, clean_id):
    propType = prop.get_type()
    for en in prop.enumerations.enumeration:
        if str(en.get_value()) == str(None):
            en.set_value(None) 
        elif propType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
            if en.get_value().find('x') != -1:
                en.set_value(int(en.get_value(),16))
            else:
                en.set_value(int(en.get_value()))
        elif propType in ['double', 'float']:
            en.set_value(float(en.get_value()))
        elif propType in ['char', 'string']:
            en.set_value(str(en.get_value()))
        elif propType == 'boolean':
            en.set_value({"TRUE": True, "FALSE": False}[en.get_value().strip().upper()])
        else:
            en.set_value(None)                          
    _enums[clean_id] = prop.enumerations
    
'''
THE CLASSES BELOW SERVE AS WRAPPER CLASSES TO BE USED EXTENSIVELY IN
THE SANDBOX BUT CAN BE USED FOR POTENTIAL UNIT TESTING SCHEMES AS WELL
'''
class Property(object):
    """
    This class will act as a wrapper for a single property, and provide some
    helper type functionality to ease the use of properties
    """
    MODES = ['readwrite', 'writeonly', 'readonly']
    
    def __init__(self, id, type, compRef, mode='readwrite'):
        """ 
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        type - (string): type of property (SCA Type or 'struct' or 'structSequence')
        id - (string): the property ID
        mode - (string): mode for the property, must be in MODES
        """
        self.id = id
        self.type = type
        self.compRef = compRef
        if mode not in self.MODES:
            print str(mode) + ' is not a valid mode, defaulting to "readwrite"'
            self.mode = 'readwrite'
        else:
            self.mode = mode            

class simpleProperty(Property):
    def __init__(self, id, valueType, compRef, defValue=None, structRef=None, structSeqRef=None, structSeqIdx=None, mode='readwrite'):
        """ 
        id - (string): the property ID
        valueType - (string): type of the property, must be in VALUE_TYPES
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        structRef - (string): name of the struct that this simple is a member of, or None
        structSeqRef - (string): name of the struct sequence the above struct is a member of, or None
        structSeqIdx - (int): index of the above struct in the struct sequence, or None
        mode - (string): mode for the property, must be in MODES
        """
        if valueType not in SCA_TYPES:
            raise('"' + str(valueType) + '"' + ' is not a valid valueType, choose from\n ' + str(SCA_TYPES))
        
        #initilaize the parent
        Property.__init__(self, id, type=valueType, compRef=compRef, mode=mode)
        
        self.valueType = valueType
        self.defValue = defValue
        
        #used when the simple is part of a struct
        self.structRef = structRef
        
        #used when the struct is part of a struct sequence
        self.structSeqRef = structSeqRef
        self.structSeqIdx = structSeqIdx
        
        #create the CF.DataType reference, change value to the correct type
        self.propRef = _CF.DataType(id=str(self.id), value=_any.to_any(None))
        self.propRef.value._t = TYPE_MAP[self.valueType][1]
    
    def queryValue(self):
        '''
        Returns the current value of the property by doing a query
        on the component and returning only the value
        '''
        if self.mode == 'writeonly':
            raise Exception, 'Could not perform query, ' + str(self.id) + ' is a writeonly property'
        else:
            #check if the simple is part of a struct
            if self.structRef != None:
                #check if the struct is part of a struct sequence
                if self.structSeqRef != None:
                    #get the struct sequence from the component and do a query
                    structSeqProp = self.compRef.__getattribute__(self.structSeqRef)
                    queryRef = _copy.deepcopy(structSeqProp.propRef)
                    results = self.compRef.query([queryRef])
                    if results == None:
                        return None
                    else:
                        structRefList = results[0].value.value()
                        
                        #find the right struct
                        structRef = structRefList[self.structSeqIdx].value()
                        for simpleRef in structRef:
                            if simpleRef.id == self.id:
                                return simpleRef.value.value()
                else:
                    #get the struct from the component and do a query
                    structProp = self.compRef.__getattribute__(self.structRef)
                    queryRef = _copy.deepcopy(structProp.propRef)
                    queryRef.value = _any.to_any(None)
                    results = self.compRef.query([queryRef])
                    if results == None:
                        return None
                    else:
                        structRef = results[0].value.value() 
                        for simpleRef in structRef:
                            if simpleRef.id == self.id:
                                return simpleRef.value.value()
            else:
                queryRef = _copy.deepcopy(self.propRef)
                queryRef.value = _any.to_any(None)
                results = self.compRef.query([queryRef])
                if results == None:
                    return None
                return results[0].value.value()
    
    def configureValue(self, value):
        '''
        Helper function for configuring a simple property
        '''
        if self.mode == 'readonly':
            raise Exception, 'Could not perform configure, ' + str(self.id) + ' is a readonly property'
        else:
            if value != None:
                # If property is an enumeration, enforce proper value
                if self.id in _enums.keys():
                    found = False
                    for en in _enums[self.id].enumeration:
                        if value == en.get_label():
                            value = en.get_value()
                            found = True
                        elif value == en.get_value():
                            found = True
                    # If enumeration is invalid list available enumerations
                    if not found:
                        print 'Could not perform configure on ' + str(self.id) + ', invalid enumeration provided'
                        print "Valid enumerations: "
                        for en in _enums[self.id].enumeration:
                            print "\t" + str(en.get_label()) + "=" + str(en.get_value())
                        return
                        
                if type(value) not in [str,int,long,bool,float]:
                        raise TypeError, 'configureValue() must be called with str, int, bool, or float instance as second argument (got ' + str(type(value))[7:-2] + ' instance instead)'
                #validate the value and provide more meaningful exception
                value = _type_helpers.checkValidValue(value, self.valueType)
            
            #check if the simple is part of a struct
            if self.structRef != None:
                #check if the struct is part of a struct sequence
                if self.structSeqRef != None:
                    #get the struct sequence from the component and do a query
                    structSeqProp = self.compRef.__getattribute__(self.structSeqRef)
                    if structSeqProp.mode == 'readonly':
                        raise Exception, 'Could not perform configure, ' + str(structSeqProp.id) + ' is a readonly property'
                    else:
                        queryRef = _copy.deepcopy(structSeqProp.propRef)
                        results = self.compRef.query([queryRef])
                        if results == None:
                            structRefList = []
                        else:
                            structRefList = results[0].value.value()
                            
                            #find the right struct
                            structRef = structRefList[self.structSeqIdx].value()
                            
                            #find the member and change it
                            for simpleRef in structRef:
                                if simpleRef.id == self.id:
                                    if value == None:
                                        simpleRef.value = _any.to_any(None)
                                    else:
                                        simpleRef.value._v = value
                                        simpleRef.value._t = self.propRef.value._t
                                    break
                            
                            #copy in all of the struct instances and then configure
                            configRef = _copy.deepcopy(structSeqProp.propRef)
                            for instance in structRefList:
                                configRef.value._v.append(instance)
                            self.compRef.configure([configRef])
                else:
                    #get the struct from the component and do a query
                    structProp = self.compRef.__getattribute__(self.structRef)
                    if structProp.mode == 'readonly':
                        raise Exception, 'Could not perform configure, ' + str(structProp.id) + ' is a readonly property'
                    else :
                        queryRef = _copy.deepcopy(structProp.propRef)
                        queryRef.value = _any.to_any(None)
                        results = self.compRef.query([queryRef])
                        if results == None:
                            structRefList = []
                        else:
                            structRef = results[0].value.value()
                            
                            #find the member and change it
                            for simpleRef in structRef:
                                if simpleRef.id == self.id:
                                    if value == None:
                                        simpleRef.value = _any.to_any(None)
                                    else:
                                        simpleRef.value._v = value
                                        simpleRef.value._t = self.propRef.value._t
                                    break
                            
                            #copy in all of the struct's members and then configure
                            configRef = _copy.deepcopy(structProp.propRef)
                            configRef.value._v = []
                            for simpleRef in structRef:
                                configRef.value._v.append(simpleRef)
                            self.compRef.configure([configRef])
            else:
                configRef = _copy.deepcopy(self.propRef)
                if value == None:
                    configRef.value = _any.to_any(None)
                    self.compRef.configure([configRef])
                else:
                    configRef.value._v = value
                    self.compRef.configure([configRef])


    '''
    BELOW ARE OVERRIDDEN FUNCTIONS FOR LITERAL TYPES IN
    PYTHON SO THAT THE PROPERTIES BEHAVE WITHIN THE CODE
    AS IF THEY WERE THEIR LITERAL TYPES
    '''
    def __abs__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__abs__(val, *args)
            if self.type in ['double', 'float']:
                return float.__abs__(val, *args)
            if self.type in ['boolean']:
                return bool.__abs__(val, *args)
            if self.type in ['char', 'string']:
                return str.__abs__(val, *args)
        except:
            raise
    
    def __add__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__add__(val, *args)
            if self.type in ['double', 'float']:
                return float.__add__(val, *args)
            if self.type in ['boolean']:
                return bool.__add__(val, *args)
            if self.type in ['char', 'string']:
                return str.__add__(val, *args)
        except:
            raise
    
    def __and__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__and__(val, *args)
            if self.type in ['double', 'float']:
                return float.__and__(val, *args)
            if self.type in ['boolean']:
                return bool.__and__(val, *args)
            if self.type in ['char', 'string']:
                return str.__and__(val, *args)
        except:
            raise
    
    def __contains__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__contains__(val, *args)
            if self.type in ['double', 'float']:
                return float.__contains__(val, *args)
            if self.type in ['boolean']:
                return bool.__contains__(val, *args)
            if self.type in ['char', 'string']:
                return str.__contains__(val, *args)
        except:
            raise
    
    def __div__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__div__(val, *args)
            if self.type in ['double', 'float']:
                return float.__div__(val, *args)
            if self.type in ['boolean']:
                return bool.__div__(val, *args)
            if self.type in ['char', 'string']:
                return str.__div__(val, *args)
        except:
            raise
    
    def __divmod__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__divmod__(val, *args)
            if self.type in ['double', 'float']:
                return float.__divmod__(val, *args)
            if self.type in ['boolean']:
                return bool.__divmod__(val, *args)
            if self.type in ['char', 'string']:
                return str.__divmod__(val, *args)
        except:
            raise
    
    def __eq__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return val == args[0]
            if self.type in ['double', 'float']:
                return float.__eq__(val, *args)
            if self.type in ['boolean']:
                return val == args[0]
            if self.type in ['char', 'string']:
                if val == None:
                    return val == args[0]
                return str.__eq__(val, *args)
        except:
            raise
    
    def __floordiv__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__floordiv__(val, *args)
            if self.type in ['double', 'float']:
                return float.__floordiv__(val, *args)
            if self.type in ['boolean']:
                return bool.__floordiv__(val, *args)
            if self.type in ['char', 'string']:
                return str.__floordiv__(val, *args)
        except:
            raise
    
    def __ge__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return val >= args[0]
            if self.type in ['double', 'float']:
                return float.__ge__(val, *args)
            if self.type in ['boolean']:
                return val >= args[0]
            if self.type in ['char', 'string']:
                return str.__ge__(val, *args)
        except:
            raise
    
    def __getitem__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__getitem__(val, *args)
            if self.type in ['double', 'float']:
                return float.__getitem__(val, *args)
            if self.type in ['boolean']:
                return bool.__getitem__(val, *args)
            if self.type in ['char', 'string']:
                return str.__getitem__(val, *args)
        except:
            raise
    
    def __getslice__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__getslice__(val, *args)
            if self.type in ['double', 'float']:
                return float.__getslice__(val, *args)
            if self.type in ['boolean']:
                return bool.__getslice__(val, *args)
            if self.type in ['char', 'string']:
                return str.__getslice__(val, *args)
        except:
            raise
    
    def __gt__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return val > args[0]
            if self.type in ['double', 'float']:
                return float.__gt__(val, *args)
            if self.type in ['boolean']:
                return val > args[0]
            if self.type in ['char', 'string']:
                return str.__gt__(val, *args)
        except:
            raise
    
    def __invert__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__invert__(val, *args)
            if self.type in ['double', 'float']:
                return float.__invert__(val, *args)
            if self.type in ['boolean']:
                return bool.__invert__(val, *args)
            if self.type in ['char', 'string']:
                return str.__invert__(val, *args)
        except:
            raise

    def __nonzero__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__nonzero__(val, *args)
            if self.type in ['double', 'float']:
                return float.__nonzero__(val, *args)
            if self.type in ['boolean']:
                return bool.__nonzero__(val, *args)
            if self.type in ['char', 'string']:
                return str.__nonzero__(val, *args)
        except:
            raise
    
    def __le__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return val <= args[0]
            if self.type in ['double', 'float']:
                return float.__le__(val, *args)
            if self.type in ['boolean']:
                return val <= args[0]
            if self.type in ['char', 'string']:
                return str.__le__(val, *args)
        except:
            raise
    
    def __lshift__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__lshift__(val, *args)
            if self.type in ['double', 'float']:
                return float.__lshift__(val, *args)
            if self.type in ['boolean']:
                return bool.__lshift__(val, *args)
            if self.type in ['char', 'string']:
                return str.__lshift__(val, *args)
        except:
            raise
    
    def __lt__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return val < args[0]
            if self.type in ['double', 'float']:
                return float.__lt__(val, *args)
            if self.type in ['boolean']:
                return val < args[0]
            if self.type in ['char', 'string']:
                return str.__lt__(val, *args)
        except:
            raise
    
    def __mod__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__mod__(val, *args)
            if self.type in ['double', 'float']:
                return float.__mod__(val, *args)
            if self.type in ['boolean']:
                return bool.__mod__(val, *args)
            if self.type in ['char', 'string']:
                return str.__mod__(val, *args)
        except:
            raise
    
    def __mul__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__mul__(val, *args)
            if self.type in ['double', 'float']:
                return float.__mul__(val, *args)
            if self.type in ['boolean']:
                return bool.__mul__(val, *args)
            if self.type in ['char', 'string']:
                return str.__mul__(val, *args)
        except:
            raise
    
    def __neg__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__neg__(val, *args)
            if self.type in ['double', 'float']:
                return float.__neg__(val, *args)
            if self.type in ['boolean']:
                return bool.__neg__(val, *args)
            if self.type in ['char', 'string']:
                return str.__neg__(val, *args)
        except:
            raise
    
    def __or__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__or__(val, *args)
            if self.type in ['double', 'float']:
                return float.__or__(val, *args)
            if self.type in ['boolean']:
                return bool.__or__(val, *args)
            if self.type in ['char', 'string']:
                return str.__or__(val, *args)
        except:
            raise
    
    def __pow__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__pow__(val, *args)
            if self.type in ['double', 'float']:
                return float.__pow__(val, *args)
            if self.type in ['boolean']:
                return bool.__pow__(val, *args)
            if self.type in ['char', 'string']:
                return str.__pow__(val, *args)
        except:
            raise
    
    def __radd__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__radd__(val, *args)
            if self.type in ['double', 'float']:
                return float.__radd__(val, *args)
            if self.type in ['boolean']:
                return bool.__radd__(val, *args)
            if self.type in ['char', 'string']:
                return str.__radd__(val, *args)
        except:
            raise
    
    def __rand__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rand__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rand__(val, *args)
            if self.type in ['boolean']:
                return bool.__rand__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rand__(val, *args)
        except:
            raise
    
    def __rdiv__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rdiv__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rdiv__(val, *args)
            if self.type in ['boolean']:
                return bool.__rdiv__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rdiv__(val, *args)
        except:
            raise
    
    def __rdivmod__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rdivmod__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rdivmod__(val, *args)
            if self.type in ['boolean']:
                return bool.__rdivmod__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rdivmod__(val, *args)
        except:
            raise
    
    def __rfloordiv__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rfloordiv__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rfloordiv__(val, *args)
            if self.type in ['boolean']:
                return bool.__rfloordiv__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rfloordiv__(val, *args)
        except:
            raise
    
    def __rlshift__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rlshift__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rlshift__(val, *args)
            if self.type in ['boolean']:
                return bool.__rlshift__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rlshift__(val, *args)
        except:
            raise
    
    def __rmod__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rmod__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rmod__(val, *args)
            if self.type in ['boolean']:
                return bool.__rmod__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rmod__(val, *args)
        except:
            raise
    
    def __rmul__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rmul__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rmul__(val, *args)
            if self.type in ['boolean']:
                return bool.__rmul__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rmul__(val, *args)
        except:
            raise
    
    def __ror__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__ror__(val, *args)
            if self.type in ['double', 'float']:
                return float.__ror__(val, *args)
            if self.type in ['boolean']:
                return bool.__ror__(val, *args)
            if self.type in ['char', 'string']:
                return str.__ror__(val, *args)
        except:
            raise
    
    def __rpow__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rpow__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rpow__(val, *args)
            if self.type in ['boolean']:
                return bool.__rpow__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rpow__(val, *args)
        except:
            raise
    
    def __rrshift__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rrshift__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rrshift__(val, *args)
            if self.type in ['boolean']:
                return bool.__rrshift__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rrshift__(val, *args)
        except:
            raise
    
    def __rshift__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rshift__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rshift__(val, *args)
            if self.type in ['boolean']:
                return bool.__rshift__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rshift__(val, *args)
        except:
            raise
    
    def __rsub__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rsub__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rsub__(val, *args)
            if self.type in ['boolean']:
                return bool.__rsub__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rsub__(val, *args)
        except:
            raise
    
    def __rtruediv__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rtruediv__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rtruediv__(val, *args)
            if self.type in ['boolean']:
                return bool.__rtruediv__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rtruediv__(val, *args)
        except:
            raise
    
    def __rxor__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__rxor__(val, *args)
            if self.type in ['double', 'float']:
                return float.__rxor__(val, *args)
            if self.type in ['boolean']:
                return bool.__rxor__(val, *args)
            if self.type in ['char', 'string']:
                return str.__rxor__(val, *args)
        except:
            raise
    
    def __sub__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__sub__(val, *args)
            if self.type in ['double', 'float']:
                return float.__sub__(val, *args)
            if self.type in ['boolean']:
                return bool.__sub__(val, *args)
            if self.type in ['char', 'string']:
                return str.__sub__(val, *args)
        except:
            raise
    
    def __truediv__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__truediv__(val, *args)
            if self.type in ['double', 'float']:
                return float.__truediv__(val, *args)
            if self.type in ['boolean']:
                return bool.__truediv__(val, *args)
            if self.type in ['char', 'string']:
                return str.__truediv__(val, *args)
        except:
            raise
    
    def __xor__(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.__xor__(val, *args)
            if self.type in ['double', 'float']:
                return float.__xor__(val, *args)
            if self.type in ['boolean']:
                return bool.__xor__(val, *args)
            if self.type in ['char', 'string']:
                return str.__xor__(val, *args)
        except:
            raise
    
    def capitalize(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.capitalize(val, *args)
            if self.type in ['double', 'float']:
                return float.capitalize(val, *args)
            if self.type in ['boolean']:
                return bool.capitalize(val, *args)
            if self.type in ['char', 'string']:
                return str.capitalize(val, *args)
        except:
            raise
    
    def center(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.center(val, *args)
            if self.type in ['double', 'float']:
                return float.center(val, *args)
            if self.type in ['boolean']:
                return bool.center(val, *args)
            if self.type in ['char', 'string']:
                return str.center(val, *args)
        except:
            raise
    
    def count(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.count(val, *args)
            if self.type in ['double', 'float']:
                return float.count(val, *args)
            if self.type in ['boolean']:
                return bool.count(val, *args)
            if self.type in ['char', 'string']:
                return str.count(val, *args)
        except:
            raise
    
    def decode(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.decode(val, *args)
            if self.type in ['double', 'float']:
                return float.decode(val, *args)
            if self.type in ['boolean']:
                return bool.decode(val, *args)
            if self.type in ['char', 'string']:
                return str.decode(val, *args)
        except:
            raise
    
    def encode(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.encode(val, *args)
            if self.type in ['double', 'float']:
                return float.encode(val, *args)
            if self.type in ['boolean']:
                return bool.encode(val, *args)
            if self.type in ['char', 'string']:
                return str.encode(val, *args)
        except:
            raise
    
    def endswith(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.endswith(val, *args)
            if self.type in ['double', 'float']:
                return float.endswith(val, *args)
            if self.type in ['boolean']:
                return bool.endswith(val, *args)
            if self.type in ['char', 'string']:
                return str.endswith(val, *args)
        except:
            raise
    
    def expandtabs(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.expandtabs(val, *args)
            if self.type in ['double', 'float']:
                return float.expandtabs(val, *args)
            if self.type in ['boolean']:
                return bool.expandtabs(val, *args)
            if self.type in ['char', 'string']:
                return str.expandtabs(val, *args)
        except:
            raise
    
    def find(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.find(val, *args)
            if self.type in ['double', 'float']:
                return float.find(val, *args)
            if self.type in ['boolean']:
                return bool.find(val, *args)
            if self.type in ['char', 'string']:
                return str.find(val, *args)
        except:
            raise
    
    def index(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.index(val, *args)
            if self.type in ['double', 'float']:
                return float.index(val, *args)
            if self.type in ['boolean']:
                return bool.index(val, *args)
            if self.type in ['char', 'string']:
                return str.index(val, *args)
        except:
            raise
    
    def isalnum(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.isalnum(val, *args)
            if self.type in ['double', 'float']:
                return float.isalnum(val, *args)
            if self.type in ['boolean']:
                return bool.isalnum(val, *args)
            if self.type in ['char', 'string']:
                return str.isalnum(val, *args)
        except:
            raise
    
    def isalpha(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.isalpha(val, *args)
            if self.type in ['double', 'float']:
                return float.isalpha(val, *args)
            if self.type in ['boolean']:
                return bool.isalpha(val, *args)
            if self.type in ['char', 'string']:
                return str.isalpha(val, *args)
        except:
            raise
    
    def isdigit(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.isdigit(val, *args)
            if self.type in ['double', 'float']:
                return float.isdigit(val, *args)
            if self.type in ['boolean']:
                return bool.isdigit(val, *args)
            if self.type in ['char', 'string']:
                return str.isdigit(val, *args)
        except:
            raise
    
    def islower(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.islower(val, *args)
            if self.type in ['double', 'float']:
                return float.islower(val, *args)
            if self.type in ['boolean']:
                return bool.islower(val, *args)
            if self.type in ['char', 'string']:
                return str.islower(val, *args)
        except:
            raise
    
    def isspace(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.isspace(val, *args)
            if self.type in ['double', 'float']:
                return float.isspace(val, *args)
            if self.type in ['boolean']:
                return bool.isspace(val, *args)
            if self.type in ['char', 'string']:
                return str.isspace(val, *args)
        except:
            raise
    
    def istitle(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.istitle(val, *args)
            if self.type in ['double', 'float']:
                return float.istitle(val, *args)
            if self.type in ['boolean']:
                return bool.istitle(val, *args)
            if self.type in ['char', 'string']:
                return str.istitle(val, *args)
        except:
            raise
    
    def isupper(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.isupper(val, *args)
            if self.type in ['double', 'float']:
                return float.isupper(val, *args)
            if self.type in ['boolean']:
                return bool.isupper(val, *args)
            if self.type in ['char', 'string']:
                return str.isupper(val, *args)
        except:
            raise
    
    def join(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.join(val, *args)
            if self.type in ['double', 'float']:
                return float.join(val, *args)
            if self.type in ['boolean']:
                return bool.join(val, *args)
            if self.type in ['char', 'string']:
                return str.join(val, *args)
        except:
            raise
    
    def ljust(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.ljust(val, *args)
            if self.type in ['double', 'float']:
                return float.ljust(val, *args)
            if self.type in ['boolean']:
                return bool.ljust(val, *args)
            if self.type in ['char', 'string']:
                return str.ljust(val, *args)
        except:
            raise
    
    def lower(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.lower(val, *args)
            if self.type in ['double', 'float']:
                return float.lower(val, *args)
            if self.type in ['boolean']:
                return bool.lower(val, *args)
            if self.type in ['char', 'string']:
                return str.lower(val, *args)
        except:
            raise
    
    def lstrip(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.lstrip(val, *args)
            if self.type in ['double', 'float']:
                return float.lstrip(val, *args)
            if self.type in ['boolean']:
                return bool.lstrip(val, *args)
            if self.type in ['char', 'string']:
                return str.lstrip(val, *args)
        except:
            raise
    
    def replace(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.replace(val, *args)
            if self.type in ['double', 'float']:
                return float.replace(val, *args)
            if self.type in ['boolean']:
                return bool.replace(val, *args)
            if self.type in ['char', 'string']:
                return str.replace(val, *args)
        except:
            raise
    
    def rfind(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.rfind(val, *args)
            if self.type in ['double', 'float']:
                return float.rfind(val, *args)
            if self.type in ['boolean']:
                return bool.rfind(val, *args)
            if self.type in ['char', 'string']:
                return str.rfind(val, *args)
        except:
            raise
    
    def rindex(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.rindex(val, *args)
            if self.type in ['double', 'float']:
                return float.rindex(val, *args)
            if self.type in ['boolean']:
                return bool.rindex(val, *args)
            if self.type in ['char', 'string']:
                return str.rindex(val, *args)
        except:
            raise
    
    def rjust(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.rjust(val, *args)
            if self.type in ['double', 'float']:
                return float.rjust(val, *args)
            if self.type in ['boolean']:
                return bool.rjust(val, *args)
            if self.type in ['char', 'string']:
                return str.rjust(val, *args)
        except:
            raise
    
    def rsplit(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.rsplit(val, *args)
            if self.type in ['double', 'float']:
                return float.rsplit(val, *args)
            if self.type in ['boolean']:
                return bool.rsplit(val, *args)
            if self.type in ['char', 'string']:
                return str.rsplit(val, *args)
        except:
            raise
    
    def rstrip(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.rstrip(val, *args)
            if self.type in ['double', 'float']:
                return float.rstrip(val, *args)
            if self.type in ['boolean']:
                return bool.rstrip(val, *args)
            if self.type in ['char', 'string']:
                return str.rstrip(val, *args)
        except:
            raise
    
    def split(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.split(val, *args)
            if self.type in ['double', 'float']:
                return float.split(val, *args)
            if self.type in ['boolean']:
                return bool.split(val, *args)
            if self.type in ['char', 'string']:
                return str.split(val, *args)
        except:
            raise
    
    def splitlines(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.splitlines(val, *args)
            if self.type in ['double', 'float']:
                return float.splitlines(val, *args)
            if self.type in ['boolean']:
                return bool.splitlines(val, *args)
            if self.type in ['char', 'string']:
                return str.splitlines(val, *args)
        except:
            raise
    
    def startswith(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.startswith(val, *args)
            if self.type in ['double', 'float']:
                return float.startswith(val, *args)
            if self.type in ['boolean']:
                return bool.startswith(val, *args)
            if self.type in ['char', 'string']:
                return str.startswith(val, *args)
        except:
            raise
    
    def strip(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.strip(val, *args)
            if self.type in ['double', 'float']:
                return float.strip(val, *args)
            if self.type in ['boolean']:
                return bool.strip(val, *args)
            if self.type in ['char', 'string']:
                return str.strip(val, *args)
        except:
            raise
    
    def swapcase(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.swapcase(val, *args)
            if self.type in ['double', 'float']:
                return float.swapcase(val, *args)
            if self.type in ['boolean']:
                return bool.swapcase(val, *args)
            if self.type in ['char', 'string']:
                return str.swapcase(val, *args)
        except:
            raise
    
    def title(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.title(val, *args)
            if self.type in ['double', 'float']:
                return float.title(val, *args)
            if self.type in ['boolean']:
                return bool.title(val, *args)
            if self.type in ['char', 'string']:
                return str.title(val, *args)
        except:
            raise
    
    def translate(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.translate(val, *args)
            if self.type in ['double', 'float']:
                return float.translate(val, *args)
            if self.type in ['boolean']:
                return bool.translate(val, *args)
            if self.type in ['char', 'string']:
                return str.translate(val, *args)
        except:
            raise
    
    def upper(self, *args):
        try:
            val = self.queryValue()
            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
                return int.upper(val, *args)
            if self.type in ['double', 'float']:
                return float.upper(val, *args)
            if self.type in ['boolean']:
                return bool.upper(val, *args)
            if self.type in ['char', 'string']:
                return str.upper(val, *args)
        except:
            raise
    
    def __repr__(self, *args):
        value = self.queryValue()
        if value != None:
            print str(value),
        return ''
        
    def __str__(self, *args):
        value = self.queryValue()
        if value != None:
            return str(value)
        else:
            return ''

'''
WE ARE AS OF YET UNABLE TO OVERRIDE THE FUNCTIONS BELOW DUE 
TO COMPLICATIONS WITH THE INITIAL SETUP OF THE COMPONENT
'''
#    def __cmp__(self, *args):
#        try:
#            val = self.queryValue()
#            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
#                return int.__cmp__(val, *args)
#            if self.type in ['double', 'float']:
#                return float.__cmp__(val, *args)
#            if self.type in ['boolean']:
#                return bool.__cmp__(val, *args)
#            if self.type in ['char', 'string']:
#                return str.__cmp__(val, *args)
#        except:
#            raise
#
#    def __len__(self, *args):
#        try:
#            val = self.queryValue()
#            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
#                return int.__len__(val, *args)
#            if self.type in ['double', 'float']:
#                return float.__len__(val, *args)
#            if self.type in ['boolean']:
#                return bool.__len__(val, *args)
#            if self.type in ['char', 'string']:
#                return str.__len__(val, *args)
#        except:
#            raise   
# 
#    def __ne__(self, *args):
#        try:
#            val = self.queryValue()
#            if self.type in ['short', 'long', 'longlong', 'octet', 'ulong', 'ushort', 'longlong', 'ulonglong']:
#                return int.__ne__(val, *args)
#            if self.type in ['double', 'float']:
#                return float.__ne__(val, *args)
#            if self.type in ['boolean']:
#                return bool.__ne__(val, *args)
#            if self.type in ['char', 'string']:
#                return str.__ne__(val, *args)
#        except:
#            raise
        

class sequenceProperty(Property, list):
    def __init__(self, id, valueType, compRef, defValue=None, mode='readwrite'):
        """ 
        id - (string): the property ID
        valueType - (string): type of the property, must be in VALUE_TYPES, or can be struct
        compRef - (domainless.componentBase) - pointer to the component that owns this property 
        mode - (string): mode for the property, must be in MODES
        
        This class inherits from list so that the property will behave
        as a list when the user wishes to manipulate it
        """
        if valueType not in SCA_TYPES and valueType != 'structSeq':
            raise('"' + str(valueType) + '"' + ' is not a valid valueType, choose from\n ' + str(SCA_TYPES))
        
        #initialize the list, and the parent Property
        list.__init__([])
        Property.__init__(self, id, type=valueType, compRef=compRef, mode=mode)
        
        self.defValue = defValue
        
        #try to set the value type in ref unless this is a sequence of structs
        if valueType != 'structSeq':
            self.valueType = valueType
            #create the CF.DataType reference, change value to the correct type
            self.propRef = _CF.DataType(id=str(self.id), value=_any.to_any(None))
            if self.valueType == "string":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.StringSeq)
            elif self.valueType == "boolean":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.BooleanSeq)
            elif self.valueType == "ulong":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ULongSeq)
            elif self.valueType == "short":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ShortSeq)
            elif self.valueType == "float":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.FloatSeq)
            elif self.valueType == "char":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.CharSeq)
            elif self.valueType == "octet":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.OctetSeq)
            elif self.valueType == "ushort":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.UShortSeq)
            elif self.valueType == "double":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.DoubleSeq)
            elif self.valueType == "long":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.LongSeq)
            elif self.valueType == "longlong":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.LongLongSequence)
            elif self.valueType == "ulonglong":
                self.propRef.value._t = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.UlongLongSequence) 
    
    def queryValue(self):
        '''
        Returns the current value of the property by doing a query
        on the component and returning only the value, or an empty list
        '''
        if self.mode == 'writeonly':
            raise Exception, 'Could not perform query, ' + str(self.id) + ' is a writeonly property'
        else:
            queryRef = _copy.deepcopy(self.propRef)
            queryRef.value = _any.to_any(None)
            results = self.compRef.query([queryRef])
            if results != None:
                val = results[0].value.value()
                if val != None and val != '':
                    # Octet sequences are stored as strings
                    if self.valueType == 'octet':
                        val = list([ord(x) for x in val])
                    elif self.valueType == 'char':
                        val = list([x for x in val])
                    return val
            if self.valueType == 'octet' or self.valueType == 'char':
                return ''
            return []
    
    def configureValue(self, value):
        '''
        Helper function for configuring a sequence property
        '''
        if self.mode == 'readonly':
            raise Exception, 'Could not perform configure, ' + str(self.id) + ' is a readonly property'
        else:
            if value == None:
                configRef = _copy.deepcopy(self.propRef)
                configRef.value = _any.to_any(None)
                self.compRef.configure([configRef])
            else:
                if type(value) != list and type(value) != tuple and type(value) != str:
                    if self.valueType == 'octet' or self.valueType == 'char':
                        msg = 'configureValue() must be called with str instance as second argument (got ' + str(type(value))[7:-2] + ' instance instead)'
                    else:
                        msg = 'configureValue() must be called with list or tuple instance as second argument (got ' + str(type(value))[7:-2] + ' instance instead)'
                    raise TypeError, msg
            
                # For octet convert list to string
                if self.valueType == 'octet':
                    strVal = ''
                    for x in value:
                        # Make sure value is correct and is within range so that pack doesn't fail
                        if type(x) != int:
                            raise TypeError, 'configureValue() must be called with type int instance as second argument (got ' + str(type(x))[7:-2] + ' instance instead)'
                        elif x < 0 or x > 255:
                            raise OutOfRangeException, '"'+str(x)+'"'+ ' is out of range for type octet(unsigned 8 bit) [0 <= x <= 255]'
                        else:
                            strVal +=  _struct.pack('B', int(x))
                    value = strVal
                elif self.valueType == 'char':
                    strVal = ''
                    for x in value:
                        # Makes sure each value is only a single char
                        if type(x) != str:
                            raise TypeError, 'configureValue() must be called with type char instance as second argument (got ' + str(type(x))[7:-2] + ' instance instead)'
                        elif len(x) > 1:
                            raise TypeError, 'configureValue() must be called with type char instance as second argument (got a str instance instead)'
                        strVal += x
                        
                    value = strVal
                #validate each value within the list
                _type_helpers.checkValidDataSet(value, self.valueType)
                configRef = _copy.deepcopy(self.propRef)
                configRef.value._v = value
                self.compRef.configure([configRef])

    '''
    BELOW ARE OVERRIDDEN FUNCTIONS FOR THE LIST CLASS IN
    PYTHON SO THAT THE PROPERTIES BEHAVE WITHIN THE CODE
    AS IF THEY ARE ACTUALLY LISTS
    '''
    def __add__(self, *args):
        return list.__add__([x for x in self.queryValue()], args)
    
    def __contains__(self, *args):
        return list.__contains__([x for x in self.queryValue()], *args)
    
    def __delitem__(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.__delitem__(tmp, *args)
        self.configureValue(tmp)
    
    def __delslice__(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.__delslice__(tmp, *args)
        self.configureValue(tmp)

    def __eq__(self, *args):
        return list.__eq__([x for x in self.queryValue()], *args)
    
    def __ge__(self, *args):
        return list.__ge__([x for x in self.queryValue()], *args)
    
    def __getslice__(self, *args):
        return list.__getslice__([x for x in self.queryValue()], *args)
    
    def __getitem__(self, *args):
        return list.__getitem__([x for x in self.queryValue()], *args)
    
    def __gt__(self, *args):
        return list.__gt__([x for x in self.queryValue()], *args)
    
    def __iadd__(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        result = list.__iadd__(tmp, *args)
        self.configureValue(result)
        return [x for x in self.queryValue()]
    
    def __imul__(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        result = list.__imul__(tmp, *args)
        self.configureValue(result)
        return [x for x in self.queryValue()]
    
    def __iter__(self, *args):
        return list.__iter__([x for x in self.queryValue()], *args)
    
    def __le__(self, *args):
        return list.__le__([x for x in self.queryValue()], *args)
    
    def __len__(self, *args):
        return list.__len__([x for x in self.queryValue()], *args)
    
    def __lt__(self, *args):
        return list.__lt__([x for x in self.queryValue()], *args)
    
    def __mul__(self, *args):
        return list.__mul__([x for x in self.queryValue()], *args)
    
    def __ne__(self, *args):
        return list.__ne__([x for x in self.queryValue()], *args)
    
    def __repr__(self):
        return str(self.queryValue())
    
    def __reversed__(self, *args):
        return list.__reversed__([x for x in self.queryValue()], *args)
    
    def __rmul__(self, *args):
        return list.__rmul__([x for x in self.queryValue()], *args)
    
    def __setitem__(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.__setitem__(tmp, *args)
        self.configureValue(tmp)
    
    def __setslice__(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.__setslice__(tmp, *args)
        self.configureValue(tmp)
        
    def __str__(self):
        return str(self.queryValue())
    
    def append(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.append(tmp, *args)
        self.configureValue(tmp)
        
    def count(self, *args):
        return list.count([x for x in self.queryValue()], *args)
    
    def extend(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.extend(tmp, *args)
        self.configureValue(tmp)
    
    def index(self, *args):
        return list.index([x for x in self.queryValue()], *args)
    
    def insert(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.insert(tmp, *args)
        self.configureValue(tmp)
    
    def pop(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        value = list.pop(tmp, *args)
        self.configureValue(tmp)
        return value
    
    def remove(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.remove(tmp, *args)
        self.configureValue(tmp)
    
    def reverse(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.reverse(tmp, *args)
        self.configureValue(tmp)
    
    def sort(self, *args):
        tmp = _copy.deepcopy([x for x in self.queryValue()])
        list.sort(tmp, *args)
        self.configureValue(tmp)
            
class structProperty(Property):
    def __init__(self, id, valueType, compRef, defValue=None, structSeqRef=None, structSeqIdx=None, mode='readwrite'):
        """ 
        id - (string): the property ID
        valueType - (list): each entry in the list is a tuple defined in the following fashion:
                                (id, valueType(as defined for simples), defValue)
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        structSeqRef - (string) - name of the struct sequence that this struct is a part of, or None
        structSeqIdx - (int) - index of the struct  int the struct sequence, or None
        mode - (string): mode for the property, must be in MODES
        """
        if type(valueType) != list:
            raise('valueType must be provided as a list')
        self.valueType = valueType
        self.defValue = defValue
        
        #used when the struct is part of a struct sequence
        self.structSeqRef = structSeqRef
        self.structSeqIdx = structSeqIdx
        
        #initialize the parent
        Property.__init__(self, id, type='struct', compRef=compRef, mode=mode)
        
        #each of these members is itself a simple property
        self.members = {}
        for _id, _type, _defValue, _clean_name in valueType:
            if self.structSeqRef:
                simpleProp = simpleProperty(_id, _type, compRef=compRef, defValue=_defValue, structRef=id, structSeqRef=self.structSeqRef, structSeqIdx=self.structSeqIdx)
                simpleProp.clean_name = _clean_name
            else:
                simpleProp = simpleProperty(_id, _type, compRef=compRef, defValue=_defValue, structRef=id)
                simpleProp.clean_name = _clean_name
            self.members[_id] = (simpleProp)
        
        #create the CF.DataType reference        
        self.propRef = _CF.DataType(id=str(self.id), value=_CORBA.Any(_CORBA.TypeCode("IDL:CF/Properties:1.0"), None))            
    
    def queryValue(self):
        '''
        Returns the current value of the property by doing a query
        on the component and returning only the value as a dictionary
        '''
        if self.mode == 'writeonly':
            raise Exception, 'Could not perform query, ' + str(self.id) + ' is a writeonly property'
        else:
            #check if this struct is a member of a struct sequence
            if self.structSeqRef != None:
                #get the struct sequence from the component and do a query
                structSeqProp = self.compRef.__getattribute__(self.structSeqRef)
                queryRef = _copy.deepcopy(structSeqProp.propRef)
                results = self.compRef.query([queryRef])
                if results == None:
                    return {}
                else:
                    structRefList = results[0].value.value()
                    
                    #build the dictionary
                    structRef = structRefList[self.structSeqIdx].value()
                    structDict = {}
                    for simpleRef in structRef:
                        structDict[simpleRef.id] = simpleRef.value.value()
                    return structDict   
            else:
                #do a query
                queryRef = _copy.deepcopy(self.propRef)
                queryRef.value = _any.to_any(None)
                results = self.compRef.query([queryRef])
                if results == None:
                    return {}
                else:
                    #build the dictionary
                    structRef = results[0].value.value()
                    structDict = {}
                    for simpleRef in structRef:
                        structDict[simpleRef.id] = simpleRef.value.value()
                    return structDict
    
    def configureValue(self, value):
        '''
        Helper function for configuring a sequence property, using a
        dictionary as the passed in value
        '''
        if self.mode == 'readonly':
            raise Exception, 'Could not perform configure, ' + str(self.id) + ' is a readonly property'
        else:
            if value != None:
                if type(value) != dict:
                    raise TypeError, 'configureValue() must be called with dict instance as second argument (got ' + str(type(value))[7:-2] + ' instance instead)'
                    #check that the value passed in matches the struct definition
                    _type_helpers.checkValidValue(value, [(_id, _type) for _id, _type, _val, _cleanName in self.valueType])
            
            #check if this struct is a member of a struct sequence
            if self.structSeqRef != None:
                #get the struct sequence from the component and do a query
                structSeqProp = self.compRef.__getattribute__(self.structSeqRef)
                queryRef = _copy.deepcopy(structSeqProp.propRef)
                results = self.compRef.query([queryRef])
                if results == None:
                    return self.compRef.configure([self.propRef])
                else:
                    structRefList = results[0].value.value()
                    
                    #find the right struct
                    structRef = structRefList[self.structSeqIdx].value()
                    
                    if value == None:
                        #go through each member of the struct and set it to None
                        for simpleRef in structRef:
                            simpleRef.value = _any.to_any(None)
                        
                        #copy in all the struct instances and do a configure    
                        configRef = _copy.deepcopy(structSeqProp.propRef)
                        for instance in structRefList:
                            configRef.value._v.append(instance)
                        self.compRef.configure([configRef])
                
                    else:
                        #go through each member of the struct and change it if a value 
                        #was sent in for it, else use the value returned from the query
                        querymembers = self.queryValue()
                        for simpleRef in structRef:
                            currValue = None
                            try:
                                currValue = value[simpleRef.id]
                            except:
                                try:
                                    currValue = querymembers[simpleRef.id]
                                except:
                                    pass
                                
                            if currValue == None:
                                simpleRef.value = _any.to_any(None)
                            else:
                                simpleRef.value._v = currValue
                                #must change the type back in case it was set to TK_null by a previous configure
                                simpleRef.value._t = self.members[simpleRef.id].propRef.value._t
                        
                        #copy in all the struct instances and do a configure 
                        configRef = _copy.deepcopy(structSeqProp.propRef)
                        for instance in structRefList:
                            configRef.value._v.append(instance)
                        self.compRef.configure([configRef])
            else:
                configRef = _copy.deepcopy(self.propRef)
                configRef.value._v = [_copy.deepcopy(member.propRef) for member in self.members.values()]
                
                if value == None:
                    #go through each member of the struct and set it to None, then do a configure
                    for simpleRef in configRef.value.value():
                        simpleRef.value = _any.to_any(None)
                    self.compRef.configure([configRef])
                else:
                    #go through each member of the struct and change it if a value 
                    #was sent in for it, else use the value returned from the query
                    querymembers = self.queryValue()
                    for simpleRef in configRef.value.value():
                        currValue = None
                        try:
                            currValue = value[simpleRef.id]
                        except:
                            try:
                                currValue = querymembers[simpleRef.id]
                            except:
                                pass
                            
                        if currValue == None:
                            simpleRef.value = _any.to_any(None)
                        else:
                            simpleRef.value._v = currValue
                            #must change the type back in case it was set to TK_null by a previous configure
                            simpleRef.value._t = self.members[simpleRef.id].propRef.value._t
                    
                    #do a configure
                    self.compRef.configure([configRef])
            return value
    

    def _getMember(self, name):
        '''
        Helper function to get the simple member of a struct.  Needed when there are duplicate simple
        property names in different structs.
        '''
        try:
            return object.__getattribute__(self, "members")[_displayNames[self.compRef._refid][name]]
        except:
            for member in object.__getattribute__(self, "members").itervalues():
                if name in member.clean_name and _duplicateNames[self.compRef._refid].has_key(name):
                    return member
            return None
    
   
    def __str__(self):
        currValue = self.queryValue()
        structView = "ID: " + self.id
        for key in currValue:
            structView = structView + '\n  ' + str(self.members[key].clean_name) + ": " + str(currValue[key])
        return structView
    
    def __repr__(self):
        currValue = self.queryValue()
        structView = "ID: " + self.id
        for key in currValue:
            structView = structView + '\n  ' + str(self.members[key].clean_name) + ": " + str(currValue[key])
        print structView,
        return ''

    def __getattr__(self, name):
        '''
        If the attribute being looked up is actually a member of the struct,
        then return that simple property, otherwise default to the normal
        getattribute function
        ''' 
        member =self._getMember(name)
        if member is not None:
            return member
        return object.__getattribute__(self,name)
        
    def __setattr__(self, name, value):
        '''
        If the attribute being looked up is actually a member of the struct,
        then try to configure the simple property.  This will result in a
        configure of the entire struct in the simpleProperty class
        '''
        try:
            member = self._getMember(name)
            if member is not None:
                name = member.clean_name
            self.members[_displayNames[self.compRef._refid][name]].configureValue(value)
        except AttributeError:
            return object.__setattr__(self, name, value)
        except KeyError:
            return object.__setattr__(self, name, value)
            
        
class structSequenceProperty(sequenceProperty):
    def __init__(self, id, structID, valueType, compRef, defValue=[], mode='readwrite'):
        """ 
        id - (string): the property ID
        valueType - (list): each entry in the list is a tuple defined in the following fashion:
                                (id, valueType(as defined for simples), defValue)
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        mode - (string): mode for the property, must be in MODES
        """
        if type(valueType) != list:
            raise('valueType must be provided as a list')
        self.valueType = valueType
        self.structID = structID
        self.defValue = defValue
        
        #initialize the parent
        sequenceProperty.__init__(self, id, valueType='structSeq', compRef=compRef, defValue=self.defValue, mode=mode)

        #create the CF.DataType reference   
        self.propRef = _CF.DataType(id=str(self.id), value=_CORBA.Any(_CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0"), []))

    def __getitem__(self, *args):
        #the actual struct property doesn't exist, so create it and return it
        newProp = structProperty(id=self.structID, valueType=self.valueType, compRef=self.compRef, \
                                 structSeqRef=self.id, structSeqIdx=args[0], mode=self.mode)
        return newProp
    
    def __setitem__(self, *args):
        #the actual struct property doesn't exist, so create it and configure it,
        #this will trigure a configure of the entire sequence from within structProperty
        newProp = structProperty(id=self.structID, valueType=self.valueType, compRef=self.compRef, \
                                 structSeqRef=self.id, structSeqIdx=args[0], mode=self.mode)
        structProperty.configureValue(newProp, args[1])
    
    def queryValue(self):
        '''
        Returns the current value of the property by doing a query
        on the component and returning only the value as a list of dictionary
        '''
        if self.mode == 'writeonly':
            raise Exception, 'Could not perform query, ' + str(self.id) + ' is a writeonly property'
        else:
            #get the list of struct refs from the parent
            structRefs = sequenceProperty.queryValue(self)
            structRefList = []
            for structRef in structRefs:
                simpleRefs = structRef.value()
                struct = {}
                for simpleRef in simpleRefs:
                    struct[simpleRef.id] = simpleRef.value.value()
                structRefList.append(struct)
            return structRefList

    def configureValue(self, value):
        '''
        Helper function for configuring a struct sequence property
        '''
        if self.mode == 'readonly':
            raise Exception, 'Could not perform configure, ' + str(self.id) + ' is a readonly property'
        else:
            if value == None:
                configRef = _copy.deepcopy(self.propRef)
                self.compRef.configure([configRef])
            else:
                if type(value) != list:
                    raise TypeError, 'configureValue() must be called with list instance as second argument (got ' + str(type(value))[7:-2] + ' instance instead)'
            
                _type_helpers.checkValidDataSet(value, [(_id, _type) for _id, _type, _val, _cleanName in self.valueType])
                
                structRefs = []
                for dict in value:
                    newProp = structProperty(id=self.structID, valueType=self.valueType, compRef=self.compRef, mode=self.mode)
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
                            for _id, _type, _defValue, _cleanName in self.valueType:
                                if _id == simpleRef.id:
                                    if _defValue != None:
                                        simpleRef.value._v = _defValue
                                        simpleRef.value._t = oldType    
                        simpleRefs.append(_copy.deepcopy(simpleRef))
                    tmpRef = _CORBA.Any(_CORBA.TypeCode("IDL:CF/Properties:1.0"), simpleRefs)
                    structRefs.append(_copy.deepcopy(tmpRef))
                configRef = _copy.deepcopy(self.propRef)
                configRef.value._v = structRefs
                self.compRef.configure([configRef])
