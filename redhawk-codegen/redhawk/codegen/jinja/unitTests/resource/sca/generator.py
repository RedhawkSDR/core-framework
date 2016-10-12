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


from redhawk.codegen.jinja.generator import TopLevelGenerator
from redhawk.codegen.jinja.python import PythonTemplate
from redhawk.codegen.jinja.mapping import ProjectMapper

import jinja2

if not '__package__' in locals():
    # Python 2.4 compatibility
    __package__ = __name__.rsplit('.', 1)[0]

loader = jinja2.PackageLoader(__package__)

class TestGenerator(TopLevelGenerator):
    def projectMapper(self):
        return ProjectMapper()

    def loader(self, project):
        return loader

    def templates(self, project):
        return [
            PythonTemplate('test.py', 'test_' + project['basename'] + '.py')
            ]
