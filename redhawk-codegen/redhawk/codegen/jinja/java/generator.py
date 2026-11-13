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
from redhawk.codegen.jinja.java import JavaTemplate
from redhawk.codegen import versions

import os

class JavaCodeGenerator(CodeGenerator):
    def sourceFiles(self, component):
        for template in self.templates(component):
            if template.filename.endswith('.java'):
                yield template.filename
        for _template in self.templatesChildren(component):
            child = list(_template.keys())[0]
            template = _template.values()[0]
            filename, ext = os.path.splitext(template.filename)
            if ext in ('.java'):
                yield template.filename

    def rpmRequires(self):
        return super(JavaCodeGenerator,self).rpmRequires() + ['java >= '+versions.java]

    def rpmBuildRequires(self):
        return super(JavaCodeGenerator,self).rpmRequires() + ['java-devel >= '+versions.java]

    def templatesChildren(self, component):
        templates = []
        if not component.get('children'):
            return templates

        pkgpath = os.path.join('src', *component['package'].split('.')) #os.path.join(pkgpath, userfile)
        for child_key in component['children']:
            templates.append({child_key: JavaTemplate('resource.java', os.path.join(pkgpath, child_key+'/'+component['children'][child_key]['userclass']['file']), userfile=True)})
            templates.append({child_key: JavaTemplate('resource_base.java', os.path.join(pkgpath, child_key+'/'+component['children'][child_key]['baseclass']['file']))})
            #templates.append({child_key: JavaTemplate('resource.java', child_key+'/'+component['children'][child_key]['userclass']['file'], userfile=True)})
            #templates.append({child_key: JavaTemplate('resource_base.java', child_key+'/'+component['children'][child_key]['baseclass']['file'])})
            #if component['children'][child_key]['structdefs']:
            #    templates.append({child_key: JavaTemplate('struct_props.h', child_key+'/'+child_key+'_struct_props.h')})
            #if len(self.getPortTemplates(component['children'][child_key])) > 0:
            #    for fn in self.getPortTemplates(component['children'][child_key]):
            #        templates.append({child_key: JavaTemplate('pull/'+fn, child_key+'/'+child_key+'_'+fn)})

            portpkg = component['package'] + '.' + 'ports'
            portpkgpath = os.path.join(pkgpath, child_key+'/ports')
            for generator in component['children'][child_key]['portgenerators']:
                if not generator.hasImplementation():
                    continue
                generator.package = portpkg
                template = generator.implementation()
                filename = os.path.join(portpkgpath, generator.className()+'.java')
                context = {'portgenerator': generator}
                templates.append(JavaTemplate(template, filename, package=portpkg, context=context))

        return templates
