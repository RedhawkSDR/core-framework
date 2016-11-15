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
from redhawk.codegen.model import softpkg
from redhawk.packagegen.resourcePackage import ResourcePackage

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

class BinaryPackage(ResourcePackage):

    def __init__(
            self,
            bFile,
            compName=None,
            cmdArgs=None,
            inputFile=None,
            outputFile=None,
            inputFmt='8o',
            outputFmt='8o',
            outputDir = ".",
            sharedLibraries  = [],
            bufferingEnabled = False,
            loggingConfigUri = None):
        '''
        Create an binary component
        '''
        self.bufferingEnabled = bufferingEnabled

        self.bFile = bFile

        if compName == None:
            compName = bFile

        ResourcePackage.__init__(
            self,
            name = compName,
            implementation = "python",
            outputDir = outputDir,
            generator = "python.component.binary",
            loggingConfigUri = loggingConfigUri)

        self._addDefaultProps(inputfile=inputFile,outputfile=outputFile,inputFmt=inputFmt,outputFmt=outputFmt)

        # Add input ports
        self.addProvidesPort('dataDouble_in', "IDL:BULKIO/dataDouble:1.0")
        self.addProvidesPort('dataFloat_in', "IDL:BULKIO/dataFloat:1.0")
        self.addProvidesPort('dataShort_in', "IDL:BULKIO/dataShort:1.0")
        self.addProvidesPort('dataUshort_in', "IDL:BULKIO/dataUshort:1.0")
        self.addProvidesPort('dataLong_in', "IDL:BULKIO/dataLong:1.0")
        self.addProvidesPort('dataUlong_in', "IDL:BULKIO/dataUlong:1.0")
        self.addProvidesPort('datadataOctet_in', "IDL:BULKIO/dataOctet:1.0")

        # Add output ports
        self.addUsesPort('dataDouble_out', "IDL:BULKIO/dataDouble:1.0")
        self.addUsesPort('dataFloat_out', "IDL:BULKIO/dataFloat:1.0")
        self.addUsesPort('dataShort_out', "IDL:BULKIO/dataShort:1.0")
        self.addUsesPort('dataUshort_out', "IDL:BULKIO/dataUshort:1.0")
        self.addUsesPort('dataLong_out', "IDL:BULKIO/dataLong:1.0")
        self.addUsesPort('dataUlong_out', "IDL:BULKIO/dataUlong:1.0")
        self.addUsesPort('datadataOctet_out', "IDL:BULKIO/dataOctet:1.0")

        for sharedLibrary in sharedLibraries:
            self.addSoftPackageDependency(sharedLibrary, resolve_implref=True)

    def copyBinary(self):
        outputfile = self.outputDir+"/"+self.name+"/python/"+self.bFile
        fp = open(self.bFile,'r')
        stuff = fp.read()
        fp.close()
        fp = open(outputfile, 'w')
        fp.write(stuff)
        fp.close()

    def _addDefaultProps(self, inputfile=None, outputfile=None, inputFmt=None, outputFmt=None):
        bufferingEnabledStr = str(self.bufferingEnabled).lower()

        self.addSimpleProperty(
            complex=False,
            type="boolean",
            id="bufferingEnabled",
            value=bufferingEnabledStr)

        self.addSimpleProperty(
            complex=False,
            type="string",
            mode="readonly",
            id="inputFileArg",
            value=inputfile)

        self.addSimpleProperty(
            complex=False,
            type="string",
            mode="readonly",
            id="outputFileArg",
            value=outputfile)

        self.addSimpleProperty(
            complex=False,
            type="string",
            mode="readonly",
            id="inputFmt",
            value=inputFmt)

        self.addSimpleProperty(
            complex=False,
            type="string",
            mode="readonly",
            id="outputFmt",
            value=outputFmt)

        self.addSimpleProperty(
            complex=False,
            type="string",
            id="binary",
            mode="readonly",
            value=self.bFile,
            kindtypes=["configure","execparam"])

        self.addStructSequencProperty(
            "cmdArgs",
            struct_id='cmdArgs_str',
            type={'cmdId':'string','cmdValue':'string'})
        
