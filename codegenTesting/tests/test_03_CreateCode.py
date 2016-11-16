import unittest, os, sys, commands, copy, shutil
from xml.dom.minidom import parse
import ossie.utils.testing
import lxml.etree as ET
import lxml.etree

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
    artifact_type = 'Component'
    remove_list = []
    comp_list = [ 'octaveTest1', 'bintest1' ]
    def __init__(self, methodName='runTest'):
        self.impl_list = [ os.uname()[4] ]
        scatest.CodeGenTestCase.__init__(self, methodName)

    def test_00_GenerateCode(self):
        # Copy project to build directory
        shutil.rmtree(self.build_dir, True)
        shutil.copytree(self.source_dir, self.build_dir)
        cdir=os.getcwd()
        if os.path.isfile(self.build_dir + "/generate.sh"):
            # Some components might have a generate script for testing package
            # generation.
            os.chdir(self.build_dir)
            (status,output) = commands.getstatusoutput("./generate.sh")
            self.assertEquals(
                status,
                0,
                msg='generate script failed' + '\n\n' + output)
        else:
            self.generateCode( impls=self.impl_list)
        os.chdir(cdir)

    def test_01_CheckGeneratedCode(self):
        implref=None
        try:
            rnode=ET.ElementTree(file=self.build_dir+'/'+self.artifact_name+'.spd.xml').getroot()
            spr=rnode.findall('.//softpkgref')
            for x in spr[0].getchildren():
                if x.tag == 'implref':
                    implref=x.attrib
        except:
            pass

        self.assertEquals( implref, {'refid':'cpp'})


if __name__ == '__main__':
    unittest.main()
