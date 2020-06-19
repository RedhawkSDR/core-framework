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

from redhawk.codegen.jinja.loader import CodegenLoader
from redhawk.codegen.jinja.common import ShellTemplate, AutomakeTemplate, AutoconfTemplate, PkgconfigTemplate
from redhawk.codegen.jinja.cpp import CppCodeGenerator, CppTemplate
from mapping import PluginMapper

from redhawk.codegen.lang.automake import libtoolName

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

loader = CodegenLoader(__package__,
                       {'common': 'redhawk.codegen.jinja.common',
                        'base':   'redhawk.codegen.jinja.cpp.component.base'})

class GPPPluginGenerator(CppCodeGenerator):
    def __init__(self, **opts):
        super(CppCodeGenerator,self).__init__('cpp', **opts)
        self.plugin_name = opts['plugin_name']

    def parseopts(self, **opts):
        pass

    def loader(self, component):
        return loader

    def componentMapper(self):
        return None

    def propertyMapper(self):
        return None

    def portMapper(self):
        return None

    def map(self, softpkg):
        # Apply template-specific mapping for component.
        impl = 'cpp'
        compmapper = PluginMapper().mapComponent(self.plugin_name)
        return compmapper

    def templates(self, library):
        templates = [
            #CppTemplate('resource.h', userHeader, userfile=True),
            #CppTemplate('resource.cpp', userSource, userfile=True),
            CppTemplate('resource.h', self.plugin_name+'.h', userfile=True),
            CppTemplate('resource.cpp', self.plugin_name+'.cpp', userfile=True),
            CppTemplate('struct_props.h'),
            CppTemplate('main.cpp'),
            AutomakeTemplate('Makefile.am'),
            AutomakeTemplate('base/Makefile.am.ide', userfile=True),
            AutoconfTemplate('configure.ac'),
            ShellTemplate('base/build.sh'),
            ShellTemplate('common/reconf')
        ]

        return templates
