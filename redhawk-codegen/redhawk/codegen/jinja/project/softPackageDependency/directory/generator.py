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

from redhawk.codegen.jinja.common import ShellTemplate, SpecfileTemplate
from redhawk.codegen.jinja.project.component.generator import ComponentProjectGenerator
from redhawk.codegen.jinja.loader import CodegenLoader

from redhawk.codegen.jinja.cpp import CppCodeGenerator
from mapping import DependencyDirectoryProjectMapper

from redhawk.codegen.jinja.common import ShellTemplate, AutomakeTemplate, AutoconfTemplate

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

loader = CodegenLoader(__package__,
                       {'common'     : 'redhawk.codegen.jinja.common',
                        'base'       : 'redhawk.codegen.jinja.cpp.component.base'})

class DependencyDirectoryProjectGenerator(CppCodeGenerator):

    def loader(self, project):
        return loader

    def templates(self, project):
        return [
            AutomakeTemplate('Makefile.am'),
            AutoconfTemplate('configure.ac'),
            ShellTemplate('base/build.sh'),
            ShellTemplate('common/reconf')
            ]

    def componentMapper(self):
        return DependencyDirectoryProjectMapper()

    def propertyMapper(self):
        return None

    def portMapper(self):
        return None

