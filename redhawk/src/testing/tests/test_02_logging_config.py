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
from _unitTestHelpers import scatest
from omniORB import URI, any
from ossie.cf import CF

class LoggingConfigTest(scatest.CorbaTestCase):

    def _testLoggingConfigURI(self, domLoggingConfigArg, devLoggingConfigArg):
        """A common function used to test various forms of the log4cxx URI."""
        if not domLoggingConfigArg:
            domLoggingConfigArg = ""
        domNB, domMgr = self.launchDomainManager(loggingURI=domLoggingConfigArg)
        self.assertNotEqual(domMgr, None)

        prop = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any(None))
        result = domMgr.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, "LOGGING_CONFIG_URI")
        domLoggingConfigURI = result[0].value._v

        # Launch a device manager
        if not devLoggingConfigArg:
            devLoggingConfigArg = ""
        devNB, devMgr = self.launchDeviceManager("/nodes/test_EmptyNode/DeviceManager.dcd.xml", loggingURI=devLoggingConfigArg)
        self.assertNotEqual(devMgr, None)

        prop = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any(None))
        result = devMgr.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, "LOGGING_CONFIG_URI")
        devLoggingConfigURI = result[0].value._v

        return domLoggingConfigURI, devLoggingConfigURI


    def test_NoFileURI(self):
        domLoggingConfigArg = None
        devLoggingConfigArg = None

        domLoggingConfigUri, devLoggingConfigUri = self._testLoggingConfigURI(domLoggingConfigArg, devLoggingConfigArg)

        self.assert_(domLoggingConfigUri in (None, ""))
        self.assert_(devLoggingConfigUri in (None, ""))

    def test_RelativeFileURI(self):
        domLoggingConfigArg = "dom/mgr/logging.properties"
        devLoggingConfigArg = "dev/mgr/logging.properties"

        domLoggingConfigUri, devLoggingConfigUri = self._testLoggingConfigURI(domLoggingConfigArg, devLoggingConfigArg)

        expectedDomLoggingConfigUri = "file://" + os.path.join(scatest.getSdrPath(), "dom/mgr/logging.properties")
        expectedDevLoggingConfigUri = "file://" + os.path.join(scatest.getSdrPath(), "dev/mgr/logging.properties")
        self.assertEqual(domLoggingConfigUri, expectedDomLoggingConfigUri)
        self.assertEqual(devLoggingConfigUri, expectedDevLoggingConfigUri)

    def test_AbsoluteFileURI(self):
        domLoggingConfigArg = os.path.join(scatest.getSdrPath(), "dom/mgr/logging.properties")
        devLoggingConfigArg = os.path.join(scatest.getSdrPath(), "dev/mgr/logging.properties")

        domLoggingConfigUri, devLoggingConfigUri = self._testLoggingConfigURI(domLoggingConfigArg, devLoggingConfigArg)

        expectedDomLoggingConfigUri = "file://" + os.path.join(scatest.getSdrPath(), "dom/mgr/logging.properties")
        expectedDevLoggingConfigUri = "file://" + os.path.join(scatest.getSdrPath(), "dev/mgr/logging.properties")
        self.assertEqual(domLoggingConfigUri, expectedDomLoggingConfigUri)
        self.assertEqual(devLoggingConfigUri, expectedDevLoggingConfigUri)

    def test_ScaURI(self):
        domLoggingConfigArg = "sca:///mgr/logging.properties"
        devLoggingConfigArg = "sca:///mgr/logging.properties"

        domLoggingConfigUri, devLoggingConfigUri = self._testLoggingConfigURI(domLoggingConfigArg, devLoggingConfigArg)

        self.assertEqual(domLoggingConfigUri, domLoggingConfigArg)
        self.assertEqual(devLoggingConfigUri, devLoggingConfigArg)

    def test_ApplicationFactoryURIPassDown(self):
        domNB, domMgr = self.launchDomainManager(loggingURI="sca:///mgr/logging.properties")
        self.assertNotEqual(domMgr, None)

        # Launch a device manager
        devNB, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", loggingURI="dev/mgr/logging.properties")
        self.assertNotEqual(domMgr, None)

        # Double check the DomainManager LOGGING_CONFIG_URI
        prop = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any(None))
        result = domMgr.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, "LOGGING_CONFIG_URI")
        devLoggingConfigURI = result[0].value._v
        self.assertEqual(devLoggingConfigURI, "sca:///mgr/logging.properties")

        # Launch an application
        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        app = appFact.create(appFact._get_name(), [], [])

        # Get the desired component
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        # Check the components exec params
        execparams = comp.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = any.from_any(execparams.value)
        execparams = {}
        for b in args:
            a = eval(b)
            name = a[0]
            value = a[1]
            execparams[name] = value

        self.assert_(execparams.has_key("LOGGING_CONFIG_URI"))
        self.assertEqual(execparams["LOGGING_CONFIG_URI"].split("?fs=")[0], "sca:///mgr/logging.properties")
        execparamObj = self._orb.string_to_object(execparams["LOGGING_CONFIG_URI"].split("?fs=")[1])
        # Need to compare actual objects since the IOR strings could potentially differ for the same object
        self.assert_(domMgr._get_fileMgr()._is_equivalent(execparamObj))

        # Launch an application with a C++ component to exercise Resource_impl logging configure
        domMgr.installApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 2)
        for appFact in domMgr._get_applicationFactories():
            if appFact._get_name() == "TestCppProps":
                app = appFact.create(appFact._get_name(), [], [])
                break
        self.assertEqual(len(domMgr._get_applications()), 2)

    def test_ApplicationFactoryCreateURIOverride(self):
        domNB, domMgr = self.launchDomainManager(loggingURI="dom/mgr/logging.properties")
        self.assertNotEqual(domMgr, None)

        # Launch a device manager
        devNB, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", loggingURI="dev/mgr/logging.properties")
        self.assertNotEqual(domMgr, None)

        # Double check the DomainManager LOGGING_CONFIG_URI
        prop = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any(None))
        result = domMgr.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, "LOGGING_CONFIG_URI")
        devLoggingConfigURI = result[0].value._v
        expectedDomLoggingConfigUri = "file://" + os.path.join(scatest.getSdrPath(), "dom/mgr/logging.properties")
        self.assertEqual(devLoggingConfigURI, expectedDomLoggingConfigUri)

        # Launch an application
        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        uriOverride = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any("sca:///mgr/logging.properties"))
        app = appFact.create(appFact._get_name(), [uriOverride], [])

        # Get the desired component
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        # Check the components exec params
        execparams = comp.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = any.from_any(execparams.value)
        execparams = {}
        for b in args:
            a = eval(b)
            name = a[0]
            value = a[1]
            execparams[name] = value

        self.assert_(execparams.has_key("LOGGING_CONFIG_URI"))
        self.assertEqual(execparams["LOGGING_CONFIG_URI"].split("?fs=")[0], "sca:///mgr/logging.properties")
        execparamObj = self._orb.string_to_object(execparams["LOGGING_CONFIG_URI"].split("?fs=")[1])
        # Need to compare actual objects since the IOR strings could potentially differ for the same object
        self.assert_(domMgr._get_fileMgr()._is_equivalent(execparamObj))

        # Launch an application with a C++ component to exercise Resource_impl logging configure
        domMgr.installApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 2)
        for appFact in domMgr._get_applicationFactories():
            if appFact._get_name() == "TestCppProps":
                app = appFact.create(appFact._get_name(), [uriOverride], [])
                break
        self.assertEqual(len(domMgr._get_applications()), 2)

    def test_ComponentLoggingRedirect(self):
        logfile = '/var/tmp/commandwrapper.log'
        fp=open(logfile,'w')
        fp.close()

        domNB, domMgr = self.launchDomainManager(loggingURI="dom/mgr/logging_2.properties")
        self.assertNotEqual(domMgr, None)

        # Launch a device manager
        devNB, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", loggingURI="dev/mgr/logging.properties")
        self.assertNotEqual(domMgr, None)

        # Double check the DomainManager LOGGING_CONFIG_URI
        prop = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any(None))
        result = domMgr.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, "LOGGING_CONFIG_URI")
        devLoggingConfigURI = result[0].value._v
        expectedDomLoggingConfigUri = "file://" + os.path.join(scatest.getSdrPath(), "dom/mgr/logging_2.properties")
        self.assertEqual(devLoggingConfigURI, expectedDomLoggingConfigUri)

        # Launch an application
        domMgr.installApplication("/waveforms/CommandWrapper/CommandWrapper.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 1)
        self.assertEqual(len(domMgr._get_applications()), 0)

        appFact = domMgr._get_applicationFactories()[0]
        uriOverride = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any("sca:///mgr/logging_2.properties"))
        app = appFact.create(appFact._get_name(), [uriOverride], [])

        # Get the desired component
        self.assertEqual(len(app._get_componentNamingContexts()), 1)
        compName = app._get_componentNamingContexts()[0]
        comp = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        self.assertNotEqual(comp, None)

        # Check the components exec params
        execparams = comp.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = any.from_any(execparams.value)
        execparams = {}
        for a in args:
            name = a[0]
            value = a[1]
            execparams[name] = value

        app.start()
        app.stop()

        app.releaseObject()

        fp=open(logfile,'r')
        log_contents=fp.read()
        fp.close()
        count_debugs=log_contents.count('DEBUG:')
        count_command=log_contents.count('DEBUG:CommandWrapper')
        self.assertEqual(count_debugs,count_command)

        # Launch an application with a C++ component to exercise Resource_impl logging configure
        domMgr.installApplication("/waveforms/TestCppProps/TestCppProps.sad.xml")
        self.assertEqual(len(domMgr._get_applicationFactories()), 2)
        for appFact in domMgr._get_applicationFactories():
            if appFact._get_name() == "TestCppProps":
                app = appFact.create(appFact._get_name(), [uriOverride], [])
                break
        self.assertEqual(len(domMgr._get_applications()), 1)

    def test_DeviceManagerURIPassDown(self):
        # Test that the device manager passes the LOGGING_CONFIG_URI to the devices
        domNB, domMgr = self.launchDomainManager(loggingURI="")
        self.assertNotEqual(domMgr, None)

        # Launch a device manager
        devNB, devMgr = self.launchDeviceManager("/nodes/test_BasicTestDevice_node/DeviceManager.dcd.xml", loggingURI="sca:///mgr/logging.properties")
        self.assertNotEqual(devMgr, None)

        # Double check the DeviceManager LOGGING_CONFIG_URI
        prop = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any(None))
        result = devMgr.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, "LOGGING_CONFIG_URI")
        devLoggingConfigURI = result[0].value._v
        self.assertEqual(devLoggingConfigURI, "sca:///mgr/logging.properties")

        # Check the devices exec params
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        execparams = device.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = execparams.value._v.split()
        execparams = {}
        while len(args) > 0:
            name = args.pop(0)
            value = args.pop(0)
            execparams[name] = value

        self.assert_(execparams.has_key("LOGGING_CONFIG_URI"))
        self.assertEqual(execparams["LOGGING_CONFIG_URI"].split("?fs=")[0], "sca:///mgr/logging.properties")
        execparamObj = self._orb.string_to_object(execparams["LOGGING_CONFIG_URI"].split("?fs=")[1])
        # Need to compare actual objects since the IOR strings could potentially differ for the same object
        self.assert_(devMgr._get_fileSys()._is_equivalent(execparamObj))

    def test_DeviceManagerURIOverride(self):
        # Test that the device manager DCD can override the log4cxx URI
        domNB, domMgr = self.launchDomainManager(loggingURI="")
        self.assertNotEqual(domMgr, None)

        # Launch a device manager
        devNB, devMgr = self.launchDeviceManager("/nodes/test_LoggingBasicTestDevice_node/DeviceManager.dcd.xml", loggingURI="dev/mgr/logging.properties")
        self.assertNotEqual(devMgr, None)

        # Double check the DeviceManager LOGGING_CONFIG_URI
        prop = CF.DataType(id="LOGGING_CONFIG_URI", value=any.to_any(None))
        result = devMgr.query([prop])
        self.assertEqual(len(result), 1)
        self.assertEqual(result[0].id, "LOGGING_CONFIG_URI")
        devLoggingConfigURI = result[0].value._v
        expectedDevLoggingConfigUri = "file://" + os.path.join(scatest.getSdrPath(), "dev/mgr/logging.properties")
        self.assertEqual(devLoggingConfigURI, expectedDevLoggingConfigUri)

        # Check the devices exec params
        self.assertEqual(len(devMgr._get_registeredDevices()), 1)
        device = devMgr._get_registeredDevices()[0]
        execparams = device.query([CF.DataType(id="DCE:85d133fd-1658-4e4d-b3ff-1443cd44c0e2", value=any.to_any(None))])[0]
        args = execparams.value._v.split()
        execparams = {}
        while len(args) > 0:
            name = args.pop(0)
            value = args.pop(0)
            execparams[name] = value

        self.assert_(execparams.has_key("LOGGING_CONFIG_URI"))
        devMgrFileSysIOR = self._orb.object_to_string(devMgr._get_fileSys())
        self.assertEqual(execparams["LOGGING_CONFIG_URI"].split("?fs=")[0], "sca:///mgr/logging.properties")
        execparamObj = self._orb.string_to_object(execparams["LOGGING_CONFIG_URI"].split("?fs=")[1])
        # Need to compare actual objects since the IOR strings could potentially differ for the same object
        self.assert_(devMgr._get_fileSys()._is_equivalent(execparamObj))


