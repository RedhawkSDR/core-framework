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
import warnings

from redhawk.codegen.model.properties import Kinds
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface
import redhawk.codegen.model.properties

class PluginMapper(object):
    def setImplementation(self, impl=None):
        pass

    def mapSoftpkg(self, softpkg):
        component = {}
        component['name'] = str(softpkg)
        component['version'] = '1.0'
        component['basename'] = str(softpkg)
        component['sdrpath'] = os.getenv('SDRROOT')+'/dev/devices/GPP/plugins/'+component['name']
        component['artifacttype'] = 'plugin'
        return component

    def mapComponent(self, softpkg):
        component = self.mapSoftpkg(softpkg)
        component.update(self._mapComponent(softpkg))
        return component

    def _mapComponent(self, softpkg):
        return {}

    def mapImplementation(self, impl):
        impldict = {}
        return impldict

    def _mapImplementation(self, implementation):
        return {}

    def _mapSoftpkgDependency(self, dependency):
        depdict = {}
        return depdict

    def getInterfaceNamespaces(self, softpkg):
        return

