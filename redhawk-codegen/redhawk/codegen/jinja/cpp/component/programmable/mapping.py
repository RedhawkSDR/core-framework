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

from redhawk.codegen.model.softwarecomponent import ComponentTypes
from redhawk.codegen.lang.idl import IDLInterface

from redhawk.codegen.jinja.cpp.component.pull.mapping import PullComponentMapper
import sys

class ProgrammableComponentMapper(PullComponentMapper):
    def _mapComponent(self, softpkg):
        cppcomp = PullComponentMapper._mapComponent(self, softpkg)
        cppcomp['programmable'] = True
        cppcomp['reprogclass'] = self.reprogClass(softpkg)
        cppcomp['executesHWComponents'] = False # TODO: Implement this 
        self._validateAggregateDevice(cppcomp)
        return cppcomp

    def _validateAggregateDevice(self, comp):
        if not comp.has_key('superclasses'):
            print >> sys.stderr, "WARNING: Programmable device MUST be an aggregate device!"
            return

        missingAggDevice = True
        for superclass in comp['superclasses']:
            if not superclass.has_key('name'):
                continue
            if superclass['name'] == "AggregateDevice_impl":
                missingAggDevice = False
                break
        if missingAggDevice:
            print >> sys.stderr, "WARNING: Programmable device MUST be an aggregate device!"

    @staticmethod
    def reprogClass(softpkg):
        reprogclass = softpkg.basename() + '_prog_base'
        return {'name'  : reprogclass,
                'header': reprogclass+'.h',
                'file'  : reprogclass+'.cpp'}
