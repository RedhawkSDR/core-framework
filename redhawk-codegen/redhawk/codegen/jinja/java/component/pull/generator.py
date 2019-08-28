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

from redhawk.codegen import utils

from redhawk.codegen.jinja.loader import CodegenLoader
from redhawk.codegen.jinja.common import ShellTemplate, AutomakeTemplate, AutoconfTemplate
from redhawk.codegen.jinja.java import JavaCodeGenerator, JavaTemplate
from redhawk.codegen.jinja.java.properties import JavaPropertyMapper
from redhawk.codegen.jinja.java.ports import PortFactoryList, JavaPortMapper
from redhawk.codegen.jinja.java.ports.portfactory import JavaPortFactory
from redhawk.codegen.jinja.java.ports.frontend import FrontendProvidesPortFactory
from redhawk.codegen.jinja.java.component.base import BaseComponentGenerator

from mapping import PullComponentMapper

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

loader = CodegenLoader(__package__,
                       {'base': 'redhawk.codegen.jinja.java.component.base',
                        'common': 'redhawk.codegen.jinja.common'})

class ConsolidatedPortFactory(PortFactoryList):
    def __init__(self):
        factories = (FrontendProvidesPortFactory(), JavaPortFactory())
        super(ConsolidatedPortFactory,self).__init__(*factories)

class PullComponentGenerator(BaseComponentGenerator):
    # Need to keep use_jni, auto_start and queued_ports to handle legacy options
    def parseopts (self, java_package='',use_jni=True, auto_start=True,queued_ports=False):
        self.package = java_package

    def loader(self, component):
        return loader

    def componentMapper(self):
        return PullComponentMapper(self.package)

    def propertyMapper(self):
        return JavaPropertyMapper()

    def portMapper(self):
        return JavaPortMapper()

    def portFactory(self):
        return ConsolidatedPortFactory()

    def templates(self, component):
        # Put generated Java files in "src" subdirectory, followed by their
        # package path.
        pkgpath = os.path.join('src', *component['package'].split('.'))
        userfile = component['userclass']['file']
        basefile = component['baseclass']['file']
        templates = [
            JavaTemplate('resource.java', os.path.join(pkgpath, userfile), userfile=True),
            JavaTemplate('resource_base.java', os.path.join(pkgpath, basefile)),
            AutomakeTemplate('base/Makefile.am'),
            AutoconfTemplate('base/configure.ac'),
            ShellTemplate('base/startJava.sh'),
            ShellTemplate('common/reconf')
        ]

        portpkg = component['package'] + '.' + 'ports'
        portpkgpath = os.path.join(pkgpath, 'ports')
        for generator in component['portgenerators']:
            if not generator.hasImplementation():
                continue
            generator.package = portpkg
            template = generator.implementation()
            filename = os.path.join(portpkgpath, generator.className()+'.java')
            context = {'portgenerator': generator}
            templates.append(JavaTemplate(template, filename, package=portpkg, context=context))

        return templates
