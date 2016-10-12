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

"""
REDHAWK-specific unit testing module, extending Python's built-in unittest
module. Supports running soft package (component/device/service) unit tests
against all implementations.
"""

import unittest
import types
import getopt
import inspect
import os

from ossie.parsers import SPDParser

__all__ = ('RHTestCase', 'RHTestLoader', 'RHTestProgram')


class RHTestCaseMeta(type):
    """
    Custom metaclass for REDHAWK unit tests to support testing of multiple
    implementations for components/devices/services. It works by overloading
    the __dir__ and __getattr__ methods so that when a TestLoader inspects a
    class object derived from RHTestCase (not an instance), it will see a
    version of each test method for each implementation.

    For example, if a component has two implementations 'A' and 'B', a test
    method on its RHTestCase class named 'testBasic' will show up twice, as
    'testBasic [A]' and 'testBasic [B]'. While these names are not normally
    valid as attribute names, since everything is done programatically it
    works as expected, and makes the implementation clear at run time.

    If the class has an attribute called 'SPD_FILE' on it, at the time the
    class object is created (i.e., when the class is parsed), it is assumed
    to be a relative path to the soft package's SPD file. The real path to
    the SPD is determined at this time and the SPD is parsed to get the
    implementations IDs. This real path is stored as a class attribute so
    that tests can locate their soft package irrespective of where the test
    script is run.
    """
    def __init__(self, name, bases, classdict):
        type.__init__(self, name, bases, classdict)
        spd_file = classdict.get('SPD_FILE', None)
        if spd_file:
            # Get the module that contains the test case class. This allows us
            # to find the SPD file relative to its location.
            module = inspect.getmodule(self)
            module_path = os.path.dirname(module.__file__)
            spd_path = os.path.normpath(os.path.join(module_path, spd_file))

            # Parse the SPD to get the implementation IDs.
            spd = SPDParser.parse(spd_path)
            self.__impls__ = [impl.get_id() for impl in spd.get_implementation()]

            # Update the path to the SPD file.
            self.spd_file = spd_path
        else:
            self.spd_file = None
            self.__impls__ = ()

    def __dir__(self):
        # Start with the base class's contents.
        names = set(dir(self.__base__))
        # For any non-special attribute that is callable (i.e., a function),
        # return a modified name for each implementation, where the name has
        # the implementation ID appended.
        for attr, value in self.__dict__.iteritems():
            if not attr.startswith('__') and callable(value):
                names.update(self.getMethodNames(attr))
            else:
                names.add(attr)
        return sorted(names)

    def __getattr__(self, name):
        # Split off the implementation, and check for the base name.
        base, impl = self.splitImpl(name)
        # If there's no implementation, or the implementation is invalid, don't
        # bother looking it up.
        if not impl or not impl in self.__impls__:
            raise AttributeError(name)
        return getattr(self, base)

    def addImpl(self, name, impl):
        """
        Mangles a name with the given implementation.
        """
        # NOTE: The name mangling scheme is entirely implemented between this
        # method and splitImpl().

        # Only perform name mangling if it's required--i.e., if there are
        # multiple implementations.
        if len(self.__impls__) > 1:
            return name + ' [' + impl + ']'
        else:
            return name

    def splitImpl(self, name):
        """
        Returns the base name and implementation, if 'name' has been mangled.
        Otherwise, returns 'name' and an empty string.
        """
        # NOTE: The name mangling scheme is entirely implemented between this
        # method and addImpl().

        if len(self.__impls__) <= 1:
            # Single implementation, name should not be mangled.
            return name, ''
        try:
            name, impl = name.split(' ')
            return name, impl[1:-1]
        except ValueError:
            return name, ''

    def getTestMethodNames(self, prefix, impl=None):
        """
        Returns the list of method names that start with 'prefix', mangled for
        all selected implementations. If 'impl' is given, only the mangled
        names for that implementation will be returned.
        """
        names = []
        for attr, value in self.__dict__.iteritems():
            if attr.startswith(prefix) and callable(value):
                names.extend(self.getMethodNames(attr, impl))
        return names

    def getMethodNames(self, name, impl=None):
        """
        For the given method name, returns the list of mangled names for all
        selected implementations. If 'impl' is given, and matches an existing
        implementation ID, the returned list only contains the method name
        for that implementation. In the case that 'impl' is not valid, the
        returned list is empty.
        """
        if impl is None:
            # Return methods names for all implementations.
            impls = self.__impls__
        elif impl in self.__impls__:
            # Only return the requested implementation.
            impls = [impl]
        else:
            # Non-matching implementation.
            return []
        return [self.addImpl(name, impl) for impl in impls]


class RHTestCase(unittest.TestCase):
    """
    Unit test base class for REDHAWK components, devices and services.

    Subclasses must define an 'SPD_FILE' attribute on the class that is the
    the relative path to the SPD file, e.g.,:

      class MyTest(RHUnitTestCase):
        SPD_FILE = "../my_component.spd.xml"
        ...

    When a test case is run, 'spd_file' and 'impl' will be set to the
    absolute path to the SPD and the selected implementation, respectively.
    These can be used in the setUp() method to launch the soft package:

       def setUp(self):
         self.comp = sb.launch(self.spd_file, impl=self.impl)
    """
    __metaclass__ = RHTestCaseMeta

    def __init__(self, methodName):
        # Pass the method name unmodified to the base class; this ensures that
        # when run in Eclipse's PyDev, the implementation shows up next to the
        # test name. This also means that __getattr__ must be overridden to
        # find the original method.
        unittest.TestCase.__init__(self, methodName)

        # Save the implementation so that it is available in setUp().
        # NOTE: Using type() ensures that the correct class object is queried,
        # so the implementation list is correct--a static call via RHTestCase
        # always returns an un-mangled name.
        name, impl = type(self).splitImpl(methodName)
        if not impl:
            # This should only occur with a single implementation, where name
            # mangling is disabled.
            self.impl = self.__impls__[0]
        else:
            self.impl = impl

    def __getattr__(self, name):
        # Regular lookup failed, so split off the implementation and try the
        # base name. If it still doesn't exist, __getattribute__ will throw an
        # AttributeError, so it won't get stuck in infinite recursion.
        # NOTE: Using type() ensures that the correct class object is queried,
        # so the implementation list is correct--a static call via RHTestCase
        # always returns an un-mangled name.
        base, impl = type(self).splitImpl(name)
        return unittest.TestCase.__getattribute__(self, base)

    def __dir__(self):
        # Because __dir__ is a protocol function, it will try to look it up in
        # the metaclass, but the behavior is kind of odd--it returns a bound
        # function, and then the instance is passed as a *second* argument.
        # Re-implementing it here enables normal use of dir() on instances.
        # Start with the base TestCase class, then add the class' dictionary
        # keys directly (so that the implementation-mangled names don't appear)
        # and finally the instance dictionary keys.
        names = set(dir(unittest.TestCase))
        names.update(type(self).__dict__.keys())
        names.update(self.__dict__.keys())
        return sorted(names)


class RHTestLoader(unittest.TestLoader):
    """
    Extended test loader to support implementation-based filtering for REDHAWK
    component unit tests.
    """
    def __init__(self, impl=None):
        unittest.TestLoader.__init__(self)
        self.impl = impl

    def getTestCaseNames(self, testCaseClass):
        if issubclass(testCaseClass, RHTestCase):
            return testCaseClass.getTestMethodNames(self.testMethodPrefix, self.impl)
        else:
            return unittest.TestLoader.getTestCaseNames(self, testCaseClass)

    def loadSpecificTestFromTestCase(self, testCaseClass, testName):
        if issubclass(testCaseClass, RHTestCase):
            testNames = testCaseClass.getMethodNames(testName, self.impl)
        else:
            testNames = [testName]
        return self.suiteClass([testCaseClass(name) for name in testNames])

    def _resolveName(self, name, module):
        # This was split out from the base class loadTestsFromName()
        parts = name.split('.')
        if module is None:
            parts_copy = parts[:]
            while parts_copy:
                try:
                    module = __import__('.'.join(parts_copy))
                    break
                except ImportError:
                    del parts_copy[-1]
                    if not parts_copy: raise
            parts = parts[1:]
        obj = module
        for part in parts:
            parent, obj = obj, getattr(obj, part)

        return parent, obj

    def loadTestsFromName(self, name, module=None):
        """Return a suite of all tests cases given a string specifier.

        The name may resolve either to a module, a test case class, a
        test method within a test case class, or a callable object which
        returns a TestCase or TestSuite instance.

        The method optionally resolves the names relative to a given module.
        """
        parent, obj = self._resolveName(name, module)

        if type(obj) == types.ModuleType:
            return self.loadTestsFromModule(obj)
        elif (isinstance(obj, (type, types.ClassType)) and
              issubclass(obj, unittest.TestCase)):
            return self.loadTestsFromTestCase(obj)
        elif (type(obj) == types.UnboundMethodType and
              isinstance(parent, (type, types.ClassType)) and
              issubclass(parent, unittest.TestCase)):
            return self.loadSpecificTestFromTestCase(parent, obj.__name__)
        elif isinstance(obj, unittest.TestSuite):
            return obj
        elif hasattr(obj, '__call__'):
            test = obj()
            if isinstance(test, unittest.TestSuite):
                return test
            elif isinstance(test, unittest.TestCase):
                return self.suiteClass([test])
            else:
                raise TypeError("calling %s returned %s, not a test" %
                                (obj, test))
        else:
            raise TypeError("don't know how to make test from: %s" % obj)


class RHTestProgram(unittest.TestProgram):
    def __init__(self, module='__main__', defaultTest=None, argv=None,
                 testRunner=None, impl=None):
        unittest.TestProgram.__init__(self, module=module,
                                      defaultTest=defaultTest, argv=argv,
                                      testRunner=testRunner,
                                      testLoader=RHTestLoader(impl))

    def parseArgs(self, argv):
        try:
            options, args = getopt.getopt(argv[1:], 'hHvqi:',
                                          ['help','verbose','quiet','impl='])
            for opt, value in options:
                if opt in ('-h','-H','--help'):
                    self.usageExit()
                if opt in ('-q','--quiet'):
                    self.verbosity = 0
                if opt in ('-v','--verbose'):
                    self.verbosity = 2
                if opt in ('-i','--impl'):
                    self.testLoader.impl = value
            if len(args) == 0 and self.defaultTest is None:
                self.test = self.testLoader.loadTestsFromModule(self.module)
                return
            if len(args) > 0:
                self.testNames = args
            else:
                self.testNames = (self.defaultTest,)
            self.createTests()
        except getopt.error, msg:
            self.usageExit(msg)

    def runTests(self):
        if self.testRunner is None:
            try:
                import xmlrunner
                self.testRunner = xmlrunner.XMLTestRunner(verbosity=self.verbosity)
            except ImportError:
                self.testRunner = unittest.TextTestRunner(verbosity=self.verbosity)

        unittest.TestProgram.runTests(self)
