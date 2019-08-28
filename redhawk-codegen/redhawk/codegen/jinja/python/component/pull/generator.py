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

from redhawk.codegen import utils
from redhawk.codegen.jinja.loader import CodegenLoader
from redhawk.codegen.jinja.common import ShellTemplate, AutomakeTemplate, AutoconfTemplate
from redhawk.codegen.jinja.python import PythonCodeGenerator, PythonTemplate
from redhawk.codegen.jinja.python.properties import PythonPropertyMapper
from redhawk.codegen.jinja.ports import PortFactoryList
from redhawk.codegen.jinja.python.ports.frontend import FrontendProvidesPortFactory
from redhawk.codegen.jinja.python.ports import PythonPortMapper, PythonPortFactory

from mapping import PullComponentMapper

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

loader = CodegenLoader(__package__,
                       {'common': 'redhawk.codegen.jinja.common',
                        'base':   'redhawk.codegen.jinja.python.component.base'})

class ConsolidatedPortFactory(PortFactoryList):
    def __init__(self):
        factories = (FrontendProvidesPortFactory(), PythonPortFactory())
        super(ConsolidatedPortFactory,self).__init__(*factories)

class PullComponentGenerator(PythonCodeGenerator):
    # Need to keep auto_start and queued_ports to handle legacy options
    def parseopts (self, auto_start=True, queued_ports=False, legacy_structs=True):
        self.legacy_structs = utils.parseBoolean(legacy_structs)

    def loader(self, component):
        return loader

    def componentMapper(self):
        return PullComponentMapper()

    def propertyMapper(self):
        return PythonPropertyMapper(legacy_structs=self.legacy_structs)

    def portMapper(self):
        return PythonPortMapper()

    def portFactory(self):
        return ConsolidatedPortFactory()

    def templates(self, component):
        templates = [
            PythonTemplate('resource_base.py', component['baseclass']['file']),
            PythonTemplate('resource.py', component['userclass']['file'], executable=True, userfile=True),
            AutoconfTemplate('configure.ac'),
            AutomakeTemplate('base/Makefile.am'),
            AutomakeTemplate('base/Makefile.am.ide', userfile=True),
            ShellTemplate('common/reconf')
        ]
        return templates
