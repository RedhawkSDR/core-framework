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
from omniORB import CORBA, any
from ossie.cf import CF
from _unitTestHelpers import runtestHelpers

java_support = runtestHelpers.haveJavaSupport('../Makefile')

class DeviceExceptionsTest(object):
    def test_InvalidPropertyException(self):
        if java_support:
            bad_prop = CF.DataType(id='bad_cap', value=any.to_any('foo'))
            self.assertRaises(CF.Device.InvalidCapacity, self._device.allocateCapacity, [bad_prop])
            self._device.start()
            self.assertRaises(CF.Device.InvalidCapacity, self._device.deallocateCapacity, [bad_prop])
        else:
            pass

    def test_InvalidStateException(self):
        if java_support:
            test_prop = CF.DataType('test_cap', value=any.to_any('foo'))

            self._device._set_adminState(CF.Device.LOCKED)
            self.assertRaises(CF.Device.InvalidState, self._device.allocateCapacity, [test_prop])
            self.assertRaises(CF.Device.InvalidState, self._device.deallocateCapacity, [test_prop])

            self._device._set_adminState(CF.Device.SHUTTING_DOWN)
            self.assertRaises(CF.Device.InvalidState, self._device.allocateCapacity, [test_prop])

            self._device._set_adminState(CF.Device.UNLOCKED)
        else:
            pass


class JavaDeviceExceptionsTest(DeviceExceptionsTest, scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        if java_support:
            devBooter, self._devMgr = self.launchDeviceManager("/nodes/issue_111_node_java/DeviceManager.dcd.xml")
            self.assertEqual(len(self._devMgr._get_registeredDevices()), 1)
            self._device = self._devMgr._get_registeredDevices()[0]

class CppDeviceExceptionsTest(DeviceExceptionsTest, scatest.CorbaTestCase):
    def setUp (self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/issue_111_node_cpp/DeviceManager.dcd.xml")
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 1)
        self._device = self._devMgr._get_registeredDevices()[0]

class PythonDeviceExceptionsTest(DeviceExceptionsTest, scatest.CorbaTestCase):
    def setUp (self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/issue_111_node/DeviceManager.dcd.xml")
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 1)
        self._device = self._devMgr._get_registeredDevices()[0]
