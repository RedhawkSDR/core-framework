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
from ossie.cf import CF, ExtendedCF
from omniORB import any, CORBA
from ossie import properties

class SADUsesDeviceTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager(debug=self.debuglevel)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_SADUsesDevice/DeviceManager.dcd.xml", debug=self.debuglevel)
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def createAppFact(self, name):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        sadpath = "/waveforms/SADUsesDeviceWave/SADUsesDeviceWave"+name+".sad.xml"
        self._domMgr.installApplication(sadpath)

        for curr in self._domMgr._get_applicationFactories():
            if curr._get_name() == "SADUsesDeviceWave"+name:
                return curr

    def _checkSimplePropValue(self, device_name, prop_id, expected_value):
        # Make sure that the allocation was made to the device
        prop = CF.DataType(id=prop_id, value=any.to_any(None))
        for dev in self._devMgr._get_registeredDevices():
            if dev._get_label() == device_name:
                allocRes = dev.query([prop])
        self.assertEquals(allocRes[0].value.value(), expected_value)

    def test_Matching(self):
        appFact = self.createAppFact("Eq")
        self._app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self._app, None)

        appFact = self.createAppFact("BadEq")
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        appFact = self.createAppFact("Ne")
        self._app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self._app, None)

        appFact = self.createAppFact("BadNe")
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        appFact = self.createAppFact("Ge")
        self._app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self._app, None)

        appFact = self.createAppFact("BadGe")
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        appFact = self.createAppFact("Le")
        self._app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self._app, None)

        appFact = self.createAppFact("BadLe")
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

    def test_SimpleExternal(self):
        appFact = self.createAppFact("ExternalSimple")
        self._app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self._app, None)

        # Make sure that the allocation was made to the device
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 8)

        # Make sure values are deallocated on release
        self._app.releaseObject()
        self._app = None
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 10)

    def test_StructExternal(self):
        appFact = self.createAppFact("ExternalStruct")
        self._app = appFact.create(appFact._get_name(), [], [])
        self.assertNotEqual(self._app, None)

        # Make sure that the allocation was made to the device
        prop = CF.DataType(id="DCE:001fad60-b4b3-4ed2-94cb-40e1d956bf4f", value=any.to_any(None))
        for dev in self._devMgr._get_registeredDevices():
            if dev._get_label() == 'BasicTestDevice1':
                allocRes = dev.query([prop])
        self.assertEquals(allocRes[0].value.value()[0].value.value(), 90)
        self.assertAlmostEquals(allocRes[0].value.value()[1].value.value(), 0.9)

        # Make sure values are deallocated on release
        self._app.releaseObject()
        self._app = None
        prop = CF.DataType(id="DCE:001fad60-b4b3-4ed2-94cb-40e1d956bf4f", value=any.to_any(None))
        for dev in self._devMgr._get_registeredDevices():
            if dev._get_label() == 'BasicTestDevice1':
                allocRes = dev.query([prop])
        self.assertEquals(allocRes[0].value.value()[0].value.value(), 100)
        self.assertAlmostEquals(allocRes[0].value.value()[1].value.value(), 1.0)

    def test_connections(self):
        appFact = self.createAppFact("ConnectionDevProvides")
        self._app = appFact.create(appFact._get_name(), [], [])

        appFact = self.createAppFact("ConnectionDevUses")
        self._app = appFact.create(appFact._get_name(), [], [])

    def test_externalFail(self):
        appFact = self.createAppFact("ExternalFail")
        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])

        # Make sure that none of the allocations were made
        prop = CF.DataType(id="DCE:001fad60-b4b3-4ed2-94cb-40e1d956bf4f", value=any.to_any(None))
        prop2 = CF.DataType(id="simple_alloc", value=any.to_any(None))
        for dev in self._devMgr._get_registeredDevices():
            if dev._get_label() == 'BasicTestDevice1':
                allocRes = dev.query([prop])
            elif dev._get_label() == "SADUsesDevice_1":
                allocRes2 = dev.query([prop2])
        self.assertEquals(allocRes[0].value.value()[0].value.value(), 100)
        self.assertAlmostEquals(allocRes[0].value.value()[1].value.value(), 1.0)
        self.assertEquals(allocRes2[0].value.value(), 10)


    def test_math(self):
        appFact = self.createAppFact("MathAdd")
        self._app = appFact.create(appFact._get_name(), [], [])
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 6)
        # Make sure values are deallocated on release
        self._app.releaseObject()
        self._app = None
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 10)

        appFact = self.createAppFact('MathMult')
        self._app = appFact.create(appFact._get_name(), [], [])
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 4)
        # Make sure values are deallocated on release
        self._app.releaseObject()
        self._app = None
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 10)

        appFact = self.createAppFact('MathSub')
        self._app = appFact.create(appFact._get_name(), [], [])
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 0)
        # Make sure values are deallocated on release
        self._app.releaseObject()
        self._app = None
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 10)

        appFact = self.createAppFact('MathDiv')
        self._app = appFact.create(appFact._get_name(), [], [])
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 5)
        # Make sure values are deallocated on release
        self._app.releaseObject()
        self._app = None
        self._checkSimplePropValue('SADUsesDevice_1', 'simple_alloc', 10)

    def test_connectionBadRefid(self):
        """
        Test that specifiying an invalid usesdeviceref for a connection does not
        crash the domain.
        """
        appFact = self.createAppFact('ConnectionBadDeviceRef')

        try:
            self._app = appFact.create(appFact._get_name(), [], [])
        except CF.ApplicationFactory.CreateApplicationError:
            # The app should fail creation, due to the invalid connection
            pass
        except CORBA.COMM_FAILURE:
            self.fail('Invalid usesdeviceref crashed domain manager')
        else:
            self.fail('Invalid usesdeviceref did not prevent app creation')

    def test_MultipleUsesDevices(self):
        devices = dict((dev._get_label(), dev) for dev in self._devMgr._get_registeredDevices())

        appFact = self.createAppFact('Multiple')
        self._app = appFact.create(appFact._get_name(), [], [])

        # Check the usesdevice connections by inspecting the app's external
        # ports against the devices
        for connection in self._app.getPort('resource_out')._get_connections():
            deviceId = connection.port._get_identifier()
            if connection.connectionId == 'connection1':
                self.assertTrue(deviceId, devices['SADUsesDevice_1']._get_identifier())
            elif connection.connectionId == 'connection2':
                self.assertTrue(deviceId, devices['BasicTestDevice1']._get_identifier())

        # Try to create another instance of the application; there should only
        # be enough capacity to satisfy one of the usesdevices
        prop = CF.DataType('DCE:001fad60-b4b3-4ed2-94cb-40e1d956bf4f', any.to_any(None))
        pre_value = properties.props_to_dict(devices['BasicTestDevice1'].query([prop])[0].value._v)
        try:
            app2 = appFact.create(appFact._get_name(), [], [])
        except CF.ApplicationFactory.CreateApplicationError:
            # The first app instance should have exhausted the capacity on the
            # SADUsesDevice
            pass
        else:
            self.fail('Device should not have enough capacity to satisfy usesdevice')

        # Make sure that the successful usesdevice was deallocated
        post_value = properties.props_to_dict(devices['BasicTestDevice1'].query([prop])[0].value._v)
        self.assertEqual(pre_value['long_capacity'], post_value['long_capacity'])
        self.assertAlmostEqual(pre_value['float_capacity'], post_value['float_capacity'])
