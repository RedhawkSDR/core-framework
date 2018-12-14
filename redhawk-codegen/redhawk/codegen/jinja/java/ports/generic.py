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

from redhawk.codegen.lang import java
from redhawk.codegen.jinja.ports import PortFactory
from redhawk.codegen.jinja.java import JavaTemplate

from generator import JavaPortGenerator

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

_baseMap = {
    CORBA.tk_short:     java.Types.SHORT,
    CORBA.tk_long:      java.Types.INT,
    CORBA.tk_ushort:    java.Types.SHORT,
    CORBA.tk_ulong:     java.Types.INT,
    CORBA.tk_float:     java.Types.FLOAT,
    CORBA.tk_double:    java.Types.DOUBLE,
    CORBA.tk_boolean:   java.Types.BOOLEAN,
    CORBA.tk_char:      java.Types.CHAR,
    CORBA.tk_octet:     java.Types.BYTE,
    CORBA.tk_longlong:  java.Types.LONG,
    CORBA.tk_ulonglong: java.Types.LONG
}

def packageName(scopedName):
    # Identifier is always the last element of the scoped name; the namespace
    # is everything leading up to it.
    identifier = scopedName[-1]
    namespace = scopedName[:-1]
    # Assume the first element of the namespace (if any) is a module, and any
    # subsequenct elements are interfaces; this is not a safe assumption in the
    # general sense, but is true within REDHAWK.
    namespace = namespace[:1] + [ns+'Package' for ns in namespace[1:]]
    return '.'.join(namespace+[identifier])

def baseType(typeobj):
    kind = typeobj.kind()
    if kind in _baseMap:
        return _baseMap[kind]
    elif kind == CORBA.tk_void:
        return 'void'
    elif kind == CORBA.tk_string:
        return 'String'
    elif kind == CORBA.tk_any:
        return 'org.omg.CORBA.Any'
    elif kind == CORBA.tk_sequence:
        return baseType(typeobj.sequenceType()) + '[]'
    elif kind == CORBA.tk_alias:
        return baseType(typeobj.aliasType())
    else:
        name = packageName(typeobj.scopedName())
        if name.startswith('CORBA'):
            return 'org.omg.'+name
        else:
            return name

def outType(typeobj):
    kind = typeobj.kind()
    if kind in _baseMap:
        name = "org.omg.CORBA." + _baseMap[kind].capitalize()
    elif kind == CORBA.tk_alias:
        name = packageName(typeobj.scopedName())
    elif kind == CORBA.tk_string:
        name = "org.omg.CORBA.String"
    else:
        name = baseType(typeobj)
    return name + 'Holder'

def paramType(param):
    if param.direction == 'in':
        return baseType(param.paramType)
    else:
        return outType(param.paramType)

def defaultValue(typeobj):
    kind = typeobj.kind()
    if kind == CORBA.tk_alias:
        return defaultValue(typeobj.aliasType())
    elif kind == CORBA.tk_objref:
        return java.NULL
    elif kind == CORBA.tk_struct:
        # The default constructor for CORBA structs sets all object-type fields
        # to null, rather than a default value; the user will need to fill them
        # in to avoid a CORBA.BAD_PARAM, but returning null for a struct is
        # also unsafe
        return 'new %s()' % baseType(typeobj)
    elif kind == CORBA.tk_sequence:
        return 'new %s[0]' % baseType(typeobj.sequenceType())
    elif kind == CORBA.tk_enum:
        value = typeobj.enumValues()[0].identifier()
        return '%s.%s' % (baseType(typeobj), value)
    else:
        return java.defaultValue(baseType(typeobj))

class GenericPortFactory(PortFactory):
    def match(cls, port):
        return True

    def generator(cls, port):
        if port.isProvides():
            return GenericProvidesPortGenerator(port)
        else:
            return GenericUsesPortGenerator(port)

class GenericPortGenerator(JavaPortGenerator):
    def loader(self):
        return jinja2.PackageLoader(__package__)

class GenericProvidesPortGenerator(GenericPortGenerator):
    def _implementation(self):
        return JavaTemplate('generic.provides.java')

    def _ctorArgs(self, name):
        return ('this', java.stringLiteral(name))

    def operations(self):
        for op in self.idl.operations():
            yield {'name': op.name,
                   'arglist': ', '.join('%s %s' % (paramType(p), p.name) for p in op.params),
                   'argnames': [p.name for p in op.params],
                   'throws': ', '.join(baseType(r) for r in op.raises),
                   'defaultval': defaultValue(op.returnType),
                   'returns': baseType(op.returnType)}
        for attr in self.idl.attributes():
            yield {'name': attr.name,
                   'arglist': '',
                   'argnames': tuple(),
                   'defaultval': defaultValue(attr.attrType),
                   'returns': baseType(attr.attrType)}
            if not attr.readonly:
                yield {'name': attr.name,
                       'arglist': baseType(attr.attrType)+ ' data',
                       'argnames': ('data',),
                       'returns': 'void'}

class GenericUsesPortGenerator(GenericPortGenerator):
    def _implementation(self):
        return JavaTemplate('generic.uses.java')

    def _ctorArgs(self, name):
        return (java.stringLiteral(name),)

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
            _raises = [baseType(r) for r in op.raises]
            _raises.append('PortCallError')
            yield {'name': op.name,
                   'arglist': ', '.join('%s %s' % (paramType(p), p.name) for p in op.params),
                   'argnames': [p.name for p in op.params],
                   'hasout': _out,
                   'hasinout': _inout,
                   'throws': ', '.join(_raises),
                   'defaultval': defaultValue(op.returnType),
                   'returns': baseType(op.returnType)}
        for attr in self.idl.attributes():
            readwrite_attr = False
            if not attr.readonly:
                readwrite_attr = True
            yield {'name': attr.name,
                   'arglist': '',
                   'argnames': tuple(),
                   'readwrite_attr': readwrite_attr,
                   'throws': 'PortCallError',
                   'defaultval': defaultValue(attr.attrType),
                   'returns': baseType(attr.attrType)}
            if not attr.readonly:
                yield {'name': attr.name,
                       'arglist': baseType(attr.attrType)+ ' data',
                       'throws': 'PortCallError',
                       'argnames': ('data',),
                       'returns': 'void'}
