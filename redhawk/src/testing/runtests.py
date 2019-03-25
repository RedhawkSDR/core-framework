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
# NOTE: all REDHAWK framework imports must occur after configureTestPaths()
import commands
import functools
import os
import re
import shutil
import sys
import time
import functools
import unittest

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

def prependPath(varname, path):
    varpath = os.environ.get(varname, '').split(':')
    varpath = [ os.path.abspath(path) ] + varpath
    os.environ[varname] = ':'.join(varpath)

def appendClassPath(path):
    appendPath('CLASSPATH', path)

def configureTestPaths():
    # Point to the testing SDR folder
    testdir = os.path.abspath(os.path.dirname(__file__))
    os.environ['SDRROOT'] = os.path.join(testdir, "sdr")

    # The top source directory is one levels up from this file
    topdir = os.path.abspath(os.path.join(testdir, '..'))

    # Bring in the Python OSSIE stuff for things running in this process
    prependPythonPath(os.path.join(topdir, 'base/framework/python'))

    # Add Java libraries to CLASSPATH so that test components can find them
    # regardless of where they run.
    jarfiles = [ 'CFInterfaces.jar', 'apache-commons-lang-2.4.jar',
                 'log4j-1.2.15.jar', 'ossie/ossie.jar' ]
    for jarfile in jarfiles:
        appendClassPath(os.path.join(topdir, 'base/framework/java', jarfile))

    #  Add path to libomnijni.so to LD_LIBRARY_PATH for Java components
    prependPath('LD_LIBRARY_PATH', os.path.join(topdir, 'base/framework/idl/.libs'))
    prependPath('LD_LIBRARY_PATH', os.path.join(topdir, 'base/framework/.libs'))
    appendPath('LD_LIBRARY_PATH', os.path.join(topdir, 'omnijni/src/cpp/.libs'))
    appendPath('LD_LIBRARY_PATH', os.path.join(topdir, 'base/plugin/logcfg/.libs'))

    # use nodeBooter in the current source tree path..
    prependPath('PATH', os.path.join(topdir, 'control/framework'))

    # Set the model IDL paths to point to the (uninstalled) REDHAWK IDLs.
    from ossie.utils import model
    from ossie.utils.idllib import IDLLibrary
    model._idllib = IDLLibrary()
    model._idllib.addSearchPath(os.path.join(topdir, 'idl'))
    model._idllib.addSearchPath(os.path.join(topdir, '../../bulkioInterfaces/idl'))

# Set up the system paths (LD_LIBRARY_PATH, PYTHONPATH, CLASSPATH), IDL paths
# and SDRROOT to allow testing against an uninstalled framework.
configureTestPaths()

# Point the SDR cache to a different location so that it's easy to clean/ignore
os.environ['SDRCACHE'] = os.path.join(os.environ['SDRROOT'], "cache")
shutil.rmtree(os.environ['SDRCACHE'], ignore_errors=True)

from _unitTestHelpers import runtestHelpers

class PromptTestLoader(unittest.TestLoader):
    PROMPT = False

    def getTestCaseNames(self, testCaseClass):
        """Return a sorted sequence of method names found within testCaseClass
        """
        def isTestMethod(attrname, testCaseClass=testCaseClass, prefix=self.testMethodPrefix):
            if not (attrname.startswith(prefix) and hasattr(getattr(testCaseClass, attrname), '__call__')):
                return False
            function = getattr(testCaseClass, attrname, None)
            if function:
                reason = getattr(function, 'skip_reason', False)
                if reason:
                    print "SKIPPING:  {0}.{1} - '{2}'".format(testCaseClass.__name__, function.__name__, reason)
                    return False
            return True
        testFnNames = filter(isTestMethod, dir(testCaseClass))
        if self.sortTestMethodsUsing:
            cmp_to_key = None
            if hasattr(unittest, '_CmpToKey'):  # Python 2.6
                cmp_to_key = unittest._CmpToKey
            elif hasattr(functools, 'cmp_to_key'):  # Python 2.7
                cmp_to_key = functools.cmp_to_key
            if cmp_to_key:
                testFnNames.sort(key=cmp_to_key(self.sortTestMethodsUsing))
            else:
                print 'Conversion function "cmp_to_key" not found.  Not sorting.'
        return testFnNames

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
                        reason = getattr(candidate, 'skip_reason', False)
                        if reason:
                            print "SKIPPING:  {0} - '{1}'".format(candidate.__name__, reason)
                            continue
                        print "LOADING"
                        loader = PromptTestLoader()
                        loader.PROMPT = self.__prompt
                        loader.testMethodPrefix = self.__testMethodPrefix
                        self.addTest(loader.loadTestsFromTestCase(candidate))
                except TypeError, e:
                    pass

if __name__ == "__main__":
    if os.path.abspath(os.path.dirname(__file__)) != os.getcwd():
        print "runtests.py *must* be run from within the testing folder"
        sys.exit(-1)

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
    
    from _unitTestHelpers import scatest
    scatest.DEBUG_NODEBOOTER = options.gdb
    scatest.GDB_CMD_FILE = options.gdbfile

    if len(args) == 0:
        files = runtestHelpers.getUnitTestFiles("tests")
    else:
        files = args

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

    
