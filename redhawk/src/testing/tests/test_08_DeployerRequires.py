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

class DeviceRequires(scatest.CorbaTestCase):
    def setUp(self):
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def _createApp(self, app):
        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        sadpath = '/waveforms/'+app+'/'+app+'.sad.xml'
        self._domMgr.installApplication(sadpath)
        self.assertEqual(len(self._domMgr._get_applicationFactories()), 1)
        appFact = self._domMgr._get_applicationFactories()[0]

        try:
            self._app = appFact.create(appFact._get_name(), [], [])
        except:
            self.fail("Did not create application ")

    def test_createNoDeployerRequires(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        self._createApp('device_requires')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])

class DeployerRequiresTest(scatest.CorbaTestCase):
    def setUp(self):
        self._app = None

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_launchRedNode(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

    def test_launchGreenNode(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)


    def test_launchMixNode(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._greennode, None)
        self.assertNotEqual(self._plainnode, None)
