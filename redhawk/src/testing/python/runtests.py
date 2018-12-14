#!/usr/bin/python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest
import sys
import getopt

from omniORB import CORBA

from ossie.utils.log4py import logging
import ossie.utils.log4py.config

class MultiTestLoader(unittest.TestLoader):
    """
    Extend the default TestLoader to support a list of modules, at least for
    the purposes of loadTestsFromName and loadTestsFromNames.
    """
    def loadTestsFromName(self, name, modules):
        if not isinstance(modules, list):
            return unittest.TestLoader.loadTestsFromName(self, name, modules)
        else:
            # Try all modules in order, returning the first one that has
            # matching tests
            for mod in modules:
                try:
                    return unittest.TestLoader.loadTestsFromName(self, name, mod)
                except AttributeError:
                    pass
            raise AttributeError("test '%s' not found" % (name,))

class TestProgram(object):
    def __init__(self, modules=None):
        if modules is None:
            self.modules = [sys.modules['__main__']]
        else:
            self.modules = modules
        self.verbosity = 1
        self.testRunner = None

        self.parseArgs(sys.argv[1:])
        self.createTests()
        self.runTests()

    def createTests(self):
        # Load tests, filtering by name (if arguments were given).
        loader = MultiTestLoader()
        if self.testNames:
            self.test = loader.loadTestsFromNames(self.testNames, self.modules)
        else:
            self.test = unittest.TestSuite()
            for mod in self.modules:
                self.test.addTests(loader.loadTestsFromModule(mod))

    def parseArgs(self, argv):
        import getopt
        short_options = 'vx'
        long_options = ['xunit', 'log-level=', 'log-config=', 'verbose']

        xunit = False
        log_level = None
        log_config = None
        options, args = getopt.getopt(argv, short_options, long_options)
        for opt, value in options:
            if opt in ('-v', '--verbose'):
                self.verbosity = 2
            elif opt in ('-x', '--xunit'):
                xunit = True
            elif opt == '--log-level':
                # Map from string names to Python levels (this does not appear to
                # be built into Python's logging module)
                log_level = ossie.utils.log4py.config._LEVEL_TRANS.get(value.upper(), None)
            elif opt == '--log-config':
                log_config = value


        # If requested, use XML output (but the module is non-standard, so it
        # may not be available).
        if xunit:
            try:
                import xmlrunner
                self.testRunner = xmlrunner.XMLTestRunner(verbosity=self.verbosity)
            except ImportError:
                print >>sys.stderr, 'WARNING: XML test runner module is not installed'
            except TypeError:
                # Maybe it didn't like the verbosity argument
                self.testRunner = xmlrunner.XMLTestRunner()

        # If a log4j configuration file was given, read it.
        if log_config:
            ossie.utils.log4py.config.fileConfig(log_config)
        else:
            # Set up a simple configuration that logs on the console.
            logging.basicConfig()

        # Apply the log level (can override config file).
        if log_level:
            logging.getLogger().setLevel(log_level)

        # Any additional arguments are test names
        self.testNames = args

    def runTests(self):
        # Many tests require CORBA, so initialize up front
        orb = CORBA.ORB_init()
        root_poa = orb.resolve_initial_references("RootPOA")
        manager = root_poa._get_the_POAManager()
        manager.activate()

        # Default: use text output.
        if not self.testRunner:
            self.testRunner = unittest.TextTestRunner(verbosity=self.verbosity)

        result = self.testRunner.run(self.test)

        orb.shutdown(True)

        sys.exit(not result.wasSuccessful())

main = TestProgram

if __name__ == '__main__':
    import os
    import glob
    import imp

    # Find all Python files in the current directory and import them, adding
    # their tests to the overall test suite.
    modules = []
    for filename in glob.glob('*.py'):
        modname, ext = os.path.splitext(filename)
        fd = None
        try:
            fd, fn, desc = imp.find_module(modname)
            mod = imp.load_module(modname, fd, fn, desc)
            modules.append(mod)
        finally:
            if fd:
                fd.close()

    main(modules)
