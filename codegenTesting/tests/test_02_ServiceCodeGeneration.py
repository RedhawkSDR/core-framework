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
import unittest, os, signal, time, sys, commands, copy, shutil
from xml.dom.minidom import parse, parseString
from omniORB import URI, any
from ossie.cf import CF, CF__POA
import ossie.utils.testing
import time

sys.path.append('helpers')
import scatest, common

class GeneratorTest(scatest.CodeGenTestCase):
    eclipse_workspace = 'workspace'
    artifact_type = 'Service'
    remove_list = None
    comp_list = None

    def test_00_GenerateCode(self):
        # Copy project to build directory
        shutil.rmtree(self.build_dir, True)
        shutil.copytree(self.source_dir, self.build_dir)
        self.languages = ['Python','Java','C++']
        self.generateCode()

    def test_01_CompileCode(self):
        for lang in self.impl_dict:
            if lang in common.cpp_usage:
                for impl_name in (impl['name'] for impl in self.impl_dict[lang]):
                    self.compileProject(os.path.join(self.build_dir, (impl_name.split('/'))[0]))
            if lang in common.java_usage:
                for impl_name in (impl['name'] for impl in self.impl_dict[lang]):
                    self.compileProject(os.path.join(self.build_dir, (impl_name.split('/'))[0]))

    def test_02_EvaluateUnitTests(self):
        self.runUnitTests()

if __name__ == '__main__':
    unittest.main()

