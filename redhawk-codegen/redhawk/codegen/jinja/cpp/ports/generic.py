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

import jinja2
from omniORB import CORBA

from redhawk.codegen.lang import cpp
from redhawk.codegen.lang import idl
from redhawk.codegen.jinja.ports import PortFactory
from redhawk.codegen.jinja.cpp import CppTemplate

from generator import CppPortGenerator

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

_baseMap = {
    CORBA.tk_short:     'CORBA::Short',
    CORBA.tk_long:      'CORBA::Long',
    CORBA.tk_ushort:    'CORBA::UShort',
    CORBA.tk_ulong:     'CORBA::ULong',
    CORBA.tk_float:     'CORBA::Float',
    CORBA.tk_double:    'CORBA::Double',
    CORBA.tk_boolean:   'CORBA::Boolean',
    CORBA.tk_char:      'CORBA::Char',
    CORBA.tk_octet:     'CORBA::Octet',
    CORBA.tk_longlong:  'CORBA::LongLong',
    CORBA.tk_ulonglong: 'CORBA::ULongLong'
}

def cppName(typeobj):
    return '::'.join(typeobj.scopedName())

def newObject(typeobj):
    return 'new %s()' % baseType(typeobj)

def baseType(typeobj):
    kind = typeobj.kind()
    if kind in _baseMap:
        return _baseMap[kind]
    elif kind == CORBA.tk_void:
        return 'void'
    elif kind == CORBA.tk_string:
        return 'char*'
    elif kind == CORBA.tk_any:
        return 'CORBA::Any'

    return cppName(typeobj)

def isVariableLength(typeobj):
    # Remove any aliases
    typeobj = unaliasedType(typeobj)
    kind = typeobj.kind()
    if kind in _baseMap:
        # Basic numeric types are always fixed-length
        return False
    elif kind in (CORBA.tk_struct, CORBA.tk_union):
        # Struct and unions are variable-length if any members are
        repo_id = 'IDL:' + '/'.join(typeobj.scopedName()) + ':1.0'
        idl_type = idl.idlRepo.getIdlStruct(repo_id)
        if kind == CORBA.tk_struct:
            # Work around insufficent information in public interface by using
            # "private" interface that was added for this usage
            members = idl_type._fields
        else:
            members = idl_type.members()
        for member in members:
            if isVariableLength(member.memberType()):
                return True
        return False
    else:
        # Assume everything else is variable-length
        return True

def returnType(typeobj):
    name = baseType(typeobj)
    kind = unaliasedType(typeobj).kind()
    if kind == CORBA.tk_objref:
        return name + '_ptr'
    elif kind in (CORBA.tk_struct, CORBA.tk_union):
        if isVariableLength(typeobj):
            return name + '*'
        else:
            return name
    elif kind in (CORBA.tk_any, CORBA.tk_sequence):
        return name + '*'
    else:
        return name

def unaliasedType(typeobj):
    kind = typeobj.kind()
    if kind != CORBA.tk_alias:
        return typeobj
    else:
        return unaliasedType(typeobj.aliasType())

def temporaryType(typeobj):
    name = baseType(typeobj)
    kind = unaliasedType(typeobj).kind()
    if kind == CORBA.tk_string:
        # Use the base type even when aliased
        return 'CORBA::String_var'
    elif kind in (CORBA.tk_objref, CORBA.tk_sequence, CORBA.tk_any):
        return name + '_var'
    elif kind in (CORBA.tk_struct, CORBA.tk_union):
        if isVariableLength(typeobj):
            return name + '_var'
        else:
            return name
    else:
        return name

def temporaryValue(typeobj):
    kind = unaliasedType(typeobj).kind()
    if kind in _baseMap:
        return '0'
    elif kind == CORBA.tk_enum:
        enum_values = unaliasedType(typeobj).enumValues()
        return cppName(enum_values[0])
    elif kind == CORBA.tk_string:
        return 'CORBA::string_dup("")'
    elif kind in (CORBA.tk_sequence, CORBA.tk_any):
        return newObject(typeobj)
    elif kind in (CORBA.tk_struct, CORBA.tk_union):
        if isVariableLength(typeobj):
            return newObject(typeobj)
        else:
            # The default constructor is fine, no need to generate a temporary
            # that does the same thing
            return None
    else:
        return None

def inType(typeobj):
    kind = unaliasedType(typeobj).kind()
    if kind == CORBA.tk_string:
        # Strings are always "const char*" even when aliased because for a
        # typedef the const applies to the full type (i.e., the pointer), but
        # the contract is that the contents (char) cannot be modified
        return 'const char*'

    name = baseType(typeobj)
    if kind in _baseMap or kind == CORBA.tk_enum:
        # Basic types are passed by value
        return name
    elif kind == CORBA.tk_objref:
        return name + '_ptr'
    else:
        return 'const '+name+'&'

def outType(typeobj):
    if typeobj.kind() == CORBA.tk_string:
        # Strings (just the base type, not aliases) require special handling in
        # that the out type is not derived from the normal mapping (char*)
        return 'CORBA::String_out'

    name = baseType(typeobj)
    kind = unaliasedType(typeobj).kind()
    if kind in _baseMap or kind == CORBA.tk_enum:
        # CORBA technically has "_out" typedefs for primitive types, but for
        # consistency with prior versions just use a reference
        return name + '&'
    elif kind in (CORBA.tk_struct, CORBA.tk_union) and not isVariableLength(typeobj):
        # Since omniORB directly uses the reference type, copy that here
        return name + '&'
    else:
        return name+'_out'

def inoutType(typeobj):
    kind = unaliasedType(typeobj).kind()
    if kind == CORBA.tk_string:
        # Just for consistency with omniORB, remap all string types to the base
        # type
        return 'char*&'

    name = baseType(typeobj)
    if kind == CORBA.tk_objref:
        return name + '_ptr&'
    else:
        return name + '&'

def argumentType(argType, direction):
    if direction == 'in':
        return inType(argType)
    elif direction == 'out':
        return outType(argType)
    else:
        return inoutType(argType)

class GenericPortFactory(PortFactory):
    def match(self, port):
        return True

    def generator(self, port):
        if port.isProvides():
            return GenericProvidesPortGenerator(port)
        else:
            return GenericUsesPortGenerator(port)

class GenericPortGenerator(CppPortGenerator):
    def _ctorArgs(self, name):
        return (cpp.stringLiteral(name), 'this')

    def dependencies(self):
        return ['<boost/thread/locks.hpp>', '<ossie/Port_impl.h>', '<ossie/debug.h>', cpp.idlHeader(self.idl)]

    def loader(self):
        return jinja2.PackageLoader(__package__)

    def hasOut(self):
        for op in self.idl.operations():
            for p in op.params:
                if p.direction == 'out':
                    return True
        return False

    def hasInOut(self):
        for op in self.idl.operations():
            for p in op.params:
                if p.direction == 'inout':
                    return True
        return False

    def operations(self):
        for op in self.idl.operations():
            yield {'name': op.name,
                   'arglist': ', '.join('%s %s' % (argumentType(p.paramType,p.direction), p.name) for p in op.params),
                   'argnames': ', '.join(p.name for p in op.params),
                   'hasout': any(p.direction == 'out' for p in op.params),
                   'hasinout': any(p.direction == 'inout' for p in op.params),
                   'temporary': temporaryType(op.returnType),
                   'initializer': temporaryValue(op.returnType),
                   'returns': returnType(op.returnType)}
        #
        # for attributes of an interface...provide manipulator methods
        # 
        for attr in self.idl.attributes():
            readwrite_attr = False
            if not attr.readonly:
                readwrite_attr = True
            yield {'name': attr.name,
                   'arglist': '',
                   'argnames': '',
                   'readwrite_attr': readwrite_attr,
                   'temporary': temporaryType(attr.attrType),
                   'initializer': temporaryValue(attr.attrType),
                   'returns': returnType(attr.attrType)}
            if not attr.readonly:
                yield {'name': attr.name,
                       'arglist':   argumentType(attr.attrType,'in') + ' data',
                       'argnames': 'data',
                       'returns': 'void'}

class GenericUsesPortGenerator(GenericPortGenerator):
    def dependencies(self):
        # Uses ports use an std::vector of std::pairs to track connections, and
        # support the QueryablePort CORBA interface
        deps = ['<vector>', '<utility>', '<ossie/CF/QueryablePort.h>']
        return super(GenericUsesPortGenerator, self).dependencies() + deps

    def _declaration(self):
        return CppTemplate('generic.uses.h')

    def _implementation(self):
        return CppTemplate('generic.uses.cpp')

class GenericProvidesPortGenerator(GenericPortGenerator):
    def _declaration(self):
        return CppTemplate('generic.provides.h')

    def _implementation(self):
        return CppTemplate('generic.provides.cpp')
