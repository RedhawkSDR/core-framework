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
    artifact_type = 'Device'
    remove_list = None
    comp_list = None

    def test_00_GenerateCode(self):
        # Copy project to build directory
        shutil.rmtree(self.build_dir, True)
        shutil.copytree(self.source_dir, self.build_dir)
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

