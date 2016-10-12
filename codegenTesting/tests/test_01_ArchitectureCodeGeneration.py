import unittest, os, sys, commands, copy, shutil
from xml.dom.minidom import parse
import ossie.utils.testing

sys.path.append('helpers')
import scatest, common

##
## ArchitectureCodeGeneration test performs architecture
## specific code generation, compilation and execution.
## To accomplish this, the implementation id for the 
## the component is passed to the code generator, and 
## referenced during the test. The implementation id
## is named from the return of the uname command (processor type)
##  e.g "x86_64" or "i686"
##
## This implementation id will be the tie to 
## the comp.spd.xml and also any soft package dependency records.
## These records must include the the implref element for the proper soft
## package dependency to be included during compilation and execution.
## 
##

class GeneratorTest(scatest.CodeGenTestCase):

    language = None
    output_dir = 'build'
    artifact_type = 'Architecture'
    remove_list = []
    comp_list = [ 'test_spd_dep' ]
    def __init__(self, methodName='runTest'):
        self.impl_list = [ os.uname()[4] ]
        scatest.CodeGenTestCase.__init__(self, methodName)

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
            self.generateCode( impls=self.impl_list)

    def test_01_CompileCode(self):
        for lang in self.impl_dict:
            if lang in common.cpp_usage:
                for impl in self.impl_dict[lang]:
                    impl_name=impl['name']
                    if impl['id'] in self.impl_list:
                        self.compileProject(os.path.join(self.build_dir, (impl_name.split('/'))[0]))

    def test_02_EvaluateUnitTests(self):
        valid_keys = common.cpp_usage + common.java_usage + common.python_usage

        for lang in self.impl_dict:
            for impl in self.impl_dict[lang]:
                impl_id = impl['id']
                impl_name = impl['name']
                if impl['id'] in self.impl_list:
                    spd_file = os.path.join(self.build_dir, self.artifact_name+'.spd.xml')
                    retval = ossie.utils.testing.ScaComponentTestProgram(spd_file,
                                                                         module='test_'+self.artifact_name,
                                                                         impl=impl_id)
            for result in retval.results:
                self.assertEquals(len(result.errors), 0, msg='failed for ' + lang)
                self.assertEquals(len(result.failures), 0, msg='failed for ' + lang)
                self.assertNotEqual(result.testsRun, 0, msg='failed for ' + lang)

if __name__ == '__main__':
    unittest.main()
