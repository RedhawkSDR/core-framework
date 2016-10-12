#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK code-generator.
#
# REDHAWK code-generator is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK code-generator is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import subprocess
import os
import shutil
from ossie.parsers import spd
from redhawk.packagegen.softPackage import SoftPackage

OSSIEHOME=os.environ["OSSIEHOME"]
DEFAULT_RESOURCE_TEMPLATE="/lib/python/redhawk/packagegen/templates/resourceTemplate.spd.xml"

class DirectoryPackageDependency(SoftPackage):

    def __init__(
            self,
            name,
            implementation,
            libraryLocation,
            outputDir=".",
            sharedLibraries  = [],
            spdTemplateFile = OSSIEHOME + DEFAULT_RESOURCE_TEMPLATE,
            variant = ""):

        SoftPackage.__init__(self, name, implementation, outputDir)

        # intantiate the class object representing the spd file
        self.spd = spd.parse(spdTemplateFile)

        self._setNameInSpd()
        self._setImplementation()

        self._createWavedevContent(generator="project.softPackageDependency.directory")

        # Create the output directory strucutre.  A share directory will be
        # created in the implemetation directory.  The share directory is
        # a symbolic link to the target directory.
        fullOutputDir = outputDir+"/" + name + "/"
        if not os.path.exists(fullOutputDir + implementation):
            os.makedirs(fullOutputDir + implementation)
        if not os.path.exists(fullOutputDir + implementation + "/share"):
            if not os.path.isabs(libraryLocation):
                libraryLocation = os.path.join(os.getcwd(), libraryLocation)
            shutil.copytree(libraryLocation, fullOutputDir + implementation + "/share")

        # Add soft package dependencies to the package being created (in the
        # case that the soft package dependency being created has its own
        # soft package dependencies).
        for sharedLibrary in sharedLibraries:
            self.addSoftPackageDependency(sharedLibrary)


    def _setImplementation(self):
        '''
        Enter the appropriate values into the implementation element of the
        spd file.

        '''

        localfile = spd.localFile(name = self.implementation + "/share")
        code = spd.code(type_= "SharedLibrary", localfile = localfile)
        compiler = spd.compiler(version="4.1.2", name="/usr/bin/gcc")

        implementation = spd.implementation(
            id_=self.implementation,
            description="",
            code = code,
            compiler = compiler,
            programminglanguage = spd.programmingLanguage(name="Octave"),
            humanlanguage = spd.humanLanguage(name="EN"))
        os = spd.os(name="Linux")
        implementation.add_os(value=os)
        self.spd.add_implementation(value=implementation)

