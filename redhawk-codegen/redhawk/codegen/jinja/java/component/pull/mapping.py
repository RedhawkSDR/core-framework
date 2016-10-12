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
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface
from redhawk.codegen import libraries

from redhawk.codegen.jinja.java.component.base import BaseComponentMapper

class PullComponentMapper(BaseComponentMapper):
    def __init__(self, package):
        self.package = package

    def _mapComponent(self, softpkg):
        javacomp = {}
        javacomp['package'] = self.package
        userclass = softpkg.name()
        baseclass = userclass + '_base'
        javacomp['baseclass'] = {'name': baseclass,
                                 'file': baseclass+'.java'}
        javacomp['userclass'] = {'name': userclass,
                                 'file': userclass+'.java'}
        javacomp['superclass'] = self.superclass(softpkg)
        javacomp['mainclass'] = java.qualifiedName(userclass, self.package)
        javacomp['jarfile'] = softpkg.name() + '.jar'
        javacomp['interfacedeps'] = list(self.getInterfaceDependencies(softpkg))
        javacomp['interfacejars'] = self.getInterfaceJars(softpkg)
        javacomp['softpkgcp'] = self.softPkgDeps(softpkg, format='cp')
        javacomp['hasmultioutport'] = self.hasMultioutPort(softpkg)
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
        if softpkg.type() == ComponentTypes.RESOURCE:
            name = 'ThreadedResource'
        elif softpkg.type() == ComponentTypes.DEVICE:
            name = 'ThreadedDevice'
        elif softpkg.type() == ComponentTypes.LOADABLEDEVICE:
            # NOTE: If java gets support for Loadable Devices, this needs to change
            name = 'ThreadedDevice'
        elif softpkg.type() == ComponentTypes.EXECUTABLEDEVICE:
            # NOTE: If java gets support for Executable Devices, this needs to change
            name = 'ThreadedDevice'
        else:
            raise ValueError, 'Unsupported software component type', softpkg.type()
        return {'name': name}
