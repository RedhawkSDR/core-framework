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
import tempfile

def killDomain(name):
    pids = [pid for pid in os.listdir('/proc') if pid.isdigit()]
    for pid in pids:
        try:
            cmdline=open(os.path.join('/proc', pid, 'cmdline'), 'rb').read()
            if 'DOMAIN_NAME' in cmdline:
                if name in cmdline:
                    os.kill(int(pid), 2)
        except:
            pass

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
        try:
            os.remove('sdr/dom/waveforms/logger_overload_w/tmp.sad.xml')
        except:
            pass
        try:
            os.remove('sdr/dom/waveforms/logger_config/tmp.sad.xml')
        except:
            pass
        # need to let event service clean up event channels
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        redhawk.setTrackApps(False)

    def test_logconfiguri_application(self):
        self.cname = "logger"
        # Automatically clean up
        redhawk.setTrackApps(True)
        # Create Application from $SDRROOT path
        app_1 = self._rhDom.createApplication("/waveforms/logger_w/logger_w.sad.xml", initConfiguration={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
        self.assertEquals(app_1.getLogLevel('logger_1'), 30000)
        self.assertEquals(app_1.getLogLevel('logger_2'), 30000)
        loggers_1 = app_1.getNamedLoggers()
        app_2 = self._rhDom.createApplication("/waveforms/logger_w/logger_w.sad.xml")
        loggers_2 = app_2.getNamedLoggers()
        self.assertEquals(app_1.getLogLevel('logger_1'), 30000)
        self.assertEquals(app_1.getLogLevel('logger_2'), 30000)
        self.assertEquals(app_2.getLogLevel('logger_1'), 40000)
        self.assertEquals(app_2.getLogLevel('logger_2'), 40000)

    def test_logconfiguri_overload(self):
        self.cname = "logger"
        # Automatically clean up
        redhawk.setTrackApps(True)
        # Create Application from $SDRROOT path
        fp = open('sdr/dom/waveforms/logger_overload_w/logger_overload_w.sad.xml','r')
        sad_contents = fp.read()
        fp.close()
        sad_contents = sad_contents.replace('@@@CWD@@@', os.getcwd())
        fp = open('sdr/dom/waveforms/logger_overload_w/tmp.sad.xml','w')
        fp.write(sad_contents)
        fp.close()
        app_1 = self._rhDom.createApplication("/waveforms/logger_overload_w/tmp.sad.xml")
        self.assertEquals(app_1.getLogLevel('logger_2'), 30000)
        loggers_1 = app_1.getNamedLoggers()
        app_2 = self._rhDom.createApplication("/waveforms/logger_w/logger_w.sad.xml")
        loggers_2 = app_2.getNamedLoggers()
        self.assertEquals(app_1.getLogLevel('logger_2'), 30000)
        self.assertEquals(app_2.getLogLevel('logger_1'), 40000)

    def test_loggingconfig(self):
        self.cname = "logger"
        fp=open('./runtest.props','r')
        runtest_props = fp.read()
        fp.close()
        fp=open('./high_thresh.cfg','r')
        high_thresh_cfg = fp.read()
        fp.close()
        # Automatically clean up
        redhawk.setTrackApps(True)
        # Create Application from $SDRROOT path
        fp = open('sdr/dom/waveforms/logger_config/logger_config.sad.xml','r')
        sad_contents = fp.read()
        fp.close()
        sad_contents = sad_contents.replace('@@@CWD@@@', os.getcwd())
        fp = open('sdr/dom/waveforms/logger_config/tmp.sad.xml','w')
        fp.write(sad_contents)
        fp.close()
        app_1 = self._rhDom.createApplication("/waveforms/logger_config/tmp.sad.xml")
        self.assertEquals(app_1.getLogLevel('logger_1'), 0)
        self.assertEquals(app_1.getLogLevel('logger_2'), 50000)
        logger_1 = -1
        logger_2 = -1
        for comp_idx in range(len(app_1.comps)):
            if app_1.comps[comp_idx].instanceName == 'logger_1':
                logger_1 = comp_idx
                break
        if logger_1 == 0:
            logger_2 = 1
        if logger_1 == 1:
            logger_2 = 0
        self.assertNotEqual(logger_1, -1)
        self.assertNotEqual(logger_2, -1)
        self.assertEquals(app_1.comps[logger_1].getLogConfig(), high_thresh_cfg)
        self.assertEquals(app_1.comps[logger_2].getLogConfig(), runtest_props)

        loggers_1 = app_1.getNamedLoggers()
        app_2 = self._rhDom.createApplication("/waveforms/logger_config/tmp.sad.xml", initConfiguration={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
        loggers_2 = app_2.getNamedLoggers()
        self.assertEquals(app_2.getLogLevel('logger_1'), 30000)
        self.assertEquals(app_2.getLogLevel('logger_2'), 30000)
        self.assertEquals(app_2.comps[logger_1].getLogConfig(), high_thresh_cfg)
        self.assertEquals(app_2.comps[logger_2].getLogConfig(), high_thresh_cfg)
        app_1.start()
        time.sleep(1.5)

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

@scatest.requireLog4cxx
class ApplicationDomainLogging(scatest.CorbaTestCase):
    def setUp(self):
        self.tmpfile=tempfile.mktemp()
        devmgrs = ['test_ExecutableDevice_node']
        self._rhDom = redhawk.kickDomain(domain_name=scatest.getTestDomainName(),
                                         kick_device_managers=True,
                                         device_managers = devmgrs,
                                         stdout=self.tmpfile,
                                         detached=False)

        try:
            self.waitForDeviceManager(devmgrs[0])
        except:
            traceback.print_exc()
            pass
        self.assertEquals(len(self._rhDom._get_applications()), 0)

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        try:
            os.remove(self.tmpfile)
        except:
            pass

        try:
            self._rhDom.terminate()
        except:
            pass
        killDomain(scatest.getTestDomainName())
        # need to let event service clean up event channels
        # cycle period is 10 milliseconds
        time.sleep(0.1)

    def test_domain_hierarchy_all(self):
        self._rhDom.setLogLevel('DomainManager', 'all')
        self._rhDom.devMgrs[0].setLogLevel('DeviceManager', 'all')
        props = self._rhDom.query([])
        props = self._rhDom.devMgrs[0].query([])
        fp = open(self.tmpfile, 'r')
        output=fp.read()
        fp.close()
        self.assertTrue('DomainManager.PropertySet' in output)
        self.assertTrue('DeviceManager.PropertySet' in output)

    def test_domain_hierarchy(self):
        self._rhDom.setLogLevel('DomainManager', 'info')
        self._rhDom.devMgrs[0].setLogLevel('DeviceManager', 'info')
        props = self._rhDom.query([])
        props = self._rhDom.devMgrs[0].query([])
        fp = open(self.tmpfile, 'r')
        output=fp.read()
        fp.close()
        self.assertFalse('DomainManager.PropertySet' in output)
        self.assertFalse('DeviceManager.PropertySet' in output)

    def application_default_log(self, appname, compname):
        app = self._rhDom.createApplication(appname)
        app.setLogLevel(compname+'_1.user.more_stuff', 'all')
        app.start()
        begin_time = time.time()
        while time.time()-begin_time < 1.5:
            fp = open(self.tmpfile, 'r')
            output=fp.read()
            fp.close()
            if output.find(compname+'_1.user.more_stuff') != -1:
                break
            time.sleep(0.1)
        app.stop()
        self.assertNotEqual(output.find(compname+'_1.user.more_stuff'), -1)

    def test_application_default_log_cpp(self):
        self.application_default_log("/waveforms/logger_w/logger_w.sad.xml", 'logger')

    def test_application_default_log_py(self):
        self.application_default_log("/waveforms/logger_py_w/logger_py_w.sad.xml", 'logger_py')

    @scatest.requireJava
    def test_application_default_log_java(self):
        self.application_default_log("/waveforms/logger_java_w/logger_java_w.sad.xml", 'logger_java')


def all_log_levels(_obj):
    _obj.comp = sb.launch(_obj.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
    _obj.comp.start()
    # make sure that all the named loggers appear
    loggers = _obj.comp.getNamedLoggers()
    _obj.assertTrue(_obj.cname+'_1' in loggers)
    _obj.assertTrue(_obj.cname+'_1.lower' in loggers)
    _obj.assertTrue(_obj.cname+'_1.lower.second.first' in loggers)
    _obj.assertTrue(_obj.cname+'_1.lower.third' in loggers)
    _obj.assertTrue(_obj.cname+'_1.namespace.lower' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.more_stuff' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.some_stuff' in loggers)
    _obj.assertTrue(_obj.cname+'_1.system.PortSupplier' in loggers)
    _obj.assertTrue(_obj.cname+'_1.system.PropertySet' in loggers)
    _obj.assertTrue(_obj.cname+'_1.system.Resource' in loggers)
    for logger in loggers:
        _obj.assertTrue(_obj.cname+'_1' in logger)

    # verify that the logger level is inherited
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    time.sleep(0.5)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.third'), CF.LogLevels.ALL)
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
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.third'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

    # set the log with a value rather than the string
    _obj.comp.setLogLevel(_obj.cname+'_1', CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'), CF.LogLevels.DEBUG)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.third'), CF.LogLevels.DEBUG)
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
    _obj.assertTrue(_obj.cname+'_1.lower.second.first' in loggers)
    _obj.assertTrue(_obj.cname+'_1.lower.third' in loggers)
    _obj.assertTrue(_obj.cname+'_1.namespace.lower' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.more_stuff' in loggers)
    _obj.assertTrue(_obj.cname+'_1.user.some_stuff' in loggers)
    orig_loggers = {}

    orig_loggers[_obj.cname+'_1'] = _obj.comp.getLogLevel(_obj.cname+'_1')
    orig_loggers[_obj.cname+'_1.lower'] = _obj.comp.getLogLevel(_obj.cname+'_1.lower')
    orig_loggers[_obj.cname+'_1.lower.second.first'] = _obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first')
    orig_loggers[_obj.cname+'_1.lower.third'] = _obj.comp.getLogLevel(_obj.cname+'_1.lower.third')
    orig_loggers[_obj.cname+'_1.namespace.lower'] = _obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower')
    orig_loggers[_obj.cname+'_1.user.more_stuff'] = _obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff')
    orig_loggers[_obj.cname+'_1.user.some_stuff'] = _obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff')

    # verify that the logger level is inherited
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    time.sleep(0.5)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.third'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.ALL)
    _obj.comp.setLogLevel(_obj.cname+'_1', 'off')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.third'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.OFF)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.OFF)

    # break the level inheritance
    _obj.comp.setLogLevel(_obj.cname+'_1.user', 'trace')
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.lower.third'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
    _obj.assertEquals(_obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

    # verify that the levels are reset back to their original level
    _obj.comp.resetLog()
    _obj.assertEquals(orig_loggers[_obj.cname+'_1'], _obj.comp.getLogLevel(_obj.cname+'_1'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.lower'], _obj.comp.getLogLevel(_obj.cname+'_1.lower'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.lower.second.first'], _obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.lower.third'], _obj.comp.getLogLevel(_obj.cname+'_1.lower.third'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.namespace.lower'], _obj.comp.getLogLevel(_obj.cname+'_1.namespace.lower'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.user.more_stuff'], _obj.comp.getLogLevel(_obj.cname+'_1.user.more_stuff'))
    _obj.assertEquals(orig_loggers[_obj.cname+'_1.user.some_stuff'], _obj.comp.getLogLevel(_obj.cname+'_1.user.some_stuff'))

    # verify that inheritance is re-established
    _obj.comp.setLogLevel(_obj.cname+'_1', 'all')
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1'))
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1.lower'))
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1.lower.second.first'))
    _obj.assertEquals(CF.LogLevels.ALL, _obj.comp.getLogLevel(_obj.cname+'_1.lower.third'))
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
    count_log4cxx_test = test_content.count('this is the log4cxx logger')
    count_java_eventchannel_messages = test_content.count('Unable to resolve EventChannelManager')
    count_logger_test = logger_test_content.count('message from namespaced_logger')
    count_logger_newline = logger_test_content.count('\n')
    _obj.assertEquals(count_test, count_newline-count_log4cxx_test-count_java_eventchannel_messages)
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

    def test_access_log4cxx(self):
        self.comp = sb.launch(self.cname, properties={'LOGGING_CONFIG_URI':'file://'+os.getcwd()+'/high_thresh.cfg'})
        self.comp.start()
        loggers = self.comp.getNamedLoggers()
        self.assertTrue(self.cname+'_1.l4.access' in loggers)
        self.comp.setLogLevel(self.cname+'_1.l4.access', 'info')
        time.sleep(0.5)
        self.comp.stop()
        content = self.readLogFile('foo/bar/test.log')
        self.assertNotEqual(content.find('this is the log4cxx logger'), -1)

    def test_all_log_levels(self):
        all_log_levels(self)

    def test_reset_logger(self):
        reset_logger(self)

    def test_single_log_level(self):
        single_log_level(self)

    def test_selective_log_setting(self):
        selective_log_setting(self)

@scatest.requireLog4cxx
class CppDeviceHierarchicalLogging(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "log_test_cpp"

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

    def test_cpp_dev_all_log_levels(self):
        all_log_levels(self)

    def test_cpp_dev_reset_logger(self):
        reset_logger(self)

    def test_cpp_dev_single_log_level(self):
        single_log_level(self)

    def test_cpp_dev_selective_log_setting(self):
        selective_log_setting(self)

@scatest.requireJava
class JavaDeviceHierarchicalLogging(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "log_test_java"

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

    def test_java_dev_all_log_levels(self):
        all_log_levels(self)

    def test_java_dev_reset_logger(self):
        reset_logger(self)

    def test_java_dev_single_log_level(self):
        single_log_level(self)

    def test_java_dev_selective_log_setting(self):
        selective_log_setting(self)

class PyDeviceHierarchicalLogging(scatest.CorbaTestCase):
    def setUp(self):
        self.cname = "log_test_py"

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

    def test_py_dev_all_log_levels(self):
        all_log_levels(self)

    def test_py_dev_reset_logger(self):
        reset_logger(self)

    def test_py_dev_single_log_level(self):
        single_log_level(self)

    def test_py_dev_selective_log_setting(self):
        selective_log_setting(self)

@scatest.requireLog4cxx
class CppDeviceHierarchicalDomainLogging(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        try:
            os.remove('sdr/dev/nodes/log_test_cpp_override_node/tmp.dcd.xml')
        except:
            pass

    def test_domMgr_log_level(self):
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        current_level = self._rhDom._get_log_level()
        self.assertNotEqual(current_level, 5000)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.AllocationManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.ConnectionManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.File'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.FileManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.proputils'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.EventChannelManager'),current_level)
        self._rhDom._set_log_level(5000)
        current_level = self._rhDom._get_log_level()
        self.assertEquals(self._rhDom.getLogLevel('DomainManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.AllocationManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.ConnectionManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.File'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.FileManager'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.proputils'),current_level)
        self.assertEquals(self._rhDom.getLogLevel('DomainManager.EventChannelManager'),current_level)

    def test_devMgr_cpp_access(self):
        self.cname = "log_test_cpp"
        self.devMgrAccess("/nodes/log_test_cpp_node/DeviceManager.dcd.xml")

    def test_devMgr_py_access(self):
        self.cname = "log_test_py"
        self.devMgrAccess("/nodes/log_test_py_node/DeviceManager.dcd.xml")

    @scatest.requireJava
    def test_devMgr_java_access(self):
        self.cname = "log_test_java"
        self.devMgrAccess("/nodes/log_test_java_node/DeviceManager.dcd.xml")

    def devMgrAccess(self, dcdfile):
        devBooter, self._devMgr = self.launchDeviceManager(dcdfile)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 1)
        # Create Application from $SDRROOT path
        devMgr = self._rhDom.devMgrs[0]
        loggers = devMgr.getNamedLoggers()

        orig_loggers = {}
        orig_loggers[self.cname+'_1'] = devMgr.getLogLevel(self.cname+'_1')
        orig_loggers[self.cname+'_1.lower'] = devMgr.getLogLevel(self.cname+'_1.lower')
        orig_loggers[self.cname+'_1.namespace.lower'] = devMgr.getLogLevel(self.cname+'_1.namespace.lower')
        orig_loggers[self.cname+'_1.user.more_stuff'] = devMgr.getLogLevel(self.cname+'_1.user.more_stuff')
        orig_loggers[self.cname+'_1.user.some_stuff'] = devMgr.getLogLevel(self.cname+'_1.user.some_stuff')

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

        self.assertRaises(CF.UnknownIdentifier, devMgr.setLogLevel, self.cname+'_1.foo', 'all')
        self.assertRaises(CF.UnknownIdentifier, devMgr.getLogLevel, self.cname+'_1.foo')

        devMgr.setLogLevel(self.cname+'_1', 'all')
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1'), CF.LogLevels.ALL)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.ALL)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.ALL)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.ALL)
        devMgr.setLogLevel(self.cname+'_1', 'off')
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1'), CF.LogLevels.OFF)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.OFF)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.OFF)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.OFF)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.OFF)

        # break the level inheritance
        devMgr.setLogLevel(self.cname+'_1.user', 'trace')
        devMgr.setLogLevel(self.cname+'_1', 'all')
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1'), CF.LogLevels.ALL)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.ALL)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.ALL)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

        # set the log with a value rather than the string
        devMgr.setLogLevel(self.cname+'_1', CF.LogLevels.DEBUG)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1'), CF.LogLevels.DEBUG)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.lower'), CF.LogLevels.DEBUG)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.namespace.lower'), CF.LogLevels.DEBUG)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.more_stuff'), CF.LogLevels.TRACE)
        self.assertEquals(devMgr.getLogLevel(self.cname+'_1.user.some_stuff'), CF.LogLevels.TRACE)

        devMgr.resetLog()
        self.assertEquals(orig_loggers[self.cname+'_1'], devMgr.getLogLevel(self.cname+'_1'))
        self.assertEquals(orig_loggers[self.cname+'_1.lower'], devMgr.getLogLevel(self.cname+'_1.lower'))
        self.assertEquals(orig_loggers[self.cname+'_1.namespace.lower'], devMgr.getLogLevel(self.cname+'_1.namespace.lower'))
        self.assertEquals(orig_loggers[self.cname+'_1.user.more_stuff'], devMgr.getLogLevel(self.cname+'_1.user.more_stuff'))
        self.assertEquals(orig_loggers[self.cname+'_1.user.some_stuff'], devMgr.getLogLevel(self.cname+'_1.user.some_stuff'))

        # verify that inheritance is re-established
        devMgr.setLogLevel(self.cname+'_1', 'all')
        self.assertEquals(CF.LogLevels.ALL, devMgr.getLogLevel(self.cname+'_1'))
        self.assertEquals(CF.LogLevels.ALL, devMgr.getLogLevel(self.cname+'_1.lower'))
        self.assertEquals(CF.LogLevels.ALL, devMgr.getLogLevel(self.cname+'_1.namespace.lower'))
        self.assertEquals(CF.LogLevels.ALL, devMgr.getLogLevel(self.cname+'_1.user.more_stuff'))
        self.assertEquals(CF.LogLevels.ALL, devMgr.getLogLevel(self.cname+'_1.user.some_stuff'))

    def test_devMgr_overload(self):
        fp = open('sdr/dev/nodes/log_test_cpp_override_node/DeviceManager.dcd.xml','r')
        dcd_contents = fp.read()
        fp.close()
        dcd_contents = dcd_contents.replace('@@@CWD@@@', os.getcwd())
        fp = open('sdr/dev/nodes/log_test_cpp_override_node/tmp.dcd.xml','w')
        fp.write(dcd_contents)
        fp.close()
        devBooter, self._devMgr = self.launchDeviceManager('/nodes/log_test_cpp_override_node/tmp.dcd.xml')
        devBooter_2, self._devMgr_2 = self.launchDeviceManager('/nodes/log_test_cpp_node/DeviceManager.dcd.xml')
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 2)
        # Create Application from $SDRROOT path
        _devMgr_o = None
        _devMgr = None
        for _d in self._rhDom.devMgrs:
            if _d.name == 'log_test_cpp_override_node':
                _devMgr_o = _d
            if _d.name == 'log_test_cpp_node':
                _devMgr = _d
        self.assertNotEqual(_devMgr, None)
        self.assertNotEqual(_devMgr_o, None)
        self.assertEquals(_devMgr_o.getLogLevel('log_test_cpp_1'), 40000)
        self.assertEquals(_devMgr_o.getLogLevel('log_test_cpp_2'), 30000)
        self.assertEquals(_devMgr.getLogLevel('log_test_cpp_1'), 40000)
        self.assertEquals(_devMgr.getLogLevel('log_test_cpp_2'), 40000)

    def test_devMgr_level_trace(self):
        devBooter, self._devMgr = self.launchDeviceManager('/nodes/node_device_deps/DeviceManager.dcd.xml.cpp', debug=5)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 1)
        self.assertEquals(self._rhDom.devMgrs[0]._get_log_level(), CF.LogLevels.ALL) # CF ALL and CF TRACE overlap for compatibility reasons
        self.assertEquals(self._rhDom.devMgrs[0].devs[0]._get_log_level(), CF.LogLevels.ALL)

    def test_devMgr_level_debug(self):
        devBooter, self._devMgr = self.launchDeviceManager('/nodes/node_device_deps/DeviceManager.dcd.xml.cpp', debug=4)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 1)
        self.assertEquals(self._rhDom.devMgrs[0]._get_log_level(), CF.LogLevels.DEBUG)
        self.assertEquals(self._rhDom.devMgrs[0].devs[0]._get_log_level(), CF.LogLevels.DEBUG)

    def test_devMgr_level_info(self):
        devBooter, self._devMgr = self.launchDeviceManager('/nodes/node_device_deps/DeviceManager.dcd.xml.cpp', debug=3)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 1)
        self.assertEquals(self._rhDom.devMgrs[0]._get_log_level(), CF.LogLevels.INFO)
        self.assertEquals(self._rhDom.devMgrs[0].devs[0]._get_log_level(), CF.LogLevels.INFO)

    def test_devMgr_level_warn(self):
        devBooter, self._devMgr = self.launchDeviceManager('/nodes/node_device_deps/DeviceManager.dcd.xml.cpp', debug=2)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 1)
        self.assertEquals(self._rhDom.devMgrs[0]._get_log_level(), CF.LogLevels.WARN)
        self.assertEquals(self._rhDom.devMgrs[0].devs[0]._get_log_level(), CF.LogLevels.WARN)

    def test_devMgr_level_error(self):
        devBooter, self._devMgr = self.launchDeviceManager('/nodes/node_device_deps/DeviceManager.dcd.xml.cpp', debug=1)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 1)
        self.assertEquals(self._rhDom.devMgrs[0]._get_log_level(), CF.LogLevels.ERROR)
        self.assertEquals(self._rhDom.devMgrs[0].devs[0]._get_log_level(), CF.LogLevels.ERROR)

    def test_devMgr_level_fatal(self):
        devBooter, self._devMgr = self.launchDeviceManager('/nodes/node_device_deps/DeviceManager.dcd.xml.cpp', debug=0)
        self._rhDom = redhawk.attach(scatest.getTestDomainName())
        self.assertEquals(len(self._rhDom.devMgrs), 1)
        self.assertEquals(self._rhDom.devMgrs[0]._get_log_level(), CF.LogLevels.FATAL)
        self.assertEquals(self._rhDom.devMgrs[0].devs[0]._get_log_level(), CF.LogLevels.FATAL)

if __name__ == "__main__":
  # Run the unittests
  unittest.main()
