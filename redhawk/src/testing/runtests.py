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

# System imports
# NOTE: all REDHAWK framework imports must occur after prepending
# ../base/framework/python to the PYTHONPATH.
import commands
import unittest
import os
import re
import shutil
import sys
import time

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

def appendPath(varname, path):
    varpath = os.environ.get(varname, '').split(':')
    varpath.append(os.path.abspath(path))
    os.environ[varname] = ':'.join(varpath)

def appendClassPath(path):
    appendPath('CLASSPATH', path)

# Point to the testing SDR folder
os.environ['SDRROOT'] = os.path.join(os.getcwd(), "sdr")

# Point the SDR cache to a different location so that it's easy to clean/ignore
os.environ['SDRCACHE'] = os.path.join(os.environ['SDRROOT'], "cache")
shutil.rmtree(os.environ['SDRCACHE'], ignore_errors=True)

# Bring in the Python OSSIE stuff for things running in this process
prependPythonPath("../base/framework/python")

# Add Java libraries to CLASSPATH so that test components can find them
# regardless of where they run.
appendClassPath('../base/framework/java/CFInterfaces.jar')
appendClassPath('../base/framework/java/apache-commons-lang-2.4.jar')
appendClassPath('../base/framework/java/log4j-1.2.15.jar')
appendClassPath('../base/framework/java/ossie/ossie.jar')

# Add path to libomnijni.so to LD_LIBRARY_PATH for Java components
appendPath('LD_LIBRARY_PATH', '../omnijni/src/cpp/.libs')
appendPath('LD_LIBRARY_PATH', '../base/plugin/logcfg/.libs')

# Set the model IDL paths to point to the (uninstalled) REDHAWK IDLs.
from _unitTestHelpers import scatest
from _unitTestHelpers import runtestHelpers
from ossie.utils import model
from ossie.utils.idllib import IDLLibrary
model._idllib = IDLLibrary()
model._idllib.addSearchPath('../idl')

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
            mod = runtestHelpers.loadModule(file)
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

if __name__ == "__main__":
    from optparse import OptionParser
    sys.path.append("helpers")
    import xmlrunner

    parser = OptionParser()

    parser.add_option(
        "--debug",
        dest="debug",
        help="run the unittest with the python debugger",
        default=False,
        action="store_true")
    parser.add_option("--prompt",
        dest="prompt",
        help="prompt for the set of tests to run",
        default=False,
        action="store_true")
    parser.add_option(
        "--prefix",
        dest="prefix",
        help="only run tests with this prefix",
        default="test_")
    parser.add_option(
        "--log4cxx",
        dest="logconfig",
        help="specify a new log4cxx config file",
        default="runtest.props")
    parser.add_option(
        "--gdb",
        dest="gdb",
        help="debug nodebooter with gdb",
        default=False,
        action="store_true")
    parser.add_option(
        "--gdb-file",
        dest="gdbfile",
        help="initial gdb commands",
        default=None)
    parser.add_option(
        "--xml",
        dest="xmlfile",
        help="output to XML file",
        default=None)
    parser.add_option(
        "--verbosity",
        dest="verbosity",
        help="verbosity of test output",
        default=2,
        type="int")
    parser.add_option(
        "--debuglevel",
        dest="debuglevel",
        help="debug level to be passed to nodebooter",
        default=3)
    
    (options, args) = parser.parse_args()
    
    scatest.DEBUG_NODEBOOTER = options.gdb
    scatest.GDB_CMD_FILE = options.gdbfile

    if len(args) == 0:
        files = runtestHelpers.getUnitTestFiles("tests")
    else:
        files = args

    java_support = runtestHelpers.haveJavaSupport('../Makefile')
    log4cxx_support = runtestHelpers.haveLoggingSupport('../Makefile')
    if not java_support:
        if 'tests/test_05_JavaDevice.py' in files:
            files.remove('tests/test_05_JavaDevice.py')
        if 'tests/test_08_MessagingJava.py' in files:
            files.remove('tests/test_08_MessagingJava.py')
        if 'tests/test_11_JavaProperties.py' in files:
            files.remove('tests/test_11_JavaProperties.py')
    if not log4cxx_support:
        if 'tests/test_02_logging_config.py' in files:
            files.remove('tests/test_02_logging_config.py')
        if 'tests/test_15_LoggingConfig.py' in files:
            files.remove('tests/test_15_LoggingConfig.py')

    if os.environ.has_key('OSSIEUNITTESTSLOGCONFIG'):
        ans = raw_input("OSSIEUNITTESTSLOGCONFIG already exists as %s. Do you want to continue [Y]/N? " % os.environ[OSSIEUNITTESTSLOGCONFIG]).upper()
        if ans == "N":
            sys.exit()
    else:
        os.environ['OSSIEUNITTESTSLOGCONFIG'] = os.path.abspath(options.logconfig)

    from datetime import datetime
    test_start_time = datetime.now()

    print ""
    print "Creating the Test Domain"
    print ""
    scatest.createTestDomain()

    print ""
    print "R U N N I N G  T E S T S"
    print "SDRROOT: ", scatest.getSdrPath(), " Start:", test_start_time.strftime("%m/%d/%Y %H:%M:%S")
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

    test_end_time = datetime.now()
    dur=    test_end_time - test_start_time
    print "Completed Execution: End:", test_end_time.strftime("%m/%d/%Y %H:%M:%S"), " Duration: ", str(dur)

    
