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

class DeviceExceptionsTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_BrokenExecutableDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        try:
            self._device = self._devMgr._get_registeredDevices()[0]
        except:
            self._device = None

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def preconditions (self):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)
        self.assertNotEqual(self._device, None)

    def test_AppReleaseUnloadFail (self):
        self.preconditions()

        # Make the device throw a CF.InvalidFileName exception on unload.
        props = [CF.DataType(id="DCE:58f5720f-3a33-4056-8359-6b560613815d", value=any.to_any("unload")),
                 CF.DataType(id="DCE:7f4ca822-5497-43d0-b92d-fe97c561e450", value=any.to_any("CF.InvalidFileName"))]
        self._device.configure(props)

        # Test that the exception occurs as expected
        self._device.load(self._domMgr._get_fileMgr(), "/waveforms/CapacityUsage/CapacityUsage.sad.xml", CF.LoadableDevice.EXECUTABLE)
        self.assertRaises(CF.InvalidFileName, self._device.unload, "/waveforms/CapacityUsage/CapacityUsage.sad.xml")

        # Create an application that uses capacity, then release it
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_identifier(), [], [])
        app.releaseObject()

        # Verify that the capacity has been returned to the device.
        prop = CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))
        props = self._device.query([prop])
        self.assertEqual(props[0].value._v, 100000000)

    def test_LoadFailUnexpectedException (self):
        self.preconditions()

        # Make the device throw a generic Python exception on allocate, which will become
        # a CORBA::UNKNOWN to the DomainManager
        props = [CF.DataType(id="DCE:58f5720f-3a33-4056-8359-6b560613815d", value=any.to_any("load"))]
        self._device.configure(props)

        # Test that the exception occurs as expected
        self.assertRaises(CORBA.UNKNOWN, self._device.load, self._domMgr._get_fileMgr(), "/waveforms/CapacityUsage/CapacityUsage.sad.xml", CF.LoadableDevice.EXECUTABLE)

        # Try to create an application that uses capacity; it should fail,
        # but the failure should be handled gracefully and the device's
        # capacity should be restored.
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]
        try:
            app = appFact.create(appFact._get_identifier(), [], [])
            self.fail("Application should have failed due to exception")
        except CF.ApplicationFactory.CreateApplicationError:
            pass

        # Verify that the capacity has been returned to the device.
        prop = CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))
        props = self._device.query([prop])
        self.assertEqual(props[0].value._v, 100000000)

    def test_AllocateFailUnexpectedException (self):
        self.preconditions()

        # Make the device throw a generic Python exception on allocate, which will become
        # a CORBA::UNKNOWN to the DomainManager
        props = [CF.DataType(id="DCE:58f5720f-3a33-4056-8359-6b560613815d", value=any.to_any("allocateCapacity"))]
        self._device.configure(props)

        # Test that the exception occurs as expected
        capacity = [CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(0))]
        self.assertRaises(CORBA.UNKNOWN, self._device.allocateCapacity, capacity)

        # Create an application that uses capacity; it's fine if it fails, but it shouldn't
        # kill the DomainManager.
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]
        try:
            app = appFact.create(appFact._get_identifier(), [], [])
            self.fail("Application creation should have failed due to failed allocation")
        except CF.ApplicationFactory.CreateApplicationError:
            pass
        except:
            self.fail("Unhandled error in application creation")

    def test_AllocateRaiseUnexpectedException (self):
        devBooter_2, self._devMgr_2 = self.launchDeviceManager("/nodes/test_UnusableAllocateCapacity_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        local_device = self._devMgr_2._get_registeredDevices()[0]

        # Cause a failure during the allocation process
        props = [CF.DataType(id="invalidCapacity", value=any.to_any(1000))]
        retval = local_device.allocateCapacity(props)
        self.assertEqual(retval, False)

        # Cause a failure during the allocation process
        props = [CF.DataType(id="configurationProperty", value=any.to_any(1000))]
        self.assertRaises(CF.Device.InvalidCapacity, local_device.allocateCapacity, props)

    def test_DeallocateFailUnexpectedException (self):
        self.preconditions()

        # Make the device throw a generic Python exception on deallocate, which will become
        # a CORBA::UNKNOWN to the DomainManager
        props = [CF.DataType(id="DCE:58f5720f-3a33-4056-8359-6b560613815d", value=any.to_any("deallocateCapacity"))]
        self._device.configure(props)

        # Test that the exception occurs as expected
        capacity = [CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(0))]
        self.assertRaises(CORBA.UNKNOWN, self._device.deallocateCapacity, capacity)

        # Create and release an application; the DomainManager should handle the exception
        # in stride.
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_identifier(), [], [])
        app.releaseObject()

    def test_ExecuteFailUnexpectedException (self):
        self.preconditions()

        # Make the device throw a generic Python exception on execute, which will become
        # a CORBA::UNKNOWN to the DomainManager
        props = [CF.DataType(id="DCE:58f5720f-3a33-4056-8359-6b560613815d", value=any.to_any("execute"))]
        self._device.configure(props)

        # Test that the exception occurs as expected
        self.assertRaises(CORBA.UNKNOWN, self._device.execute, "", [], [])

        # Try to create an application that uses capacity; it should fail,
        # but the failure should be handled gracefully and the device's
        # capacity should be restored.
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]
        try:
            appFact.create(appFact._get_identifier(), [], [])
            self.fail("Application should have failed due to exception")
        except CF.ApplicationFactory.CreateApplicationError:
            pass

        # Verify that the capacity has been returned to the device.
        prop = CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))
        props = self._device.query([prop])
        self.assertEqual(props[0].value._v, 100000000)

    def test_TerminateFailUnexpectedException (self):
        self.preconditions()

        # Make the device throw a generic Python exception on terminate, which will become
        # a CORBA::UNKNOWN to the DomainManager
        props = [CF.DataType(id="DCE:58f5720f-3a33-4056-8359-6b560613815d", value=any.to_any("terminate"))]
        self._device.configure(props)

        # Test that the exception occurs as expected
        self.assertRaises(CORBA.UNKNOWN, self._device.terminate, 0)

        # Create an application.
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_identifier(), [], [])

        # Get the component pid, then release the app; the component will be orphaned
        # but the release should succeed, restoring the device's capacity.
        pid = app._get_componentProcessIds()[0].processId
        app.releaseObject()

        # Restore the normal operation of terminate to kill the orphan.
        props = [CF.DataType(id="DCE:58f5720f-3a33-4056-8359-6b560613815d", value=any.to_any(""))]
        self._device.configure(props)
        self._device.terminate(pid)

        # Verify that the capacity has been returned to the device.
        prop = CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))
        props = self._device.query([prop])
        self.assertEqual(props[0].value._v, 100000000)
