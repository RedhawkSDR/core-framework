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
from omniORB import any
from ossie.cf import CF
from xml.dom import minidom
import os

class ComplexApplicationFactoryTest(scatest.CorbaTestCase):
    def setUp(self):
        nodebooter, self._domMgr = self.launchDomainManager()

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)

    def test_NodeCleanupOnDevMgrShutdown(self):
        # Test that if we destroy a device, all applications using that device
        # are released
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        self._domMgr.installApplication("/waveforms/CommandWrapperLight/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Start the executable devices first. This will ensure that matching properties
        # work because CommandWrapper specifies matching properties that only work
        # with BasicTestDevice
        nb1, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]
        apps = []
        totalApps = 10
        for entry in range(totalApps):
            apps.append(appFact.create(appFact._get_name(), [], []))

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), totalApps)

        devMgr.shutdown()
        if not self.waitTermination(nb1):
            self.fail("DeviceManager did not die after shutdown")

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

    def test_NodeCleanupOnNodeBooterKill(self):
        # Test that if we destroy a device, all applications using that device
        # are released
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        self._domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Start the executable devices first. This will ensure that matching properties
        # work because CommandWrapper specifies matching properties that only work
        # with BasicTestDevice
        nb1, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        self.terminateChild(nb1)

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

    def test_NodeCleanupOnDeviceRelease(self):
        # Test that if we destroy a device, all applications using that device
        # are released
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        self._domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Start the executable devices first. This will ensure that matching properties
        # work because CommandWrapper specifies matching properties that only work
        # with BasicTestDevice
        nb1, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]

        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        device.releaseObject()

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        self._domMgr.uninstallApplication(appFact._get_identifier())

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)


    def test_ComplexAllocation(self):
        # Install the waveform before registering devices to ensure the bug #281
        # doesn't come back
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 0)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        self._domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)

        # Start the executable devices first. This will ensure that matching properties
        # work because CommandWrapper specifies matching properties that only work
        # with BasicTestDevice
        nb1, execDevNode1 = self.launchDeviceManager("/nodes/test_MultipleExecutableDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(execDevNode1, None)

        nb2, basicDevNode1 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")
        self.assertNotEqual(basicDevNode1, None)

        nb3, basicDevNode2 = self.launchDeviceManager("/nodes/test_BasicTestDevice3_node/DeviceManager.dcd.xml")
        self.assertNotEqual(basicDevNode2, None)

        # Ensure the expected devices are available
        self.assertEqual(len(basicDevNode1._get_registeredDevices()), 1)
        for device in basicDevNode1._get_registeredDevices():
            # Query the known allocation properties
            memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
            bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
            self.assertEqual(memCapacity.value._v, 100000000)
            self.assertEqual(bogoMips.value._v, 100000000)

        self.assertEqual(len(basicDevNode2._get_registeredDevices()), 1)
        for device in basicDevNode2._get_registeredDevices():
            # Query the known allocation properties
            memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
            bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
            self.assertEqual(memCapacity.value._v, 100000000)
            self.assertEqual(bogoMips.value._v, 100000000)

        self.assertEqual(len(execDevNode1._get_registeredDevices()), 4)

        appFact = self._domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], []) # LOOK MA, NO DAS!

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        # We don't know which device actually got allocated so check that one of them has
        freeDevice = None
        allocatedDevice = None
        for device in (basicDevNode1._get_registeredDevices()[0], basicDevNode2._get_registeredDevices()[0]):
            self.assertNotEqual(device._get_usageState(), CF.Device.BUSY)
            if device._get_usageState() == CF.Device.IDLE:
                memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
                bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
                self.assertEqual(memCapacity.value._v, 100000000)
                self.assertEqual(bogoMips.value._v, 100000000)
                freeDevice = device
            elif device._get_usageState() == CF.Device.ACTIVE:
                # Verify that capacity was allocated
                memCapacity = device.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
                bogoMips = device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
                self.assertEqual(memCapacity.value._v, 100000000-5000)
                self.assertEqual(bogoMips.value._v, 100000000-1000)
                allocatedDevice = device

        self.assertNotEqual(freeDevice, None)
        self.assertNotEqual(allocatedDevice, None)

        app.stop()
        app.releaseObject()

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 0)
#
#        # Verify that capacity was deallocated
#
        memCapacity = freeDevice.query([CF.DataType(id="DCE:8dcef419-b440-4bcf-b893-cab79b6024fb", value=any.to_any(None))])[0]
        bogoMips = freeDevice.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0]
        self.assertEqual(memCapacity.value._v, 100000000)
        self.assertEqual(bogoMips.value._v, 100000000)

        self._domMgr.uninstallApplication(appFact._get_identifier())


    def _getBogoMips(self, device):
        return device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0].value._v


    def test_MultipleAllocations(self):
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")

        # Start the first DeviceManager
        nb1, node1 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(node1, None)
        self.assertEqual(len(node1._get_registeredDevices()), 1)
        device1 = node1._get_registeredDevices()[0]
        self.assertEqual(self._getBogoMips(device1), 100000000)

        appFact = self._domMgr._get_applicationFactories()[0]

        # Create two applications to take up the capacity of the first device
        app1 = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(self._getBogoMips(device1), 50000000)

        app2 = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(self._getBogoMips(device1), 0)

        # Try to create another app, which should fail due to lack of capacity.
        try:
            failApp = appFact.create(appFact._get_name(), [], [])
            failApp.releaseObject()
            self.fail("Created application with no capacity")
        except:
            pass

        # TODO: Check that no naming context was left behind from the failed creation (ticket #352).

        # Start the second DeviceManager
        nb2, node2 = self.launchDeviceManager("/nodes/test_BasicTestDevice3_node/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(node2, None)
        self.assertEqual(len(node2._get_registeredDevices()), 1)
        device2 = node2._get_registeredDevices()[0]
        self.assertEqual(self._getBogoMips(device2), 100000000)

        # Create two more applications to take up the capacity of the second device
        app3 = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(self._getBogoMips(device1), 0)
        self.assertEqual(self._getBogoMips(device2), 50000000)

        app4 = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(self._getBogoMips(device1), 0)
        self.assertEqual(self._getBogoMips(device2), 0)

        # Release the applications and make sure that the capacities are correctly restored
        app4.releaseObject()
        self.assertEqual(self._getBogoMips(device1), 0)
        self.assertEqual(self._getBogoMips(device2), 50000000)
        app3.releaseObject()
        self.assertEqual(self._getBogoMips(device1), 0)
        self.assertEqual(self._getBogoMips(device2), 100000000)
        app2.releaseObject()
        self.assertEqual(self._getBogoMips(device1), 50000000)
        self.assertEqual(self._getBogoMips(device2), 100000000)
        app1.releaseObject()
        self.assertEqual(self._getBogoMips(device1), 100000000)
        self.assertEqual(self._getBogoMips(device2), 100000000)


        # Create four more copies of the application
        app1 = appFact.create(appFact._get_name(), [], [])
        app2 = appFact.create(appFact._get_name(), [], [])
        app3 = appFact.create(appFact._get_name(), [], [])
        app4 = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(self._getBogoMips(device1), 0)
        self.assertEqual(self._getBogoMips(device2), 0)

        # Release the applications (again) and make sure that the capacities are correctly restored
        app4.releaseObject()
        app3.releaseObject()
        app2.releaseObject()
        app1.releaseObject()
        self.assertEqual(self._getBogoMips(device1), 100000000)
        self.assertEqual(self._getBogoMips(device2), 100000000)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_DASAllocationFail(self):
        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")

        # Start the first DeviceManager
        nb1, node1 = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml")

        # Ensure the expected device is available
        self.assertNotEqual(node1, None)
        self.assertEqual(len(node1._get_registeredDevices()), 1)
        device1 = node1._get_registeredDevices()[0]
        self.assertEqual(self._getBogoMips(device1), 100000000)

        appFact = self._domMgr._get_applicationFactories()[0]

        # load the device assignment sequence
        das = minidom.parse(os.path.join(scatest.getSdrPath(), "dom/waveforms/CapacityUsage/CapacityUsage_DAS.xml"))
        ds = []
        deviceAssignmentTypeNodeList = das.getElementsByTagName("deviceassignmenttype")
        for node in deviceAssignmentTypeNodeList:
            componentid = node.getElementsByTagName("componentid")[0].firstChild.data
            assigndeviceid = node.getElementsByTagName("assigndeviceid")[0].firstChild.data
            ds.append( CF.DeviceAssignmentType(str(componentid),str(assigndeviceid)) )

        # Create two applications to take up the capacity of the first device

        # first on uses DAS
        app1 = appFact.create(appFact._get_name(), [], ds)
        self.assertEqual(self._getBogoMips(device1), 50000000)

        # second one is sans DAS
        app2 = appFact.create(appFact._get_name(), [], [])
        self.assertEqual(self._getBogoMips(device1), 0)

        # Try to create another app, which should fail due to lack of capacity.
        # doing this with DAS since it exercises a different piece of code
        try:
            failApp = appFact.create(appFact._get_name(), [], ds)
            failApp.releaseObject()
            self.fail("Created application with no capacity")
        except:
            pass

        app2.releaseObject()
        self.assertEqual(self._getBogoMips(device1), 50000000)
        app1.releaseObject()
        self.assertEqual(self._getBogoMips(device1), 100000000)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_DomainLockupOnDeviceCrash(self):
        """
        Test to recreate a scenario in which:

          1. One application already deployed
          2. Attempt to create a new application
          3. Allocation succeeds, but device crashes in execute()
          4. Device crash causes DeviceManager to unregister it
          5. Unregistration causes DomainManager to release existing app
          6. Failed app cleanup and release/cleanup of existing app overlap

        This situation lead to deadlock in 1.10.0, due to a priority inversion
        of the AllocationManager and DomainManager locks.
        """
        nb, node = self.launchDeviceManager("/nodes/CrashableNode/DeviceManager.dcd.xml")

        self._domMgr.installApplication("/waveforms/CapacityUsage/CapacityUsage.sad.xml")
        appFact = self._domMgr._get_applicationFactories()[0]

        app = appFact.create(appFact._get_name(), [], [])

        device = node._get_registeredDevices()[0]
        device.configure([CF.DataType('crashEnabled', any.to_any(True))])

        try:
            app2 = appFact.create(appFact._get_name(), [], [])
            self.fail('Application creation should have failed')
        except CF.ApplicationFactory.CreateApplicationError:
            pass
