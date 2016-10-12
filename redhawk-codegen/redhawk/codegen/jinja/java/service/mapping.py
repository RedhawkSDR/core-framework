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
from redhawk.codegen.lang.idl import IDLInterface
from redhawk.codegen.jinja.mapping import ComponentMapper
from redhawk.codegen.jinja.java.ports.generic import *
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen import libraries

class ServiceMapper(ComponentMapper):
    def __init__(self, package):
        self.package = package

    def _mapComponent(self, softpkg):
        javacomp = {}
        idl = IDLInterface(softpkg.descriptor().repid().repid)
        javacomp['interface'] = idl.interface()
        javacomp['operations'] = self.operations(idl)
        javacomp['attributes'] = self.attributes(idl)
        javacomp.update(self.getNamespace(idl))
       
        if self.package == '':
            javacomp['package'] = softpkg.basename()+'.java'
        else:
            javacomp['package'] = self.package
        userclass = softpkg.basename()
        baseclass = userclass + '_base'
        javacomp['baseclass'] = {'name': baseclass,
                                 'file': baseclass+'.java'}
        javacomp['userclass'] = {'name': userclass,
                                 'file': userclass+'.java'}
        javacomp['superclass'] = self.superclass(softpkg)
        javacomp['mainclass'] = java.qualifiedName(userclass, javacomp['package'])
        javacomp['jarfile'] = softpkg.basename() + '.jar'
        javacomp['interfacedeps'] = list(self.getInterfaceDependencies(softpkg))
        javacomp['interfacejars'] = self.getInterfaceJars(softpkg)
        return javacomp

    def getInterfaceDependencies(self, softpkg):
        for namespace in self.getInterfaceNamespaces(softpkg):
            yield libraries.getPackageRequires(namespace)

    def getInterfaceJars(self, softpkg):
        jars = []
        for namespace in self.getInterfaceNamespaces(softpkg):
            library = libraries.getInterfaceLibrary(namespace)
            jars.extend(library['jarfiles'])
        return jars

    def superclass(self, softpkg):
        if softpkg.type() == ComponentTypes.SERVICE:
            name = 'Service'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()
        return {'name': name}

    def getNamespace(self, idl):
        if idl.namespace().startswith('omg.org'):
            return {'imports': 'org.omg.',
                    'namespace': idl.namespace().split('/')[1] }
        else:
            return {'imports': '',
                    'namespace': idl.namespace() }

    def operations(self, idl):
        ops = []
        for op in idl.operations():
            ops.append({'name': op.name,
                   'arglist': ', '.join('%s %s' % (paramType(p), p.name) for p in op.params),
                   'argnames': [p.name for p in op.params],
                   'throws': ', '.join('%s%s' % (self.getNamespace(idl)['imports'], baseType(r)) for r in op.raises),
                   'returns': baseType(op.returnType)})
        return ops
        
    def attributes(self, idl):
        attrs = []
        for attr in idl.attributes():
            attrs.append({'name': attr.name,
                     'readonly': attr.readonly,
                     'type': baseType(attr.attrType)})
        return attrs

