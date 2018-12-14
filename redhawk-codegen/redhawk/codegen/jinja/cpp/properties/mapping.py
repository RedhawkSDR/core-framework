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

from redhawk.codegen.lang import cpp
from redhawk.codegen.lang.idl import IDLInterface, CorbaTypes

from redhawk.codegen.jinja.mapping import PropertyMapper

_formats = {
    CorbaTypes.OCTET     : 'o',
    CorbaTypes.BOOLEAN   : 'b',
    CorbaTypes.CHAR      : 'c',
    CorbaTypes.SHORT     : 'h',
    CorbaTypes.USHORT    : 'H',
    CorbaTypes.LONG      : 'i',
    CorbaTypes.ULONG     : 'I',
    CorbaTypes.LONGLONG  : 'l',
    CorbaTypes.ULONGLONG : 'L',
    CorbaTypes.FLOAT     : 'f',
    CorbaTypes.DOUBLE    : 'd',
    CorbaTypes.STRING    : 's',
    CorbaTypes.OBJREF    : 's',
    CorbaTypes.UTCTIME   : 'u',
}

class CppPropertyMapper(PropertyMapper):
    def _getSimpleFormat(self, prop, isSequence):
        format = _formats[prop.type()]
        if prop.isComplex():
            format = '2' + format;
        if isSequence:
            format = '[' + format + ']'
        if prop.isOptional():
            format += '?'
        return format

    def mapProperty(self, prop):
        cppprop = {}
        if prop.hasName():
            name = prop.name()
        else:
            name = prop.identifier()
        cppprop['cppname'] = cpp.identifier(name)
        return cppprop

    def mapSimpleProperty(self, prop):
        cppprop = self.mapProperty(prop)
        cppprop['iscomplex'] = prop.isComplex()
        cppprop['isOptional'] = prop.isOptional()
        cppprop['cpptype'] = cpp.cppType(prop.type(), prop.isComplex())
        if prop.hasValue():
            _prepend = ''
            _append = ''
            if prop.type() == 'utctime':
                _prepend = '"'
                _append = '"'
            cppprop['cppvalue'] = _prepend+cpp.literal(prop.value(), 
                                              prop.type(),
                                              prop.isComplex())+_append
        cppprop['format'] = self._getSimpleFormat(prop, False)
        return cppprop

    def mapEnumeration(self, prop, label, value):
        cppenum = {}
        cppenum['cpplabel'] = cpp.identifier(label)
        cppenum['cppvalue'] = cpp.literal(value, prop.type(), prop.isComplex())
        return cppenum

    def mapSimpleSequenceProperty(self, prop):
        cppprop = self.mapProperty(prop)
        cppprop['cpptype'] = cpp.sequenceType(prop.type(), prop.isComplex())
        cppprop['iscomplex'] = prop.isComplex()
        cppprop['isOptional'] = prop.isOptional()
        if prop.hasValue():
            _prepend = ''
            _append = ''
            if prop.type() == 'utctime':
                _prepend = 'redhawk::time::utils::convert("'
                _append = '")'
            cppprop['cppvalues'] = [_prepend+cpp.literal(v, 
                                                prop.type(), 
                                                prop.isComplex())+_append
                                    for v in prop.value()]
        cppprop['format'] = self._getSimpleFormat(prop, True)
        return cppprop

    def mapStructProperty(self, prop, fields):
        cppprop = self.mapProperty(prop)
        typename = self.getStructPropertyType(prop)
        cppprop['cpptype'] = typename
        cppprop['cppvalue'] = typename + '()'
        cppprop['format'] = ''.join(f['format'] for f in fields)
        return cppprop

    def getStructPropertyType(self, prop):
        return cpp.identifier(prop.name() + '_struct')

    def mapStructSequenceProperty(self, prop, structdef):
        cppprop = self.mapProperty(prop)
        cppprop['cpptype'] = 'std::vector<%s>' % structdef['cpptype']
        cppprop['cppvalues'] = [self.mapStructValue(structdef, value) for value in prop.value()]
        return cppprop

    def mapStructValue(self, structdef, value):
        newval = {}
        for field in structdef['fields']:
            identifier = field['identifier']
            if identifier in value:
                if type(value[identifier]) == list:
                    newval[identifier] = []
                    for val in value[identifier]:
                        newval[identifier].append(cpp.literal(val, field['type'], field['iscomplex']))
                else:
                    newval[identifier] = cpp.literal(value[identifier], field['type'], field['iscomplex'])
        return newval
