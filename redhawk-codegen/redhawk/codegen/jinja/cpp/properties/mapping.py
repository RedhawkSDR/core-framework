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
from redhawk.codegen.lang.idl import IDLInterface

from redhawk.codegen.jinja.mapping import PropertyMapper

class CppPropertyMapper(PropertyMapper):
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
        cppprop['cpptype'] = cpp.cppType(prop.type(), prop.isComplex())
        if prop.hasValue():
            cppprop['cppvalue'] = cpp.literal(prop.value(), 
                                              prop.type(),
                                              prop.isComplex())
        return cppprop

    def mapSimpleSequenceProperty(self, prop):
        cppprop = self.mapProperty(prop)
        cppprop['cpptype'] = cpp.sequenceType(prop.type(), prop.isComplex())
        if prop.hasValue():
            cppprop['cppvalues'] = [cpp.literal(v, 
                                                prop.type(), 
                                                prop.isComplex()) 
                                    for v in prop.value()]
        return cppprop

    def mapStructProperty(self, prop, fields):
        cppprop = self.mapProperty(prop)
        typename = self.getStructPropertyType(prop)
        cppprop['cpptype'] = typename
        cppprop['cppvalue'] = typename + '()'
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
                newval[identifier] = cpp.literal(value[identifier], field['type'])
        return newval
