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

import sys
from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface

from redhawk.codegen.jinja.python.component.pull.mapping import PullComponentMapper

class ProgrammableComponentMapper(PullComponentMapper):
    def _mapComponent(self, softpkg):
        pycomp = PullComponentMapper._mapComponent(self, softpkg)
        pycomp['progclass'] = self.progClass(softpkg)
        return pycomp

    @staticmethod
    def progClass(softpkg):
        progclass = softpkg.basename() + '_prog_base'
        return {'name'  : progclass,
                'file'  : progclass+'.py'}
