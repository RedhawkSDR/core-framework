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

from redhawk.codegen.jinja.generator import CodeGenerator
from redhawk.codegen.jinja.loader import CodegenLoader
from redhawk.codegen.jinja.common import ShellTemplate, AutomakeTemplate, AutoconfTemplate
from redhawk.codegen.jinja.cpp import CppCodeGenerator, CppTemplate
from redhawk.codegen.jinja.cpp.properties import CppPropertyMapper
from redhawk.codegen.jinja.cpp.ports import CppPortMapper, CppPortFactory
from redhawk.codegen.jinja.template import TemplateFile

from mapping import PullComponentMapper

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

loader = CodegenLoader(__package__,
                       {'common'         : 'redhawk.codegen.jinja.common',
                        'base'           : 'redhawk.codegen.jinja.cpp.component.base',
                        'properties'     : 'redhawk.codegen.jinja.cpp.properties'})

class PullComponentGenerator(CppCodeGenerator):
    # Need to keep use_vector_impl, auto_start and queued_ports to handle legacy options 
    def parseopts (self, use_vector_impl=True,auto_start=True,queued_ports=False):
        pass

    def loader(self, component):
        return loader

    def componentMapper(self):
        return PullComponentMapper()

    def propertyMapper(self):
        return CppPropertyMapper()

    def portMapper(self):
        return CppPortMapper()

    def portFactory(self):
        return CppPortFactory()

    def templates(self, component):
        templates = [
            CppTemplate('base/main.cpp'),
            AutomakeTemplate('base/Makefile.am'),
            AutomakeTemplate('base/Makefile.am.ide', userfile=True),
            AutoconfTemplate('base/configure.ac'),
            ShellTemplate('base/build.sh'),
            ShellTemplate('common/reconf'),
            CppTemplate('resource.cpp', component['userclass']['file'], userfile=True),
            CppTemplate('resource.h', component['userclass']['header'], userfile=True),
            CppTemplate('resource_base.cpp', component['baseclass']['file']),
            CppTemplate('resource_base.h', component['baseclass']['header'])
        ]

        # Add port implementations if required
        templates.extend(CppTemplate(fn) for fn in self.getPortTemplates(component))

        if component['structdefs']:
            templates.append(CppTemplate('struct_props.h'))

        return templates

    def getPortTemplates(self, component):
        templates = set()
        for generator in component['portgenerators']:
            if generator.hasDeclaration():
                generator.setHeader('"port_impl.h"')
                templates.add('port_impl.h')
            if generator.hasImplementation():
                templates.add('port_impl.cpp')
        return templates
        
