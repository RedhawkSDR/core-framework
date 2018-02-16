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

from _unitTestHelpers import scatest
from ossie.utils import sb, redhawk
import unittest, contextlib, time, os
from ossie.cf import CF

@scatest.requireLog4cxx
class CppHierarchicalDomainLogging(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        redhawk.setTrackApps(False)

    def test_application_cpp_access(self):
        self.cname = "logger"
        self.applicationAccess("/waveforms/logger_w/logger_w.sad.xml")

    def test_application_py_access(self):
        self.cname = "logger_py"
        self.applicationAccess("/waveforms/logger_py_w/logger_py_w.sad.xml")

    @scatest.requireJava
    def test_application_java_access(self):
        self.cname = "logger_java"
        self.applicationAccess("/waveforms/logger_java_w/logger_java_w.sad.xml")

    def applicationAccess(self, sadfile):
        # Automatically clean up
        redhawk.setTrackApps(True)
        # Create Application from $SDRROOT path
        app = self._rhDom.createApplication(sadfile)
        loggers = app.getNamedLoggers()

        orig_loggers = {}
        orig_loggers[self.cname+'_1'] = app.getLogLevel(self.cname+'_1')
        orig_loggers[self.cname+'_1.lower'] = app.getLogLevel(self.cname+'_1.lower')
        orig_loggers[self.cname+'_1.namespace.lower'] = app.getLogLevel(self.cname+'_1.namespace.lower')
        orig_loggers[self.cname+'_1.user.more_stuff'] = app.getLogLevel(self.cname+'_1.user.more_stuff')
        orig_loggers[self.cname+'_1.user.some_stuff'] = app.getLogLevel(self.cname+'_1.user.some_stuff')

        self.assertTrue(self.cname+'_1' in loggers)
        self.assertTrue(self.cname+'_1.lower' in loggers)
        self.assertTrue(self.cname+'_1.namespace.lower' in loggers)
        self.assertTrue(self.cname+'_1.system.PortSupplier' in loggers)
        self.assertTrue(self.cname+'_1.system.PropertySet' in loggers)
        self.assertTrue(self.cname+'_1.system.Resource' in loggers)
        self.assertTrue(self.cname+'_1.user.more_stuff' in loggers)
        self.assertTrue(self.cname+'_1.user.some_stuff' in loggers)

        self.assertTrue(self.cname+'_2' in loggers)
        self.assertTrue(self.cname+'_2.lower' in loggers)
        self.assertTrue(self.cname+'_2.namespace.lower' in loggers)
        self.assertTrue(self.cname+'_2.system.PortSupplier' in loggers)
        self.assertTrue(self.cname+'_2.system.PropertySet' in loggers)
        self.assertTrue(self.cname+'_2.system.Resource' in loggers)
        self.assertTrue(self.cname+'_2.user.more_stuff' in loggers)
        self.assertTrue(self.cname+'_2.user.some_stuff' in loggers)

        self.assertRaises(CF.UnknownIdentifier, app.setLogLevel, self.cname+'_1.foo', 'all')
        self.assertRaises(CF.UnknownIdentifier, app.getLogLevel, self.cname+'_1.foo')

        app.setLogLevel(self.cname+'_1', 'all')
        self.assertEquals(app.getLogLevel(self.cname+'_1'), CF.LogLevels.ALL)
        self.assertEquals(app.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.ALL)
        self.assertEquals(app.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.ALL)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.ALL)
        app.setLogLevel(self.cname+'_1', 'off')
        self.assertEquals(app.getLogLevel(self.cname+'_1'), CF.LogLevels.OFF)
        self.assertEquals(app.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.OFF)
        self.assertEquals(app.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.OFF)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.OFF)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.OFF)

        # break the level inheritance
        app.setLogLevel(self.cname+'_1.user', 'trace')
        app.setLogLevel(self.cname+'_1', 'all')
        self.assertEquals(app.getLogLevel(self.cname+'_1'), CF.LogLevels.ALL)
        self.assertEquals(app.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.ALL)
        self.assertEquals(app.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

        # set the log with a value rather than the string
        app.setLogLevel(self.cname+'_1', CF.LogLevels.DEBUG)
        self.assertEquals(app.getLogLevel(self.cname+'_1'), CF.LogLevels.DEBUG)
        self.assertEquals(app.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.DEBUG)
        self.assertEquals(app.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.DEBUG)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
        self.assertEquals(app.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

        app.resetLog()
        self.assertEquals(orig_loggers[self.cname+'_1'], app.getLogLevel(self.cname+'_1'))
        self.assertEquals(orig_loggers[self.cname+'_1.lower'], app.getLogLevel(self.cname+'_1.lower'))
        self.assertEquals(orig_loggers[self.cname+'_1.namespace.lower'], app.getLogLevel(self.cname+'_1.namespace.lower'))
        self.assertEquals(orig_loggers[self.cname+'_1.user.more_stuff'], app.getLogLevel(self.cname+'_1.user.more_stuff'))
        self.assertEquals(orig_loggers[self.cname+'_1.user.some_stuff'], app.getLogLevel(self.cname+'_1.user.some_stuff'))

        # verify that inheritance is re-established
        app.setLogLevel(self.cname+'_1', 'all')
        self.assertEquals(CF.LogLevels.ALL, app.getLogLevel(self.cname+'_1'))
        self.assertEquals(CF.LogLevels.ALL, app.getLogLevel(self.cname+'_1.lower'))
        self.assertEquals(CF.LogLevels.ALL, app.getLogLevel(self.cname+'_1.namespace.lower'))
        self.assertEquals(CF.LogLevels.ALL, app.getLogLevel(self.cname+'_1.user.more_stuff'))
        self.assertEquals(CF.LogLevels.ALL, app.getLogLevel(self.cname+'_1.user.some_stuff'))

def all_log_levels(_obj):
    _obj.comp = sb.launch(_obj.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
    _obj.comp.start()
    # make sure that all the named loggers appear
    loggers = _obj.comp.getNamedLoggers()
    _obj.assertTrue(_obj.cname+'_1' in loggers)
    _obj.assertTrue(_obj.cname+'_1.lower' in loggers)
    _obj.assertTrue(_obj.cname+'_1.namespace.lower' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.more_stuff' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.some_stuff' in loggers)
    _obj.assertTrue(_obj.cname+'_1.system.PortSupplier' in loggers)
    _obj.assertTrue(_obj.cname+'_1.system.PropertySet' in loggers)
    _obj.assertTrue(_obj.cname+'_1.system.Resource' in loggers)
    _obj.assertEquals(len(loggers),8)

    # verify that the logger level is inherited
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    time.sleep(0.5)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.ALL)
    _obj.comp.setLogLevel(_obj.cname+'_1', 'off')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.OFF)

    # make sure that the log content is correct
    content = _obj.readLogFile('foo/bar/test.log')
    find_1 = content.find('message from _log')
    find_2 = content.find('message from baseline_1_logger')
    find_3 = content.find('message from baseline_2_logger')
    find_4 = content.find('message from namespaced_logger')
    find_5 = content.find('message from basetree_logger')
    _obj.assertTrue(find_1<find_2)
    _obj.assertTrue(find_2<find_3)
    _obj.assertTrue(find_3<find_4)
    _obj.assertTrue(find_4<find_5)

    # verify that the loggers are off
    time.sleep(0.5)
    content_again = _obj.readLogFile('foo/bar/test.log')
    _obj.assertEquals(len(content), len(content_again))

    # break the level inheritance
    _obj.comp.setLogLevel(_obj.cname+'_1.user', 'trace')
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

    # set the log with a value rather than the string
    _obj.comp.setLogLevel(_obj.cname+'_1', CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

    _obj.comp.stop()

def reset_logger(_obj):
    _obj.comp = sb.launch(_obj.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
    _obj.comp.start()
    # make sure that all the named loggers appear
    loggers = _obj.comp.getNamedLoggers()
    _obj.assertTrue(_obj.cname+'_1' in loggers)
    _obj.assertTrue(_obj.cname+'_1.lower' in loggers)
    _obj.assertTrue(_obj.cname+'_1.namespace.lower' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.more_stuff' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.some_stuff' in loggers)
    orig_loggers = {}

    orig_loggers[_obj.cname+'_1'] = _obj.comp.getLogLevel(_obj.cname+'_1')
    orig_loggers[_obj.cname+'_1.lower'] = _obj.comp.getLogLevel(_obj.cname+'_1.lower')
    orig_loggers[_obj.cname+'_1.namespace.lower'] = _obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower')
    orig_loggers[_obj.cname+'_1.user.more_stuff'] = _obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff')
    orig_loggers[_obj.cname+'_1.user.some_stuff'] = _obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff')

    # verify that the logger level is inherited
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    time.sleep(0.5)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.ALL)
    _obj.comp.setLogLevel(_obj.cname+'_1', 'off')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.OFF)

    # break the level inheritance
    _obj.comp.setLogLevel(_obj.cname+'_1.user', 'trace')
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

    # verify that the levels are reset back to their original level
    _obj.comp.resetLog()
    _obj.assertEquals(orig_loggers[_obj.cname+'_1'], _obj.comp.getLogLevel(_obj.cname+'_1'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.lower'], _obj.comp.getLogLevel(_obj.cname+'_1.lower'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.namespace.lower'], _obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.user.more_stuff'], _obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.user.some_stuff'], _obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'))

    # verify that inheritance is re-established
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1'))
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1.lower'))
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'))
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'))
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'))

    _obj.comp.stop()

def single_log_level(_obj):
    _obj.comp = sb.launch(_obj.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
    _obj.comp.start()
    loggers = _obj.comp.getNamedLoggers()
    _obj.assertRaises(Exception, _obj.comp.setLogLevel, _obj.cname+'_1.user.more_stuff', 'hello')
    _obj.comp.setLogLevel(_obj.cname+'_1.user.more_stuff', 'all')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.ALL)
    time.sleep(0.5)
    _obj.comp.setLogLevel(_obj.cname+'_1.user.more_stuff', 'off')
    _obj.comp.setLogLevel(_obj.cname+'_1.user.some_stuff', 'all')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.ALL)
    time.sleep(0.5)
    _obj.comp.setLogLevel(_obj.cname+'_1.user.some_stuff', 'off')
    _obj.comp.setLogLevel(_obj.cname+'_1.user.more_stuff', 'all')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.ALL)
    time.sleep(0.5)
    _obj.comp.setLogLevel(_obj.cname+'_1.user.more_stuff', 'off')
    content = _obj.readLogFile('foo/bar/test.log')
    find_1 = content.find('message from baseline_1_logger')
    find_2 = content.find('message from baseline_2_logger')
    _obj.assertTrue(find_2<find_1)
    find_3 = content.find('message from baseline_2_logger', find_1)
    _obj.assertTrue(find_1<find_3)
    _obj.comp.stop()

def selective_log_setting(_obj):
    _obj.comp = sb.launch(_obj.cname, instanceName='logger_1', properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/hierarchical_log.cfg'})
    _obj.comp.start()
    loggers = _obj.comp.getNamedLoggers()
    _obj.comp.setLogLevel('logger_1.user.more_stuff', 'all')
    _obj.assertEquals(_obj.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel('logger_1.user.some_stuff'), CF.LogLevels.WARN)
    _obj.assertEquals(_obj.comp.getLogLevel('logger_1.namespace.lower'), CF.LogLevels.TRACE)
    time.sleep(0.5)
    _obj.comp.stop()
    test_content = _obj.readLogFile('foo/bar/test.log')
    logger_test_content = _obj.readLogFile('logger_test.log')
    count_test = test_content.count('message from baseline_2_logger')
    count_newline = test_content.count('\n')
    count_logger_test = logger_test_content.count('message from namespaced_logger')
    count_logger_newline = logger_test_content.count('\n')
    _obj.assertEquals(count_test, count_newline)
    _obj.assertEquals(count_logger_test, count_logger_newline)

class PyHierarchicalLogging(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "logger_py"

    def readLogFile(self, filename):
        fp = open(filename,'r')
        stuff = fp.read()
        return stuff

    def tearDown(self):
        sb.release()

        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        try:
            os.remove('logger_test.log')
        except:
            pass

        # Try to clean up the event channel, if it was created
        context = None

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_py_all_log_levels(self):
        all_log_levels(self)

    def test_py_reset_logger(self):
        reset_logger(self)

    def test_py_single_log_level(self):
        single_log_level(self)

    def test_py_selective_log_setting(self):
        selective_log_setting(self)

@scatest.requireJava
class JavaHierarchicalLogging(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "logger_java"

    def readLogFile(self, filename):
        fp = open(filename,'r')
        stuff = fp.read()
        return stuff

    def tearDown(self):
        sb.release()

        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        try:
            os.remove('logger_test.log')
        except:
            pass

        # Try to clean up the event channel, if it was created
        context = None

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_java_all_log_levels(self):
        all_log_levels(self)

    def test_java_reset_logger(self):
        reset_logger(self)

    def test_java_single_log_level(self):
        single_log_level(self)

    def test_java_selective_log_setting(self):
        selective_log_setting(self)

@scatest.requireLog4cxx
class CppHierarchicalLogging(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "logger"

    def readLogFile(self, filename):
        fp = open(filename,'r')
        stuff = fp.read()
        return stuff

    def tearDown(self):
        sb.release()

        try:
            os.remove('foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('foo/bar')
        except:
            pass
        try:
            os.rmdir('foo')
        except:
            pass
        try:
            os.remove('logger_test.log')
        except:
            pass

        # Try to clean up the event channel, if it was created
        context = None

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_all_log_levels(self):
        all_log_levels(self)

    def test_reset_logger(self):
        reset_logger(self)

    def test_single_log_level(self):
        single_log_level(self)

    def test_selective_log_setting(self):
        selective_log_setting(self)

if __name__ == "__main__":
  # Run the unittests
  unittest.main()
