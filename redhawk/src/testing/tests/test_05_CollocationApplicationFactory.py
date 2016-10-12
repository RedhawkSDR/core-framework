# -*- coding: utf-8 -*-
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
        nodebooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)

    def tearDown(self):
        scatest.CorbaTestCase.tearDown(self)


    def test_collocationFailed(self):
        """
        This test exercises the collocation failover capability and exhusts all available devices to deploy collocation
        requests for the waveform.  For these scenarios the device in question is: test_collocation_device that
        manages a simple counter for the number of compnents that allocated against its capacity.  That is, when the
        component deploys it submits a  supported_components = 1 to reduce the devices deployment count.  When the
        count == the device's property setting, then a capacity failure occurrs and the component cannot be deployed
        on that specific instance.

        The waveform contains 2 collocation requests, each contains 2 NOOP_CAP components that request 1 unit of supported_components's
        capacity from the deployment device.

        Each node in this case, manages 2 test_collocation_devices each having a supported_components property == 1
        """
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node2_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_req_capacity/test_collocation_req_capacity.sad.xml")

        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())


    def test_collocationSuccess(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev2cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_req_capacity/test_collocation_req_capacity.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        ## need to check that all the comopnents were allocated to devices from test_collocation_node1_2dev2cap

        self.assertNotEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationMixed(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_nodes_1dev4cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_mixed/test_collocation_mixed.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        ## need to check that all the comopnents were allocated to devices from test_collocation_node1_2dev2cap

        self.assertNotEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationPartialProcMixed(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_more_mixed/test_collocation_more_mixed.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        ## need to check that all the comopnents were allocated to devices from test_collocation_node1_2dev2cap

        self.assertNotEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationPartialProcMixedReversed(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_more_mixed_reversed/test_collocation_more_mixed_reversed.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        ## need to check that all the comopnents were allocated to devices from test_collocation_node1_2dev2cap

        self.assertNotEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationFailPythonFirst(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_python_first_mismatch/test_collocation_python_first_mismatch.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        ## need to check that all the comopnents were allocated to devices from test_collocation_node1_2dev2cap

        self.assertEqual(app, None )

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationWithComponentImplementationRollover(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev2cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_implementation_rollover/test_collocation_implementation_rollover.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())



    def test_collocationNOOPMixture(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node2_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_mix2/test_collocation_mix2.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())


    def test_collocationNoCollocation(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node2_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_no_collocation/test_no_collocation.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationMixAndNoCollocation(self):

        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev1cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_node1_2dev2cap/DeviceManager.dcd.xml", debug=self.debuglevel)

        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_no_collocation_mix1/test_no_collocation_mix1.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)

        appFact = domMgr._get_applicationFactories()[0]

        app = None
        try:
          app = appFact.create(appFact._get_name(), [], [])
        except:
          pass

        self.assertNotEqual(app, None )

        ## need to check that each components's alternate implementation was deployed

        if ( app ) :
          app.stop()
          app.releaseObject()

        self._domMgr.uninstallApplication(appFact._get_identifier())


    def _getBogoMips(self, device):
        return device.query([CF.DataType(id="DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8", value=any.to_any(None))])[0].value._v


        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_collocationFailFast(self):
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        # Launch the node with a device that can handle one component first,
        # then launch the node that can't handle any (it also helps that
        # 'test_collocation_bad1_node:bad_device_1' comes before
        # 'test_collocation_bad2_node:bad_device_2' in lexicographical order)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_bad1_node/DeviceManager.dcd.xml", debug=3)
        self.assertNotEqual(devMgr, None)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_bad2_node/DeviceManager.dcd.xml", debug=3)
        self.assertNotEqual(devMgr, None)

        # Find the no-capacity device
        device = devMgr._get_registeredDevices()[0]
        self.assertEqual(self._getProperty(device, 'supported_components'), 0)

        # Use the 'test_collocation_single' waveform, which has just a single
        # 2-element collocation; the first node will be able to place one
        # component, while the second should fail on the first component.
        domMgr.installApplication("/waveforms/test_collocation_single/test_collocation_single.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        # Mark the number of allocations attempted so far on our no-capacity
        # device, then try to create a the application. The first device
        # can place the first component, but fails to place the second; the
        # second device should fail when placing the first component.
        allocations_pre = self._getProperty(device, 'allocation_attempts')
        try:
            app = appFact.create(appFact._get_name(), [], [])
            app.releaseObject()
            self.fail("Expected app creation to fail")
        except CF.ApplicationFactory.CreateApplicationRequestError:
            # This is expected
            pass

        # Clean up a little
        domMgr.uninstallApplication(appFact._get_identifier())

        # Determine the number of allocations attempted on our no-capacity
        # device; it should be 1 (for the first attempted component placement).
        # In prior releases, it would attempt to place the second component
        # (and so on), leading to extra allocation calls.
        allocations_delta = self._getProperty(device, 'allocation_attempts') - allocations_pre

        # Fail if more than one allocation was attempted
        self.assertTrue(allocations_delta <= 1, 'ApplicationFactory continued to attempt allocations on device after failure')

    def _getProperty(self, device, identifier):
        return device.query([CF.DataType(identifier, any.to_any(None))])[0].value._v

    def test_collocationNoExecDevice(self):
        """
        This test requires visual inspection to ensure that non-executable
        devices do not cause error messages when the ApplicationFactory tries
        to place collocated components.
        """
        nodebooter, domMgr = self.launchDomainManager()
        self.assertNotEqual(domMgr, None)

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_noexec_node/DeviceManager.dcd.xml")
        self.assertNotEqual(devMgr, None)

        domMgr.installApplication("/waveforms/test_collocation_single/test_collocation_single.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        try:
          app = appFact.create(appFact._get_name(), [], [])
          app.releaseObject()
          self.fail('Application created with no executable devices')
        except:
          pass

    def test_collocationLastSuccessfulDevice(self):
        nodebooter, domMgr = self.launchDomainManager(debug=self.debuglevel)
        self.assertNotEqual(domMgr, None)

        # Launch the node with a device that can't satisfy the host collocation
        # first, then launch the node with a device that can (it also helps
        # that 'test_collocation_bad1_node:bad_device_1' comes before
        # 'test_collocation_good_node:good_device_1' in lexicographical order)
        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_bad1_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)
        # Store a reference to the "bad" device
        bad_device = devMgr._get_registeredDevices()[0]
        self.assertTrue('bad' in bad_device._get_identifier())

        nodebooter, devMgr = self.launchDeviceManager("/nodes/test_collocation_good_node/DeviceManager.dcd.xml", debug=self.debuglevel)
        self.assertNotEqual(devMgr, None)

        # Use the "single" collocation waveform, which should fail on the "bad"
        # device, but succeed on the "good" device
        domMgr.installApplication("/waveforms/test_collocation_single/test_collocation_single.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        appFact = domMgr._get_applicationFactories()[0]

        # Save pre-create allocation attempt total on bad device
        allocations_pre = self._getProperty(bad_device, 'allocation_attempts')

        # Create the first instance of the application; we expect the creation
        # to succeed, advancing the domain's "last successful device" pointer
        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            self.fail('Unable to create application')

        # Release the application to free up the capacity
        app.releaseObject()

        # Ensure that the bad device was considered for deployment; this is the
        # only externally visible information we can determine about the "last
        # successful device" pointer
        allocations_post = self._getProperty(bad_device, 'allocation_attempts')
        self.assertTrue(allocations_post > allocations_pre)

        # Try to create another instance of the application; it should start
        # with the good device, not considering the bad one
        try:
            app = appFact.create(appFact._get_name(), [], [])
        except:
            self.fail('Unable to create application again')

        # Release the application to free up the capacity
        app.releaseObject()

        # Clean up a little
        domMgr.uninstallApplication(appFact._get_identifier())

        # Verify that no allocation attempts were made on the bad device
        self.assertEqual(allocations_post, self._getProperty(bad_device, "allocation_attempts"))
