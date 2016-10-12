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

import unittest
from _unitTestHelpers import scatest
from ossie.cf import CF
from omniORB import CORBA
import time
import os
from test_09_HardFail import getChildren

class DeviceManagerTest(scatest.CorbaTestCase):
    def setUp(self):
        self._nodebooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def _setupDevices(self, nodeName):
        self._devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_%s_node/DeviceManager.dcd.xml" % (nodeName,))
        self.assertNotEqual(self._devMgr, None)

        self._parentDevice = None
        self._childDevice = None

        # These tests must be kept in sync with the DCD.
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 2)
        for device in self._devMgr._get_registeredDevices():
            if device._get_identifier() == "DCE:8f3478e3-626e-45c3-bd01-0a8117dbe59b":
                self._parentDevice = device
            elif device._get_identifier() == "DCE:2427279b-ed30-4f27-8a58-022110e1f898":
                self._childDevice = device

        self.assertNotEqual(self._parentDevice, None)
        self.assertNotEqual(self._childDevice, None)

        self._aggregateDevice = self._parentDevice._narrow(CF.AggregateDevice)
        self._executableDevice = self._parentDevice._narrow(CF.ExecutableDevice)
        self.assertNotEqual(self._aggregateDevice, None)
        self.assertNotEqual(self._executableDevice, None)

    def _test_BasicAggregateDevice(self, node):
        self._setupDevices(node)

        self.assertEqual(len(self._aggregateDevice._get_devices()), 1)
        self.assertEqual(self._aggregateDevice._get_devices()[0]._get_identifier(), self._childDevice._get_identifier())

        self.assertNotEqual(self._childDevice._get_compositeDevice(), None)
        self.assertEqual(self._parentDevice._get_identifier(), self._childDevice._get_compositeDevice()._narrow(CF.Device)._get_identifier())

        self._devMgr.shutdown()
        self.assert_(self.waitTermination(self._devBooter))
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def _test_ShutdownParent(self, node):
        self._setupDevices(node)

        # test aggregate device releaseObject functionality
        pids = getChildren(self._devBooter.pid)
        self._parentDevice.releaseObject()
        for pid in pids:
            number_attempts = 0
            while True:
                try:
                    os.kill(pid, 0) # check to see if the process is still alive
                    time.sleep(0.5)
                    number_attempts+=1
                except:
                    break
                if number_attempts == 5:
                    break
        pids = getChildren(self._devBooter.pid)
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 0)
        # make sure the child device was also released
        self.assertEqual(len(pids), 0)

        self._devMgr.shutdown()
        self.assert_(self.waitTermination(self._devBooter))
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def _test_ShutdownChild(self, node):
        self._setupDevices(node)

        # test child device releaseObject functionality
        pids = getChildren(self._devBooter.pid)
        pids1 = getChildren(pids[0])
        pids2 = getChildren(pids[1])
        if len(pids2) == 1:
            child_pid = pids[0]
            parent_pid = pids[1]
        else:
            child_pid = pids[1]
            parent_pid = pids[0]
        self._childDevice.releaseObject()
        number_attempts = 0
        while True:
            try:
                os.kill(child_pid, 0) # check to see if the process is still alive
                time.sleep(0.5)
                number_attempts+=1
            except:
                break
            if number_attempts == 5:
                break
        self.assertEqual(len(self._aggregateDevice._get_devices()), 0)
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 1)
        # make sure child device no longer exists
        pids = getChildren(self._devBooter.pid)
        self.assertEqual(len(pids), 1)

        # now make sure parent device releases okay
        pids = getChildren(self._devBooter.pid)
        self._parentDevice.releaseObject()
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 0)
        number_attempts = 0
        while True:
            try:
                os.kill(parent_pid, 0) # check to see if the process is still alive
                time.sleep(0.5)
                number_attempts+=1
            except:
                break
            if number_attempts == 5:
                break
        pids = getChildren(self._devBooter.pid)
        self.assertEqual(len(pids), 0)

        self._devMgr.shutdown()
        self.assert_(self.waitTermination(self._devBooter))
        self.assertEqual(len(self._domMgr._get_deviceManagers()), 0)

    def test_BasicAggregateDevice(self):
        self._test_BasicAggregateDevice("BasicChildDevice")

    def test_BasicAggregateDeviceShutdownChild(self):
        self._test_ShutdownChild("BasicChildDevice")

    def test_BasicAggregateDeviceShutdownParent(self):
        self._test_ShutdownParent("BasicChildDevice")

    def test_CppBasicAggregateDevice(self):
        self._test_BasicAggregateDevice("BasicChildDevice_cpp")

    def test_CppBasicAggregateDeviceShutdownParent(self):
        self._test_ShutdownParent("BasicChildDevice_cpp")

    def test_CppBasicAggregateDeviceShutdownChild(self):
        self._test_ShutdownChild("BasicChildDevice_cpp")

    def test_PyBasicAggregateDevice(self):
        self._test_BasicAggregateDevice("BasicChildDevice_python")

    def test_PyBasicAggregateDeviceShutdownParent(self):
        self._test_ShutdownParent("BasicChildDevice_python")

    def test_PyBasicAggregateDeviceShutdownChild(self):
        self._test_ShutdownChild("BasicChildDevice_python")

