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
import unittest, os, sys, commands, copy, shutil
from xml.dom.minidom import parse
import ossie.utils.testing

sys.path.append('helpers')
import scatest, common

class GeneratorTest(scatest.CodeGenTestCase):

    language = None
    output_dir = 'build'
    artifact_type = 'Component'
    remove_list = [ 'test_spd_dep', 'octaveTest1', 'bintest1' ]
    comp_list = None

    def test_00_GenerateCode(self):
        # Copy project to build directory
        shutil.rmtree(self.build_dir, True)
        shutil.copytree(self.source_dir, self.build_dir)
        if os.path.isfile(self.build_dir + "/generate.sh"):
            # Some components might have a generate script for testing package
            # generation.
            (status,output) = commands.getstatusoutput(self.build_dir + "/generate.sh")
            self.assertEquals(
                status,
                0,
                msg='generate script failed' + '\n\n' + output)
        else:
            self.generateCode()

    def test_01_CompileCode(self):
        for lang in self.impl_dict:
            if lang in common.cpp_usage:
                for impl_name in (impl['name'] for impl in self.impl_dict[lang]):
                    self.compileProject(os.path.join(self.build_dir, (impl_name.split('/'))[0]))
        for lang in self.impl_dict:
            if lang in common.java_usage:
                for impl_name in (impl['name'] for impl in self.impl_dict[lang]):
                    self.compileProject(os.path.join(self.build_dir, (impl_name.split('/'))[0]))

    def test_02_EvaluateUnitTests(self):
        self.runUnitTests()


if __name__ == '__main__':
    unittest.main()
