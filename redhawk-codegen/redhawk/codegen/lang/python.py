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

from idl import CorbaTypes
from ossie.utils.prop_helpers import parseComplexString

_reservedKeywords = set(("and", "as", "assert", "break", "class", "continue", "def",
                         "del", "elif", "else", "except", "exec", "finally", "for",
                         "from", "global", "if", "import", "in", "is", "lambda", "not",
                         "or", "pass", "print", "raise", "return", "try", "while",
                         "with", "yield", "None"))

def stringLiteral(value):
    return '"%s"' % (value,)

def charLiteral(value):
    return "'"+value+"'"

def boolLiteral(value):
    if isinstance(value, basestring):
        value = stringToBoolean(value)
    else:
        value = bool(value)
    return str(value)

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

def floatLiteral(value):
    return repr(float(value))

def UTCTimeLiteral(value):
    return '"%s"' % (value,)

def intLiteral(value):
    value, base = checkValue(value)
    return repr(int(value,base))

def longLiteral(value):
    value, base = checkValue(value)
    return repr(long(value,base))

def stringToBoolean(value):
    if value.lower() == 'true':
        return True
    elif value.lower() == 'false':
        return False
    else:
        raise ValueError, 'Invalid boolean literal: "%s"' % value

_typeMap = {
    CorbaTypes.BOOLEAN:   boolLiteral,
    CorbaTypes.CHAR:      charLiteral,
    CorbaTypes.OCTET:     intLiteral,
    CorbaTypes.SHORT:     intLiteral,
    CorbaTypes.USHORT:    intLiteral,
    CorbaTypes.LONG:      intLiteral,
    CorbaTypes.ULONG:     intLiteral,
    CorbaTypes.LONGLONG:  longLiteral,
    CorbaTypes.ULONGLONG: longLiteral,
    CorbaTypes.FLOAT:     floatLiteral,
    CorbaTypes.DOUBLE:    floatLiteral,
    CorbaTypes.UTCTIME:   UTCTimeLiteral,
    CorbaTypes.STRING:    stringLiteral,
    CorbaTypes.OBJREF:    stringLiteral,
}

def identifier(name):
    """
    Returns a valid Python identifer based on the given name.
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
    # Cannot clash with a Python keyword
    if name in _reservedKeywords:
        name += '_'
    return name

def defaultValue(typename):
    if typename in (CorbaTypes.STRING,CorbaTypes.OBJREF):
        return stringLiteral('')
    elif typename == CorbaTypes.CHAR:
        return charLiteral(' ')
    elif typename in (CorbaTypes.FLOAT,CorbaTypes.DOUBLE):
        return 0.0
    elif typename == CorbaTypes.BOOLEAN:
        return False
    else:
        return 0

def literal(value, typename, complex=False):
    if isinstance(value, (list,tuple)):
        return '[%s]' % ','.join(literal(v, typename, complex) for v in value)
    if typename in _typeMap:
        if complex:
            real, imag = parseComplexString(value, typename)
            value = 'complex({0},{1})'.format(real, imag)
        else:
            value = _typeMap[typename](value)
    return value

def sequenceValue(values, typename, complex=False):
    return tuple(literal(v, typename, complex) for v in values)

def idlModule(namespace):
    if namespace.startswith('omg.org/'):
        # Remove the 'org.omg/'
        namespace = namespace[8:]
        if namespace in ('CORBA', 'PortableServer'):
            # Certain core modules must be explicitly imported from omniORB to
            # avoid collisions with ORBit.
            package = 'omniORB'
        else:
            # Importing omniORB adds "omniORB/COS" to sys.path; furthermore,
            # the COS modules are not designed to be imported via "omniORB.COS"
            # due to the way omniORB handles IDL module loading.
            return namespace
    else:
        if namespace in ('CF', 'ExtendedCF'):
            package = 'ossie.cf'
        elif namespace == 'BULKIO':
            package = 'bulkio.bulkioInterfaces'
        else:
            package = 'redhawk.' + namespace.lower() + 'Interfaces'
    return '%s.%s' % (package, namespace)

def poaModule(namespace):
    return idlModule(namespace) + '__POA'

def importModule(module):
    if not '.' in module:
        return 'import ' + module
    else:
        package, module = module.rsplit('.', 1)
        return 'from %s import %s' % (package, module)
