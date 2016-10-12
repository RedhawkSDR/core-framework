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

import jinja2

from redhawk.codegen.jinja.loader import CodegenLoader
from redhawk.codegen.jinja.common import ShellTemplate, AutomakeTemplate, AutoconfTemplate
from redhawk.codegen.jinja.cpp import CppCodeGenerator, CppTemplate

from mapping import ServiceMapper

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

loader = CodegenLoader(__package__,
                       {'common': 'redhawk.codegen.jinja.common'})

class ServiceGenerator(CppCodeGenerator):
    def loader(self, component):
        return loader

    def componentMapper(self):
        return ServiceMapper()

    def propertyMapper(self):
        return None

    def portMapper(self):
        return None

    def templates(self, component):
        templates = [
            CppTemplate('main.cpp'),
            CppTemplate('service.cpp', component['userclass']['file'], userfile=True),
            CppTemplate('service.h', component['userclass']['header'], userfile=True),
            CppTemplate('service_base.cpp', component['baseclass']['file']),
            CppTemplate('service_base.h', component['baseclass']['header']),
            AutomakeTemplate('Makefile.am'),
            AutomakeTemplate('Makefile.am.ide', userfile=True),
            AutoconfTemplate('configure.ac'),
            ShellTemplate('build.sh'),
            ShellTemplate('common/reconf')
        ]

        return templates
