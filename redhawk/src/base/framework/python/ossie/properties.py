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

# vim: sw=4: sts=4: et
# Combine SCA behavior and the python
# behavior of property()/@property (including
# the 2.6 behavior of getter, setter
# on the decorator

from ossie.cf import CF
from omniORB import any, CORBA, tcInternal
import ossie.parsers.prf
import sys
import traceback
import StringIO
import types
import struct

# From section 1.3 of the CORBA Python language mapping
__TYPE_MAP = {'boolean': (int, CORBA.TC_boolean), 
              'char': (str, CORBA.TC_char), 
              'double': (float, CORBA.TC_double), 
              'float': (float, CORBA.TC_float), 
              'long': (int, CORBA.TC_long),
              'longdouble': (float, CORBA.TC_longdouble), 
              'longlong': (long, CORBA.TC_longlong), 
              'None': (None, CORBA.TC_null), 
              'octet': (int, CORBA.TC_octet), 
              'short': (int, CORBA.TC_short), 
              'string': (str, CORBA.TC_string), 
              'ulong': (long, CORBA.TC_ulong), 
              'ulonglong':(long, CORBA.TC_ulonglong), 
              'ushort': (int, CORBA.TC_ushort), 
              'void': (None, CORBA.TC_void), 
              'wchar': (str, CORBA.TC_char), 
              'wstring': (str, CORBA.TC_string)}

_SCA_TYPES = ['boolean', 'char', 'double', 'float', 'short', 'long', 'longlong',
              'objref', 'octet', 'string', 'ulong', 'ushort', 'longlong', 'ulonglong']

def to_pyvalue(data, type_):
    """Given a data value in any python type and the desired SCA type_, attempt
    to return a python value of the correct type"""
    if data == None:
        return None

    pytype = __TYPE_MAP[type_][0]
    if type(data) != pytype:
        # Handle boolean strings as a special case
        if type_ == "boolean" and type(data) in (str, unicode):
            data = {"TRUE": 1, "FALSE": 0}[data.strip().upper()]
        # Otherwise, fall back to built in python conversion
        else:
            data = pytype(data)
    return data

def getTypeMap(type_):
    return __TYPE_MAP[type_][1]

def to_xmlvalue(data, type_):
    if data == None:
        return ""
    v = repr(to_pyvalue(data, type_))
    if (type_ == "longlong" or type_ == "long" or type_ == "short" or type_ == "ulong" or type_ == "ushort" or type_ == "ulonglong") and v.endswith("L"):
        v = v[:-1]
    if type_ == "float" or type_ == "double":
        if "e" in v:
            if v[v.find('e')+1]=='+':
                retval = v[:v.find('.')]
                retval += v[v.find('.')+1:v.find('e')]
                diff = v.find('e') - (v.find('.')+1)
                zeros = int(v[v.find('e')+2:]) - diff
                retval += zeros*'0'
                v=retval
            else:
                negative = False
                if v[0] == '-':
                    negative = True
                    retval = '-0.'
                else:
                    retval = '0.'
                diff = v.find('.')
                zeros = int(v[v.find('e')+2:]) - diff
                retval += zeros*'0'
                if negative:
                    retval += '0'+v[1:v.find('.')]
                else:
                    retval += v[:v.find('.')]
                retval += v[v.find('.')+1:v.find('e')]
                v=retval
    elif type_ == "boolean":
        v = str(bool(data)).lower()
    elif type_ == "string": # Remove quotes added by repr
        v = v[1:-1]
    return v

def to_tc_value(data, type_):
    if data is None:
        return any.to_any(None)

    # If the typecode is known, use that
    if __TYPE_MAP.has_key(type_):
        pytype, tc = __TYPE_MAP[type_]

        # If the value is already an Any, check its type; if it's already the
        # right type, nothing needs to happen, otherwise extract the value and
        # convert
        if isinstance(data, CORBA.Any):
            if data.typecode().equal(tc):
                return data
            else:
                data = data.value()

        # Convert to the correct Python type, if necessary
        if not isinstance(data, pytype):
            data = to_pyvalue(data, type_)
        return CORBA.Any(tc, data)
    else:
        # Unknown type, let omniORB decide
        return any.to_any(data)

def struct_fields(value):
    if isinstance(value, types.ClassType) or hasattr(value, '__bases__'):
        clazz = value
    else:
        clazz = type(value)
    # Try to get the correct field ordering, if it is available, otherwise
    # just look at the class dictionary to find the fields.
    fields = getattr(clazz, '__fields', None)
    if fields is None:
        fields = [p for p in clazz.__dict__.itervalues() if isinstance(p, simple_property)]
    return fields

def struct_values(value):
    result = []
    # Try to get the correct field ordering, if it is available, otherwise
    # just look at the class dictionary to find the fields.
    fields = struct_fields(value)
    for attr in fields:
        field_value = attr.get(value)
        result.append((attr.id_, field_value))
    return result

def struct_to_props(value, fields=None):
    if fields is None:
        # Try to get the correct field ordering, if it is available, otherwise
        # just look at the class dictionary to find the fields.
        fields = struct_fields(value)
    return [CF.DataType(attr.id_, attr._toAny(attr.get(value))) for attr in fields]

def struct_to_any(value, fields=None):
    return props_to_any(struct_to_props(value, fields))

def struct_from_props(value, structdef):
    # Create an uninitialized struct definition
    structval = structdef()

    # Turn the value into a dictionary
    newvalues = dict([(v["id"], v["value"]) for v in value])

    # For each field, try to set the value
    for name, attr in structdef.__dict__.items():
        if type(attr) is not simple_property:
            continue
        try:
            value = newvalues[attr.id_]
        except KeyError:
            raise ValueError, "provided value is missing element " + attr.id_
        else:
            attr.set(structval, value)

    return structval

def struct_from_any(value, structdef):
    value = any.from_any(value)
    return struct_from_props(value, structdef)

def struct_from_prop(prop, structdef):
    return struct_from_any(prop.value, structdef)

def struct_from_dict(id, dt):
    '''Creates a Struct Property from a dictionary.
         input: 
            - 'id': id of returned Struct Property
            - 'dt': dictionary containing Struct contents
         returns:
            - Struct Property is id of 'id'
    '''
    a = [CF.DataType(id=k, value=any.to_any(check_type_for_long(v))) for k, v in dt.items()]
    return CF.DataType(id=id, value=any.to_any(a))

def struct_prop_from_seq(prop, idx):
  return prop.value._v[idx]

def struct_prop_id_update(prop, id, val):
  for idx in prop:
    if idx.id == id:
      idx.value = any.to_any(check_type_for_long(val))
      break

def check_type_for_long(val):
    if type(val) == int: # any.to_any defaults int values to TC_long, which is not appropriate outside the 2**32 range
        if val < -2147483648 or val > 2147483647:
            return (long(val))
    return val

def props_from_dict(dt):
    '''Creates a Properties list from a dictionary'''
    result = []
    for _id,_val in dt.items():
        # either structsequence or simplesequence
        if type(_val) == list:
            # structsequence
            if len(_val) > 0 and type(_val[0]) == dict:
                a = [props_to_any([CF.DataType(id=k, value=any.to_any(check_type_for_long(v))) for k,v in b.items()]) for b in _val]
                result.append(CF.DataType(id=_id, value=any.to_any(a)))
            # simplesequence
            else: 
                result.append(CF.DataType(id=_id, value=any.to_any(check_type_for_long(_val))))
        # struct
        elif type(_val) == dict:
            result.append(struct_from_dict(_id, _val))
        # simple
        else:
            result.append(CF.DataType(id=_id, value=any.to_any(check_type_for_long(_val))))

    return result

def prop_to_dict(p):
    '''Creates a dictionary from a Property'''
    return props_to_dict([p])

def props_to_dict(pl):
    '''Creates a dictionary from a Properties list'''
    result = {}
    for p in pl:
        # struct property
        if p.value.typecode().equivalent(CF._tc_Properties):
           result[p.id] = dict([(x.id,any.from_any(x.value)) for x in p.value.value()])
        # structsequence property
        elif p.value.typecode().equivalent(CORBA._tc_AnySeq):
            result[p.id] = [dict([(x.id,any.from_any(x.value))for x in y.value()]) for y in p.value.value()]
        # simplesequence property
        elif p.value.typecode().kind() == CORBA.tk_sequence:
            #char and octet are special cases in Python, they should not be returned 
            #as explicit lists because that is not how they are configured
            if p.value.typecode().content_type() == CORBA.TC_octet or p.value.typecode().content_type() == CORBA.TC_char:
                result[p.id] = any.from_any(p.value)
            else:
                result[p.id] = [x for x in any.from_any(p.value)]
        # simple property
        else:
            result[p.id] = any.from_any(p.value)
            
    return result

def props_to_any(props):
    return CORBA.Any(CF._tc_Properties, props)

def _map_id(id):
    """
    Convert a DCE UUID to a legal Python identifier.
    """
    return id.replace(':', '_').replace('-', '_')

def prf_to_classes(prfFile):
    """
    Returns a dictionary of class objects that construct compatible Python
    objects for the struct types in the PRF file referenced by 'prfFile'. The
    dictionary keys are the names of the structs as defined by the XML.
    """
    props = ossie.parsers.prf.parse(prfFile)
    classes = {}

    structs = props.get_struct() + [prop.get_struct() for prop in props.get_structsequence()]
    for prop in structs:
        clazz = xml_to_class(prop)
        classes[clazz.__name] = clazz

    return classes

def xml_to_class(prop):
    """
    Given an instance of ossie.parsers.prf.struct, return a class object that
    can create a compatible Python object that may be serialized to a CORBA
    Any with struct_to_any().
    """

    # Basic __init__(), initializes all fields.
    def initialize(self):
        object.__init__(self)
        for name, attr in self.__class__.__dict__.items():
            if type(attr) is simple_property:
                attr.initialize(self)

    # Basic __str__(), prints all fields.
    def toString(self):
        out = ''
        for name, attr in self.__class__.__dict__.items():
            if type(attr) is simple_property:
                out += '%s=%s\n' % (name, str(attr.get(self)))
        return out[:-1]
  
    name = prop.get_name()
    id = str(prop.get_id())
    if not name:
        name = _map_id(id)
    else:
        name = str(name)

    # The class object knows the id and name, and keeps an ordered list of the
    # fields so that instances can be serialized into CF.Properties that match
    # the expectation of generated C++ struct properties.
    members = { '__init__' : initialize,
                '__str__'  : toString,
                '__id'     : id,
                '__name'   : name,
                '__fields' : [] }

    # For each simple element that makes up the struct definition, create a
    # simple property as a member of the class object.
    for item in prop.get_simple():
        itemId = str(item.get_id())
        itemName = item.get_name()
        if not itemName:
            itemName = _map_id(itemId)
        else:
            itemName = str(itemName)
        field = simple_property(itemId, item.get_type(), defvalue=item.get_value())
        members[itemName] = field
        members['__fields'].append(field)

    return type(name, (object,), members)

class _property(object):
    def __init__(self, 
                 id_, 
                 type_=None,
                 name=None, 
                 units=None, 
                 mode="readwrite", 
                 action="external", 
                 kinds=("configure",), 
                 description=None, 
                 fget=None, 
                 fset=None, 
                 fval=None):
        self.id_ = id_
        if type_ != None and type_ not in _SCA_TYPES:
            raise ValueError, "type %s is invalid" % type_

        self.type_ = type_
        if name == None:
            self.name = self.id_
        else:
            self.name = name
        self.units = units
        self.mode = mode
        self.action = action
        self.kinds = kinds
        self.fget = fget
        self.fset = fset
        self.fval = fval
        self.sendPropertyChangeEvent = None
         
        self._attrname = "__%s__" % self.id_

        self._query_cbname = "query_prop_%s" % self.name.replace(" ", "_")
        self._val_cbname = "validate_prop_%s" % self.name.replace(" ", "_")
        self._conf_cbname = "onconfigure_prop_%s" % self.name.replace(" ", "_")

        if description != None:
            self.__doc__ = description
        elif fget != None:
            self.__doc__ = fget.__doc__
        else:
            self.__doc__ = "SCA property %s" % self.id_

    # Subclasses of _property are expected to provide these behaviors
    def rebind(self, fget=None, fset=None, fval=None):
        """Calling rebind allows a subclass to rebind the property to use
        new fget/fset/fval functions."""
        raise NotImplementedError

    def toXML(self, level=0, version="2.2.2"):
        """Convert the property to it's equivalent SCA XML representation."""
        raise NotImplementedError
       
    def initialize(self, obj):
        """Initialize the property to it's default value."""
        raise NotImplementedError

    # Subclass may override these behaviors
    def query(self, obj, operator=None):
        # By default operators are not supported
        if operator != None:
            raise AttributeError, "this property doesn't support configure/query operators"

        value = self._query(obj)

        # Only try to do conversion to CORBA Any if the value is not already
        # one--user callbacks may be doing this conversion for us.
        if isinstance(value, CORBA.Any):
            return value
        else:
            return self._toAny(value)

    def configure(self, obj, value, operator=None):
        # By default operators are not supported
        if operator != None:
            raise AttributeError, "this property doesn't support configure/query operators"

        # Convert the value to its proper representation (e.g. structs
        # should be a class instance, not a dictionary).
        value = self._fromAny(value)
        self._configure(obj, value)

    def compareValues(self, oldValue, newValue):
        return (oldValue != newValue)

    def set(self, obj, value):
        if self.fset != None:
            self.fset(obj, value)
        else:
            setattr(obj, self._attrname, value)

    def get(self, obj):
        # See if either an explicit or implicit callback is available
        fget = self.fget
        try:
            if self.fget != None:
                return self.fget(obj)
            else:
                return getattr(obj, self._attrname)
        except AttributeError:
            return None # if the property hasn't been initialized yet, return None instead of throwing an attribute error

    # Generic behavior
    def isConfigurable(self):
        configurable = ("configure" in self.kinds)
        return (configurable and self.isWritable())

    def isSendEventChange(self):
        iseventchange = ("event" in self.kinds)
        return (iseventchange and (("configure" in self.kinds) or (("allocation" in self.kinds) and ("external" == self.action))))

    def isQueryable(self):
        queryable = (("configure" in self.kinds) or ("execparam" in self.kinds) or (("allocation" in self.kinds) and ("external" == self.action)))
        return (queryable and self.isReadable())

    def isAllocatable(self):
        allocatable = ("allocation" in self.kinds)
        external = ("external" == self.action)
        return (allocatable and external)

    def isWritable(self):
        return self.mode in ("writeonly", "readwrite")

    def isReadable(self):
        return self.mode in ("readonly", "readwrite")

    # Internal implementation
    def __get__(self, obj, objtype=None):
        """Returns the current value of this property as a python type"""
        if obj is None:
            return self
        return self.get(obj)

    def __set__(self, obj, value):
        """Sets the current value of this property from the given python type"""
        self.set(obj, value)

    def __delete__(self, obj):
        """Prevents SCA properties from being deleted"""
        raise AttributeError, "can't delete SCA property"

    def __str__(self):
        #return "%s %s" % (self.id_, self.name)
        data = """\n
                    ID:     %s
                    Type:   %s
                    Name:   %s
                    Units:  %s
                    Mode:   %s
                    Action: %s
                    Kinds:  %s
                    Fget:   %s
                    Fset:   %s
                    Fval:   %s""" % \
                    (self.id_,
                     self.type_,
                     self.name,
                     self.units,
                     self.mode,
                     self.action,
                     self.kinds,
                     self.fget,
                     self.fset,
                     self.fval)
        return data

    def __eq__(self, other):
        if other == None:
            return False
        else:
            return (self.id_ == other.id_ and \
                    self.units == other.units and \
                    self.name == other.name and \
                    self.mode == other.mode and \
                    self.action == other.action and \
                    self.kinds == other.kinds)

    def __ne__(self, other):
        return not self.__eq__(other)
             
    def _getCallback(self, obj, methodName):
        if methodName == None:
            return None

        try:
            callback = getattr(obj, methodName)
        except AttributeError:
            return None
        else:
            if callable(callback):
                return callback
            else:
                return None

    def _query(self, obj):
        # Try the implicit callback, then fall back
        # to the automatic attribute
        query = self._getCallback(obj, self._query_cbname)
        if query != None:
            return query()
        else: 
            return self.get(obj)

    def _configure(self, obj, value):
        # Try the implicit callback, then fall back
        # to the automatic attribute
        if self.fval != None and not self.fval(obj, value):
            raise ValueError, "validation failed for value %s" % value
        else:
            validate = self._getCallback(obj, self._val_cbname)
            if validate != None and not validate(value):
                raise ValueError, "validation failed for value %s" % value

        oldvalue = self.get(obj)        
        configure = self._getCallback(obj, self._conf_cbname)
        if configure != None:
            configure(oldvalue, value)
        else:
            self.set(obj, value)
            
        valueChanged = self.compareValues(oldvalue, value)
        if valueChanged and self.isSendEventChange():
            if self.sendPropertyChangeEvent:
                self.sendPropertyChangeEvent(self.id_)
            else:
                #there is no PropertyEventSupplier so we don't have a function to call
                pass

    # Subclasses may override these functions to control conversion to and from
    # CORBA Any values.
    def _toAny(self, value):
        return any.to_any(check_type_for_long(value))

    def _fromAny(self, value):
        return any.from_any(value)

class _sequence_property(_property):
    def __init__(self, 
                 id_, 
                 type_, 
                 name=None, 
                 units=None, 
                 mode="readwrite", 
                 action="external", 
                 kinds=("configure",), 
                 description=None, 
                 fget=None, 
                 fset=None, 
                 fval=None):
        _property.__init__(self, id_, type_, name, units, mode, action, kinds, description, fget, fset, fval)

    def query(self, obj, operator=None):
        # Get the complete current value via the base class interface.
        value = self._query(obj)
        if value is None:
            return any.to_any(value)

        # Don't apply an operator if the value is already a CORBA Any, which
        # should only occur in the case of a (legacy) user callback.
        if isinstance(value, CORBA.Any):
            return value

        # Attempt to apply the operator to the returned value.
        try:
            if operator in ("[]", "[*]"):
                value = self._getDefaultOperator(value, operator)
            elif operator == "[?]":
                value = self._getKeysOperator(value, operator)
                return any.to_any(check_type_for_long(value))
            elif operator == "[@]":
                value = self._getKeyValuePairsOperator(value, operator)
                return any.to_any(check_type_for_long(value))
            elif operator == "[#]":
                value = len(self._getDefaultOperator(value, operator))
                return any.to_any(check_type_for_long(value))
            elif operator != None:
                value = self._getSliceOperator(value, operator)
        except Exception, e:
            raise AttributeError, "error processing operator '%s': '%s' %s" % (operator, e, "\n".join(traceback.format_tb(sys.exc_traceback)))

        # At this point, value must be a normal sequence, so we can use the
        # standard conversion routine.
        return self._toAny(value)

    def configure(self, obj, value, operator=None):
        # Convert the input value to the desired new value for the property,
        # applying any operators.
        if operator == "[@]":
            v = any.from_any(value)
            oldvalue = self.get(obj)
            value = self._setKeyValuePairsOperator(oldvalue, v, operator)
        else:
            value = self._fromAny(value)

        # Set the new value via the base interface.
        self._configure(obj, value)

    def _itemToAny(self, value):
        return any.to_any(check_type_for_long(value))

    def _itemFromAny(self, value):
        return any.from_any(value)

    # Internal operator definitions
    def _getDefaultOperator(self, value, operator):
        if isinstance(value, dict):
            return value.values()
        else:
            return value
        
    def _getKeysOperator(self, value, operator):
        if isinstance(value, dict):
            return value.keys()
        else:
            return range(len(value))
    
    def _getKeyValuePairsOperator(self, value, operator):
        if isinstance(value, dict):
            values = value.items()
        else:
            values = enumerate(value)
        return [CF.DataType(id=str(x[0]), value=self._itemToAny(x[1])) for x in values]

    def _setKeyValuePairsOperator(self, oldvalue, newvalue, operator):
        if isinstance(oldvalue, dict):
            for dt in newvalue:
                # For now, don't allow new keys to be added
                if oldvalue.has_key(dt["id"]):
                    oldvalue[dt["id"]] = dt["value"]
                else:
                    raise ValueError, "attempt to add new key to sequence"
            return oldvalue
        elif isinstance(oldvalue, list):
            for dt in newvalue:
                oldvalue[int(dt["id"])] = dt["value"]
            return oldvalue
        elif isinstance(oldvalue, tuple):
            tmp = list(oldvalue)
            for dt in newvalue:
                tmp[int(dt["id"])] = dt["value"]
            return tuple(tmp)
        
    def _getSliceOperator(self, value, operator):
        result = []
        indexes = [s.strip() for s in operator[1:-1].split(",")]
        if isinstance(value, dict):
            for i in indexes:
                result.append(value[i])
        else:
            # If there is only one index, see if it is a range
            if len(indexes) == 1 and indexes[0].find(":") != -1:
                lower, upper = indexes[0].split(":", 1)
                if (lower == ""):
                    lower = 0
                else:
                    lower = int(lower)
                if (upper == ""):
                    upper = len(value)
                else:
                    upper = int(upper)
                result = value[lower:upper]
            else:
                # Otherwise assume int and hope for the best
                for i in indexes:
                    result.append(value[int(i)])
        return result
    
class simple_property(_property):
    def __init__(self, 
                 id_, 
                 type_, 
                 name=None, 
                 defvalue=None, 
                 units=None, 
                 mode="readwrite", 
                 action="external", 
                 kinds=("configure",), 
                 description=None, 
                 fget=None, 
                 fset=None, 
                 fval=None):
        _property.__init__(self, id_, type_, name, units, mode, action, kinds, description, fget, fset, fval)
        self.defvalue = defvalue

    def rebind(self, fget=None, fset=None, fval=None):
        return simple_property(self.id_, 
                               self.type_, 
                               self.name, 
                               self.defvalue, 
                               self.units, 
                               self.mode, 
                               self.action, 
                               self.kinds, 
                               self.__doc__, 
                               fget, 
                               fset, 
                               fval)

    def toXML(self, level=0, version="2.2.2"):
        value = None
        if self.defvalue != None:
            value = to_xmlvalue(self.defvalue, self.type_)

        simp = ossie.parsers.prf.simple(id_=self.id_, 
                                        type_=self.type_,
                                        name=self.name, 
                                        mode=self.mode,
                                        description=self.__doc__,
                                        value=value,
                                        units=self.units,
                                        action=ossie.parsers.prf.action(type_=self.action))

        for kind in self.kinds:
            simp.add_kind(ossie.parsers.prf.kind(kindtype=kind))

        xml = StringIO.StringIO()
        simp.export(xml, level)
        return xml.getvalue()

    def initialize(self, obj):
        self.set(obj, self.defvalue)

    def _toAny(self, value):
        return to_tc_value(value, self.type_)
    
class simpleseq_property(_sequence_property):
    def __init__(self, 
                 id_, 
                 type_, 
                 name=None, 
                 defvalue=None,
                 units=None, 
                 mode="readwrite", 
                 action="external", 
                 kinds=("configure",), 
                 description=None, 
                 fget=None, 
                 fset=None, 
                 fval=None):
        _sequence_property.__init__(self, id_, type_, name, units, mode, action, kinds, description, fget, fset, fval)
        self.defvalue = defvalue
        
    def rebind(self, fget=None, fset=None, fval=None):
        return simpleseq_property(self.id_,  
                                  self.type_, 
                                  self.name, 
                                  self.defvalue, 
                                  self.units, 
                                  self.mode, 
                                  self.action, 
                                  self.kinds, 
                                  self.__doc__, 
                                  fget, 
                                  fset, 
                                  fval) 

    def toXML(self, level=0, version="2.2.2"):
        simpseq = ossie.parsers.prf.simpleSequence(id_=self.id_, 
                                                   type_=self.type_,
                                                   name=self.name, 
                                                   mode=self.mode,
                                                   description=self.__doc__,
                                                   units=self.units,
                                                   action=ossie.parsers.prf.action(type_=self.action))

        values = ossie.parsers.prf.values()
        if self.defvalue:
            for v in self.defvalue:
                values.add_value(to_xmlvalue(v, self.type_))
            simpseq.set_values(values)

        xml = StringIO.StringIO()
        simpseq.export(xml, level)
        return xml.getvalue()

    def initialize(self, obj):
        self.set(obj, self.defvalue)

    def _toAny(self, value):
        if value is None:
            return any.to_any(value)
        
        result = any.to_any(None)
        if self.type_ == "char":
            result = CORBA.Any(CORBA.TypeCode(CORBA.CharSeq), str(value))
        elif self.type_ == "octet":
            result = CORBA.Any(CORBA.TypeCode(CORBA.OctetSeq), str(value))
        else:
            expectedType = getTypeMap(self.type_)
            expectedTypeCode = tcInternal.createTypeCode((tcInternal.tv_sequence, expectedType._d, 0))
            # Coerce the value into the correct simple type,  may thrown a TypeError if the
            # type cannot be coerced...let this propagate up in the hopes that it will get
            # logged somewhere
            result = CORBA.Any(expectedTypeCode, [to_pyvalue(item, self.type_) for item in value])
        
        return result

class struct_property(_property):
    def __init__(self, 
                 id_,
                 structdef,
                 name=None, 
                 mode="readwrite", 
                 configurationkind=("configure",),
                 description=None, 
                 fget=None, 
                 fset=None, 
                 fval=None):
        """Construct a property of type struct.  
        
        @param structdef    the structure definition, which is any class with simple_property attributes
        """

        # struct properties have been extended to support multiple kinds,
        # similar to simple and simplesequence. For backwards compatibility,
        # convert string values into tuples.
        if isinstance(configurationkind, str):
            configurationkind = (configurationkind,)
        _property.__init__(self, id_, None, name, None, mode, "external", configurationkind, description, fget, fset, fval)
                             
        if type(structdef) is types.ClassType:
            raise ValueError("structdef must be a new-style python class (i.e. inherits from object)")
        self.structdef = structdef
        self.fields = {} # Map field id's to attribute names
        for name, attr in self.structdef.__dict__.items():
            if type(attr) is simple_property:
                self.fields[attr.id_] = (name, attr)
    
    def rebind(self, fget=None, fset=None, fval=None):
        return struct_property(self.id_, 
                               self.structdef, 
                               self.name, 
                               self.mode,
                               self.kinds, 
                               self.__doc__,
                               fget, 
                               fset, 
                               fval)
                               
    def toXML(self, level=0, version="2.2.2"):
        struct = ossie.parsers.prf.struct(id_=self.id_, 
                                          name=self.name, 
                                          mode=self.mode,
                                          description=self.__doc__)

        for kind in self.kinds:
            struct.add_configurationkind(ossie.parsers.prf.configurationKind(kind))

        for name, attr in self.structdef.__dict__.items():
            if type(attr) is simple_property: 
                simp = ossie.parsers.prf.simple(id_=attr.id_, 
                                                type_=attr.type_,
                                                name=attr.name, 
                                                description=attr.__doc__,
                                                value=to_xmlvalue(attr.defvalue, attr.type_),
                                                units=attr.units)
                struct.add_simple(simp)


        xml = StringIO.StringIO()
        struct.export(xml, level)
        return xml.getvalue()

    def compareValues(self, oldValue, newValue):
        if newValue != None:
            new_member_values = set([x[1] for x in struct_values(newValue)])
        else:
            new_member_values = None
            
        if oldValue != None:
            old_member_values = set([x[1] for x in struct_values(oldValue)])
        else:
            old_member_values = None

        return (new_member_values != old_member_values)
        
    def initialize(self, obj):
        # Create an initial object
        structval = self.structdef()
        # Initialize all of the simples in the struct
        for name, attr in self.structdef.__dict__.items():
            if type(attr) is simple_property: 
                attr.initialize(structval)
        # Set the default value
        self.set(obj, structval)

    def _toAny(self, value):
        return struct_to_any(value)

    def _fromAny(self, value):
        return struct_from_any(value, self.structdef)


class structseq_property(_sequence_property):
    def __init__(self, 
                 id_,
                 structdef,
                 name=None,
                 defvalue=None,
                 mode="readwrite", 
                 configurationkind=("configure",),
                 description=None, 
                 fget=None, 
                 fset=None, 
                 fval=None):
        # structsequence properties have been extended to support multiple
        # kinds, similar to simple and simplesequence. For backwards
        # compatibility, convert string values into tuples.
        if isinstance(configurationkind, str):
            configurationkind = (configurationkind,)
        _sequence_property.__init__(self, id_, None, name, None, mode, "external", configurationkind, description, fget, fset, fval)
        
        if type(structdef) is types.ClassType:
            raise ValueError("structdef must be a new-style python class (i.e. inherits from object)")
        self.structdef = structdef
        self.fields = {} # Map field id's to attribute names
        for name, attr in self.structdef.__dict__.items():
            if type(attr) is simple_property:
                self.fields[attr.id_] = (name, attr)                
        self.defvalue = defvalue

    # Internal implementation
    def __get__(self, obj, objtype=None):
        """Returns the current value of this property as a python type"""
        if obj is None:
            return self
        return self.get(obj)

    def __getattribute__(self, name):
        return object.__getattribute__(self,name)
        
    def rebind(self, fget=None, fset=None, fval=None):
        return structseq_property(self.id_,
                                  self.structdef,
                                  self.name,
                                  self.defvalue,
                                  self.mode,
                                  self.kinds,
                                  self.__doc__,
                                  fget,
                                  fset,
                                  fval)
                               
    def toXML(self, level=0, version="2.2.2"):
        structseq = ossie.parsers.prf.structSequence(id_=self.id_,
                                                     name=self.name,
                                                     mode=self.mode,
                                                     description=self.__doc__)

        for kind in self.kinds:
            structseq.add_configurationkind(ossie.parsers.prf.configurationKind(kind))

        struct = ossie.parsers.prf.struct(id_="")
        for name, attr in self.structdef.__dict__.items():
            if type(attr) is simple_property: 
                simp = ossie.parsers.prf.simple(id_=attr.id_, 
                                                type_=attr.type_,
                                                name=attr.name, 
                                                description=attr.__doc__,
                                                units=attr.units)
                struct.add_simple(simp)
        structseq.set_struct(struct)

        if self.defvalue:
            for v in self.defvalue:
                structval = ossie.parsers.prf.structValue()
                for name, attr in self.structdef.__dict__.items():
                    if type(attr) is simple_property:
                        id_=attr.id_
                        value = to_xmlvalue(attr.get(v), attr.type_)
                        structval.add_simpleref(ossie.parsers.prf.simpleRef(id_, value))
                structseq.add_structvalue(structval)

        xml = StringIO.StringIO()
        structseq.export(xml, level)
        return xml.getvalue()
        
    def initialize(self, obj):
        # Set the default value
        self.set(obj, self.defvalue)

    def _itemFromAny(self, value):
        return struct_from_any(value, self.structdef)

    def _itemToAny(self, value):
        return struct_to_any(value)

    def _fromAny(self, value):
        return [struct_from_props(v, self.structdef) for v in any.from_any(value)]

    def _toAny(self, value):
        if value is None:
            return any.to_any(value)
        # Get the struct field format once for all items
        fields = struct_fields(self.structdef)
        result = [struct_to_any(item, fields) for item in value]
        return CORBA.Any(CORBA._tc_AnySeq, result)
    
    def compareValues(self, oldValue, newValue):
        if newValue != None:
            new_member_values = []
            for strct in newValue:
                new_member_values.append(set([x[1] for x in struct_values(strct)]))
        else:
            new_member_values = None
        
        if oldValue != None:
            old_member_values = []
            for strct in oldValue:
                old_member_values.append(set([x[1] for x in struct_values(strct)]))
        else:
            old_member_values = None

        return (new_member_values != old_member_values)

def rebind(property_, fget=None, fset=None, fval=None):
    """Rebinds a properties setters/getters.  This is useful
    to add behavior to a property produced by the code
    generators.

    Example usage:

      File: your_component_base.py
        class GeneratedClass(Resource):
           # This is the class produced by the generator, you 
           # should not modify it
           ...
           prop1 = simple_property(....)
           prop2 = simple_property(....)

      File: your_component.py
        class UserClass(GeneratedClass):
           # This is the class where you implement your logic
           ...
           prop1 = rebind(GeneratedClass.prop1, fset=set_prop1, fget=set)
    """
    return property_.rebind(fget=fget, fset=fset, fval=fval)

class PropertyStorage:
    """This is an internal class used by Resource."""
    
    def __init__(self, resource, propertydefs=(), execparams=()):
        self.__properties = {} # This is a list of unbound properties from the resource class
        self.__name_map = {}
        self.__execparams = execparams
        self.__resource = resource
        self.__propertyChangeEvent = None
        self._loadProperties()
        self._loadTuples(propertydefs)
        self._changeListeners = {}


    _ID_IDX = 0
    _NAME_IDX = 1
    _TYPE_IDX = 2
    _MODE_IDX = 3
    _VALUE_IDX = 4
    _UNITS_IDX = 5
    _ACTION_IDX = 6
    _KIND_IDX = 7
    def _loadTuples(self, propertydefs):
        """For backwards compatibility"""
        for propdef in propertydefs:
            propid = propdef[self._ID_IDX]
            if propdef[self._NAME_IDX] == None:
                propname = propid
            else:
                propname = propdef[self._NAME_IDX]
            value = propdef[self._VALUE_IDX]
            # use the value to determine if it's simple or simplesequence
            # the tuple format cannot be used for struct or structsequence
            # properties.  The tuple format may be removed in later releases
            property = None
            if type(value) in (tuple, list):
                property = simpleseq_property(id_ = propdef[self._ID_IDX],
                                              type_ = propdef[self._TYPE_IDX],
                                              name = propdef[self._NAME_IDX],
                                              defvalue = propdef[self._VALUE_IDX],
                                              units = propdef[self._UNITS_IDX],
                                              mode = propdef[self._MODE_IDX],
                                              action = propdef[self._ACTION_IDX],
                                              kinds = propdef[self._KIND_IDX])
            else:
                property = simple_property(id_ = propdef[self._ID_IDX],
                                           type_ = propdef[self._TYPE_IDX],
                                           name = propdef[self._NAME_IDX],
                                           defvalue = propdef[self._VALUE_IDX],
                                           units = propdef[self._UNITS_IDX],
                                           mode = propdef[self._MODE_IDX],
                                           action = propdef[self._ACTION_IDX],
                                           kinds = propdef[self._KIND_IDX])
            if property != None:
                self._addProperty(self.__resource, property)

    def _loadProperties(self):
        for name in dir(type(self.__resource)):
            attr = getattr(type(self.__resource), name)
            if isinstance(attr, _property):
                self._addProperty(self.__resource, attr)

    def _addProperty(self, obj, property):
        property.sendPropertyChangeEvent = self.__propertyChangeEvent
        # Don't add things if they are already defined
        id_ = str(property.id_)
        name_ = str(property.name)
        if self.__properties.has_key(id_):
            raise KeyError("Duplicate Property ID %s found", id_)
        if self.__name_map.has_key(name_):
            raise KeyError("Duplicate Property Name found")

        self.__properties[id_] = property
        if (property.name != None):
            self.__name_map[name_] = id_

    def __getitem__(self, key):
        prop = None
        try:
            # First lookup by propid
            prop = self.__properties[key]
        except KeyError:
            # Now try propname 
            prop = self.__properties[self.getPropId(key)]
       
        return prop.get(self.__resource)

    def __setitem__(self, key, value):
        # Don't let people add new items to the dictionary
        prop = None
        if self.__properties.has_key(key):
            prop = self.__properties[key]
        elif self.__properties.has_key(self.getPropId(key)):
            prop = self.__properties[self.getPropId(key)]
        else:
            raise KeyError
       
        prop.set(self.__resource, value)
    
    def query(self, propid):
        prop = None
        id_, operator = self.splitId(propid)
        # First lookup by propid
        prop = self.__properties[id_]
        return prop.query(self.__resource, operator)


    def configure(self, propid, value):
        prop = None
        id_, operator = self.splitId(propid)
        # First lookup by propid
        prop = self.__properties[id_]
        oldvalue = prop.get(self.__resource)
        prop.configure(self.__resource, value, operator)
        newvalue = prop.get(self.__resource)
        for callback, filter in self._changeListeners.items():
            if filter and not id_ in filter:
                continue
            callback(id_, oldvalue, newvalue)
        
    def splitId(self, propid):
        """Split a property ID into a base ID and it's operators.  Currently,
        only sequence operators are defined and supported.
        
        >>> splitId("abcd")
        ("abcd", None)
        
        >>> splitId("DCE:416f87a6-534b-4f9a-a754-ba81a954033f")
        ("DCE:416f87a6-534b-4f9a-a754-ba81a954033f", None)
        
        >>> splitId("DCE:416f87a6-534b-4f9a-a754-ba81a954033f[]")
        ("DCE:416f87a6-534b-4f9a-a754-ba81a954033f[]", "[]")
        
        >>> splitId("DCE:416f87a6-534b-4f9a-a754-ba81a954033f[*]")
        ("DCE:416f87a6-534b-4f9a-a754-ba81a954033f[]", "[*]")
        
        >>> splitId("DCE:416f87a6-534b-4f9a-a754-ba81a954033f[3,4,5]")
        ("DCE:416f87a6-534b-4f9a-a754-ba81a954033f[]", "[3,4,5]")
        
        >>> splitId("abcd[]efg")
        ("abcd[]efg", None)
        
        >>> splitId("abcd[]efg[@]")
        ("abcd[]efg[]", "[@]")
        
        >>> splitId("abcd]")
        ("abcd]", None)
        """
        id_, operator = propid, None
        if propid[-1] == "]":
            try:
                leftbracket_idx = propid.rindex("[")
                id_ = propid[:leftbracket_idx] + "[]"
                operator = propid[leftbracket_idx:]
            except ValueError:
                id_ = propid
        return id_, operator
        


    def __str__(self):
        return str(self.__properties)

    def __contains__(self, key):
        result = False
        # First lookup by propid
        id_, operator = self.splitId(key)
        if self.__properties.has_key(id_):
            return True
        else:
            try:
                id_ = self.getPropId(key)
            except KeyError:
                return False
            else:
                return self.__properties.has_key(id_)
       
    def keys(self):
        return self.__properties.keys()

    def values(self):
        return self.__properties.values()

    def items(self):
        return self.__properties.items()

    def has_key(self, key):
        return self.__contains__(key)

    def has_id(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties.has_key(id_)

    def getPropName(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties[id_].name

    def getPropId(self, propname):
        return self.__name_map[propname]

    def getPropDef(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties[id_]


    def isValidPropId(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties.has_key(id_)

    # Helper functions that implement configure/query logic per D.4.1.1.6
    def isQueryable(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties[id_].isQueryable()
    
    # Make helper functions for command truth testing inquiries
    def isConfigurable(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties[id_].isConfigurable()

    def isAllocatable(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties[id_].isAllocatable()

    def isWritable(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties[id_].isWritable()

    def isReadable(self, propid):
        id_, operator = self.splitId(propid)
        return self.__properties[id_].isReadable()
    
    def isSendEventChange(self, propid):
        id_, operator = self.splitId(propid)
        if self.__properties.has_key(id_):
            return self.__properties[id_].isSendEventChange()
        for prop_id in self.__properties:
            if type(self.__properties[prop_id]) == struct_property:
                if self.__properties[prop_id].fields.has_key(id_):
                    return self.__properties[prop_id].isSendEventChange()
        return False

    def reset(self):
        self.initialize()

    def initialize(self):
        for prop in self.__properties.values():
            prop.initialize(self.__resource)

        for propid, value in self.__execparams.items():
            if self.has_key(propid):
                # Since execparams are always strings and must be simple,
                # convert them to the right type
                self.__setitem__(propid, self._convertType(value, propid))

    def _convertType(self, value, propid):
        typename = self.__properties[propid].type_
        if value == None:
            return None
        if typename == "boolean":
            return {"TRUE": True, "FALSE": False}[value.strip().upper()]
        elif typename in ("short", "long", "ulong", "ushort"):
            return int(value)
        elif typename == "double":
            return float(value)
        elif typename == "float":
            return float(value)
        else:
            return str(value)

    def addChangeListener(self, callback, filter=None):
        self._changeListeners[callback] = filter

    def removeChangeListener(self, callback):
        del self._changeListeners[callback]
    
    def setPropertyChangeEvent(self, callback):
        self.__propertyChangeEvent = callback
        for prop_id in self.__properties:
            self.__properties[prop_id].sendPropertyChangeEvent = self.__propertyChangeEvent
            if type(self.__properties[prop_id]) == struct_property:
                for name, attr in self.__properties[prop_id].structdef.__dict__.items():
                    if type(attr) is simple_property:
                        attr.sendPropertyChangeEvent = self.__propertyChangeEvent


    def removePropertyChangeEvent(self):
        self.__propertyChangeEvent = None
        for prop_id in self.__properties:
            self.__properties[prop_id].sendPropertyChangeEvent = self.__propertyChangeEvent
