#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK codegenTesting.
#
# REDHAWK codegenTesting is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import ossie.utils.testing
from ossie.utils import sb
from ossie import properties

class ComponentTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the component.
    SPD_FILE = '../EnumTest.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a component using the sandbox API
    # to create a data source, the package sb contains sources like StreamSource or FileSource
    # to create a data sink, there are sinks like StreamSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.StreamSink()
    #  src.connect(self.comp)
    #  self.comp.connect(snk)
    #  sb.start()
    #
    # components/sources/sinks need to be started. Individual components or elements can be started
    #  src.start()
    #  self.comp.start()
    #
    # every component/elements in the sandbox can be started
    #  sb.start()

    def setUp(self):
        # Launch the component, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def _propsToDict(self, props):
        return dict((dt['id'], dt['value']) for dt in props)

    def _runTests(self, props):
        props = properties.props_from_dict(dict((p, None) for p in props))
        result = self.comp.runTest(0, props)
        return properties.props_to_dict(result)

    def testSimpleEnums(self):
        result = self._runTests(['floatenum', 'stringenum'])
        self.assertEqual(result['floatenum'], self.comp.floatenum._enums)
        self.assertEqual(result['stringenum'], self.comp.stringenum._enums)

    def testStructEnums(self):
        result = self._runTests(['structprop'])
        enums = result['structprop']

        number_enums = self._propsToDict(enums['structprop::number'])
        self.assertEqual(number_enums, self.comp.structprop.number._enums)

        alpha_enums = self._propsToDict(enums['structprop::alpha'])
        self.assertEqual(alpha_enums, self.comp.structprop.alpha._enums)

    def testStructSequenceEnums(self):
        result = self._runTests(['structseq'])
        enums = result['structseq']

        number_enums = self._propsToDict(enums['structseq::number'])
        self.assertEqual(number_enums, self.comp.structseq.structDef.number._enums)

        text_enums = self._propsToDict(enums['structseq::text'])
        self.assertEqual(text_enums, self.comp.structseq.structDef.text._enums)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
