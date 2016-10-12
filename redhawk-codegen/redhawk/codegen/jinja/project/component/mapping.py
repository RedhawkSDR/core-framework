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

import os

from redhawk.codegen import libraries
from redhawk.codegen.lang.idl import IDLInterface
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.jinja.mapping import ComponentMapper

_projectTypes = {
    ComponentTypes.RESOURCE: 'Component',
    ComponentTypes.DEVICE:   'Device',
    ComponentTypes.LOADABLEDEVICE: 'LoadableDevice',
    ComponentTypes.EXECUTABLEDEVICE: 'ExecutableDevice',
    ComponentTypes.SERVICE:  'Service',
    ComponentTypes.SHAREDPACKAGE: 'SharedPackage'
}

class ProjectMapper(ComponentMapper):
    def _mapImplementation(self, impl):
        impldict = {}
        impldict['language'] = impl.programminglanguage()
        # NB: This makes the (reasonable, in practice) assumption that each
        #     implementation is in the same subdirectory as the entry point.
        impldict['outputdir'] = os.path.dirname(impl.entrypoint())
        return impldict

    def _mapComponent(self, softpkg):
        component = {}
        component['type'] = _projectTypes[softpkg.type()]
        component['interfaces'] = [libraries.getRPMDependency(name) for name in self.getInterfaceNamespaces(softpkg)]
        return component

    def mapProject(self, softpkg):
        project = self.mapComponent(softpkg)        
        impls = [self.mapImplementation(impl) for impl in softpkg.implementations()]
        project['implementations'] = impls
        project['languages'] = set(impl['language'] for impl in impls)
        project['subdirs'] = [impl['outputdir'] for impl in impls]
        return project
