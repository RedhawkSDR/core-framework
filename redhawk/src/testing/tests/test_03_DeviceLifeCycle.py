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
from ossie.events import Subscriber
from ossie import properties
from omniORB import any as _any
import time
import Queue

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

        # Create an event channel to receive the device and service start/stop
        # messages (the name must match the findbys in the DCD), and connect a
        # subscriber
        eventMgr = domMgr._get_eventChannelMgr()
        channel = eventMgr.createForRegistrations('test_events')
        self._started = Queue.Queue()
        self._stopped = Queue.Queue()
        self._subscriber = Subscriber(channel, dataArrivedCB=self._messageReceived)

    def tearDown(self):
        self._subscriber.terminate()

        scatest.CorbaTestCase.tearDown(self)

    def _messageReceived(self, message):
        payload = message.value(CF._tc_Properties)
        if not payload:
            return
        for dt in payload:
            if dt.id == 'state_change':
                value = properties.props_to_dict(dt.value.value(CF._tc_Properties))
                identifier = value['state_change::identifier']
                if value['state_change::event'] == 'start':
                    self._started.put(identifier)
                elif value['state_change::event'] == 'stop':
                    self._stopped.put(identifier)

    def _verifyStartOrder(self, startorder):
        for identifier in startorder:
            try:
                received = self._started.get(timeout=1.0)
            except Queue.Empty:
                self.fail('Did not receive start message for ' + identifier)
            self.assertEqual(received, identifier)
        self.failUnless(self._started.empty(), msg='Too many start messages received')

    def _verifyStopOrder(self, startorder):
        for identifier in startorder[::-1]:
            try:
                received = self._stopped.get(timeout=1.0)
            except Queue.Empty:
                self.fail('Did not receive stop message for ' + identifier)
            self.assertEqual(received, identifier)
        self.failUnless(self._stopped.empty(), msg='Too many stop messages received')

    def test_StartOrder(self):
        """
        Test that device/service start order runs correctly
        """
        devBooter, devMgr = self.launchDeviceManager("/nodes/startorder_events/DeviceManager.dcd.xml")

        startorder = ('startorder_events:start_event_device_3',
                      'start_event_service_1',
                      'startorder_events:start_event_device_1')

        # Verify that start calls were received in the right order
        self._verifyStartOrder(startorder)

        # Check that the devices are started as expected
        for dev in devMgr._get_registeredDevices():
            dev_id = dev._get_identifier()
            expected = dev_id in startorder
            self.assertEqual(expected, dev._get_started(), msg='Device '+dev_id+' started state is incorrect')

        # Also services, if supported
        for svc in devMgr._get_registeredServices():
            expected = svc.serviceName in startorder
            if svc.serviceObject._is_a(CF.Resource._NP_RepositoryId):
                started = svc.serviceObject._narrow(CF.Resource)._get_started()
            else:
                started = False
            self.assertEqual(expected, started, msg='Service '+svc.serviceName+' started state is incorrect')

        # Shut down the node so that it stops all of the devices and services
        devMgr.shutdown()

        # Check that stop was called in the reverse order of start
        self._verifyStopOrder(startorder)

    def test_StartOrderException(self):
        """
        Test that the node continues along the device/service start order even
        if one of them throws an exception
        """
        devBooter, devMgr = self.launchDeviceManager("/nodes/startorder_fail/DeviceManager.dcd.xml")

        startorder = ('startorder_fail:start_event_device_1',
                      'startorder_fail:fail_device_1',
                      'startorder_fail:start_event_device_2')

        # Verify that start calls were received in the right order, and that
        # the device manager continued after the failing device
        self._verifyStartOrder(startorder)

        # Check that the devices are started as expected, with the device that
        # was configured to fail not started
        for dev in devMgr._get_registeredDevices():
            label = dev._get_label()
            expected = not label.startswith('fail_')
            self.assertEqual(expected, dev._get_started(), msg='Device '+label+' started state is incorrect')

        # Shut down the node so that it stops all of the devices and services
        devMgr.shutdown()

        # Check that stop was called in the reverse order of start
        self._verifyStopOrder(startorder)

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
