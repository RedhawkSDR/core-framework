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

from redhawk.codegen.utils import parseBoolean
from ossie.utils.prop_helpers import parseComplexString
from ossie import properties

from idl import CorbaTypes

TRUE = 'true'
FALSE = 'false'

_reservedKeywords = set(("and", "and_eq", "alignas", "alignof", "asm", "auto", "bitand",
                         "bitor", "bool", "break", "case", "catch", "char", "char16_t",
                         "char32_t", "class", "compl", "const", "constexpr", "const_cast",
                         "continue", "decltype", "default", "delete", "double",
                         "dynamic_cast", "else", "enum", "explicit", "export", "extern",
                         "false", "float", "for", "friend", "goto", "if", "inline", "int",
                         "long", "mutable", "namespace", "new", "noexcept", "not",
                         "not_eq", "nullptr", "operator", "or", "or_eq", "private",
                         "protected", "public", "register", "reinterpret_cast", "return",
                         "short", "signed", "sizeof", "static", "static_assert",
                         "static_cast", "struct", "switch", "template", "this",
                         "thread_local", "throw", "true", "try", "typedef", "typeid",
                         "typename", "union", "unsigned", "using", "virtual", "void",
                         "volatile", "wchar_t", "while", "xor", "xor_eq" ))

_typeMap = {
    CorbaTypes.OCTET:     'unsigned char',
    CorbaTypes.BOOLEAN:   'bool',
    CorbaTypes.CHAR:      'char',
    CorbaTypes.SHORT:     'short',
    CorbaTypes.USHORT:    'unsigned short',
    CorbaTypes.LONG:      'CORBA::Long',
    CorbaTypes.ULONG:     'CORBA::ULong',
    CorbaTypes.LONGLONG:  'CORBA::LongLong',
    CorbaTypes.ULONGLONG: 'CORBA::ULongLong',
    CorbaTypes.FLOAT:     'float',
    CorbaTypes.DOUBLE:    'double',
    CorbaTypes.STRING:    'std::string',
    CorbaTypes.UTCTIME:   'CF::UTCTime',
    CorbaTypes.OBJREF:    'std::string'
}

_complexTypes = [
    CorbaTypes.FLOAT,
    CorbaTypes.DOUBLE,
    CorbaTypes.CHAR,
    CorbaTypes.OCTET,
    CorbaTypes.SHORT,
    CorbaTypes.USHORT,
    CorbaTypes.BOOLEAN,
    CorbaTypes.LONG,
    CorbaTypes.ULONG,
    CorbaTypes.LONGLONG,
    CorbaTypes.ULONGLONG
]

def stringLiteral(string):
    return '"' + string + '"'

def charLiteral(string):
    return "'" + string + "'"

def complexType(typename):
    return 'std::complex<%s>' % (cppType(typename),)

def cppType(typename, complex=False):
    """
    Returns the C++ type for the given CORBA type.
    """
    if complex:
        if not typename in _complexTypes:
            raise TypeError("Cannot create complex '"+typename+"' type")
        return complexType(typename)
    else:
        return _typeMap[typename]

def identifier(name):
    """
    Returns a valid C++ identifer based on the given name.
    """
    # Replace invalid characters with '_'
    _name = ''
    for ch in name:
        if ch.isalnum():
            _name += ch
        else:
            _name += '_'
    name = _name
    # Cannot start with a digit
    if name[0].isdigit():
        name = '_' + name
    # Cannot clash with a C++ keyword
    if name in _reservedKeywords:
        name += '_'
    return name

def literal(value, typename, complex=False):

    if complex:
        # Parse real and imaginary components
        real, imag = parseComplexString(value, typename)
        # Convert real and imaginary into type-specific literals
        real, imag = (literal(str(x), typename) for x in (real, imag))
        return "%s(%s,%s)" % (cppType(typename, complex), real, imag)
    elif typename == CorbaTypes.STRING:
        if type(value) == list:
            return
        else:
            return stringLiteral(value)
    elif typename == CorbaTypes.BOOLEAN:
        if parseBoolean(value):
            return TRUE
        else:
            return FALSE
    elif typename in (CorbaTypes.LONGLONG, CorbaTypes.ULONGLONG):
        # Explicitly mark the literal as a 'long long' for 32-bit systems
        if not isinstance(value, list):
            return value + 'LL'
    elif typename == CorbaTypes.CHAR:
        return charLiteral(value)
    else:
        return value

def sequenceType(typename, complex=False):
    sequence = 'std::vector<%s>' % (cppType(typename, complex),)
    # Ensure that templates-of-templates have space between closing tokens
    return sequence.replace('>>', '> >')

def insert(name, typename, complex=False):
    if typename in (CorbaTypes.CHAR, CorbaTypes.OCTET) and not complex:
        return 'CORBA::Any::from_%s(%s)' % (typename, name)
    else:
        return name

def extract(name, typename, complex=False):
    if typename in (CorbaTypes.CHAR, CorbaTypes.OCTET) and not complex:
        return 'CORBA::Any::to_%s(%s)' % (typename, name)
    else:
        return name

def idlNamespace(idl):
    # Never include the omg.org from COS modules in C++ namespaces
    return idl.namespace().replace('omg.org/', '').replace('/', '::')

def idlClass(idl):
    interface = idl.interface()
    if idl.namespace():
        interface = idlNamespace(idl) + '::' + interface
    return interface

def idlHeader(idl):
    if idl.namespace().startswith('omg.org/'):
        # By convention, COS headers use '.hh'
        extension = '.hh'
    else:
        # By convention, REDHAWK headers use '.h'
        extension = '.h'
    # Assume the relative path for the header is the last two elements of the
    # full IDL path
    idlpath = '/'.join(idl.idl().fullpath.split('/')[-2:])
    header = idlpath.replace('.idl', extension)
    return '<'+header+'>'
