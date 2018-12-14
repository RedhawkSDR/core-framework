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

from redhawk.codegen.utils import strenum, parseBoolean
from ossie.utils.prop_helpers import parseComplexString
from ossie import properties

from idl import CorbaTypes

NULL = 'null'
TRUE = 'true'
FALSE = 'false'

Types = strenum('boolean', 'char', 'byte', 'short', 'int', 'long', 'float', 'double', 'utctime')
BoxTypes = strenum('Boolean', 'Character', 'Byte', 'Short', 'Integer', 'Long', 'Float', 'Double', 'UTCTime')

_reservedKeywords = set(("abstract", "assert", "boolean", "break", "byte", "case",
                         "catch", "char", "class", "const", "continue", "default",
                         "do", "double", "else", "enum", "extends", "final", "finally",
                         "float", "for", "if", "goto", "implements", "import",
                         "instanceof", "int", "interface", "long", "native", "new",
                         "package", "private", "protected", "public", "return", "short",
                         "static", "strictfp", "super", "switch", "synchronized",
                         "this", "throw", "throws", "transient", "try", "void",
                         "volatile", "while", "true", "false", "null"))

_boxMap = {
    Types.BOOLEAN: BoxTypes.BOOLEAN,
    Types.CHAR:    BoxTypes.CHARACTER,
    Types.BYTE:    BoxTypes.BYTE,
    Types.SHORT:   BoxTypes.SHORT,
    Types.INT:     BoxTypes.INTEGER,
    Types.LONG:    BoxTypes.LONG,
    Types.FLOAT:   BoxTypes.FLOAT,
    Types.DOUBLE:  BoxTypes.DOUBLE,
    Types.UTCTIME:  BoxTypes.UTCTIME
}

_typeSize = {
    Types.CHAR:   1,
    Types.BYTE:   1,
    Types.SHORT:  2,
    Types.INT:    4,
    Types.FLOAT:  4,
    Types.LONG:   8,
    Types.DOUBLE: 8
}

def stringLiteral(string):
    return '"' + string + '"'

def charLiteral(char):
    return "'" + char + "'"

def typeSize(typename):
    return _typeSize[typename]

def boxedType(typename):
    return _boxMap.get(typename, typename)

def identifier(name):
    """
    Returns a valid Java identifer based on the given name.
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
    # Cannot clash with a Java keyword
    if name in _reservedKeywords:
        name += '_'
    return name

def translateBoolean(value):
    if parseBoolean(value):
        return TRUE
    else:
        return FALSE

def _complexLiteral(value, typename):
    memberType = properties.getMemberType(typename)
    real, imag = parseComplexString(value, memberType)
    if typename == "complexBoolean":
        real = translateBoolean(real)
        imag = translateBoolean(imag)
    elif typename == "complexFloat":
        real = str(real) + "F"
        imag = str(imag) + "F"
    elif typename in ["complexShort", "complexUShort"]:
        real = '(short)%d' % int(real)
        imag = '(short)%d' % int(imag)
    elif typename == "complexChar":
        real = charLiteral(str(real))
        imag = charLiteral(str(imag))
    elif typename == "complexOctet":
        real = '(byte)%d' % int(real)
        imag = '(byte)%d' % int(imag)

    return "new CF." + typename + "(" + str(real) +  "," + str(imag) + ")"


def checkValue(value):
    base=10
    if type(value) == str:
        _v=value.upper()
        if _v.startswith('0X') or _v.startswith('X'):
            if _v.startswith('X'): value='0'+value
            base=16
        if _v.startswith('0O') or _v.startswith('O'):
            if _v.startswith('O'): value='0'+value
            base=8
        if _v.startswith('0B') or _v.startswith('B'):
            if _v.startswith('B'): value='0'+value
            base=2
    return value, base


def literal(value, typename, complex=False):
    if complex:
        return _complexLiteral(value, typename)
    elif typename == 'CF.UTCTime':
        return stringLiteral(value)
    elif typename == 'String':
        return stringLiteral(value)
    elif typename == 'Object':
        return NULL
    elif typename in (Types.FLOAT, BoxTypes.FLOAT, Types.DOUBLE, BoxTypes.DOUBLE):
        value = repr(float(value))
        if typename in (Types.FLOAT, BoxTypes.FLOAT):
            return value + 'F'
        else:
            return value
    elif typename in (Types.LONG, BoxTypes.LONG):
        value, base = checkValue(value)
        return repr(long(value,base))
    elif typename in (Types.BOOLEAN, BoxTypes.BOOLEAN):
        return translateBoolean(value)
    elif typename in (Types.BYTE, BoxTypes.BYTE):
        value, base = checkValue(value)
        return '(byte)%d' % int(value,base)
    elif typename in (Types.SHORT, BoxTypes.SHORT):
        value, base = checkValue(value)
        return '(short)%d' % int(value,base)
    elif typename in (Types.CHAR, BoxTypes.CHARACTER):
        return charLiteral(value)
    elif typename in (Types.INT, BoxTypes.INTEGER):
        value, base = checkValue(value)
        return str(int(value,base))
    else:
        return NULL

def defaultValue(typename):
    if typename == 'String':
        return stringLiteral('')
    elif typename == 'CF.UTCTime':
        return stringLiteral('(CF.UTCTime)'+NULL)
    elif typename == 'Object':
        return NULL
    elif typename == Types.BOOLEAN:
        return FALSE
    else:
        return literal('0', typename)

def qualifiedName(name, package):
    if package:
        return package + '.' + name
    else:
        return name
