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
import unittest,os,sys,imp,commands,copy
from optparse import OptionParser

sys.path.append("helpers")
from _unitTestHelpers import scatest
from _unitTestHelpers import runtestHelpers

# Create a symbolic link to $SDRROOT/dom/mgr so that the sandbox can find
# ComponentHost for shared library components
sdrroot = os.path.join(os.getcwd(), "sdr")
mgrpath = os.path.join(sdrroot, 'dom/mgr')
if os.path.exists(mgrpath):
    os.unlink(mgrpath)
os.symlink(os.path.join(os.environ['SDRROOT'], 'dom/mgr'), mgrpath)

# Point to the testing SDR folder
os.environ['SDRROOT'] = sdrroot
os.environ['CODEGENTESTHOME'] = os.path.join(os.getcwd())

def removeAll( src, items):
    retval=src
    if items  and type(items) == list:
        retval=[ x for x in src if x not in items ]
    else:
        try:
            retval.remove(items)
        except:
            pass
    return retval


class TestCollector(unittest.TestSuite):
    def __init__(self, files, testMethodPrefix, comp_name, lang, prompt=True, enableOctave=False):
        unittest.TestSuite.__init__(self)
        self.__files = files
        self.__prompt = prompt
        self.__testMethodPrefix = testMethodPrefix

        self.__comp_name = comp_name
        self.__lang = lang

        self.__enableOctave = enableOctave
        if type(enableOctave) == type(""):
            if enableOctave.lower() == "true":
                self.__enableOctave == True

        self.loadTests()

    def setTestCaseRe(self, testCaseRe):
        self.__tcRe = re.compile(testCaseRe)

    def getTestNames(self, rootpath):
        rootpath = os.path.normpath(rootpath) + "/"
        artifacts = commands.getoutput("ls %s" % rootpath)

        if artifacts == '':
            return None

        artifact_names = artifacts.split('\n')
        artifact_names.sort();
        return artifact_names

    def getArtifactDir(self, artifact_type):
        if artifact_type == 'Component':
            return 'sdr/dom/components'
        if artifact_type == 'Architecture':
            return 'sdr/dom/components'
        elif artifact_type == 'Device':
            return 'sdr/dev/devices'
        elif artifact_type == 'Service':
            return 'sdr/dev/services'
    ##
    ## all unit tests i.e. candidate objects need to include some class params
    ##   artifact_type - name to resolve with getArtifactDir
    ##   comp_list  - list of components to build
    ##   remove_list - list of directory name to ignore when searching the results of getArtifactDir
    ##   output_dir - when to place output of code generator
    ## 
    def loadTests(self):
        for file in self.__files:
            mod = runtestHelpers.loadModule(file)
            candidates = dir(mod)
            for candidate in candidates:
                candidate = getattr(mod, candidate)
                try:
                    if issubclass(candidate, unittest.TestCase):
                        loader = unittest.TestLoader()
                        loader.testMethodPrefix = self.__testMethodPrefix

                        artifact_dir = self.getArtifactDir(candidate.artifact_type)
                        candidate.language = self.__lang
                        if candidate.comp_list:
                            artifact_list = candidate.comp_list
                        else:
                            artifact_list = self.getTestNames(artifact_dir)

                        # temporarily remove test_spd_dep test until it is fix
                        artifact_list =removeAll(artifact_list, candidate.remove_list )
                        if not self.__enableOctave:
                            # If not testing octave, remove the octave test
                            # from the list.
                            #  TODO: intelligently get a list of all tests
                            # beginning with "octave"
                            artifact_list=removeAll(artifact_list, [ "octaveTest0", "octaveTest1"] )
                        if self.__comp_name:
                            if not (self.__comp_name in artifact_list):
                                artifact_list = []
                            else:
                                artifact_list = [self.__comp_name]

                        #change attributes for the loaded test classes
                        for curr in artifact_list:
                            test_list = loader.loadTestsFromTestCase(candidate)
                            for subtest in test_list:
                                subtest.artifact_name = curr
                                # Strip namespace to get the base name
                                subtest.base_name = curr.split('.')[-1]
                                subtest.artifact_dir = artifact_dir
                                subtest.artifact_module_directory = os.path.join(artifact_dir, '[NAME]/tests')
                                if self.__enableOctave:
                                    if 'octaveTest' in subtest.artifact_name:
                                        subtest.octave_test_dir=True
                            self.addTest(test_list)
                except TypeError, e:
                    pass

if __name__ == "__main__":
    parser = OptionParser()
    parser.add_option(
        "--prefix",
        dest="prefix",
        help="only run tests with this prefix",
        default="test_")

    parser.add_option(
        "--lang",
        dest="lang",
        help="only generate code and test this implementation (C++, Java, Python)",
        choices=['C++', 'Java', 'Python'])

    parser.add_option(
        "--verbosity",
        dest="verbosity",
        help="verbosity of test output",
        default=2,
        type="int")

    parser.add_option(
        "--comp",
        dest="comp",
        help="a specific component within the sdr to target",
        default=None)

    parser.add_option(
        "--enableOctave",
        dest="enableOctave",
        help="Whether or not to enable Octave component testing (required octave-devel to be installed)",
        default=False,
        choices=['True', 'False', 'true', "false"])

    parser.add_option(
        "--xmlfile",
        dest="xmlfile",
        default="nosetests.xml",
        help="output to XML file")

    (options, args) = parser.parse_args()

    if len(args) == 0:
        files = runtestHelpers.getUnitTestFiles("tests")
    else:
        files = args

    sys.argv=['']

    suite = TestCollector(
        files,
        testMethodPrefix=options.prefix,
        comp_name=options.comp,
        lang=options.lang,
        prompt=True,
        enableOctave=options.enableOctave)

    try:
        import xmlrunner
        stream = open(options.xmlfile, "w")
        runner = xmlrunner.XMLTestRunner(stream, verbosity=options.verbosity)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=options.verbosity)
    runner.run(suite)
