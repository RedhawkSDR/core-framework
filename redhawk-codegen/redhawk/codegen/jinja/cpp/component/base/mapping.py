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

from redhawk.codegen.jinja.mapping import ComponentMapper
from redhawk.codegen import libraries

class BaseComponentMapper(ComponentMapper):
    def _mapComponent(self, softpkg):
        cppcomp = {}
        cppcomp['userclass'] = { 'name'  : softpkg.basename()+'_i',
                                 'header': softpkg.basename()+'.h' }
        cppcomp['interfacedeps'] = tuple(self.getInterfaceDependencies(softpkg))
        return cppcomp

    def _mapImplementation(self, impl):
        impldict = {}
        if impl.isModule():
            impldict['module'] = True
            impldict['target'] = impl.entrypoint().replace('.so', '.la')
        else:
            impldict['target'] = impl.entrypoint()
        return impldict

    def getInterfaceDependencies(self, softpkg):
        for namespace in self.getInterfaceNamespaces(softpkg):
            yield libraries.getPackageRequires(namespace)
