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
from ossie.utils import sb
import unittest, contextlib, time, os
from ossie.cf import CF

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
        self.comp = sb.launch(self.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
        self.comp.start()
        # make sure that all the named loggers appear
        loggers = self.comp.getNamedLoggers()
        self.assertTrue('logger_1' in loggers)
        self.assertTrue('logger_1.lower' in loggers)
        self.assertTrue('logger_1.namespace.lower' in loggers)
        self.assertTrue('logger_1.user.more_stuff' in loggers)
        self.assertTrue('logger_1.user.some_stuff' in loggers)
        self.assertEquals(len(loggers),5)

        # verify that the logger level is inherited
        self.comp.setLogLevel('logger_1', 'all')
        time.sleep(0.5)
        self.assertEquals(self.comp.getLogLevel('logger_1'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.lower'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.namespace.lower'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.some_stuff'), CF.LogLevels.ALL)
        self.comp.setLogLevel('logger_1', 'off')
        self.assertEquals(self.comp.getLogLevel('logger_1'), CF.LogLevels.OFF)
        self.assertEquals(self.comp.getLogLevel('logger_1.lower'), CF.LogLevels.OFF)
        self.assertEquals(self.comp.getLogLevel('logger_1.namespace.lower'), CF.LogLevels.OFF)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.OFF)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.some_stuff'), CF.LogLevels.OFF)

        # make sure that the log content is correct
        content = self.readLogFile('foo/bar/test.log')
        find_1 = content.find('message from _log')
        find_2 = content.find('message from baseline_1_logger')
        find_3 = content.find('message from baseline_2_logger')
        find_4 = content.find('message from namespaced_logger')
        find_5 = content.find('message from basetree_logger')
        self.assertTrue(find_1<find_2)
        self.assertTrue(find_2<find_3)
        self.assertTrue(find_3<find_4)
        self.assertTrue(find_4<find_5)

        # verify that the loggers are off
        time.sleep(0.5)
        content_again = self.readLogFile('foo/bar/test.log')
        self.assertEquals(len(content), len(content_again))

        # break the level inheritance
        self.comp.setLogLevel('logger_1.user', 'trace')
        self.comp.setLogLevel('logger_1', 'all')
        self.assertEquals(self.comp.getLogLevel('logger_1'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.lower'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.namespace.lower'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.TRACE)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.some_stuff'), CF.LogLevels.TRACE)

        # set the log with a value rather than the string
        self.comp.setLogLevel('logger_1', CF.LogLevels.DEBUG)
        self.assertEquals(self.comp.getLogLevel('logger_1'), CF.LogLevels.DEBUG)
        self.assertEquals(self.comp.getLogLevel('logger_1.lower'), CF.LogLevels.DEBUG)
        self.assertEquals(self.comp.getLogLevel('logger_1.namespace.lower'), CF.LogLevels.DEBUG)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.TRACE)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.some_stuff'), CF.LogLevels.TRACE)

        self.comp.stop()

    def test_single_log_level(self):
        self.comp = sb.launch(self.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
        self.comp.start()
        loggers = self.comp.getNamedLoggers()
        self.assertRaises(Exception, self.comp.setLogLevel, 'logger_1.user.more_stuff', 'hello')
        self.comp.setLogLevel('logger_1.user.more_stuff', 'all')
        self.assertEquals(self.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.ALL)
        time.sleep(0.5)
        self.comp.setLogLevel('logger_1.user.more_stuff', 'off')
        self.comp.setLogLevel('logger_1.user.some_stuff', 'all')
        self.assertEquals(self.comp.getLogLevel('logger_1.user.some_stuff'), CF.LogLevels.ALL)
        time.sleep(0.5)
        self.comp.setLogLevel('logger_1.user.some_stuff', 'off')
        self.comp.setLogLevel('logger_1.user.more_stuff', 'all')
        self.assertEquals(self.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.ALL)
        time.sleep(0.5)
        self.comp.setLogLevel('logger_1.user.more_stuff', 'off')
        content = self.readLogFile('foo/bar/test.log')
        find_1 = content.find('message from baseline_1_logger')
        find_2 = content.find('message from baseline_2_logger')
        self.assertTrue(find_2<find_1)
        find_3 = content.find('message from baseline_2_logger', find_1)
        self.assertTrue(find_1<find_3)
        self.comp.stop()

    def test_selective_log_setting(self):
        self.comp = sb.launch(self.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/hierarchical_log.cfg'})
        self.comp.start()
        loggers = self.comp.getNamedLoggers()
        self.comp.setLogLevel('logger_1.user.more_stuff', 'all')
        self.assertEquals(self.comp.getLogLevel('logger_1.user.more_stuff'), CF.LogLevels.ALL)
        self.assertEquals(self.comp.getLogLevel('logger_1.user.some_stuff'), CF.LogLevels.WARN)
        self.assertEquals(self.comp.getLogLevel('logger_1.namespace.lower'), CF.LogLevels.TRACE)
        time.sleep(0.5)
        self.comp.stop()
        test_content = self.readLogFile('foo/bar/test.log')
        logger_test_content = self.readLogFile('logger_test.log')
        count_test = test_content.count('message from baseline_2_logger')
        count_newline = test_content.count('\n')
        count_logger_test = logger_test_content.count('message from namespaced_logger')
        count_logger_newline = logger_test_content.count('\n')
        self.assertEquals(count_test, count_newline)
        self.assertEquals(count_logger_test, count_logger_newline)

if __name__ == "__main__":
  # Run the unittests
  unittest.main()
