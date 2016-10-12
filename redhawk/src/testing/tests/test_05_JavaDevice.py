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
import scatest
import buildconfig
from ossie.cf import CF
from omniORB import any

class ApplicationFactoryTest(scatest.CorbaTestCase):
    def setUp(self):
        self._testFiles = []

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)
        for file in self._testFiles:
            os.unlink(file)

    def test_BasicDevice(self):
        if buildconfig.HAVE_JAVASUPPORT != "yes":
            return

        nodebooter, domMgr = self.launchDomainManager(debug=9)
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_java_node/DeviceManager.dcd.xml", debug=9)

        # Ensure the expected device is available
        self.assertNotEqual(devMgr, None)
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        # Checks init state
        self.assertEqual(device._get_usageState(), CF.Device.IDLE)
        self.assertEqual(device._get_adminState(), CF.Device.UNLOCKED)
        self.assertEqual(device._get_operationalState(), CF.Device.ENABLED)

        # Make sure we can do limited things
        self.assertEqual(device._get_started(), False)
        device.start()
        self.assertEqual(device._get_started(), True)
        device.stop()
        self.assertEqual(device._get_started(), False)

        # Makes sure the prop query is correct
        int_id = "DCE:0f99b2e4-9903-4631-9846-ff349d18eaaa"
        short_id = "DCE:d3aa6040-b731-4110-b814-376967264728"

        res = device.query([])
        self.assertEqual(len(res), 9)
        ids = []
        for r in res :
            ids.append(r.id)
        self.assertEqual("DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d" in ids, True)
        self.assertEqual("DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb" in ids, True)
        self.assertEqual(int_id in ids, True)
        self.assertEqual(short_id in ids, True)

        # Makes initial prop values are correct
        for r in res :
            if r.id == int_id:
                self.assertEqual(any.from_any(r.value), 10)
            elif r.id == short_id:
                self.assertEqual(any.from_any(r.value), 22)

        # Make sure allocs can be done
        props_int = [CF.DataType(id=int_id, value=any.to_any(2))]
        props_short = [CF.DataType(id=short_id, value=any.to_any(20))]
        self.assertEqual(device.allocateCapacity(props_int), True)
        self.assertEqual(device.allocateCapacity(props_short), True)

        # Makes sure values are correct after alloc
        res = device.query([])
        for r in res :
            if r.id == int_id:
                self.assertEqual(any.from_any(r.value), 8)
            elif r.id == short_id:
                self.assertEqual(any.from_any(r.value), 2)

        # Makes sure device becomes BUSY when all allocation has been done
        props = [CF.DataType(id=int_id, value=any.to_any(4)), CF.DataType(id=short_id, value=any.to_any(1))]
        self.assertEqual(device.allocateCapacity(props), True)
        self.assertEqual(device._get_usageState(), CF.Device.ACTIVE)
        self.assertEqual(device.allocateCapacity(props), True)
        self.assertEqual(device._get_usageState(), CF.Device.BUSY)

        # Makes sure that no more allocation can be done
        self.assertEqual(device.allocateCapacity(props_int), False)
        self.assertEqual(device.allocateCapacity(props_short), False)
        self.assertEqual(device.allocateCapacity(props), False)

        # Makes sure that the dealloc works properly
        device.deallocateCapacity(props)
        res = device.query([])
        for r in res :
            if r.id == int_id:
                self.assertEqual(any.from_any(r.value), 4)
            elif r.id == short_id:
                self.assertEqual(any.from_any(r.value), 1)

        # Makes sure that alloc fail of 1 prop prevents both from writing
        self.assertEqual(device.allocateCapacity(props_int), True)
        self.assertEqual(device.allocateCapacity(props_int), True)
        self.assertEqual(device.allocateCapacity(props), False)
        res = device.query([])
        for r in res:
            if r.id == int_id:
                self.assertEqual(any.from_any(r.value), 0)
            elif r.id == short_id:
                self.assertEqual(any.from_any(r.value), 1)

        # Makes sure that props can't be dealloced beyond their orig capacity
        props = [CF.DataType(id=int_id, value=any.to_any(9)), CF.DataType(id=short_id, value=any.to_any(20))]
        device.deallocateCapacity(props)
        self.assertRaises(CF.Device.InvalidCapacity, device.deallocateCapacity, props_int)
        self.assertRaises(CF.Device.InvalidCapacity, device.deallocateCapacity, props_short)

        # Makes sure if if 1 dealloc fails, neither are written
        props = [CF.DataType(id=int_id, value=any.to_any(1)), CF.DataType(id=short_id, value=any.to_any(20))]
        self.assertRaises(CF.Device.InvalidCapacity, device.deallocateCapacity, props)
        res = device.query([])
        for r in res:
            if r.id == int_id:
                self.assertEqual(any.from_any(r.value), 9)
            elif r.id == short_id:
                self.assertEqual(any.from_any(r.value), 21)

        # Makes sure if props are at full capacity, usage State returns to IDLE
        props = [CF.DataType(id=int_id, value=any.to_any(1)), CF.DataType(id=short_id, value=any.to_any(1))]
        device.deallocateCapacity(props)
        self.assertEqual(device._get_usageState(), CF.Device.IDLE)
        res = device.query([])
        for r in res:
            if r.id == int_id:
                self.assertEqual(any.from_any(r.value), 10)
            elif r.id == short_id:
                self.assertEqual(any.from_any(r.value), 22)

        # Makes sure that props with aren't allocatable, don't get allocated
        prop = [CF.DataType(id="no_allocation", value=any.to_any(1))]
        self.assertEqual(device.allocateCapacity(prop), False)
        prop = [CF.DataType(id="not_external", value=any.to_any(1.1))]
        self.assertEqual(device.allocateCapacity(prop), False)
        prop = [CF.DataType(id="non_simple1", value=any.to_any("Hello"))]
        self.assertEqual(device.allocateCapacity(prop), False)
        prop = [CF.DataType(id="non_simple2", value=any.to_any([]))]
        self.assertEqual(device.allocateCapacity(prop), False)
        res = device.query([])
        for r in res:
            if r.id == int_id:
                self.assertEqual(any.from_any(r.value), 10)
            elif r.id == short_id:
                self.assertEqual(any.from_any(r.value), 22)

        # Check that execparams are set in initialize (issue #561).
        for prop in device.runTest(561, []):
            self.assertNotEqual(any.from_any(prop.value), None, 'execparams not set in intialize()')

        device.releaseObject()

        self.assertEqual(len(devMgr._get_registeredDevices()), 0)
        self.assertEqual(len(domMgr._get_deviceManagers()), 1)

        devMgr.shutdown()

        self.assertEqual(len(domMgr._get_deviceManagers()), 0)

