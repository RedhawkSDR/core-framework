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

from redhawk.codegen.jinja.cpp.component.pull.mapping import PullComponentMapper
from redhawk.codegen.lang import mfile

class MFunctionMapper(PullComponentMapper):
    def __init__(self, outputdir):
        PullComponentMapper.__init__(self)
        self._outputdir = outputdir

    def _mapComponent(self, softpkg):
        '''
        Extends the pull mapper _mapComponent method by defining the
        'mFunction' and 'license' key/value pairs to the component dictionary.

        '''

        component = PullComponentMapper._mapComponent(self, softpkg)

        mFunction = None
        for prop in softpkg.properties():
            if str(prop.identifier()) == "__mFunction":
                mFunction = prop.value()
                break

        if mFunction:
            # point towards the m file that has been copied
            # to the implementation directory
            mFilePath = os.path.join(softpkg.path(), self._outputdir, mFunction+".m")
            parameters = mfile.parse(mFilePath)
            name = parameters.functionName
            inputs = parameters.inputs
            outputs = parameters.outputs
        else:
            name = ""
            inputs = []
            outputs = []

        component['mFunction'] = {'name'      : name,
                                  'inputs'    : inputs,
                                  'numInputs' : len(inputs),
                                  'outputs'   : outputs}
        component['license'] = "GPL"

        return component
