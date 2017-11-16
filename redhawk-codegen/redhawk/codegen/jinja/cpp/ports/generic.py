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
from redhawk.codegen.lang.idl import IDLInterface, IDLStruct
from redhawk.codegen.jinja.ports import PortFactory
from redhawk.codegen.jinja.cpp import CppTemplate

from generator import CppPortGenerator

from ossie.utils.sca.importIDL import SequenceType, BaseType

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

def baseType(typeobj, direction=None):
    kind = typeobj.kind()
    if kind in _baseMap:
        return _baseMap[kind]
    elif kind == CORBA.tk_void:
        return 'void'
    elif kind == CORBA.tk_string:
        if direction == 'out':
            return 'CORBA::String_out'
        else:
            return 'char*'
    elif kind == CORBA.tk_any:
        return 'CORBA::Any'
    elif kind == CORBA.tk_alias and \
        typeobj.aliasType().kind() == CORBA.tk_string: 
        return 'char*' 
    
    name = '::'.join(typeobj.scopedName())
    if kind == CORBA.tk_objref:
        if direction == 'out':
            return name + '_out'
        else:
            return name + '_ptr'
    elif kind == CORBA.tk_alias and isinstance(typeobj.aliasType(), SequenceType):
        if direction == 'out':
            return name + '_out'
        else:
            return name
    else:
        return name 

def doesStructIncludeInterface(scopedName):
    repo_id = 'IDL:'
    if len(scopedName) == 1:
        repo_id += scopedName[0]
    else:
        for name in scopedName:
            repo_id += name + '/'
        repo_id = repo_id[:-1]
    repo_id += ':1.0'
    idl = IDLStruct(repo_id)
    _members = idl.members()
    for member_key in _members:
        if _members[member_key] in ['objref', 'alias', 'struct']:
            return True
    return False

def baseReturnType(typeobj):
    kind = typeobj.kind()
    if kind in _baseMap:
        return _baseMap[kind]
    elif kind == CORBA.tk_void:
        return 'void'
    elif kind == CORBA.tk_string:
        return 'char*'
    elif kind == CORBA.tk_any:
        return 'CORBA::Any'
    
    name = '::'.join(typeobj.scopedName())
    if kind == CORBA.tk_objref:
        return name + '_ptr'
    elif kind == CORBA.tk_struct:
        if doesStructIncludeInterface(typeobj.scopedName()):
            return name + '*'
        else:
            return name
    elif kind == CORBA.tk_alias and isinstance(typeobj.aliasType(), SequenceType):
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
    kind = typeobj.kind()
    if kind in _baseMap:
        return _baseMap[kind]
    elif kind == CORBA.tk_void:
        return 'void'
    elif kind == CORBA.tk_string:
        return 'CORBA::String_var'
    elif kind == CORBA.tk_any:
        return 'CORBA::Any'
    
    name = '::'.join(typeobj.scopedName())
    kind = unaliasedType(typeobj).kind()
    if kind in (CORBA.tk_objref, CORBA.tk_sequence) or ((kind == CORBA.tk_struct) and doesStructIncludeInterface(typeobj.scopedName())):
        return name + '_var'
    else:
        return name

def temporaryValue(typeobj):
    kind = unaliasedType(typeobj).kind()
    if kind in _baseMap:
        return '0'
    elif kind == CORBA.tk_enum:
        return '::'.join(typeobj.enumValues()[0].scopedName())
    elif kind == CORBA.tk_string:
        return 'CORBA::string_dup("")'
    elif kind == CORBA.tk_sequence or ((kind == CORBA.tk_struct) and doesStructIncludeInterface(typeobj.scopedName())):
        return 'new %s()' % '::'.join(typeobj.scopedName())
    elif ((kind == CORBA.tk_struct) and not doesStructIncludeInterface(typeobj.scopedName())):
        return '%s()' % '::'.join(typeobj.scopedName())
    else:
        return None

def passByValue(argType,direction):
    if direction == 'in':
        if not passConst(argType,direction):
            return True
        else:
            kind = argType.kind()
            if kind == CORBA.tk_string or \
               (kind == CORBA.tk_alias and argType.aliasType().kind() == CORBA.tk_string):
                return True
            else:
                return False
    else:
        kind = argType.kind()
        if kind == CORBA.tk_alias and isinstance(argType.aliasType(), SequenceType):
            if direction == 'out':
                return True
            elif direction == 'inout':
                return False
        elif kind == CORBA.tk_string:
            if direction == 'out':
                return True
            elif direction == 'inout':
                return False
        elif kind == CORBA.tk_objref:
            return True
        else:
            return False

def passConst(argType,direction):
    if direction == 'in':
        kind = argType.kind()
        if kind in _baseMap or \
            (kind == CORBA.tk_alias and isinstance(argType.aliasType(), BaseType) and argType.aliasType().kind() != CORBA.tk_string) or \
            kind == CORBA.tk_objref or kind == CORBA.tk_enum:
            return False 
        else:
            return True
    else:
        return False 

def argumentType(argType, direction):
    name = baseType(argType,direction)
    if not direction == 'in':
        # sequence is special case because arg is different in C++ than the IDL
        if argType == 'sequence':
            return name + '_out'
    if passConst(argType,direction):
        name = 'const '+name
    if not passByValue(argType,direction):
        name = name + '&'
    return name

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
            _out = False
            for p in op.params:
                if p.direction == 'out':
                    _out = True
                    break
            _inout = False
            for p in op.params:
                if p.direction == 'inout':
                    _inout = True
                    break
            yield {'name': op.name,
                   'arglist': ', '.join('%s %s' % (argumentType(p.paramType,p.direction), p.name) for p in op.params),
                   'argnames': ', '.join(p.name for p in op.params),
                   'hasout': _out,
                   'hasinout': _inout,
                   'temporary': temporaryType(op.returnType),
                   'initializer': temporaryValue(op.returnType),
                   'returns': baseReturnType(op.returnType)}
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
                   'returns': baseReturnType(attr.attrType)}
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
