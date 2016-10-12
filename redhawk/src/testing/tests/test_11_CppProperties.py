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

from omniORB import any
import unittest
import scatest
from ossie.cf import CF
from omniORB import CORBA

class CppPropertiesTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager(debug=9)
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_ExecutableDevice_node/DeviceManager.dcd.xml", debug=9)
        self._app = None
        if self._domMgr:
            try:
                sadpath = "/waveforms/TestCppProps/TestCppProps.sad.xml"
                self._domMgr.installApplication(sadpath)
                appFact = self._domMgr._get_applicationFactories()[0]
                self._app = appFact.create(appFact._get_name(), [], [])
            except:
                pass

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def preconditions(self):
        self.assertNotEqual(self._domMgr, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "DeviceManager not available")
        self.assertNotEqual(self._app, None, "Application not created")

    def test_PropertyCallbacks(self):
        self.preconditions()

        # The TestCppProps component is also the assembly controller, so we can
        # use the TestableObject interface via the Application object to run
        # the property callback tests for member functions...
        props = self._app.query([])
        for result in self._app.runTest(0, props):
            self.assert_(result.value._v)

        # ...and static functions.
        for result in self._app.runTest(1, props):
            self.assert_(result.value._v)

    def test_NilProperty(self):
        self.preconditions()

        # Use the prop_str property for this test.
        prop = CF.DataType("DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", any.to_any(None))

        # Check that the default value is not 'None'.
        propVal = self._app.query([prop])[0]
        self.assertNotEqual(propVal.value.value(), None)

        # Enable nil values for the property.
        prop.value = any.to_any(True)
        self._app.runTest(2, [prop])

        # Explicitly configure the value to None and verify that it reports back
        # as 'None'.
        prop.value = any.to_any(None)
        self._app.configure([prop])
        propVal = self._app.query([prop])[0]
        self.assertEqual(propVal.value.value(), None)        

        # Clear the property's nil status and check that it no longer returns
        # 'None'.
        prop.value = any.to_any(False)
        self._app.runTest(3, [prop])
        propVal = self._app.query([prop])[0]
        self.assertNotEqual(propVal.value.value(), None)

    def test_fullQuery(self):
        devBooter_2, self._devMgr_2 = self.launchDeviceManager("/nodes/props_test_node/DeviceManager.dcd.xml", debug=9)
        dev = self._devMgr_2._get_registeredDevices()[0]
        results = dev.query([])
        ids = []
        for result in results:
            ids.append(result.id)
        self.assertEqual("DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb" in ids, True)
        self.assertEqual("DCE:1d9bed5e-1e6c-4607-9391-0e692f8fd1ae" in ids, True)
        self.assertEqual("DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d" in ids, True)
        self.assertEqual("execparam_readwrite" in ids, True)
        self.assertEqual("simple_config" in ids, True)
        self.assertEqual(len(results), 5)

    def test_NilRead(self):
        #nodebooter, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", debug=9)
        #self.assertNotEqual(devMgr, None)
        
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        self._domMgr.installApplication("/waveforms/ticket_2093/ticket_2093.sad.xml")
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        appFact = self._domMgr._get_applicationFactories()[1]
        app = appFact.create(appFact._get_name(), [], [])

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(self._domMgr._get_applications()), 2)

        props = app.query([])
        # Sequences cannot be nil, just a zero-length array
        #self.assertEqual(props[0].value._t, CORBA.TC_null)
        self.assertEqual(props[1].value._t, CORBA.TC_null)
        self.assertEqual(props[2].value._t, CORBA.TC_null)
        # Sequences cannot be nil, just a zero-length array
        #self.assertEqual(props[3].value._t, CORBA.TC_null)

        ## Clean-up
        app.stop()
        app.releaseObject()

        self.assertEqual(len(self._domMgr._get_applicationFactories()), 2)
        self.assertEqual(len(self._domMgr._get_applications()), 1)

        self._domMgr.uninstallApplication(appFact._get_identifier())

    def test_InvalidConfigureProperty(self):
        self.preconditions()

        # Use the prop_str property for this test.
        props = [CF.DataType("not valide property id", any.to_any(None))]
        props.append(CF.DataType("DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", any.to_any(None)))

        # Make sure that the appropriate exception is raised
        self.assertRaises(CF.PropertySet.PartialConfiguration, self._app.configure, props)

        # Make sure that the app is still good
        prop = CF.DataType("DCE:4e7c1977-5f53-4061-bae7-cb8c1072f4b7", any.to_any(None))
        # Check that the default value is not 'None'.
        propVal = self._app.query([prop])[0]
        self.assertNotEqual(propVal.value.value(), None)

