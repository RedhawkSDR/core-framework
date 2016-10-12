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

from redhawk.codegen.lang import java
import commands, os
import ossie.parsers

from redhawk.codegen.jinja.mapping import ComponentMapper

class BaseComponentMapper(ComponentMapper):
    def __init__(self, package):
        self.package = package

    def _mapComponent(self, softpkg):
        javacomp = {}
        javacomp['softpkgcp'] = self.softPkgDeps(softpkg, format='cp')
        return javacomp

    def softPkgDeps(self, softpkg, format='cp'):
        deps = ''
        for spd_dep in softpkg.getSoftPkgDeps():
            spd_file = '/dom'+spd_dep['localfile']
            spd_dir = '$SDRROOT'+spd_file[:spd_file.rfind('/')]
            try:
                spd = ossie.parsers.spd.parse(os.getenv('SDRROOT')+spd_file)
            except:
                continue
            found_dep = False
            for impl in spd.get_implementation():
                localfile = impl.get_code().get_entrypoint()
                if localfile[-4:] != '.jar':
                    continue
                found_dep = True
                if format == 'cp':
                    deps += spd_dir+'/'+localfile+':'
                break
        return deps
