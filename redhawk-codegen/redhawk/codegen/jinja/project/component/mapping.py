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

from redhawk.codegen import libraries
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.jinja.mapping import ProjectMapper

_projectTypes = {
    ComponentTypes.RESOURCE: 'Component',
    ComponentTypes.DEVICE:   'Device',
    ComponentTypes.LOADABLEDEVICE: 'LoadableDevice',
    ComponentTypes.EXECUTABLEDEVICE: 'ExecutableDevice',
    ComponentTypes.SERVICE:  'Service',
    ComponentTypes.SHAREDPACKAGE: 'SharedPackage'
}

class ComponentProjectMapper(ProjectMapper):
    def _mapComponent(self, softpkg):
        component = {}
        component['type'] = _projectTypes[softpkg.type()]
        component['interfaces'] = [libraries.getRPMDependency(name) for name in self.getInterfaceNamespaces(softpkg)]
        component['specfile'] = softpkg.name()+'.spec'
        return component

    def _mapImplementation(self, impl, generator):
        impldict = {}
        impldict['requires'] = generator.rpmRequires()
        impldict['buildrequires'] = generator.rpmBuildRequires()
        impldict['module'] = impl.isModule()
        return impldict
