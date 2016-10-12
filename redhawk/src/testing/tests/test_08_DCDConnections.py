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

import time
import unittest
from _unitTestHelpers import scatest
from omniORB import URI, any
from ossie.cf import CF

class DCDConnectionsTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)

    def test_DCDConnection(self):
        self.assertNotEqual(self._domMgr, None)

        devBooter, devMgr = self.launchDeviceManager("/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # These UUIDs must be kept in sync with the DCD
        usesId = "DCE:322fb9b2-de57-42a2-ad03-217bcb244262"
        providesId = "DCE:47dc45d8-19b5-4b7e-bcd4-b165babe5b84"

        # Get the device connections.
        connections = {}
        for device in devMgr._get_registeredDevices():
            id = device._get_identifier()

            # Check that the UUID matches what we expect; a failure here is
            # probably more indicative of a faulty test.
            if id != usesId and id != providesId:
                self.fail("Unexpected device ID: " + id)

            connections[id] = device.runTest(0, [])

        # Provide side should have no connections; uses side should have 1.
        self.assertEqual(len(connections[providesId]), 0)
        self.assertEqual(len(connections[usesId]), 1)

        # Verify that the connection is to the correct provides port; implicitly,
        # we know that the uses side is correct because device.runTest returned
        # a valid connection.
        self.assertEqual(connections[usesId][0].value.value(), providesId + '/resource_in')

    def test_DCDPendingConnection(self):
        self.assertNotEqual(self._domMgr, None)

        # Start the second node first; it should not report any connections yet
        # because the connection is pending.
        devBooter2, devMgr2 = self.launchDeviceManager("/nodes/test_PortTestDevice2_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr2, None)
        device = devMgr2._get_registeredDevices()[0]
        self.assertEqual(device._get_label(), "PortTestDevice3")
        self.assertEqual(len(device.runTest(0, [])), 0)

        # Start the first node; the pending connection from the second node
        # should be completed now.
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Verify that the connection is completed, and points to a device on
        # the first node.
        connections = device.runTest(0, [])
        self.assertEqual(len(connections), 1)
        identifier = connections[0].value.value()
        identifier, port = identifier.split('/')
        device2 = None
        for dev in devMgr._get_registeredDevices():
            if identifier == dev._get_identifier():
                device2 = dev
        self.assertNotEqual(device2, None)

        # Terminate the first node and verify that the connection is broken.
        self.terminateChild(devBooter)
        self.assertEqual(len(device.runTest(0, [])), 0)

        # Restart the first node; the pending connection from the second node
        # should be re-established.
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        device = devMgr2._get_registeredDevices()[0]
        self.assertEqual(len(device.runTest(0, [])), 1)

    def test_MultipleService(self):
        svcBooter, svcMgr = self.launchDeviceManager("/nodes/test_BasicService_node/DeviceManager.dcd.xml")
        self.assertNotEqual(svcMgr, None)
        svcBooterSecond, svcMgrSecond = self.launchDeviceManager("/nodes/test_SecondBasicService_node/DeviceManager.dcd.xml")
        self.assertNotEqual(svcMgrSecond, None)
        svcBooterSecond, svcMgrThird = self.launchDeviceManager("/nodes/test_PortTestDeviceService_node/DeviceManager.dcd.xml")
        self.assertNotEqual(svcMgrSecond, None)

        #confirm connection is there
        device = svcMgrThird._get_registeredDevices()[0]
        service_1 = svcMgr._get_registeredServices()[0]

        props = service_1.serviceObject.query([])
        testOut = device.runTest(2, [])
        for ii in range(len(props)):
            self.assertEqual(props[ii].id, testOut[ii].id)
            self.assertEqual(any.from_any(props[ii].value), any.from_any(testOut[ii].value))

        svcMgrSecond.shutdown()
        devmgrs = []
        begin_time = time.time()
        while len(devmgrs) != 2:
            devmgrs = self._domMgr._get_deviceManagers()
            time.sleep(0.1)
            if (time.time() - begin_time) > 5.0:
                self.fail('Device manager test_SecondBasicService_node did not unregister')

        props = service_1.serviceObject.query([])
        testOut = device.runTest(2, [])
        for ii in range(len(props)):
            self.assertEqual(props[ii].id, testOut[ii].id)
            self.assertEqual(any.from_any(props[ii].value), any.from_any(testOut[ii].value))

        svcMgrThird.shutdown()
        devmgrs = []
        begin_time = time.time()
        while len(devmgrs) != 1:
            devmgrs = self._domMgr._get_deviceManagers()
            time.sleep(0.1)
            if (time.time() - begin_time) > 5.0:
                self.fail('Device manager test_PortTestDeviceService_node did not unregister')
        svcMgr.shutdown()
        devmgrs = []
        begin_time = time.time()
        while len(devmgrs) != 0:
            devmgrs = self._domMgr._get_deviceManagers()
            time.sleep(0.1)
            if (time.time() - begin_time) > 5.0:
                self.fail('Device manager test_BasicService_node did not unregister')


