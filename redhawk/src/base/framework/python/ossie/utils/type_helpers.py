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

class OutOfRangeException(Exception): pass

class EnumValueError(ValueError):
    def __init__(self, identifier, value, enums):
        ValueError.__init__(self, "Invalid enumeration value '%s'" % (value,))
        self.id = identifier
        self.enums = enums

__INT_RANGE = {
    'octet':     (0, 255),
    'short':     (-2**15, 2**15-1),
    'ushort':    (0, 2**16-1),
    'long':      (-2**31, 2**31-1),
    'ulong':     (0, 2**32-1),
    'longlong':  (-2**63, 2**63-1),
    'ulonglong': (0, 2**64-1)
}

def _splitComplex(value):
    """
    Returns the real and imaginary parts of a value, for any of these
    types:
      - complex
      - 2-tuple
      - 2-item list
      - scalar value (imaginary is 0)
    """
    try:
        # Treat value as complex instance.
        return value.real, value.imag
    except AttributeError:
        # Value is not a Python complex.
        pass
    try:
        # Unpack 2-item tuple/list; exception due to wrong number of items
        # propagates up...
        real, imag = value
        return real, imag
    except TypeError:
        # Type doesn't support iteration (i.e., is not a sequence)
        pass
    # Assume value is a scalar.
    return value, 0

def _SIStringToNumeric(value):
    num=""
    suffix=""
    siMap = {('K','KB'):1,
             ('M','MB'):2,
             ('G','GB'):3,
             ('T','TB'):4,
             ('P','PB'):5,
             ('E','EB'):6}   
    
    for c in value:
        if c.isdigit() or c == ".":
            num += c
        if c.isalpha():
            suffix += c
    if len(num) > 0 and len(suffix) > 0:
        for suffixKey in siMap.keys():
            if suffixKey[0] == suffix:
                if "." in num:
                    return float(num) * pow(1000,siMap[suffixKey])
                else:
                    return int(num) * pow(1000,siMap[suffixKey])
            elif suffixKey[1] == suffix:
                if "." in num:
                    return float(num) * pow(1024,siMap[suffixKey])
                else:
                    return int(num) * pow(1024,siMap[suffixKey])
    return value

def checkValidValue(value, dataType):
    if isinstance(value, str) and dataType != "string":
        value = _SIStringToNumeric(value)
    if dataType in ('char', 'string'):
        if not isinstance(value, basestring):
            raise TypeError, '%s is not valid for type %s' % (type(value), dataType)
        if dataType == 'char' and len(value) != 1:
            raise TypeError, 'expected a character, but string of length %d found' % len(value)
        return value
    elif isinstance(value, basestring):
        raise TypeError, "Cannot convert string to type '%s'" % dataType
    elif dataType in ('double', 'float'):
        return float(value)
    elif dataType in ('octet', 'short', 'ushort', 'long', 'ulong', 'longlong', 'ulonglong'):
        value = int(value)
        typeMin, typeMax = __INT_RANGE[dataType]
        if value > typeMax or value < typeMin:
            raise OutOfRangeException, '%d is out of range for type %s [%d <= x <= %d]' % (value, dataType, typeMin, typeMax)
        return value
    elif dataType == 'boolean':
        return bool(value)
    #this is used for structs
    #datatype is a list of (ID, propType) pairs, value is a dict mapping an ID to a value for validation
    elif isinstance(dataType, list):
        for memberID in value:
            if memberID not in [id for id,propType in dataType]:
                raise TypeError, '"' + str(memberID) + '" is not a member of this struct'
        for memberID in value:
            if value[memberID] != None:
                for id, propType in dataType:
                    if id == memberID:
                        checkValidValue(value[memberID], propType)
                    break
        return value
    else:
        raise TypeError, str(type(value)) + ' is not a valid type for ' + dataType

def checkValidDataSet(dataSet, dataType):
    value = [checkValidValue(v, dataType) for v in dataSet]
    if dataType == 'char':
        # Char sequence is serialized as a string in CORBA.
        value = ''.join(value)
    elif dataType == 'octet':
        # Octet sequence, likewise.
        value = ''.join(chr(v) for v in value)
    return value
