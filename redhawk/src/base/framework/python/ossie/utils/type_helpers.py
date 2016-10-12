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

def checkValidValue(value, dataType):
    if dataType == 'char':
        return str(value)[0]
    elif dataType == 'double':
        validDataTypes = [int, long, float]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type double: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        return value
    elif dataType == 'float':
        validDataTypes = [int, long, float]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type float: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        return value
    elif dataType == 'long':
        validDataTypes = [int, long]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type long: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        if value > 2147483647 or value < -2147483648:
            raise OutOfRangeException, '"'+str(value)+'"'+ ' is out of range for type long(signed 32 bit) [-2147483648 <= x <= 2147483647]'
        return value
    elif dataType == 'longlong':
        validDataTypes = [int, long]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type longlong: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        if value > 9223372036854775807L or value < -9223372036854775808L:
            raise OutOfRangeException, '"'+str(value)+'"'+ ' is out of range for type longlong(signed 64 bit) [-9223372036854775808 <= x <= 9223372036854775807]'
        return value
    elif dataType == 'octet':
        if type(value) == int and (value > 255 or value < 0):
            raise OutOfRangeException, '"'+str(value)+'"'+ ' is out of range for type octet(unsigned 8 bit) [0 <= x <= 255]'
        if type(value) != int:
            raise TypeError, '"'+str(value)+'"' + ' can not be interpreted as type octet: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        return value
    elif dataType == 'short':
        validDataTypes = [int, long]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type short: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        if value > 32767 or value < -32768:
            raise OutOfRangeException, '"'+str(value)+'"'+ ' is out of range for type short(signed 16 bit) [-32768 <= x <= 32767]'
        return value
    elif dataType == 'ulong':
        validDataTypes = [int, long]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type ulong: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        if value > 4294967295 or value < 0:
            raise OutOfRangeException, '"'+str(value)+'"'+ ' is out of range for type ulong(unsigned 32 bit) [0 <= x <= 4294967295]'
        return value
    elif dataType == 'ulonglong':
        validDataTypes = [int, long]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type ulonglong: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        if value > 18446744073709551615 or value < 0:
            raise OutOfRangeException, '"'+str(value)+'"' + ' is out of range for type ulonglong(unsigned 64 bit) [0 <= x <= 18446744073709551615]'
        return value
    elif dataType == 'ushort':
        validDataTypes = [int, long]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type ushort: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        if value > 65535 or value < 0:
            raise OutOfRangeException, '"'+str(value)+'"'+ ' is out of range for type ushort(unsigned 16 bit) [0 <= x <= 65536]'
        return value
    elif dataType == 'string':
        validDataTypes = [str]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type string: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        return value
    elif dataType == 'boolean':
        validDataTypes = [bool]
        if type(value) not in validDataTypes:
            raise TypeError, '"'+str(value)+'"' + ' is invalid for type boolean: ' + 'type(' + str(value) + ') = ' + ('<'+str(type(value))[7:-2]+'>')
        return value
    #this is used for structs
    #datatype is a list of (ID, propType) pairs, value is a dict mapping an ID to a value for validation
    elif type(dataType) == list:
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
        raise TypeError, str(type) + ' is not a valid type'

def checkValidDataSet(dataSet, dataType):
    if dataType == 'char':
        if type(dataSet) != str:
            raise TypeError, 'dataSet is invalid for type char: ' + 'type(' + str(dataSet) + ') = ' + ('<'+str(type(dataSet))[7:-2]+'>') + ', should be str'
        return dataSet
    elif dataType == 'octet':
        if type(dataSet) != str:
            raise TypeError, 'dataSet is invalid for type octet: ' + 'type(' + str(dataSet) + ') = ' + ('<'+str(type(dataSet))[7:-2]+'>') + ', should be str'
        return dataSet
    else:
        if type(dataSet) != list:
            raise TypeError, 'dataSet must be a list of values unless dataType is octet or char'
        else:
            for value in dataSet:
                try:
                    checkValidValue(value, dataType)
                except TypeError, e1:
                    raise TypeError, 'dataSet[' + str(dataSet.index(value)) + '] = ' + str(e1)
                except OutOfRangeException, e2:
                    raise OutOfRangeException, 'dataSet[' + str(dataSet.index(value)) + '] = ' + str(e2)
            return dataSet