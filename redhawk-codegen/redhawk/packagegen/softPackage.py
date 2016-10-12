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
from ossie.parsers import spd

OSSIEHOME=os.environ["OSSIEHOME"]

class SoftPackage(object):

    def __init__(
            self,
            name,
            implementation,
            outputDir="."):

        self.name = name
        self.implementation = implementation
        self.outputDir = outputDir

        self.autotoolsDir = self.outputDir+'/'+self.name+'/'+self.implementation+'/'

        self.mFiles = []

        # Create empty objects that can be populated by classes inheriting
        # from SoftPackage
        self.spd = None
        self.scd = None
        self.prf = None
        self.wavedevContent = None

    def _setNameInSpd(self):
        self.spd.id_ = self.name
        self.spd.name = self.name

    def runCompileRpm(self):
        process = subprocess.Popen(
            './build.sh rpm',
            shell=True,
            cwd=self.outputDir+'/'+self.name)
        process.wait()

    def runInstall(self):
        '''
        Use subprocess.Popen() to call ./reconf; ./configure; make install.

        '''

        # Popen calls are non-blocking.  Need to call process.wait() to make
        # sure each step is done before continuing.
        process = subprocess.Popen(
            './reconf',
            shell=True,
            cwd=self.autotoolsDir)
        process.wait()
        process = subprocess.Popen(
            './configure',
            shell=True,
            cwd=self.autotoolsDir)
        process.wait()
        process = subprocess.Popen(
            'make install',
            shell=True,
            cwd=self.autotoolsDir)
        process.wait()

    def callCodegen(self, force = False, variant = ""):
        """
        Format command line arguments and call redhawk-codegen.

        For example:

            $ redhawk-codegen -m foo1.m -m foo2.m -f /home/user/bar.spd.xml

        """

        codegenArgs = ["redhawk-codegen"]
        for mFile in self.mFiles:
            codegenArgs.append("-m")
            codegenArgs.append(mFile)

        if force:
            codegenArgs.append("-f")

        if variant != "":
            codegenArgs.append("--variant=" + variant)

        codegenArgs.append(self.outputDir+"/"+self.name+"/"+self.name+".spd.xml")
        subprocess.call(codegenArgs)

    def _createWavedevContent(self, generator):
        # TODO: replace this with an XML template
        self.wavedevContent='<?xml version="1.0" encoding="ASCII"?>\n'
        self.wavedevContent+='<codegen:WaveDevSettings xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:codegen="http://www.redhawk.gov/model/codegen">\n'
        self.wavedevContent+='<implSettings key="__IMPLEMENTATION">\n'
        self.wavedevContent+='<value outputDir="__IMPLEMENTATION" template="redhawk.codegen.jinja.__GENERATOR" generatorId="redhawk.codegen.jinja.__GENERATOR" primary="true"/>\n'
        self.wavedevContent+='</implSettings>\n'
        self.wavedevContent+='</codegen:WaveDevSettings>\n'
        self.wavedevContent = self.wavedevContent.replace("__GENERATOR", generator)
        self.wavedevContent = self.wavedevContent.replace("__IMPLEMENTATION", self.implementation)

    def writeWavedev(self, outputDir="."):
        '''
        Write the hidden .resource.wavedev file.

        '''

        self.createOutputDirIfNeeded()
        outfile=open(self.outputDir+"/"+self.name+"/."+ self.name+".wavedev", 'w')
        outfile.write(self.wavedevContent)
        outfile.close()

    def createOutputDirIfNeeded(self):
        if not os.path.exists(self.outputDir + "/" + self.name):
            os.makedirs(self.outputDir + "/" + self.name)

    def writeXML(self):
        '''
        Call methods to Write resource.spd.xml, resource.prf.xml,
        resource.scd.xml, and .resource.wavedev.

        '''

        if self.spd:
            self.writeSPD()
        if self.scd:
            self.writeSCD()
        if self.prf:
            self.writePRF()
        if self.wavedevContent:
            self.writeWavedev()

    def _writeXMLwithHeader(self, xmlObject, fileType, dtdName, name_=None):
        '''
        The xml files contain two header lines that are outside of the primary
        file element.  Write the two header lines followed by the primary file
        element to output file.

        '''

        outFile = open(self.outputDir+"/"+self.name+"/"+self.name+"."+fileType+".xml", 'w')
        outFile.write('<?xml version="1.0" encoding="UTF-8"?>\n')
        outFile.write('<!DOCTYPE _DTDNAME_ PUBLIC "-//JTRS//DTD SCA V2.2.2 SPD//EN" "_DTDNAME_.dtd">\n'.replace("_DTDNAME_", dtdName))
        if name_ == None:
            name_ = dtdName
        xmlObject.export(
            outfile = outFile,
            level = 0,
            pretty_print = True,
            name_ = name_)
        outFile.close()

    def writeSPD(self):
        self.createOutputDirIfNeeded()
        self._writeXMLwithHeader(self.spd, "spd", "softpkg", name_="softpkg")

    def writeSCD(self):
        self.createOutputDirIfNeeded()
        self._writeXMLwithHeader(self.scd, "scd", "softwarecomponent")

    def writePRF(self):
        self.createOutputDirIfNeeded()
        self._writeXMLwithHeader(self.prf, "prf", "properties")

    def addSoftPackageDependency(self, dep, arch="noarch"):
        softpkgref = spd.softPkgRef(localfile=spd.localFile(name=dep),
                                    implref=spd.implRef(refid=arch))
        dependency = spd.dependency(type_="runtime_requirements",
                                    softpkgref=softpkgref)
        for index in range(len(self.spd.implementation)):
            self.spd.implementation[index].add_dependency(dependency)
