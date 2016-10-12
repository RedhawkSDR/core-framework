#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file 
# distributed with this source distribution.
# 
# This file is part of REDHAWK core.
# 
# REDHAWK core is free software: you can redistribute it and/or modify it under 
# the terms of the GNU Lesser General Public License as published by the Free 
# Software Foundation, either version 3 of the License, or (at your option) any 
# later version.
# 
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License 
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import commands
import unittest
import imp
import os
import re
import sys

if os.path.abspath(os.path.dirname(__file__)) != os.getcwd():
    print "runtests.py *must* be run from within the testing folder"
    sys.exit(-1)

def prependPythonPath(thepath):
    theabspath = os.path.abspath(thepath)
    sys.path.insert(0, theabspath)
    # And set the PythonPath for any processes that are spawned
    if os.environ.has_key('PYTHONPATH'):
        os.environ['PYTHONPATH'] = "%s:%s" % (theabspath, os.environ['PYTHONPATH']) 
    else:
        os.environ['PYTHONPATH'] = "%s" % (theabspath) 


if sys.hexversion < 0x020400F0:
    # Python 2.3 requires us to import the backports module
    prependPythonPath("../base/framework/python/backports")

# Point to the testing SDR folder
os.environ['SDRROOT'] = os.path.join(os.getcwd(), "sdr")
# Bring in the Python OSSIE stuff for things runing in this process

prependPythonPath("../base/framework/python")

def loadModule(filename):
    if filename == '':
        raise RuntimeError, 'Empty filename cannot be loaded'
    print "Loading module %s" % (filename)
    searchPath, file = os.path.split(filename)
    if not searchPath in sys.path: 
        sys.path.append(searchPath)
        sys.path.append(os.path.normpath(searchPath+"/../"))
    moduleName, ext = os.path.splitext(file)
    fp, pathName, description = imp.find_module(moduleName, [searchPath,])

    try:
        module = imp.load_module(moduleName, fp, pathName, description)
    finally:
        if fp:
            fp.close()

    return module

class PromptTestLoader(unittest.TestLoader):
    PROMPT = False

    def loadTestsFromTestCase(self, testCaseClass):
        """Return a suite of all tests cases contained in testCaseClass"""
        if issubclass(testCaseClass, unittest.TestSuite):
            raise TypeError("Test cases should not be derived from TestSuite. Maybe you meant to derive from TestCase?")
        testCaseNames = self.getTestCaseNames(testCaseClass)
        if not testCaseNames and hasattr(testCaseClass, 'runTest'):
            testCaseNames = ['runTest']
        if self.PROMPT:
            ans = raw_input("Would you like to execute all tests in %s [Y]/N? " % testCaseClass).upper()
            if ans == "N":
                testCaseNamesToRun = []
                for name in testCaseNames:
                    ans = raw_input("Would you like to execute test %s [Y]/N? " % name).upper()
                    if ans == "N":
                        continue
                    else:
                        testCaseNamesToRun.append(name)
                testCaseNames = testCaseNamesToRun
                    
        return self.suiteClass(map(testCaseClass, testCaseNames))

class TestCollector(unittest.TestSuite):
    def __init__(self, files, testMethodPrefix, prompt=True):
        unittest.TestSuite.__init__(self)
        self.__files = files
        self.__prompt = prompt
        self.__testMethodPrefix = testMethodPrefix
        self.loadTests()

    def setTestCaseRe(self, testCaseRe):
        self.__tcRe = re.compile(testCaseRe)

    def loadTests(self):
        for file in self.__files:
            mod = loadModule(file)
            candidates = dir(mod)
            for candidate in candidates:
                candidate = getattr(mod, candidate)
                try:
                    if issubclass(candidate, unittest.TestCase):
                        print "LOADING"
                        loader = PromptTestLoader()
                        loader.PROMPT = self.__prompt
                        loader.testMethodPrefix = self.__testMethodPrefix
                        self.addTest(loader.loadTestsFromTestCase(candidate))
                except TypeError, e:
                    pass

def getUnitTestFiles(rootpath, testFileGlob="test_*.py"):
    rootpath = os.path.normpath(rootpath) + "/"
    print "Searching for files in %s with prefix %s" % (rootpath, testFileGlob)
    test_files = commands.getoutput("find %s -name '%s'" % (rootpath, testFileGlob))
    files = test_files.split('\n')
    if files == ['']:
        files = []
    files.sort()
    return files

if __name__ == "__main__":
    from optparse import OptionParser
    sys.path.append("helpers")
    import scatest
    import xmlrunner

    parser = OptionParser()
    parser.add_option("--debug", dest="debug", help="run the unittest with the python debugger", default=False, action="store_true")
    parser.add_option("--prompt", dest="prompt", help="prompt for the set of tests to run", default=False, action="store_true")
    parser.add_option("--prefix", dest="prefix", help="only run tests with this prefix", default="test_")
    parser.add_option("--log4cxx", dest="logconfig", help="specify a new log4cxx config file", default="runtest.props")
    parser.add_option("--gdb", dest="gdb", help="debug nodebooter with gdb", default=False, action="store_true")
    parser.add_option("--gdb-file", dest="gdbfile", help="initial gdb commands", default=None)
    parser.add_option("--xml", dest="xmlfile", help="output to XML file", default=None)
    parser.add_option("--verbosity", dest="verbosity", help="verbosity of test output", default=2, type="int")
    parser.add_option("--valgrind", dest="valgrind", help="run nodebooter through valgrind", default=False, action="store_true")
    (options, args) = parser.parse_args()

    scatest.DEBUG_NODEBOOTER = options.gdb
    scatest.GDB_CMD_FILE = options.gdbfile
    scatest.VALGRIND = options.valgrind

    if len(args) == 0:
        files = getUnitTestFiles("tests")
    else:
        files = args

    if os.environ.has_key('OSSIEUNITTESTSLOGCONFIG'):
        ans = raw_input("OSSIEUNITTESTSLOGCONFIG already exists as %s. Do you want to continue [Y]/N? " % os.environ[OSSIEUNITTESTSLOGCONFIG]).upper()
        if ans == "N":
            sys.exit()
    else:
        os.environ['OSSIEUNITTESTSLOGCONFIG'] = os.path.abspath(options.logconfig)
                
    print ""
    print "Creating the Test Domain"
    print ""
    scatest.createTestDomain()

    print ""
    print "R U N N I N G  T E S T S"
    print "SDRROOT: ", scatest.getSdrPath()
    print ""

    suite = TestCollector(files, testMethodPrefix=options.prefix, prompt=options.prompt)

    if options.xmlfile == None:
        runner = unittest.TextTestRunner(verbosity=options.verbosity)
    else:
        stream = open(options.xmlfile, "w")
        runner = xmlrunner.XMLTestRunner(stream)
    if options.debug:
        import pdb
        pdb.run("runner.run(suite)")
    else:
        runner.run(suite)                                                               
