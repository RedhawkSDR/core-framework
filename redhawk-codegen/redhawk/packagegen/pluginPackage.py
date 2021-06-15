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
import os
from redhawk.codegen.model import softpkg
from redhawk.packagegen.softPackage import SoftPackage
from redhawk.codegen.generate import importTemplate
from redhawk.codegen.settings import importWavedevSettings

def _cleanQuotes(value):
    '''
    Strip out quotation marks within the input string

    '''
    if len(value) == 0:
        return
    if value[0] == "'":
        return value.replace("'","")
    if value[0] == '"':
        return value.replace('"',"")

class PluginPackage(SoftPackage):

    def __init__(
            self,
            plugin_name,
            outputDir = "."):
        '''
        Create an binary component
        '''
        self.plugin_name = plugin_name

        SoftPackage.__init__(
            self,
            name = self.plugin_name,
            implementation = "cpp",
            outputDir = outputDir)

        self._createWavedevContent(generator="cpp.plugin")

    def callCodegen(self, force = False, variant = ""):
        wavedev = '.' + self.plugin_name+'.wavedev'
        wavedev = os.path.join(os.path.dirname(self.outputDir), wavedev)
        os.chdir(self.outputDir+'/'+self.plugin_name)
        settings = importWavedevSettings(wavedev)
        implList = list(settings.keys())
        package = importTemplate(settings['cpp'].template)
        projectGenerator = package.factory(plugin_name=self.plugin_name, outputdir='')
        implFiles = []
        projectGenerator.generate('', *implFiles)

    def writeXML(self):
        self.createOutputDirIfNeeded()
        if self.wavedevContent:
            self.writeWavedev()

    def _createWavedevContent(self, generator):
        # TODO: replace this with an XML template
        self.wavedevContent='<?xml version="1.0" encoding="ASCII"?>\n'
        self.wavedevContent+='<codegen:WaveDevSettings xmi:version="2.0" xmlns:xmi="http://www.omg.org/XMI" xmlns:codegen="http://www.redhawk.gov/model/codegen">\n'
        self.wavedevContent+='  <implSettings key="cpp">\n'
        self.wavedevContent+='    <value outputDir="." template="redhawk.codegen.jinja.cpp.plugin" generatorId="gov.redhawk.ide.codegen.jinja.cplusplus.CplusplusGenerator" primary="true"/>\n'
        self.wavedevContent+='  </implSettings>\n'
        self.wavedevContent+='</codegen:WaveDevSettings>\n'

