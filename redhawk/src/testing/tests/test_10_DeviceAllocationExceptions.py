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

class DeviceExceptionsTest(object):
    def setUp(self):
        self.dev=None
        self._domMgr=None
        self._devMgr=None

    def teadDown(self):
        if self.dev != None:
            self.dev.releaseObject()
        

    def test_InvalidPropertyException(self):
        bad_prop = CF.DataType(id='bad_cap', value=any.to_any('foo'))
        self.assertRaises(CF.Device.InvalidCapacity, self._device.allocateCapacity, [bad_prop])
        self._device.start()
        self.assertRaises(CF.Device.InvalidCapacity, self._device.deallocateCapacity, [bad_prop])

    def test_InvalidStateException(self):
        test_prop = CF.DataType('test_cap', value=any.to_any('foo'))

        self._device._set_adminState(CF.Device.LOCKED)
        self.assertRaises(CF.Device.InvalidState, self._device.allocateCapacity, [test_prop])
        self.assertRaises(CF.Device.InvalidState, self._device.deallocateCapacity, [test_prop])

        self._device._set_adminState(CF.Device.SHUTTING_DOWN)
        self.assertRaises(CF.Device.InvalidState, self._device.allocateCapacity, [test_prop])

        self._device._set_adminState(CF.Device.UNLOCKED)


@scatest.requireJava
class JavaDeviceExceptionsTest(DeviceExceptionsTest, scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/issue_111_node_java/DeviceManager.dcd.xml")
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 1)
        self._device = self._devMgr._get_registeredDevices()[0]

class CppDeviceExceptionsTest(DeviceExceptionsTest, scatest.CorbaTestCase):
    def setUp (self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/issue_111_node_cpp/DeviceManager.dcd.xml")
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 1)
        self._device = self._devMgr._get_registeredDevices()[0]

class CppDeviceCapacityExceptions(scatest.CorbaTestCase):
    def setUp(self):
        self.dev=None
        self._domMgr=None
        self._devMgr=None

    def teadDown(self):
        if self.dev != None:
            self.dev.releaseObject()

    def test_InvalidCapacity_simple(self):
        from ossie.utils import sb
        self.dev=sb.launch('DevC', impl="cpp")
        
        res=self.dev.allocateCapacity({'myulong': 3 } )
        self.assertEquals(res,False)
        self.dev.releaseObject()
        self.dev=None
        sb.release()

    def test_InvalidCapacity_node(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/invalid_capacity/DeviceManager.dcd.xml.cpp")
        
        from ossie.utils import redhawk
        dom=redhawk.attach(scatest.getTestDomainName())
        d=dom.devices[0]
        self.assertNotEquals(d,None)
        b=[ CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3))]
        res=d.allocateCapacity(b)
        self.assertEquals(res,False)


    def test_deallocate_overage(self):
        from ossie.utils import sb
        self.dev=sb.launch('DevC', impl="cpp",configure={'myulong': 3 } )

        res=self.dev.allocateCapacity({'myulong': 3 } )
        self.assertEquals(res,True)

        a=[ CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3)), CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3))]
        self.assertRaises(CF.Device.InvalidCapacity, self.dev.deallocateCapacity, a)

        self.dev.releaseObject()
        self.dev=None
        sb.release()

    def test_deallocate_overage_node(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/invalid_capacity/DeviceManager.dcd.xml.overage.cpp")

        from ossie.utils import redhawk
        dom=redhawk.attach(scatest.getTestDomainName())
        d=dom.devices[0]
        self.assertNotEquals(d,None)
        res=d.allocateCapacity({'myulong': 3 } )
        self.assertEquals(res,True)

        a=[ CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3)), CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3))]
        self.assertRaises(CF.Device.InvalidCapacity,d.deallocateCapacity, a)



class PythonDeviceExceptionsTest(DeviceExceptionsTest, scatest.CorbaTestCase):
    def setUp (self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/issue_111_node/DeviceManager.dcd.xml")
        self.assertEqual(len(self._devMgr._get_registeredDevices()), 1)
        self._device = self._devMgr._get_registeredDevices()[0]


@scatest.requireJava
class JavaDeviceCapacityExceptions(scatest.CorbaTestCase):
    def setUp(self):
        self.dev=None
        self._domMgr=None
        self._devMgr=None

    def tearDown(self):
        if self.dev != None:
            self.dev.releaseObject()
        scatest.CorbaTestCase.tearDown(self)

    def test_InvalidCapacity_simple(self):
        from ossie.utils import sb
        self.dev=sb.launch('DevC', impl="java")

        self.assertRaises(CF.Device.InvalidCapacity, self.dev.allocateCapacity, {'myulong': 3 } )
        self.dev.releaseObject()
        self.dev=None
        sb.release()

    def test_InvalidCapacity_node(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/invalid_capacity/DeviceManager.dcd.xml.java")

        from ossie.utils import redhawk
        dom=redhawk.attach(scatest.getTestDomainName())
        dev=dom.devices[0]
        self.assertNotEquals(dev,None)
        b=[ CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3))]
        self.assertRaises(CF.Device.InvalidCapacity, dev.allocateCapacity, b)

    def test_deallocate_overage_java(self):
        from ossie.utils import sb
        self.dev=sb.launch('DevC', impl="java",configure={'myulong': 3 } )

        res=self.dev.allocateCapacity({'myulong': 3 } )
        self.assertEquals(res,True)

        a=[ CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3)), CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3))]
        self.assertRaises(CF.Device.InvalidCapacity, self.dev.deallocateCapacity, a)

        self.dev.releaseObject()
        self.dev=None
        sb.release()

    def test_deallocate_overage_node_java(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/invalid_capacity/DeviceManager.dcd.xml.overage.java")

        from ossie.utils import redhawk
        dom=redhawk.attach(scatest.getTestDomainName())
        d=dom.devices[0]
        self.assertNotEquals(d,None)
        res=d.allocateCapacity({'myulong': 3 } )
        self.assertEquals(res,True)

        a=[ CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3)), CF.DataType(id='myulong', value=CORBA.Any(CORBA.TC_ulong,3))]
        self.assertRaises(CF.Device.InvalidCapacity,d.deallocateCapacity, a)
