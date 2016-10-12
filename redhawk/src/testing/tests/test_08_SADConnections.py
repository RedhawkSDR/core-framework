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
from omniORB import CORBA, URI, any
from ossie.cf import CF
import CosNaming

class SADConnectionsTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_PortTestDevice_node/DeviceManager.dcd.xml")
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def _createApp(self, appName):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        sadpath = "/waveforms/PortConnect" + appName + "/PortConnect" + appName + ".sad.xml"
        self._domMgr.installApplication(sadpath)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        try:
            self._app = appFact.create(appFact._get_name(), [], [])
        except:
            self.fail("Did not create application " + appName)

    def _getComponents(self):
        # Get references to the waveform's components.
        components = {}
        for compName in self._app._get_componentNamingContexts():
            usageName = compName.elementId.split('/')[-1]
            components[usageName] = self._root.resolve(URI.stringToName(compName.elementId))._narrow(CF.Resource)
        return components

    def test_ProvidesPort(self):
        self._createApp('ProvidesPort')
        components = self._getComponents()

        # Use the TestableObject interface to check that 'PortTest1' has
        # exactly one connection, and that it's connected to the provides port
        # of 'PortTest2'.
        ids = components['PortTest1'].runTest(0, [])
        self.assertEqual(len(ids), 1)
        expectedId = components['PortTest2']._get_identifier() + '/resource_in'
        self.assertEqual(str(ids[0].value.value()), expectedId)

    def test_ComponentSupportedInterface(self):
        self._createApp('ComponentSupportedInterface')
        components = self._getComponents()

        # Use the TestableObject interface to check that 'PortTest1' has
        # exactly one connection, and that it's connected to 'PortTest2'.
        ids = components['PortTest1'].runTest(0, [])
        self.assertEqual(len(ids), 1)
        expectedId = components['PortTest2']._get_identifier()
        self.assertEqual(str(ids[0].value.value()), expectedId)

    def test_FindByNamingService(self):
        self._createApp('FindByNamingService')
        components = self._getComponents()

        # Use the TestableObject interface to check that 'PortTest1' has
        # exactly one connection, and that it's connected to 'PortTest2'.
        ids = components['PortTest1'].runTest(0, [])
        self.assertEqual(len(ids), 1)
        expectedId = components['PortTest2']._get_identifier()
        self.assertEqual(str(ids[0].value.value()), expectedId)

    def test_FindByAbsoluteNamingService(self):
        from ossie.utils import sb
        comp=sb.Component('PortTest')
        orb = CORBA.ORB_init()
        obj = orb.resolve_initial_references("NameService")
        rootContext = obj._narrow(CosNaming.NamingContext)
        bindingName = URI.stringToName("PortTest2")
        rootContext.rebind(bindingName, comp.ref)

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        sadpath = "/waveforms/PortConnectFindByAbsoluteNamingService/PortConnectFindByAbsoluteNamingService.sad.xml"
        self._domMgr.installApplication(sadpath)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        try:
            self._app = appFact.create(appFact._get_name(), [], [])
        except:
            rootContext.unbind(bindingName)
            self.fail("Did not create application PortConnectFindByAbsoluteNamingService")
        components = self._getComponents()

        # Use the TestableObject interface to check that 'PortTest1' has
        # exactly one connection, and that it's connected to 'PortTest2'.
        ids = components['PortTest1'].runTest(0, [])
        if (len(ids) != 1):
            rootContext.unbind(bindingName)
            self.fail("There are more than one connection on PortTest1")
        expectedId = comp.ref._get_identifier()
        if (str(ids[0].value.value()) != expectedId):
            rootContext.unbind(bindingName)
            self.fail("The expected id does not match")
        rootContext.unbind(bindingName)
        sb.domainless._cleanUpLaunchedComponents()

    def test_FindByFileManager(self):
        self._createApp('FindByFileManager')
        components = self._getComponents()

        listing = components['FileManagerPortTest1'].runTest(0, [])
        value = any.from_any(listing[0].value)
        foundMgrDir = False
        for entry in value:
            if entry['kind'] == CF.FileSystem.DIRECTORY:
                if entry['name'] == 'mgr':
                    foundMgrDir = True
                    break
        self.assertEqual(foundMgrDir, True)

    def test_DeviceThatLoadedThisComponent(self):
        self._createApp('DeviceThatLoadedComponent')
        components = self._getComponents()

        # Use the TestableObject interface to check that 'PortTest1' has
        # exactly one connection, and that it's connected to the provides port
        # of the device on which it's loaded.
        ids = components['PortTest1'].runTest(0, [])
        self.assertEqual(len(ids), 1)
        # NB: There is only one component, so we can "cheat" here.
        deviceId = self._app._get_componentDevices()[0].assignedDeviceId
        self.assertEqual(ids[0].value.value(), deviceId + '/resource_in')

    def test_DeviceUsedByThisComponent(self):
        self._createApp('DeviceUsedByComponent')
        components = self._getComponents()

        # Use the TestableObject interface to check that 'PortTest1' has
        # exactly one connection
        # NB: The `usesdevice` can be satisfied by either of the devices, so
        #     just test that it's connected to one of them.
        ids = components['PortTest1'].runTest(0, [])
        self.assertEqual(len(ids), 1)
        usedDevId = any.from_any(ids[0].value).split('/')[0]
        devIds = [dev._get_identifier() for dev in self._devMgr._get_registeredDevices()]
        self.assert_(usedDevId in devIds)

    def test_ExternalPorts(self):
        self._createApp('ExternalPort')
        components = self._getComponents()
        expectedId = components['PortTest1']._get_identifier() + '/resource_in'

        # Make sure the application's "resource_in" matches the first component's
        # "resource_in" (must be kept in sync with SAD).
        providesPort = self._app.getPort('resource_in')
        self.assertEqual(providesPort._get_identifier(), expectedId)

        # Connect the application's external ports together.
        usesPort = self._app.getPort('resource_out')
        connectionId = 'test_connection'
        usesPort.connectPort(providesPort, connectionId)

        # Use the TestableObject interface to check that 'PortTest2' has
        # exactly one connection, and that it's connected to the provides port
        # of 'PortTest1'.
        ids = components['PortTest2'].runTest(0, [])
        self.assertEqual(len(ids), 1)
        self.assertEqual(str(ids[0].value.value()), expectedId)

        # Disconnect the test connection.
        usesPort.disconnectPort(connectionId)

    def test_ExternalPortsRename(self):
        # Makes sure that duplicate external port name throws an error
        self.assertRaises(CF.DomainManager.ApplicationInstallationError, self._createApp, 'ExternalPortRenameDuplicate')

        self._createApp('ExternalPortRename')
        components = self._getComponents()

        # Make sure old names raise errors
        self.assertRaises(CF.PortSupplier.UnknownPort, self._app.getPort, 'resouce_in')
        self.assertRaises(CF.PortSupplier.UnknownPort, self._app.getPort, 'resource_out')

        # Make sure we can get the renamed port
        providesPort = self._app.getPort('rename_resource_in')
        usesPort = self._app.getPort('rename_resource_out')
        self.assertNotEqual(providesPort, None)
        self.assertNotEqual(usesPort, None)

        # Connect the application's external ports together.
        connectionId = 'test_connection'
        usesPort.connectPort(providesPort, connectionId)

        # Disconnect the test connection.
        usesPort.disconnectPort(connectionId)

        # Makes sure that the components can still get ports based of their orig name
        usesPort = components['PortTest1'].getPort('resource_out')
        providesPort = components['PortTest1'].getPort('resource_in')
        usesPort = components['PortTest2'].getPort('resource_out')
        providesPort = components['PortTest2'].getPort('resource_in')

    def _test_Service(self, connection):
        svcBooter, svcMgr = self.launchDeviceManager("/nodes/test_BasicService_node/DeviceManager.dcd.xml")
        self.assertNotEqual(svcMgr, None)

        self._createApp('Service' + connection)
        components = self._getComponents()

        self.assertEqual(len(svcMgr._get_registeredServices()), 1)
        service = svcMgr._get_registeredServices()[0]

        # Compare all of the properties returned directly from the service, and
        # via the runTest interface. They should all be the same, assuming that
        # the port was connected correctly.
        props = service.serviceObject.query([])
        testOut = components['PortTest1'].runTest(1, [])
        for ii in range(1):
            self.assertEqual(props[ii].id, testOut[ii].id)
            self.assertEqual(any.from_any(props[ii].value), any.from_any(testOut[ii].value))

    def _test_NoService(self, connection):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)
        sadpath = "/waveforms/PortConnectService" + connection + "/PortConnectService" + connection + ".sad.xml"
        self._domMgr.installApplication(sadpath)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        self.assertRaises(CF.ApplicationFactory.CreateApplicationError, appFact.create, appFact._get_name(), [], [])
        self.assertEqual(self._app, None)
        self.assertEqual(len(self._domMgr._get_applications()),0)
        self.assertEqual(self._domMgr._get_identifier(),'DCE:5f52f645-110f-4142-8cc9-4d9316ddd958')

    def test_ServiceName(self):
        self._test_Service('Name')

    def test_ServiceType(self):
        self._test_Service('Type')

    def test_NoServiceType(self):
        self._test_NoService('Type')

    def test_NoServiceName(self):
        self._test_NoService('Name')

    def test_UnregisterServiceName(self):
        svcBooter, svcMgr = self.launchDeviceManager("/nodes/test_BasicService_node/DeviceManager.dcd.xml")
        self.assertNotEqual(svcMgr, None)

        self._createApp('ServiceName')
        components = self._getComponents()

        self.assertEqual(len(svcMgr._get_registeredServices()), 1)
        service = svcMgr._get_registeredServices()[0]

        # Terminate the node to unregister the service
        svcMgr.shutdown()
        if not self.waitTermination(svcBooter):
            self.fail('Service node did not shut down')

        # Ensure that the app was removed
        self.assertEqual(len(self._domMgr._get_applications()), 0)
        self._app = None

    def test_DomainManagerPort(self):
        self._createApp('DomainManager')
        component = self._getComponents()['PortTest1']

        expectedId = self._domMgr._get_identifier()
        actualId = component.runTest(2, [])
        self.assertEqual(len(actualId), 1)
        actualId = any.from_any(actualId[0].value)
        self.assertEqual(expectedId, actualId)

    def test_DeviceManagerPort(self):
        self._createApp('DeviceManager')
        component = self._getComponents()['PortTest1']

        expectedId = self._devMgr._get_identifier()
        actualId = component.runTest(3, [])
        self.assertEqual(len(actualId), 1)
        actualId = any.from_any(actualId[0].value)
        self.assertEqual(expectedId, actualId)


    def test_ExternalPorts_getPortSet(self):
        # CF-1285
        self._createApp('ExternalPort')

        #  grab list of external ports
        plist = self._app.getPortSet()

        nports = len(plist)
        self.assertEqual(nports, 2 )

        # check name and direction of each port
        self.assertEqual(plist[0].name, 'resource_in')
        self.assertEqual(plist[0].direction.upper(), 'PROVIDES')

        # check name and direction of each port
        self.assertEqual(plist[1].name, 'resource_out')
        self.assertEqual(plist[1].direction.upper(), 'USES')

        component = self._getComponents()['PortTest1']
        plist = component.getPortSet()
        nports = len(plist)
        self.assertEqual(nports, 7 )
        self.assertEqual(plist[0].name, 'resource_in')
        self.assertEqual(plist[0].direction.upper(), 'PROVIDES')

        component = self._getComponents()['PortTest2']
        plist = component.getPortSet()
        pinfo = filter(lambda x : x.name == 'resource_out', plist)
        nports = len(plist)
        self.assertEqual(nports, 7 )
        self.assertEqual(pinfo[0].name, 'resource_out')
        self.assertEqual(pinfo[0].direction.upper(), 'USES')


