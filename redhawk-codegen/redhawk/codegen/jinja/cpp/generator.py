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

from redhawk.codegen.jinja.generator import CodeGenerator
from redhawk.codegen.jinja.cpp import CppTemplate

class CppCodeGenerator(CodeGenerator):

    def sourceFiles(self, component):
        for template in self.templates(component):
            filename, ext = os.path.splitext(template.filename)
            if ext in ('.h', '.cpp'):
                yield template.filename
        for _template in self.templatesChildren(component):
            child = _template.keys()[0]
            template = _template.values()[0]
            print 'template.filename', template.filename
            filename, ext = os.path.splitext(template.filename)
            if ext in ('.h', '.cpp'):
                yield template.filename

    def templatesChildren(self, component):
        templates = []
        if not component.has_key('children'):
            return templates

        for child_key in component['children']:
            templates.append({child_key: CppTemplate('resource.cpp', child_key+'/'+component['children'][child_key]['userclass']['file'], userfile=True)})
            templates.append({child_key: CppTemplate('resource.h', child_key+'/'+component['children'][child_key]['userclass']['header'], userfile=True)})
            templates.append({child_key: CppTemplate('resource_base.cpp', child_key+'/'+component['children'][child_key]['baseclass']['file'])})
            templates.append({child_key: CppTemplate('resource_base.h', child_key+'/'+component['children'][child_key]['baseclass']['header'])})
            if component['children'][child_key]['structdefs']:
                templates.append({child_key: CppTemplate('struct_props.h', child_key+'/struct_props.h')})
        return templates
