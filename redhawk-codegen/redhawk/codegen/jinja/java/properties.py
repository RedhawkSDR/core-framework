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

from redhawk.codegen.lang import java
from redhawk.codegen.lang.idl import CorbaTypes

from redhawk.codegen.jinja.mapping import PropertyMapper

from ossie import properties

# CORBA type to native Java
_propertyType = {
    CorbaTypes.OCTET:     java.Types.BYTE,
    CorbaTypes.BOOLEAN:   java.Types.BOOLEAN,
    CorbaTypes.CHAR:      java.Types.CHAR,
    CorbaTypes.SHORT:     java.Types.SHORT,
    CorbaTypes.USHORT:    java.Types.SHORT,
    CorbaTypes.LONG:      java.Types.INT,
    CorbaTypes.ULONG:     java.Types.INT,
    CorbaTypes.LONGLONG:  java.Types.LONG,
    CorbaTypes.ULONGLONG: java.Types.LONG,
    CorbaTypes.FLOAT:     java.Types.FLOAT,
    CorbaTypes.DOUBLE:    java.Types.DOUBLE,
    CorbaTypes.UTCTIME:   'CF.UTCTime',
    CorbaTypes.STRING:    'String',
    CorbaTypes.OBJREF:    'String'
}

# CORBA type to Java property container prefix
_propertyClass = {
    CorbaTypes.OCTET:     'Octet',
    CorbaTypes.BOOLEAN:   'Boolean',
    CorbaTypes.CHAR:      'Char',
    CorbaTypes.SHORT:     'Short',
    CorbaTypes.USHORT:    'UShort',
    CorbaTypes.LONG:      'Long',
    CorbaTypes.ULONG:     'ULong',
    CorbaTypes.LONGLONG:  'LongLong',
    CorbaTypes.ULONGLONG: 'ULongLong',
    CorbaTypes.FLOAT:     'Float',
    CorbaTypes.DOUBLE:    'Double',
    CorbaTypes.UTCTIME:   'UTCTime',
    CorbaTypes.STRING:    'String',
    CorbaTypes.OBJREF:    'Objref'
}

class JavaPropertyMapper(PropertyMapper):
    def mapProperty(self, prop):
        javaprop = {}
        if prop.hasName():
            name = prop.name()
        else:
            name = prop.identifier()
        javaprop['javaname'] = java.identifier(name)
        javaprop['javakinds'] = ['Kind.'+k.upper() for k in prop.kinds()]
        return javaprop

    def javaType(self, typename):
        return _propertyType[typename]

    def javaClass(self, typename):
        return _propertyClass[typename]

    def _createComplexJavaProp(self, prop):
        '''
        Create a javaprop that may or may not be complex.
        '''
        javaprop = self.mapProperty(prop)
        javaprop['isComplex'] = prop.isComplex()
        if prop.isComplex():
            javatype = properties.mapComplexType(prop.type())
            javaprop['javaclass'] = 'Complex'+self.javaClass(prop.type())
            javaprop['javatype'] = 'CF.' + javatype
        else:
            javatype = self.javaType(prop.type())
            javaprop['javaclass'] = self.javaClass(prop.type())
            javaprop['javatype'] = java.boxedType(javatype)
        return javaprop, javatype
 
    def mapSimpleProperty(self, prop):
        javaprop, javatype = self._createComplexJavaProp(prop)

        if prop.hasValue():
            value = java.literal(prop.value(), 
                                 javatype, 
                                 complex = prop.isComplex())
        else:
            if javaprop['javatype'] == 'CF.UTCTime':
                value = '(CF.UTCTime)'+java.NULL
            else:
                value = java.NULL
        javaprop['javavalue'] = value
        javaprop['isOptional'] = prop.isOptional()
        return javaprop

    def mapEnumeration(self, prop, label, value):
        javaenum = {}
        enumtype = self.javaType(prop.type())
        javaenum['javatype'] = enumtype
        javaenum['javalabel'] = java.identifier(label)
        javaenum['javavalue'] = java.literal(value, enumtype, prop.isComplex())
        return javaenum

    def mapSimpleSequenceProperty(self, prop):
        javaprop, javatype = self._createComplexJavaProp(prop)
        values = []
        if prop.hasValue():
            for value in prop.value():
                values.append(java.literal(value,
                                           javatype,
                                           complex = prop.isComplex()))
        javaprop['javavalues'] = values
        javaprop['isOptional'] = prop.isOptional()
        return javaprop

    def mapStructProperty(self, prop, fields):
        javaprop = self.mapProperty(prop)
        javaprop['javatype'] = javaprop['javaname'] + '_struct'
        return javaprop

    def mapStructSequenceProperty(self, prop, structdef):
        javaprop = self.mapProperty(prop)
        javaprop['javatype'] = 'ArrayList<%s>' % structdef['javatype']
        javaprop['javavalues'] = [self.mapStructValue(structdef, value) for value in prop.value()]
        return javaprop

    def mapStructValue(self, structdef, value):
        newval = []
        for field in structdef['fields']:
            identifier = field['identifier']
            if identifier in value:
                itemvalue = value[identifier]
            else:
                itemvalue = field.get('value', None)
            if itemvalue is not None:
                if isinstance(itemvalue, list):
                    vals = []
                    for val in itemvalue:
                        vals.append(java.literal(val, field['javatype']))
                    addval = field['javaclass']+'SequenceProperty.asList('+','.join(vals)+')'
                    newval.append(addval)
                else:
                    newval.append(java.literal(itemvalue, field['javatype']))
            else:
                newval.append(java.NULL)
        return newval
