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
from redhawk.codegen import libraries
from redhawk.codegen.jinja.mapping import ComponentMapper
from redhawk.codegen.jinja.cpp.ports import generic

class ServiceMapper(ComponentMapper):
    def _mapComponent(self, softpkg):
        cppcomp = {}
        idl = IDLInterface(softpkg.descriptor().repid().repid)
        cppcomp['baseclass'] = self.baseClass(softpkg)
        cppcomp['userclass'] = self.userClass(softpkg)
        cppcomp['superclass'] = self.superClass(softpkg)
        cppcomp['interfacedeps'] = tuple(self.getInterfaceDependencies(softpkg))
        cppcomp['poaclass'] = self.poaClass(idl)
        cppcomp['operations'] = self.getOperations(idl)
        return cppcomp

    @staticmethod
    def userClass(softpkg):
        return {'name'  : softpkg.basename()+'_i',
                'header': softpkg.basename()+'.h',
                'file'  : softpkg.basename()+'.cpp'}

    @staticmethod
    def baseClass(softpkg):
        baseclass = softpkg.basename() + '_base'
        return {'name'  : baseclass,
                'header': baseclass+'.h',
                'file'  : baseclass+'.cpp'}

    @staticmethod
    def superClass(softpkg):
        name = 'Service_impl'
        return {'name': name,
                'header': '<ossie/'+name+'.h>'}

    def poaClass(self, idl):
        poaclass = 'POA_%s::%s' % (cpp.idlNamespace(idl), idl.interface())
        return {'name': poaclass,
                'header': cpp.idlHeader(idl)}

    def getInterfaceDependencies(self, softpkg):
        for namespace in self.getInterfaceNamespaces(softpkg):
            yield libraries.getPackageRequires(namespace)

    def getOperations(self, idl):
        operations = []
        for op in idl.operations():
            operations.append({'name': op.name,
                   'arglist': ', '.join('%s %s' % (generic.argumentType(p.paramType,p.direction), p.name) for p in op.params),
                   'argnames': ', '.join(p.name for p in op.params),
                   'returns': generic.baseReturnType(op.returnType)})
        for attr in idl.attributes():
            operations.append({'name': attr.name,
                   'arglist': '',
                   'argnames': '',
                   'returns': generic.baseReturnType(attr.attrType)})
            if not attr.readonly:
                operations.append({'name': attr.name,
                       'arglist': generic.baseType(attr.attrType) + ' data',
                       'argnames': 'data',
                       'returns': 'void'})
        return operations
