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
import shutil

from redhawk.codegen.lang import mfile
from redhawk.packagegen.resourcePackage import ResourcePackage

def _getMFunctionParameters(function, mFiles):
    '''
    Parse the function declaration of the primary m file to get the input
    and output parameters.

    '''

    mFunctionParameters = None

    # find the master m file and parse its m function
    for mFile in mFiles:
        if mFile.find(function + '.m') != -1:
            mFunctionParameters = mfile.parse(mFile)
            break
    if mFunctionParameters is None:
        raise SystemExit('ERROR: No matching m file for specified function')

    return mFunctionParameters

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

def _isStringProp(value):
    '''
    Search for quotes in order to determine if the property is a string.

    '''
    if type(value) == type([]):
        # simple sequence
        if len(value) > 0:
            return value[0].find('"') != -1 or value[0].find("'") != -1
        else:
            # if empty, assume not a string
            return False
    else:
        # simple
        return value.find('"') != -1 or value.find("'") != -1

class OctavePackage(ResourcePackage):

    def __init__(
            self,
            mFiles,
            function,
            outputDir = ".",
            sharedLibraries  = [],
            diaryEnabled = False,
            bufferingEnabled = False,
            loggingConfigUri = None):
        '''
        Create an octave component using tags in the function arguments.

        All Octave components will have the following properties:

            bufferingEnabled
            diaryOnOrOff

        '''
        ResourcePackage.__init__(
            self,
            name = function,
            implementation = "cpp",
            outputDir = outputDir,
            generator = "cpp.component.octave",
            loggingConfigUri = loggingConfigUri)

        self.mFiles = mFiles
        mFunctionParameters = _getMFunctionParameters(function, mFiles)

        self.diaryEnabled = diaryEnabled
        self.bufferingEnabled = bufferingEnabled

        self._addDefaultProps()

        propArgs = ["__sampleRate"]

        # Add properties
        for propName in mFunctionParameters.defaults.keys():
            value = mFunctionParameters.defaults[propName]
            if type(value) == type([]):
                # simple sequence
                if _isStringProp(value):
                    # string
                    for index in range(len(value)):
                        value[index]=_cleanQuotes(value[index])
                    self.addSimpleSequencProperty(
                        id=propName,
                        values=value,
                        type="string",
                        complex=False)
                else:
                    # double
                    self.addSimpleSequencProperty(
                        id=propName,
                        values=value)
            else:
                # simple
                if _isStringProp(value):
                    # string
                    value=_cleanQuotes(value)
                    self.addSimpleProperty(
                        id=propName,
                        value=value,
                        type="string",
                        complex=False)
                else:
                    # double
                    self.addSimpleProperty(
                        id=propName,
                        value=value)
            propArgs.append(propName)

        # Add input ports
        for input in mFunctionParameters.inputs:
            if propArgs.count(input) == 0:
                self.addProvidesPort(input, "IDL:BULKIO/dataDouble:1.0")

        # Add output ports
        for output in mFunctionParameters.outputs:
            if propArgs.count(output) == 0:
                self.addUsesPort(output, "IDL:BULKIO/dataDouble:1.0")

        for sharedLibrary in sharedLibraries:
            self.addSoftPackageDependency(sharedLibrary, resolve_implref=True)

    def _preCodegen(self):
        impldir = os.path.join(self.outputDir, self.name, self.implementation)
        if not os.path.isdir(impldir):
            os.makedirs(impldir)
        else:
            # Update the modification time, so that the device will see an
            # updated timestamp when loading.
            os.utime(impldir, None)

        for mFile in self.mFiles:
            outfile = os.path.join(impldir, os.path.basename(mFile))
            shutil.copyfile(mFile, outfile)

    def _addDefaultProps(self):
        '''
        Add the diaryEnabled, bufferingEnabled, and __mFunction properties to
        the PRF.

        '''

        diaryEnabledStr = str(self.diaryEnabled).lower()
        bufferingEnabledStr = str(self.bufferingEnabled).lower()

        self.addSimpleProperty(
            complex=False,
            type="boolean",
            id="diaryEnabled",
            value=diaryEnabledStr)

        self.addSimpleProperty(
            complex=False,
            type="boolean",
            id="bufferingEnabled",
            value=bufferingEnabledStr)

        self.addSimpleProperty(
            complex=False,
            type="string",
            id="__mFunction",
            mode="readonly",
            value=self.name,
            kindtypes=["configure","execparam"])
