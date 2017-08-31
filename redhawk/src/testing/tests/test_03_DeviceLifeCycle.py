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

import unittest, os
from _unitTestHelpers import scatest
from test_01_DeviceManager import killChildProcesses
from ossie.utils import redhawk
from ossie.cf import CF
from omniORB import any as _any
import time

class DeviceLifeCycleTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

    def test_DeviceLifeCycle(self):
        q=os.walk('/proc')
        for root,dirs,files in q:
            all_dirs = root.split('/')
            num_slash = len(all_dirs)
            if num_slash>3:
                continue
            process_number = all_dirs[-1]
            if 'status' in files:
                fp=open(root+'/status','r')
                stuff=fp.read()
                fp.close()
                if 'GPP' in stuff:
                    print "Killing process "+process_number+" (presumably a GPP)"
                    os.kill(int(process_number),9)

    def test_DeviceLifeCycleNoKill(self):
        pass

class DeviceStartorder(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/devmgr_startorder/DeviceManager.dcd.xml", loggingURI='file://'+os.getcwd()+'/devmgr_config.cfg')

    def test_DeviceStarted(self):
        rhdom = redhawk.attach(scatest.getTestDomainName())
        startfrom = {}
        q=CF.DataType(id='start_from',value=_any.to_any(None))
        for dev in rhdom.devMgrs[0].devs:
            startfrom[dev._id] = dev.query([q])[0].value._v
        for svc in rhdom.devMgrs[0].services:
            startfrom[svc._id] = svc.query([q])[0].value._v
        self.assertEquals(startfrom['devmgr_startorder:dev_startorder_1'], ',devmgr_startorder:dev_startorder_2,svc_startorder_1')
        self.assertEquals(startfrom['devmgr_startorder:dev_startorder_2'], ',devmgr_startorder:dev_startorder_2,svc_startorder_1,devmgr_startorder:dev_startorder_1')
        self.assertEquals(startfrom['devmgr_startorder:dev_startorder_3'], '')
        self.assertEquals(startfrom['devmgr_startorder:svc_startorder_1'], ',devmgr_startorder:dev_startorder_2')

        rhdom.devMgrs[0].shutdown()

        fp = None
        try:
            fp = open('sdr/cache/.devmgr_startorder/foo/bar/test.log','r')
        except:
            pass
        if fp != None:
            log_contents = fp.read()
            fp.close()
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
            os.remove('sdr/cache/.devmgr_startorder/foo/bar/test.log')
        except:
            pass
        try:
            os.rmdir('sdr/cache/.devmgr_startorder/foo/bar')
        except:
            pass
        try:
            os.rmdir('sdr/cache/.devmgr_startorder/foo')
        except:
            pass
        start_dev_1 = log_contents.find('starting devmgr_startorder:dev_startorder_1')
        start_dev_2 = log_contents.find('starting devmgr_startorder:dev_startorder_2')
        start_dev_3 = log_contents.find('starting devmgr_startorder:dev_startorder_3')
        start_svc = log_contents.find('starting svc_startorder_1')
        stop_dev_1 = log_contents.find('stopping devmgr_startorder:dev_startorder_1')
        stop_dev_2 = log_contents.find('stopping devmgr_startorder:dev_startorder_2')
        stop_dev_3 = log_contents.find('stopping devmgr_startorder:dev_startorder_3')
        stop_svc = log_contents.find('stopping svc_startorder_1')

        self.assertTrue(start_dev_2<start_svc)
        self.assertTrue(start_svc<start_dev_1)
        self.assertEquals(start_dev_3, -1)
        self.assertTrue(stop_dev_1<stop_svc)
        self.assertTrue(stop_svc<stop_dev_2)
        self.assertTrue(stop_dev_2<stop_dev_3)

class DeviceDeviceManagerTest(scatest.CorbaTestCase):
    def setUp(self):
    
        cfg = "log4j.rootLogger=TRACE,CONSOLE,FILE\n" + \
            "log4j.debug=false\n" + \
            "# Direct log messages to FILE\n" + \
            "log4j.appender.CONSOLE=org.apache.log4j.ConsoleAppender\n" + \
            "log4j.appender.CONSOLE.File=stdout\n" + \
            "log4j.appender.FILE=org.apache.log4j.FileAppender\n" + \
            "log4j.appender.FILE.File="+os.getcwd()+"/tmp_logfile.log\n" + \
            "log4j.appender.CONSOLE.threshold=TRACE\n" + \
            "log4j.appender.FILE.threshold=TRACE\n" + \
            "log4j.appender.CONSOLE.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.CONSOLE.layout.ConversionPattern=%p:%c - %m [%F:%L]%n\n" + \
            "log4j.appender.FILE.layout=org.apache.log4j.PatternLayout\n" + \
            "log4j.appender.FILE.layout.ConversionPattern=%d %p:%c - %m [%F:%L]%n\n"
            
        fp = open('tmp_logfile.config','w')
        fp.write(cfg)
        fp.close()
        
        nodebooter, self._domMgr = self.launchDomainManager()
        self._domBooter = nodebooter

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        if os.path.exists('tmp_logfile.config'):
            os.remove('tmp_logfile.config')
        if os.path.exists('tmp_logfile.log'):
            os.remove('tmp_logfile.log')

        killChildProcesses(os.getpid())

    @scatest.requireLog4cxx
    def test_deviceKillDeviceManager(self):
        # This test requires log4cxx support because it checks the device
        # manager's log output
        devmgr_nb, devMgr = self.launchDeviceManager("/nodes/dev_kill_devmgr_node/DeviceManager.dcd.xml", loggingURI=os.getcwd()+'/tmp_logfile.config', wait=False)
        time.sleep(2)
        self.assertEquals(devMgr, None)
        fp = open('tmp_logfile.log', 'r')
        logcontents = fp.read()
        fp.close()
        self.assertNotEqual(logcontents.find('Unable to complete Device construction: CORBA'), -1)
