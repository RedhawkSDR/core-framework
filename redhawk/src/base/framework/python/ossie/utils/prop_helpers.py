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
from ossie.properties import _SCA_TYPES
from ossie.properties import __TYPE_MAP
from ossie.properties import getPyType 
from ossie.properties import getTypeCode
from ossie.properties import getCFType 
from ossie.properties import getMemberType 
from ossie.properties import getCFSeqType 
from ossie import parsers as _parsers
from ossie.utils import type_helpers as _type_helpers
from ossie.cf import CF as _CF
from ossie.cf import PortTypes as _PortTypes
from omniORB import any as _any
from omniORB import CORBA as _CORBA
from omniORB import tcInternal as _tcInternal
import copy as _copy
import struct as _struct
import string as _string
import operator as _operator
import warnings as _warnings
from ossie.utils.type_helpers import OutOfRangeException, EnumValueError
from ossie.utils.formatting import TablePrinter
from ossie.parsers.prf import configurationKind as _configurationKind
SCA_TYPES = globals()['_SCA_TYPES']

# Map the type of the complex number (e.g., complexFloat) to the 
# type of the real and imaginary members (e.g., float).
__COMPLEX_SIMPLE_TYPE_MAP = {
    'complexFloat'    : 'float',
    'complexBoolean'  : 'boolean',
    'complexULong'    : 'ulong',
    'complexShort'    : 'short',
    'complexOctet'    : 'octet',
    'complexChar'     : 'char',
    'complexUShort'   : 'ushort',
    'complexDouble'   : 'double',
    'complexLong'     : 'long',
    'complexLongLong' : 'longlong',
    'complexULongLong': 'ulonglong'
}

def mapComplexToSimple(complexType):
    return __COMPLEX_SIMPLE_TYPE_MAP[complexType]
    

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
        strTypes   = [getTypeCode('string'), 
                      getTypeCode('char')]
        boolTypes  = [getTypeCode('boolean')]
        longTypes  = [getTypeCode('ulong'), 
                      getTypeCode('short'), 
                      getTypeCode('octet'), 
                      getTypeCode('ushort'),
                      getTypeCode('long'), 
                      getTypeCode('longlong'), 
                      getTypeCode('ulonglong')]
        floatTypes = [getTypeCode('float'), 
                      getTypeCode('double')]
        
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
        for prop in struct.get_simple():
            if prop.get_name() == None:
                name = prop.get_id()
            else:
                name = prop.get_name()
            if nameDict.has_key( tagbase + str(name) ):
                print "WARN: struct element with duplicate name %s" % tagbase
                continue

            nameDict[str(tagbase + str(name))] = str(prop.get_id())
            
        for prop in struct.get_simplesequence():
            if prop.get_name() == None:
                name = prop.get_id()
            else:
                name = prop.get_name()
            if nameDict.has_key( tagbase + str(name) ):
                print "WARN: struct element with duplicate name %s" % tagbase
                continue

            nameDict[str(tagbase + str(name))] = str(prop.get_id())

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
        for prop in struct.get_simple():
            # make sure this struct element's name is unique
            if prop.get_name() == None:
                name = prop.get_id()
            else:
                name = prop.get_name()
            if nameDict.has_key( tagbase + str(name)):
                print "WARN: property with non-unique name %s" % name
                continue

            nameDict[str(tagbase + str(name))] = str(prop.get_id())
        for prop in struct.get_simplesequence():
            # make sure this struct element's name is unique
            if prop.get_name() == None:
                name = prop.get_id()
            else:
                name = prop.get_name()
            if nameDict.has_key( tagbase + str(name)):
                print "WARN: property with non-unique name %s" % name
                continue

            nameDict[str(tagbase + str(name))] = str(prop.get_id())
          
    return nameDict


'''
-Maps a properties clean, display and access name to its ID
-Prevents duplicate entries within a component
-Allows for get/set on components with invalid chars in ID
'''
def addCleanName(cleanName, id, _displayNames, _duplicateNames):
    if not _displayNames.has_key(cleanName):
        _displayNames[cleanName] = id
        _duplicateNames[cleanName] = 0
        return cleanName
    elif _displayNames[cleanName] == id:
        return cleanName
    else:
        count = _duplicateNames[cleanName] + 1
        _displayNames[cleanName + str(count)] = id
        _duplicateNames[cleanName] = count
        return cleanName + str(count)
    
def _cleanId(prop):
    translation = 48*"_"+_string.digits+7*"_"+_string.ascii_uppercase+6*"_"+_string.ascii_lowercase+133*"_"
    prop_id = prop.get_name()
    if prop_id is None:
        prop_id = prop.get_id()
    return str(prop_id).translate(translation)

def isMatch(prop, modes, kinds, actions):
    """
    Tests whether a given SCA property (as an XML node) matches the given modes,
    kinds and actions.
    """
    if prop.get_mode() == None:
        m = "readwrite"
    else:
        m = prop.get_mode()
    matchMode = (m in modes)

    if isinstance(prop, (_parsers.PRFParser.simple, _parsers.PRFParser.simpleSequence)):
        if prop.get_action() == None:
            a = "external"
        else:
            a = prop.get_action().get_type()
        matchAction = (a in actions)

        matchKind = False
        if prop.get_kind() == None or prop.get_kind() == []:
            k = [_configurationKind()]
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
            if "property" == kind.get_kindtype():
                foundConf = True
            if "allocation" == kind.get_kindtype():
                foundAlloc = True
        if foundConf and foundAlloc:
            matchAction = True

    elif isinstance(prop, (_parsers.PRFParser.struct, _parsers.PRFParser.structSequence)):
        matchAction = True # There is no action, so always match

        matchKind = False
        if prop.get_configurationkind() == None or prop.get_configurationkind() == []:
            k = [_configurationKind()]
        else:
            k = prop.get_configurationkind()
        for kind in k:
            if kind.get_kindtype() in kinds:
                matchKind = True

        if k in kinds:
            matchKind = True


    return matchMode and matchKind and matchAction


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
    
    def __init__(self, id, type, kinds, compRef, mode='readwrite', action='external', parent=None, defValue=None):
        """ 
        compRef - (domainless.componentBase) - pointer to the component that owns this property
        type - (string): type of property (SCA Type or 'struct' or 'structSequence')
        id - (string): the property ID
        mode - (string): mode for the property, must be in MODES
        parent - (Property): the property that contains this instance (e.g., struct that holds a simple)
        """
        self.id = id
        self.type = type
        self.compRef = compRef
        if mode not in self.MODES:
            print str(mode) + ' is not a valid mode, defaulting to "readwrite"'
            self.mode = 'readwrite'
        else:
            self.mode = mode
        self.action = action
        self._parent = parent
        self.defValue = defValue
        if kinds:
            self.kinds = kinds
        else:
            # Default to "configure" if no kinds given
            self.kinds = ('configure',"property")

    def _getStructsSimpleProps(self,simple,prop):
        kinds = []
        enums = []
        value = None
        defVal = None
        for i in self.compRef._properties:
            if i.clean_name == prop.id_:
                for k in prop.get_configurationkind():
                    kinds.append(k.get_kindtype())
                if i.members[_cleanId(simple)]._enums != None:
                    enums = i.members[_cleanId(simple)]._enums
                if self.mode != "writeonly":
                    value = str(i.members[_cleanId(simple)])
                defVal = str(i.members[_cleanId(simple)].defValue)
        type = str(self.compRef._getPropType(simple))
        return defVal, value, type, kinds, enums

    def _getStructsSimpleSeqProps(self, sprop, prop):
        kinds = []
        enums = []
        values = None
        defVal = None
        for i in self.compRef._properties:
            if i.clean_name == prop.id_:
                for k in prop.get_configurationkind():
                    kinds.append(k.get_kindtype())
                if i.members[_cleanId(sprop)].__dict__.has_key("_enums"):
                  if i.members[_cleanId(sprop)]._enums != None:
                    enums = i.members[_cleanId(sprop)]._enums
                if self.mode != "writeonly":
                    values = i.members[_cleanId(sprop)]
                defVal = i.members[_cleanId(sprop)].defValue
        type = str(self.compRef._getPropType(sprop))
        return defVal, values, type, kinds, enums

    def api(self):
        kinds = []
        print "\nProperty\n--------"
        print "% -*s %s" % (17,"ID:",self.id)
        print "% -*s %s" % (17,"Type:",self.type)
        simpleOrSequence = False
        if self.type != "structSeq" and self.type != "struct":
            simpleOrSequence = True
            print "% -*s %s" % (17,"Default Value:", self.defValue)
            if self.mode != "writeonly":
                print "% -*s %s" % (17,"Value: ", self.queryValue())
            try:
                if self._enums != None:
                    print "% -*s %s" % (17,"Enumumerations:", self._enums)
            except:
                simpleOrSequence = True
        if self.type != "struct":
            print "% -*s %s" % (17,"Action:", self.action) 
        print "% -*s %s" % (17,"Mode: ", self.mode)

        if self.type == "struct":
            structTable = TablePrinter('Name','Data Type','Default Value', 'Current Value','Enumerations')
            structTable.limit_column(1,20)
            structTable.limit_column(2,15)
            structTable.limit_column(3,15)
            structTable.limit_column(4,40)
            for prop in self.compRef._prf.get_struct():
                if prop.id_ == self.id:
                    first = True
                    for sprop in prop.get_simple():
                        defVal,value, type, kinds,enums = self._getStructsSimpleProps(sprop,prop)
                        structTable.append(sprop.get_id(),type,str(defVal),str(value),enums)
                        if first:
                            print "% -*s %s" % (17,"Kinds: ", ', '.join(kinds))
                            first = False
                    for sprop in prop.get_simplesequence():
                        defVal,values,type,kinds,enums = self._getStructsSimpleSeqProps(sprop, prop)
                        structTable.append(sprop.get_id(),type,defVal,values,enums)
                        if first:
                            print "% -*s %s" % (17,"Kinds: ",', '.join(kinds))
                            first = False
            structTable.write()
        elif self.type == "sequence":
            print "sequence: ",type(self)

        elif self.type == "structSeq":
            structNum = -1
            structTable = TablePrinter('Name','Data Type')
            structTable.limit_column(1,35)
            for prop in self.compRef._prf.get_structsequence():
                for kind in prop.get_configurationkind():
                    kinds.append(kind.get_kindtype())
                if prop.id_ == self.id and prop.get_struct() != None:
                    for prop in prop.get_struct().get_simple():
                        structTable.append(prop.id_, prop.get_type())
                    for prop in prop.get_struct().get_simplesequence():
                        structTable.append(prop.id_, prop.get_type())
            print "% -*s %s" % (17,"Kinds: ", ', '.join(kinds))
            print "\nStruct\n======"
            structTable.write()

            simpleTable = TablePrinter('Index','Name','Value')
            simpleTable.limit_column(1,30)
            simpleTable.limit_column(2,35)
            for i in self.compRef._properties:
                if i.type == "structSeq" and self.mode != "writeonly":
                    for s in i:
                        structNum +=1
                        for key in s.keys():
                            simpleTable.append(str(structNum),key,str(s[key]))
            if self.mode != "writeonly":
                print "\nSimple Properties\n================="
                simpleTable.write()

        elif simpleOrSequence:
            for prop in self.compRef._prf.get_simple() + self.compRef._prf.get_simplesequence():
                if prop.id_ == self.id:
                    for kind in prop.get_kind():
                        kinds.append(kind.get_kindtype())
            print "% -*s %s" % (17,"Kinds: ", ', '.join(kinds))
          

    def _isNested(self):
        return self._parent is not None

    def _checkRead(self):
        if self._isNested():
            return self._parent._checkRead()
        return self.mode != 'writeonly'

    def _checkWrite(self):
        if self._isNested():
            return self._parent._checkWrite()
        return self.mode != 'readonly'

    def _getItemKey(self):
        raise AssertionError(self.__class__.__name__ + ' cannot be nested')

    def _queryItem(self, key):
        raise AssertionError(self.__class__.__name__ + ' cannot contain other properties')

    def _configureItem(self, key, value):
        raise AssertionError(self.__class__.__name__ + ' cannot contain other properties')

    def _queryValue(self):
        if self._isNested():
            return self._parent._queryItem(self._getItemKey())
        else:
            try:
                results = self.compRef.query([_CF.DataType(str(self.id), _any.to_any(None))])
            except:
                results = None
                if self.mode == "writeonly":
                    print "Invalid Action: can not query a partial property if it is writeonly"
            if results is None:
                return None
            else:
                return results[0].value
            
    def _configureValue(self, value):
        if self._isNested():
            self._parent._configureItem(self._getItemKey(), value)
        else:
            self.compRef.configure([_CF.DataType(str(self.id), value)])

    @property
    def propRef(self):
        # DEPRECATED: create the CF.DataType reference, change value to the correct type
        _warnings.warn("'propRef' is deprecated; use 'toAny' to create Any value", DeprecationWarning)
        return _CF.DataType(id=str(self.id), value=_CORBA.Any(self.typecode, None))

    def queryValue(self):
        '''
        Returns the current value of the property by doing a query
        on the component and returning only the value
        '''
        if not self._checkRead():
            raise Exception, 'Could not perform query, ' + str(self.id) + ' is a writeonly property'

        value = self._queryValue()
        return self.fromAny(value)

    def configureValue(self, value):
        '''
        Sets the current value of the property by doing a configure
        on the component. If this property is a child of another property,
        the parent property is configured.
        '''
        if not self._checkWrite():
            raise Exception, 'Could not perform configure, ' + str(self.id) + ' is a readonly property'

        try:
            value = self.toAny(value)
        except EnumValueError, ex:
            # If enumeration value is invalid, list available enumerations.
            print 'Could not perform configure on ' + str(ex.id) + ', invalid enumeration provided'
            print "Valid enumerations: "
            for name, value in ex.enums.iteritems():
                print "\t%s=%s" % (name, value)
            return
        self._configureValue(value)

    # Wrapper functions for common operations on the property value. These
    # allow the data type (e.g. int, float, list) to determine the behavior
    # of common operations, rather than having to explicitly write each
    # operator.
    def proxy_operator(op):
        """
        Performs an operator on the result of queryValue().
        """
        def wrapper(self, *args, **kwargs):
            return op(self.queryValue(), *args, **kwargs)
        return wrapper

    def proxy_reverse_operator(op):
        """
        Performs a reflected operator on the result of queryValue().
        """
        def wrapper(self, y, *args, **kwargs):
            return op(y, self.queryValue(), *args, **kwargs)
        return wrapper

    def proxy_inplace_operator(op):
        """
        Performs an in-place operator on the result of queryValue(). After
        the value is modified, the new value is stored via configureValue().
        """
        def wrapper(self, y, *args, **kwargs):
            temp = op(self.queryValue(), y, *args, **kwargs)
            self.configureValue(temp)
            return temp
        return wrapper

    def proxy_modifier_function(op):
        """
        Calls a member function that modifies the state of its instance on the
        result queryValue(). After the value is modified, the  new value is
        stored via configureValue().
        """
        def wrapper(self, *args, **kwargs):
            temp = self.queryValue()
            op(temp, *args, **kwargs)
            self.configureValue(temp)
        return wrapper

    '''
    BELOW ARE OVERRIDDEN FUNCTIONS FOR LITERAL TYPES IN
    PYTHON SO THAT THE PROPERTIES BEHAVE WITHIN THE CODE
    AS IF THEY WERE THEIR LITERAL TYPES
    '''
    # Binary arithmetic operations
    __add__ = proxy_operator(_operator.add)
    __sub__ = proxy_operator(_operator.sub)
    __mul__ = proxy_operator(_operator.mul)
    __div__ = proxy_operator(_operator.div)
    __truediv__ = proxy_operator(_operator.truediv)
    __floordiv__ = proxy_operator(_operator.floordiv)
    __mod__= proxy_operator(_operator.mod)
    __divmod__ = proxy_operator(divmod)
    __pow__ = proxy_operator(pow)
    __lshift__ = proxy_operator(_operator.lshift)
    __rshift__ = proxy_operator(_operator.rshift)
    __and__ = proxy_operator(_operator.and_)
    __xor__ = proxy_operator(_operator.xor)
    __or__ = proxy_operator(_operator.or_)

    # In-place arithmetic operations
    __iadd__ = proxy_inplace_operator(_operator.add)
    __isub__ = proxy_inplace_operator(_operator.sub)
    __imul__ = proxy_inplace_operator(_operator.mul)
    __idiv__ = proxy_inplace_operator(_operator.div)
    __itruediv__ = proxy_inplace_operator(_operator.truediv)
    __ifloordiv__ = proxy_inplace_operator(_operator.floordiv)
    __imod__ = proxy_inplace_operator(_operator.mod)
    __ipow__ = proxy_inplace_operator(pow)
    __ilshift__ = proxy_inplace_operator(_operator.lshift)
    __irshift__ = proxy_inplace_operator(_operator.rshift)
    __iand__ = proxy_inplace_operator(_operator.and_)
    __ixor__ = proxy_inplace_operator(_operator.xor)
    __ior__ = proxy_inplace_operator(_operator.or_)

    # Reflected binary arithmetic operations
    __radd__ = proxy_reverse_operator(_operator.add)
    __rsub__ = proxy_reverse_operator(_operator.sub)
    __rmul__ = proxy_reverse_operator(_operator.mul)
    __rdiv__ = proxy_reverse_operator(_operator.div)
    __rtruediv__ = proxy_reverse_operator(_operator.truediv)
    __rfloordiv__ = proxy_reverse_operator(_operator.floordiv)
    __rmod__ = proxy_reverse_operator(_operator.mod)
    __rdivmod__ = proxy_reverse_operator(divmod)
    __rpow__ = proxy_reverse_operator(pow)
    __rlshift__ = proxy_reverse_operator(_operator.lshift)
    __rrshift__ = proxy_reverse_operator(_operator.rshift)
    __rand__ = proxy_reverse_operator(_operator.and_)
    __rxor__ = proxy_reverse_operator(_operator.xor)
    __ror__ = proxy_reverse_operator(_operator.or_)

    # Unary arithmetic operations
    __neg__ = proxy_operator(_operator.neg)
    __pos__ = proxy_operator(_operator.pos)
    __abs__ = proxy_operator(abs)
    __invert__ = proxy_operator(_operator.invert)

    # Type conversions
    __complex__ = proxy_operator(complex)
    __int__ = proxy_operator(int)
    __long__ = proxy_operator(long)
    __float__ = proxy_operator(float)
    __nonzero__ = proxy_operator(bool)

    # Base conversion
    __oct__ = proxy_operator(oct)
    __hex__ = proxy_operator(hex)

    # Coercion
    __coerce__ = proxy_operator(coerce)

    # Rich comparison methods
    __lt__ = proxy_operator(_operator.lt)
    __le__ = proxy_operator(_operator.le)
    __eq__ = proxy_operator(_operator.eq)
    __ne__ = proxy_operator(_operator.ne)
    __gt__ = proxy_operator(_operator.gt)
    __ge__ = proxy_operator(_operator.ge)

    # Sequence/string operations
    __len__ = proxy_operator(len)
    __getitem__ = proxy_operator(_operator.getitem)
    __contains__ = proxy_operator(_operator.contains)

    # Turn wrapper creation methods into static methods so that they can be
    # called by subclasses.
    proxy_operator = staticmethod(proxy_operator)
    proxy_inplace_operator = staticmethod(proxy_inplace_operator)
    proxy_reverse_operator = staticmethod(proxy_reverse_operator)
    proxy_modifier_function = staticmethod(proxy_modifier_function)
    
def _convertToComplex(value):
    if value == None:
        return None
    if isinstance(value.real, str):
        # for characters, we need to map to an integer as
        # the complex() method will not accept 2 strings as input
        value = complex(ord(value.real), ord(value.imag))
    else:
        value = complex(value.real, value.imag)
    return value
 
class simpleProperty(Property):
    def __init__(self, id, valueType, enum, compRef, kinds,defValue=None, parent=None, mode='readwrite', action='external',
                 structRef=None, structSeqRef=None, structSeqIdx=None):
        """ 
        Create a new simple property.

        Arguments:
          id        - The property ID
          valueType - Type of the property, must be in VALUE_TYPES
          compRef   - Reference to the PropertySet that owns this property
          defValue  - Default Python value for this property (default: None)
          parent    - Parent property that contains this property (default: None)
          mode      - Mode for the property, must be in MODES (default: 'readwrite')
          action    - Allocation action type (default: 'external')

        Deprecated arguments:
          structRef, structSeqRef, structSeqIdx
        """
        if valueType not in SCA_TYPES:
            raise('"' + str(valueType) + '"' + ' is not a valid valueType, choose from\n ' + str(SCA_TYPES))
        
        # Initialize the parent
        Property.__init__(self, id, type=valueType, kinds=kinds,compRef=compRef, mode=mode, action=action, parent=parent,
                          defValue=defValue)
        
        self.valueType = valueType
        self.typecode = getTypeCode(self.valueType)
        if enum != None:
            self._enums = self._parseEnumerations(enum)
        else:
            self._enums = None

    def _getItemKey(self):
        return self.id

    def _parseEnumerations(self,enum):
        enums = {}
        for en in enum:
            if en.get_value() is None:
                value = None
            elif self.valueType in ['long', 'longlong', 'octet', 'short', 'ulong', 'ulonglong', 'ushort']: 
                if en.get_value().find('x') != -1:
                    value = int(en.get_value(),16)
                else:
                    value = int(en.get_value())
            elif self.valueType in ['double', 'float']:
                value = float(en.get_value())
            elif self.valueType in ['char', 'string']:
                value = str(en.get_value())
            elif self.valueType == 'boolean':
                value = {"TRUE": True, "FALSE": False}[en.get_value().strip().upper()]
            else:
                value = None
            enums[str(en.get_label())] = value
        return enums


    def _enumValue(self, value):
         if value in self._enums.values():
             return value
         elif value in self._enums.keys():
             return self._enums.get(value)
         raise EnumValueError(self.id, value, self._enums)

    @property
    def structRef(self):
        if self._isNested():
            return self._parent.id
        else:
            return None

    @property
    def structSeqRef(self):
        # DEPRECATED: used when the struct is part of a struct sequence
        try:
            # NB: Deprecation warning issued by parent
            return self._parent.structSeqRef
        except:
            _warnings.warn("'structSeqRef' is deprecated", DeprecationWarning)
            return None

    @property
    def structSeqIdx(self):
        # DEPRECATED: used when the struct is part of a struct sequence
        _warnings.warn("'structSeqIdx' is deprecated for simple properties", DeprecationWarning)
        try:
            return self._parent.structSeqIdx
        except:
            return None
        
    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return None

        value = value.value()
        if self.valueType.find("complex") != -1:
            return _convertToComplex(value)
        else:
            return value

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value == None:
            return _any.to_any(None)

        # If property is an enumeration, enforce proper value
        if self._enums != None:
            value = self._enumValue(value)

        if self.valueType.startswith("complex"):
            memberTypeStr = mapComplexToSimple(self.valueType)
            real, imag = _type_helpers._splitComplex(value)
            realValue = _type_helpers.checkValidValue(real, memberTypeStr)
            imagValue = _type_helpers.checkValidValue(imag, memberTypeStr)

            # Convert to CORBA type (e.g., CF.complexFloat)
            value = getCFType(self.valueType)(realValue, imagValue)
        else:
            # Validate the value
            value = _type_helpers.checkValidValue(value, self.valueType)

        return _CORBA.Any(self.typecode, value)

    def __repr__(self, *args):
        ret=''
        if self.mode != "writeonly":
            value = self.queryValue()
            if value != None:
                ret=str(value)
        return ret
        
    def __str__(self, *args):
        return self.__repr__()

    def enums(self):
        print self._enums
        
    def __getattr__(self, name):
        # If attribute is not found on simpleProperty, defer to the value; this
        # allows things like '.real' and '.imag', or string methods to work
        # without explicit support.
        return getattr(self.queryValue(), name)


class sequenceProperty(Property):
    def __init__(self, id, valueType, kinds, compRef, defValue=None, parent=None, mode='readwrite'):
        """
        Create a new sequence property. Instances behave like list objects.

        Arguments:
          id        - The property ID
          valueType - Type of the property, must be in VALUE_TYPES, or can be struct
          compRef   - Reference to the PropertySet that owns this property
          defValue  - Default Python value for this property (default: None)
          mode      - Mode for the property, must be in MODES (default: 'readwrite')
        """
        if valueType not in SCA_TYPES and valueType != 'structSeq':
            raise('"' + str(valueType) + '"' + ' is not a valid valueType, choose from\n ' + str(SCA_TYPES))
        
        # Initialize the parent Property
        Property.__init__(self, id, type=valueType, kinds=kinds, compRef=compRef, parent=parent, mode=mode, action='external',
                          defValue=defValue)
        
        self.complex = False
        
        #try to set the value type in ref unless this is a sequence of structs
        if valueType != 'structSeq':
            self.valueType = valueType
            if self.valueType == "string":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.StringSeq)
            elif self.valueType == "boolean":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.BooleanSeq)
            elif self.valueType == "ulong":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ULongSeq)
            elif self.valueType == "short":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.ShortSeq)
            elif self.valueType == "float":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.FloatSeq)
            elif self.valueType == "char":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.CharSeq)
            elif self.valueType == "octet":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.OctetSeq)
            elif self.valueType == "ushort":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.UShortSeq)
            elif self.valueType == "double":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.DoubleSeq)
            elif self.valueType == "long":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_CORBA.LongSeq)
            elif self.valueType == "longlong":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.LongLongSequence)
            elif self.valueType == "ulonglong":
                self.typecode = _tcInternal.typeCodeFromClassOrRepoId(_PortTypes.UlongLongSequence) 
            elif self.valueType.find("complex") == 0:
                self.typecode = getCFSeqType(self.valueType)

                # It is important to have a means other than .find("complex")
                # to determine complexity, as in the case of a struct sequence,
                # the value of self.valueType may not be a string.
                self.complex = True

    def _getItemKey(self):
        return self.id

    def _mapCFtoComplex(self, CFVal):
        return complex(CFVal.real, CFVal.imag)

    def _getComplexConfigValues(self, value):
        '''
        Go from:
            [complex(a,b), complex(a,b), ..., complex(a,b)]
        to:
            [CF.complexType(a,b), CF.complexType(a,b), ..., complexType(a,b)]
        '''

        memberTypeStr = mapComplexToSimple(self.valueType)
        newValues = []
        for val in value:
            # value is actually a list.  loop through each val
            # in the list and validate it
            try:
                real = val.real
                imag = val.imag
            except AttributeError:
                # In Python 2.4, basic types don't support 'real' and 'imag',
                # so assume that val is the real component and use its type to
                # create an imaginary component with a value of 0
                real = val
                imag = type(val)(0)
            realValue = _type_helpers.checkValidValue(real, memberTypeStr)
            imagValue = _type_helpers.checkValidValue(imag, memberTypeStr)
        
            # convert to CORBA type (e.g., CF.complexFloat)
            newValues.append(getCFType(self.valueType)(realValue, imagValue))
        return newValues

    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return []

        values = value.value()
        if values is not None and values != '':
            if self.complex:
                values = [self._mapCFtoComplex(x) for x in values]
            elif self.valueType == 'octet':
                # Octet sequences are stored as strings
                values = [ord(x) for x in values]
            elif self.valueType == 'char':
                values = [x for x in values]
            return values
        if self.valueType == 'octet' or self.valueType == 'char':
            return ''
        return []

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value == None:
            return _any.to_any(None)

        if self.complex:
            value = self._getComplexConfigValues(value)
        else:
            value = _type_helpers.checkValidDataSet(value, self.valueType)

        return _CORBA.Any(self.typecode, value)
        
    '''
    BELOW ARE OVERRIDDEN FUNCTIONS FOR THE LIST CLASS IN
    PYTHON SO THAT THE PROPERTIES BEHAVE WITHIN THE CODE
    AS IF THEY ARE ACTUALLY LISTS
    '''
    # Container methods
    # __getitem__ and __contains__ are implemented in Property
    __setitem__ = Property.proxy_modifier_function(_operator.setitem)
    __delitem__ = Property.proxy_modifier_function(_operator.delitem)
    __iter__ = Property.proxy_operator(iter)
    # NB: __reversed__ is ignored prior to Python 2.6
    __reversed__ = Property.proxy_operator(reversed)

    # List methods
    append = Property.proxy_modifier_function(list.append)
    count = Property.proxy_operator(list.count)
    extend = Property.proxy_modifier_function(list.extend)
    index = Property.proxy_operator(list.index)
    insert = Property.proxy_modifier_function(list.insert)
    pop = Property.proxy_modifier_function(list.pop)
    remove = Property.proxy_modifier_function(list.remove)
    reverse = Property.proxy_modifier_function(list.reverse)
    sort = Property.proxy_modifier_function(list.sort)

    def __repr__(self):
        if self.mode != "writeonly":
            return repr(self.queryValue())
        return ''
    
    def __str__(self):
        return self.__repr__()
    
            
class structProperty(Property):
    # All structs have the same CORBA typecode.
    typecode = _CORBA.TypeCode("IDL:CF/Properties:1.0")

    def __init__(self, id, valueType, kinds, compRef, props, defValue=None, parent=None, structSeqIdx=None, mode='readwrite',
                 structSeqRef=None):
        """ 
        Create a struct property.

        Arguments:
          id           - The property ID
          valueType    - List of tuples describing members, in the following format:
                         (id, valueType(as defined for simples), defValue, name)
          compRef      - Reference to the PropertySet that owns this property
          defValue     - Default Python value for this property (default: None)
          parent       - Parent property that contains this property (default: None)
          structSeqIdx - Index of the struct in parent struct sequence (default: None)
          mode         - Mode for the property, must be in MODES (default: 'readwrite')

        Deprecated arguments:
          structSeqRef
        """
        if not isinstance(valueType, list):
            raise TypeError('valueType must be provided as a list')

        # Since members is used for attribute lookup, initialize it first
        self.members = {}
        self._memberNames = {}
        
        #initialize the parent
        Property.__init__(self, id, type='struct', kinds=kinds, compRef=compRef, mode=mode, action='external', parent=parent,
                          defValue=defValue)
        
        self.valueType = valueType

        #used when the struct is part of a struct sequence
        self.structSeqIdx = structSeqIdx
       
        #each of these members is itself a simple property
        propIndex = 0
        for _id, _type, _defValue, _clean_name in valueType:
            try:
                enum = props[propIndex].get_enumerations().get_enumeration()
            except:
                enum = None
            classType = ''
            for p in props:
                if _id == p.get_id():
                    if isinstance(p, _parsers.prf.simple):
                        classType = 'simple'
                        break
                    elif isinstance(p, _parsers.prf.simpleSequence):
                        classType = 'simpleSequence'
                        break
            if classType == 'simple':
                prop = simpleProperty(_id, _type, enum, compRef=compRef, kinds=kinds, defValue=_defValue, parent=self)
            elif classType == 'simpleSequence':
                prop = sequenceProperty(_id, _type, compRef=compRef, kinds=kinds, defValue=_defValue, parent=self)
            prop.clean_name = _clean_name
            self.members[_id] = prop
            # Map the clean name back to the ID, and if it was a duplicate
            # (and thus had a count appended), map the non-unique name as well
            self._memberNames[_clean_name] = _id
            baseName = _cleanId(props[propIndex])
            if baseName != _clean_name:
                self._memberNames[baseName] = _id
            propIndex += 1

    def _getItemKey(self):
        return self.structSeqIdx

    def _queryItem(self, propId):
        results = self._queryValue()
        if results is None:
            return None
        for item in results.value():
            if item.id == propId:
                return item.value
        return None

    def _configureItem(self, propId, value):
        structValue = self._queryValue()
        if structValue is None:
            return
        foundItem = False
        for prop in structValue._v:
            if prop.id == propId:
                prop.value = value
                foundItem = True
        if not foundItem:
            structValue._v.append(_CF.DataType(id=propId, value=value))
        self._configureValue(structValue)

    def _checkValue(self, value):
        for memberId in value.iterkeys():
            self._getMemberId(memberId)

    def _getMemberId(self, name):
        if name in self.members:
            return name
        memberId = self._memberNames.get(name, None)
        if memberId:
            return memberId
        raise TypeError, "'%s' is not a member of '%s'" % (name, self.id)

    def _remapValue(self, value):
        valout = {}
        for memberId, memberVal in value.iteritems():
            memberId = self._getMemberId(memberId)
            valout[memberId] = memberVal
        return valout

    def _getMember(self, name):
        memberId = self._memberNames.get(name, None)
        if memberId:
            return self.members[memberId]
        else:
            return None

    @property
    def structSeqRef(self):
        # DEPRECATED: used when the struct is part of a struct sequence
        _warnings.warn("'structSeqRef' is deprecated", DeprecationWarning)
        if self._isNested():
            return self._parent.id
        else:
            return None

    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return {}

        structVal = {}
        for prop in value.value():
            try:
                member = self.members[prop.id]
                structVal[prop.id] = member.fromAny(prop.value)
            except KeyError:
                structVal[prop.id] = _any.from_any(prop.value) 
                
        return structVal

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value is None:
            props = [_CF.DataType(str(m.id), m.toAny(None)) for m in self.members.values()]
            return _CORBA.Any(self.typecode, props)

        if not isinstance(value, dict):
            raise TypeError, 'configureValue() must be called with dict instance as second argument (got ' + str(type(value))[7:-2] + ' instance instead)'

        # Remap the value keys, which may be names, to IDs; this also checks
        # that the value passed in matches the struct definition
        value = self._remapValue(value)

        # Convert struct items into CF::Properties.
        props = []
        for _id, member in self.members.iteritems():
            memberVal = value.get(_id, member.defValue)
            props.append(_CF.DataType(str(_id), member.toAny(memberVal)))

        return _CORBA.Any(self.typecode, props)

    def configureValue(self, value):
        '''
        Helper function for configuring a struct property, using a
        dictionary as the passed in value
        '''
        if value is not None:
            # Remap the keys in the dictionary to ensure that they are all
            # valid member IDs, throwing an exception if there are any unknown
            # IDs; this will also make a copy, so that any updates do not
            # affect the passed-in value
            value = self._remapValue(value)
            
            # We now know that all the keys in the dictionary are also in the
            # members dictionary, so if the sizes are not equal, there are
            # values missing; query the current value for those
            if len(value) != len(self.members):
                current = self.queryValue()
                current.update(value)
                value = current
        super(structProperty,self).configureValue(value)
    
    def __str__(self):
        return self.__repr__()
    
    def __repr__(self):
        currValue = ""
        if self.mode != "writeonly":
            currValue = self.queryValue()
        structView = "ID: " + self.id
        for key in currValue:
            try:
                structView = structView + '\n  ' + str(self.members[key].clean_name) + ": " + str(currValue[key])
            except KeyError:
                structView = structView + '\n  ' + key + ": " + str(currValue[key])  
        return structView

    def __getattr__(self, name):
        '''
        If the attribute being looked up is actually a member of the struct,
        then return that simple property, otherwise default to the normal
        getattribute function
        '''
        member = self._getMember(name)
        if member is not None:
            return member
        else:
            try:
                return super(structProperty, self).__getattribute__(name)
            except AttributeError:
                return self.queryValue()[name]

    def __setattr__(self, name, value):
        '''
        If the attribute being looked up is actually a member of the struct,
        then try to configure the simple property.  This will result in a
        configure of the entire struct in the simpleProperty class
        '''
        if name not in ('members', '_memberNames'):
            member = self._getMember(name)
            if member is not None:
                member.configureValue(value)
                return
        super(structProperty, self).__setattr__(name, value)

    # Container methods
    # __getitem__ and __contains__ are implemented in Property
    __setitem__ = Property.proxy_modifier_function(_operator.setitem)
    __iter__ = Property.proxy_operator(iter)

    # Dict methods
    has_key = Property.proxy_operator(dict.has_key)
    items = Property.proxy_operator(dict.items)
    iteritems = Property.proxy_operator(dict.iteritems)
    iterkeys = Property.proxy_operator(dict.iterkeys)
    itervalues = Property.proxy_operator(dict.itervalues)
    keys = Property.proxy_operator(dict.keys)
    update = Property.proxy_modifier_function(dict.update)
    values = Property.proxy_operator(dict.values)


class structSequenceProperty(sequenceProperty):
    # All struct sequences have the same CORBA typecode.
    typecode = _CORBA.TypeCode("IDL:omg.org/CORBA/AnySeq:1.0")

    def __init__(self, id, structID, valueType, kinds, props, compRef, defValue=[], mode='readwrite'):
        """ 
        Create a struct sequence property.

        Arguments:
          id           - The property ID
          structID     - The struct definition ID
          valueType    - List of tuples describing members, in the following format:
                         (id, valueType(as defined for simples), defValue, name)
          compRef      - Reference to the PropertySet that owns this property
          defValue     - Default Python value for this property (default: [])
          mode         - Mode for the property, must be in MODES (default: 'readwrite')
        """
        if not isinstance(valueType, list):
            raise TypeError('valueType must be provided as a list')
        
        #initialize the parent
        sequenceProperty.__init__(self, id, valueType='structSeq', kinds=kinds, compRef=compRef, defValue=defValue, mode=mode)
        self.valueType = valueType
        self.props = props
        # Create a property for the struct definition.
        self.structDef = structProperty(id=structID, valueType=self.valueType, kinds=kinds, props=props, compRef=self.compRef, mode=self.mode)

    @property
    def propRef(self):
        # DEPRECATED: Create the CF.DataType reference
        # NB: Use the superclass propRef property to issue the deprecation
        #     warning, then alter the returned value to match old behavior
        value = super(structSequenceProperty, self).propRef
        value.value._v = []
        return value

    @property
    def structID(self):
        return self.structDef.id

    def __getitem__(self, index):
        # The actual struct property doesn't exist, so create and return it
        return structProperty(id=self.structDef.id, valueType=self.valueType, kinds=self.kinds, props=self.props, compRef=self.compRef,
                              parent=self, structSeqIdx=index, mode=self.mode)
    
    def __setitem__(self, index, value):
        # Use __getitem__ to get a struct property, then configure it; this
        # will trigger a configure of the entire sequence
        self[index].configureValue(value)

    def _queryItem(self, index):
        # Get the full struct sequence value.
        results = self._queryValue()
        if results is None:
            return None
        return results.value()[index]

    def _configureItem(self, index, value):
        # Get the full struct sequence value.
        results = self._queryValue()
        if results is None:
            return

        # Replace the struct in the list.
        results._v[index] = value

        # Configure the complete, updated struct sequence.
        self._configureValue(results)

    def fromAny(self, value):
        '''
        Converts the input value in CORBA Any format to Python.
        '''
        if value is None:
            return []

        return [self.structDef.fromAny(v) for v in value.value()]

    def toAny(self, value):
        '''
        Converts the input value in Python format to a CORBA Any.
        '''
        if value is None:
            return _any.to_any(None)
        return _CORBA.Any(self.typecode, [self.structDef.toAny(v) for v in value])

def parseComplexString(ajbString, baseType):
    '''
    Parse a string in the form A+jB into its real (A) and imag (B) components.

    baseType can either be a Python type (e.g., float) , or a string
    containing the name of a complex type (e.g., "float").
    '''

    if __TYPE_MAP.has_key(baseType): 
        # if the type is passed in as a string
        # e.g., "float" vs. float
        baseType = getPyType(baseType) 

    real = baseType(0)
    imag = baseType(0)
    sign = 1
    signLocation = ajbString.find("-j")
    if signLocation != -1:
        # if a negative-sign is found for imag data
        sign = -1
    jLocation = ajbString.find("j")
    if jLocation < 0:
        # if no "j", then we just have a real number
        real = baseType(ajbString)
    else:
        real = baseType(ajbString[:jLocation-1])
        imag = sign * baseType(ajbString[jLocation+1:])
    return real, imag
