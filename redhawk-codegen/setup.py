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
from setuptools import setup
from distutils.command.install_lib import install_lib

from redhawk.codegen import versions

class filtered_install_lib(install_lib):
    def byte_compile(self, files):
        # The default 'install_lib' implementation will attempt to compile all
        # '.py' files in the build tree, including any template files that
        # happen to end in '.py'; filter out everything from a 'templates'
        # directory to prevent this unwanted behavior.
        files = [f for f in files if not os.path.dirname(f).endswith('templates')]
        return install_lib.byte_compile(self,files)

import os
ossiehome = os.getenv('OSSIEHOME')
import sys
homeSys = False
buildArg = False
if 'build' in sys.argv:
    buildArg = True
for arg in sys.argv:
    if '--home' in arg:
        homeSys = True
        
if not homeSys and ossiehome != None and not buildArg:
    sys.argv.append('--home='+ossiehome)
if not ('--old-and-unmanageable' in sys.argv) and not buildArg:
    sys.argv.append('--old-and-unmanageable')

setup(name='redhawk-codegen',
      version=versions.codegen,
      install_requires = ['jinja2'],
      scripts=['redhawk-codegen','codegen_version','update_project','createOctaveComponent','createBinaryComponent','moveComponentNamespace'],
      cmdclass={'install_lib':filtered_install_lib},
      packages=['redhawk',
                'redhawk.codegen',
                'redhawk.codegen.model',
                'redhawk.codegen.lang',
                'redhawk.codegen.jinja',
                'redhawk.codegen.jinja.common',
                'redhawk.codegen.jinja.project',
                'redhawk.codegen.jinja.project.component',
                'redhawk.codegen.jinja.project.softPackageDependency',
                'redhawk.codegen.jinja.project.softPackageDependency.directory',
                'redhawk.codegen.jinja.unitTests',
                'redhawk.codegen.jinja.unitTests.resource',
                'redhawk.codegen.jinja.unitTests.resource.sca',
                'redhawk.codegen.jinja.java',
                'redhawk.codegen.jinja.java.component',
                'redhawk.codegen.jinja.java.component.base',
                'redhawk.codegen.jinja.java.component.pull',
                'redhawk.codegen.jinja.java.component.frontend',
                'redhawk.codegen.jinja.java.ports',
                'redhawk.codegen.jinja.java.service',
                'redhawk.codegen.jinja.python',
                'redhawk.codegen.jinja.python.component',
                'redhawk.codegen.jinja.python.component.base',
                'redhawk.codegen.jinja.python.component.pull',
                'redhawk.codegen.jinja.python.component.frontend',
                'redhawk.codegen.jinja.python.component.binary',
                'redhawk.codegen.jinja.python.ports',
                'redhawk.codegen.jinja.python.service',
                'redhawk.codegen.jinja.cpp',
                'redhawk.codegen.jinja.cpp.component',
                'redhawk.codegen.jinja.cpp.component.base',
                'redhawk.codegen.jinja.cpp.component.pull',
                'redhawk.codegen.jinja.cpp.component.mFunction',
                'redhawk.codegen.jinja.cpp.component.frontend',
                'redhawk.codegen.jinja.cpp.library',
                'redhawk.codegen.jinja.cpp.ports',
                'redhawk.codegen.jinja.cpp.service',
                'redhawk.codegen.jinja.cpp.properties',
                'redhawk.codegen.jinja.cpp.component.persona',
                'redhawk.codegen.jinja.cpp.component.programmable',
                'redhawk.packagegen',
                'redhawk.packagegen.templates'],
      package_data={'redhawk.codegen.jinja.common':['templates/*'],
                    'redhawk.codegen.jinja.project.component':['templates/*'],
                    'redhawk.codegen.jinja.project.softPackageDependency':['templates/*'],
                    'redhawk.codegen.jinja.project.softPackageDependency.directory':['templates/*'],
                    'redhawk.codegen.jinja.unitTests.resource.sca':['templates/*'],
                    'redhawk.codegen.jinja.java.component.base':['templates/*'],
                    'redhawk.codegen.jinja.java.component.pull':['templates/*'],
                    'redhawk.codegen.jinja.java.component.frontend':['templates/*'],
                    'redhawk.codegen.jinja.java.ports':['templates/*.java'],
                    'redhawk.codegen.jinja.java.service':['templates/*'],
                    'redhawk.codegen.jinja.python.component.base':['templates/*'],
                    'redhawk.codegen.jinja.python.component.pull':['templates/*'],
                    'redhawk.codegen.jinja.python.component.frontend':['templates/*'],
                    'redhawk.codegen.jinja.python.component.binary':['templates/*'],
                    'redhawk.codegen.jinja.python.ports':['templates/*.py'],
                    'redhawk.codegen.jinja.python.service':['templates/*'],
                    'redhawk.codegen.jinja.cpp.component.base':['templates/*'],
                    'redhawk.codegen.jinja.cpp.component.pull':['templates/*'],
                    'redhawk.codegen.jinja.cpp.component.mFunction':['templates/*'],
                    'redhawk.codegen.jinja.cpp.component.frontend':['templates/*'],
                    'redhawk.codegen.jinja.cpp.library':['templates/*'],
                    'redhawk.codegen.jinja.cpp.ports':['templates/*.cpp', 'templates/*.h'],
                    'redhawk.codegen.jinja.cpp.service':['templates/*'],
                    'redhawk.codegen.jinja.cpp.component.persona':['templates/*'],
                    'redhawk.codegen.jinja.cpp.component.programmable':['templates/*'],
                    'redhawk.codegen.jinja.cpp.properties':['templates/*.cpp'],
                    'redhawk.packagegen':['templates/*xml']}
      )
