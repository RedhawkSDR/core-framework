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

from redhawk.codegen.jinja.mapping import ComponentMapper
from redhawk.codegen import libraries
import commands, os

class BaseComponentMapper(ComponentMapper):
    def _mapComponent(self, softpkg):
        cppcomp = {}
        cppcomp['userclass'] = { 'name'  : softpkg.name()+'_i',
                                 'header': softpkg.name()+'.h' }
        cppcomp['interfacedeps'] = tuple(self.getInterfaceDependencies(softpkg))
        cppcomp['softpkgdeps'] = self.softPkgDeps(softpkg, format='deps')
        cppcomp['pkgconfigsoftpkgdeps'] = self.softPkgDeps(softpkg, format='pkgconfig')
        return cppcomp

    def getInterfaceDependencies(self, softpkg):
        for namespace in self.getInterfaceNamespaces(softpkg):
            yield libraries.getPackageRequires(namespace)

    def softPkgDeps(self, softpkg, format='deps'):
        deps = ''
        impl_id = self._impl.identifier()
        # only look at dependencies with matching implementation id
        if impl_id :
            try:
                impl_deps = [ y for y in softpkg.getSoftPkgDeps() if y['root_impl'] == impl_id ]
                for dep in impl_deps:
                    pc_filedir = dep['localfile'][:dep['localfile'].rfind('/')]
                    # check if there was a specific implementation to use from the dependency
                    if dep['implref'] :
                        for c in dep['spd'].get_implementation():
                            if c.get_id() == dep['implref']:
                                refdir=str(c.get_code().get_localfile().get_name())
                                refdir=refdir[:refdir.rfind('/')]
                                pc_filedir=pc_filedir+'/'+refdir

                    status, output, pc_filename = self.checkPkgConfig( pc_filedir, dep['name'] )
                    ##print "softPkgDeps", status, output, pc_filename, pc_filedir
                    if status == 0:
                        if format == 'deps':
                            deps += dep['name']+' >= '+output+' '
                        elif format == 'pkgconfig':
                            deps += '$SDRROOT/dom'+pc_filename+':'
            except:
                pass
        else:
            for dep in softpkg.getSoftPkgDeps():
                pc_filedir =  dep['localfile'][:dep['localfile'].rfind('/')]
                status, output, pc_filename = self.checkPkgConfig( pc_filedir, dep['name'] )
                if status == 0:
                    if format == 'deps':
                        deps += dep['name']+' >= '+output+' '
                    elif format == 'pkgconfig':
                        deps += '$SDRROOT/dom'+pc_filename+':'
        return deps


    def checkPkgConfig( self, pc_filedir, depname ):
        pc_filename = pc_filedir+'/pkgconfig'
        output=''
        ##print "pc_filename 1", pc_filename
        status,output = commands.getstatusoutput('pkg-config '+os.getenv('SDRROOT')+'/dom'+pc_filename+'/'+depname+'.pc'' --modversion')
        if status != 0:
            pc_filename = pc_filedir+'/lib/pkgconfig'
            ##print "pc_filename 2", pc_filename
            status,output = commands.getstatusoutput('pkg-config '+os.getenv('SDRROOT')+'/dom'+pc_filename+'/'+depname+'.pc'' --modversion')
            ##print status, output
            if status != 0:
                pc_filename = pc_filedir+'/lib64/pkgconfig'
                ##print "pc_filename 3", pc_filename
                status,output = commands.getstatusoutput('pkg-config '+os.getenv('SDRROOT')+'/dom'+pc_filename+'/'+depname+'.pc'' --modversion')
        return status, output, pc_filename



