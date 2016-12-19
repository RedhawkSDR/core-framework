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
        self._red_app = None
        self._green_app = None

    def tearDown(self):
        if self._app:
            self._app.stop()
            self._app.releaseObject()

        if self._red_app:
            self._red_app.stop()
            self._red_app.releaseObject()

        if self._green_app:
            self._green_app.stop()
            self._green_app.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def _createApp(self, appName, exc=None):
        self.assertNotEqual(self._domMgr, None)
        app=None

        try:
            sadpath = '/waveforms/'+appName+'/'+appName+'.sad.xml'
            self._domMgr.installApplication(sadpath)
        except Exception, e:
            return app

        appFact=None
        for x in self._domMgr._get_applicationFactories():
            if x._get_name() == appName:
                 appFact = x

        self.assertNotEqual(appFact, None)
        if exc:
            self.assertRaises(exc, appFact.create, appFact._get_name(), [], [])
        else:
            try:
                app = appFact.create(appFact._get_name(), [], [])
            except:
                self.fail("Did not create application ")
        return app

    def test_norequired_nocolor(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        self._app = self._createApp('device_requires_multicolor')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])


    def test_norequired_redprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        self._app = self._createApp('device_requires_red')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])

    def test_norequired_greeprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

        self._app = self._createApp('device_requires_green')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])

    def test_redrequired_redprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)

        self._app = self._createApp('device_requires_red')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])

    def test_redrequired_colormismatch(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)

        self._app = self._createApp('device_requires_green', CF.ApplicationFactory.CreateApplicationError)

        self.assertEqual(self._app, None)

    def test_redrequired_nonocolors(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)

        self._app = self._createApp('device_requires_multicolor', CF.ApplicationFactory.CreateApplicationError)
        self.assertEqual(self._app, None)


    def test_greenrequired_nonocolors(self):
        domBooter, self._domMgr = self.launchDomainManager()
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._greennode, None)

        self._app = self._createApp('device_requires_multicolor', CF.ApplicationFactory.CreateApplicationError)
        self.assertEqual(self._app, None)

    def test_greenrequired_redprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._greennode, None)

        self._app = self._createApp('device_requires_red', CF.ApplicationFactory.CreateApplicationError)
        self.assertEqual(self._app, None)

    def test_greenrequired_greenprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._greennode, None)

        self._app = self._createApp('device_requires_red', CF.ApplicationFactory.CreateApplicationError)
        self.assertEqual(self._app, None)

    def test_redmulti_redprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._plainnode, None)

        self._app = self._createApp('device_requires_red')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])

    def test_redmulti_greenprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._plainnode, None)

        self._app = self._createApp('device_requires_green')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])


    def test_redmulti_nocolors(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._plainnode, None)

        self._app = self._createApp('device_requires_multicolor')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])

    def test_multidevices_nocolors(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._greennode, None)
        self.assertNotEqual(self._plainnode, None)

        self._app = self._createApp('device_requires_multicolor')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])


    def test_multidevices_redprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._greennode, None)

        self._app = self._createApp('device_requires_red')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])

    def test_multidevices_greenprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._greennode, None)

        self._app = self._createApp('device_requires_green')

        self.assertNotEqual(self._app, None)
        xx=self._app.query([])


    def test_multidevices_redgreenprovided(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._greennode, None)

        self._green_app = self._createApp('device_requires_green')
        self.assertNotEqual(self._green_app, None)

        self._red_app = self._createApp('device_requires_red')
        self.assertNotEqual(self._red_app, None)


    def test_multidevices_mixedcolors(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._greennode, None)
        self.assertNotEqual(self._plainnode, None)

        self._app = self._createApp('device_requires_multicolor')
        self.assertNotEqual(self._app, None)

        self._green_app = self._createApp('device_requires_green')
        self.assertNotEqual(self._green_app, None)

        self._red_app = self._createApp('device_requires_red')
        self.assertNotEqual(self._red_app, None)



class DeployerRequiresTest(scatest.CorbaTestCase):
    def setUp(self):
        self._app = None

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_deployerRedNode(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

    def test_deployerGreenNode(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._devMgr, None)

    def test_deployerMixNode(self):
        domBooter, self._domMgr = self.launchDomainManager()
        redBooter, self._rednode = self.launchDeviceManager("/nodes/test_GPP_red/DeviceManager.dcd.xml")
        greenBooter, self._greennode = self.launchDeviceManager("/nodes/test_GPP_green/DeviceManager.dcd.xml")
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._rednode, None)
        self.assertNotEqual(self._greennode, None)
        self.assertNotEqual(self._plainnode, None)
