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
import time
from ossie.utils import redhawk

class DynamicDeviceLaunchTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/wb_receiver_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels...... 
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        redhawk.setTrackApps(False)

    def test_launch(self):
        for device in self._rhDom.devices:
            print(device.label)
            
        self.assertEqual(len(self._rhDom.devices), 8)
        devices = ['wb_receiver_1:supersimple_1:anothersimple_1', 
                   'wb_receiver_1:supersimple_1:anothersimple_2', 
                   'wb_receiver_1:supersimple_1', 
                   'wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1', 
                   'wb_receiver_1:anothersimple_1', 
                   'wb_receiver_1:anothersimple_1:anothersimple_2', 
                   'wb_receiver_1:anothersimple_1:anothersimple_1', 
                   'wb_receiver_1']

        dev_ids = ['wb_receiver_node:wb_receiver_1:supersimple_1:anothersimple_1', 
                   'wb_receiver_node:wb_receiver_1:supersimple_1:anothersimple_2', 
                   'wb_receiver_node:wb_receiver_1:supersimple_1', 
                   'wb_receiver_node:wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1', 
                   'wb_receiver_node:wb_receiver_1:anothersimple_1', 
                   'wb_receiver_node:wb_receiver_1:anothersimple_1:anothersimple_2', 
                   'wb_receiver_node:wb_receiver_1:anothersimple_1:anothersimple_1', 
                   'wb_receiver_node:wb_receiver_1']

        for dev in self._rhDom.devices:
            self.assertTrue(dev.label in devices)
            self.assertTrue(dev.identifier in dev_ids)
            devices.pop(devices.index(dev.label))
            dev_ids.pop(dev_ids.index(dev.identifier))

        parent = None
        for dev in self._rhDom.devices:
            if dev.label == 'wb_receiver_1':
                parent = dev
                break

        super_1 = None
        for dev in self._rhDom.devices:
            if dev.label == 'wb_receiver_1:supersimple_1':
                super_1 = dev
                break

        another_1 = None
        for dev in self._rhDom.devices:
            if dev.label == 'wb_receiver_1:anothersimple_1':
                another_1 = dev
                break

        grandchild_1 = None
        for dev in self._rhDom.devices:
            if dev.label == 'wb_receiver_1:anothersimple_1:anothersimple_2':
                grandchild_1 = dev
                break

        self.assertNotEqual(parent, None)
        self.assertNotEqual(super_1, None)
        self.assertNotEqual(another_1, None)
        self.assertNotEqual(grandchild_1, None)

        self.assertEqual(len(parent.ref.devices), 2)
        self.assertEqual(len(super_1.ref.devices), 2)
        self.assertEqual(len(another_1.ref.devices), 2)
        self.assertEqual(len(grandchild_1.ref.devices), 1)

        dev_count = 0
        for dev in self._rhDom.devices:
            if dev.label == 'wb_receiver_1':
                self.assertEqual(dev.compositeDevice, None)
                dev_count += 1
            if dev.label == 'wb_receiver_1:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'wb_receiver_1')
                dev_count += 1
            if dev.label == 'wb_receiver_1:supersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'wb_receiver_1')
                dev_count += 1
            if dev.label == 'wb_receiver_1:supersimple_1:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'wb_receiver_1:supersimple_1')
                dev_count += 1
            if dev.label == 'wb_receiver_1:supersimple_1:anothersimple_2':
                self.assertEqual(dev.compositeDevice.label, 'wb_receiver_1:supersimple_1')
                dev_count += 1
            if dev.label == 'wb_receiver_1:anothersimple_1:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'wb_receiver_1:anothersimple_1')
                dev_count += 1
            if dev.label == 'wb_receiver_1:anothersimple_1:anothersimple_2':
                self.assertEqual(dev.compositeDevice.label, 'wb_receiver_1:anothersimple_1')
                dev_count += 1
            if dev.label == 'wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'wb_receiver_1:anothersimple_1:anothersimple_2')
                dev_count += 1
        self.assertEqual(dev_count, 8)

        for dev in self._rhDom.devices:
            print('++++++++++++++++++++', dev._get_identifier(),'*******', dev._get_label())


class DynamicCppDeviceLaunchTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/cpp_wb_receiver_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels...... 
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        redhawk.setTrackApps(False)

    def test_cpp_launch(self):
        print(self._rhDom.devices)
        print(self._rhDom.devMgrs[0]._get_registeredDevices())

        for dev in self._rhDom.devices:
            print('++++++++++++++++++++', dev._get_identifier(),'*******', dev._get_label())

        self.assertEqual(len(self._rhDom.devices), 8)
        devices = ['cpp_wb_receiver_1:supersimple_1:anothersimple_1', 
                   'cpp_wb_receiver_1:supersimple_1:anothersimple_2', 
                   'cpp_wb_receiver_1:supersimple_1', 
                   'cpp_wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1', 
                   'cpp_wb_receiver_1:anothersimple_1', 
                   'cpp_wb_receiver_1:anothersimple_1:anothersimple_2', 
                   'cpp_wb_receiver_1:anothersimple_1:anothersimple_1', 
                   'cpp_wb_receiver_1']

        dev_ids = ['cpp_wb_receiver_node:cpp_wb_receiver_1:supersimple_1:anothersimple_1', 
                   'cpp_wb_receiver_node:cpp_wb_receiver_1:supersimple_1:anothersimple_2', 
                   'cpp_wb_receiver_node:cpp_wb_receiver_1:supersimple_1', 
                   'cpp_wb_receiver_node:cpp_wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1', 
                   'cpp_wb_receiver_node:cpp_wb_receiver_1:anothersimple_1', 
                   'cpp_wb_receiver_node:cpp_wb_receiver_1:anothersimple_1:anothersimple_2', 
                   'cpp_wb_receiver_node:cpp_wb_receiver_1:anothersimple_1:anothersimple_1', 
                   'cpp_wb_receiver_node:cpp_wb_receiver_1']

        for dev in self._rhDom.devices:
            self.assertTrue(dev.label in devices)
            self.assertTrue(dev.identifier in dev_ids)
            devices.pop(devices.index(dev.label))
            dev_ids.pop(dev_ids.index(dev.identifier))

        parent = None
        for dev in self._rhDom.devices:
            if dev.label == 'cpp_wb_receiver_1':
                parent = dev
                break

        super_1 = None
        for dev in self._rhDom.devices:
            if dev.label == 'cpp_wb_receiver_1:supersimple_1':
                super_1 = dev
                break

        another_1 = None
        for dev in self._rhDom.devices:
            if dev.label == 'cpp_wb_receiver_1:anothersimple_1':
                another_1 = dev
                break

        grandchild_1 = None
        for dev in self._rhDom.devices:
            if dev.label == 'cpp_wb_receiver_1:anothersimple_1:anothersimple_2':
                grandchild_1 = dev
                break

        self.assertNotEqual(parent, None)
        self.assertNotEqual(super_1, None)
        self.assertNotEqual(another_1, None)
        self.assertNotEqual(grandchild_1, None)

        self.assertEqual(len(parent.ref.devices), 2)
        self.assertEqual(len(super_1.ref.devices), 2)
        self.assertEqual(len(another_1.ref.devices), 2)
        self.assertEqual(len(grandchild_1.ref.devices), 1)

        dev_count = 0
        for dev in self._rhDom.devices:
            if dev.label == 'cpp_wb_receiver_1':
                self.assertEqual(dev.compositeDevice, None)
                dev_count += 1
            if dev.label == 'cpp_wb_receiver_1:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'cpp_wb_receiver_1')
                dev_count += 1
            if dev.label == 'cpp_wb_receiver_1:supersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'cpp_wb_receiver_1')
                dev_count += 1
            if dev.label == 'cpp_wb_receiver_1:supersimple_1:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'cpp_wb_receiver_1:supersimple_1')
                dev_count += 1
            if dev.label == 'cpp_wb_receiver_1:supersimple_1:anothersimple_2':
                self.assertEqual(dev.compositeDevice.label, 'cpp_wb_receiver_1:supersimple_1')
                dev_count += 1
            if dev.label == 'cpp_wb_receiver_1:anothersimple_1:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'cpp_wb_receiver_1:anothersimple_1')
                dev_count += 1
            if dev.label == 'cpp_wb_receiver_1:anothersimple_1:anothersimple_2':
                self.assertEqual(dev.compositeDevice.label, 'cpp_wb_receiver_1:anothersimple_1')
                dev_count += 1
            if dev.label == 'cpp_wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1':
                self.assertEqual(dev.compositeDevice.label, 'cpp_wb_receiver_1:anothersimple_1:anothersimple_2')
                dev_count += 1
        self.assertEqual(dev_count, 8)

        for dev in self._rhDom.devices:
            dev.start()

        time.sleep(0.2)
