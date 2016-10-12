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
from ossie.cf import CF
from omniORB import any
from ossie.utils import redhawk

@scatest.requireJava
class TestDeviceJava(scatest.CorbaTestCase):

    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_DeviceReadOnlyProps/DeviceManager_JavaDevice.dcd.xml")
        self._domain=redhawk.attach(scatest.getTestDomainName())


    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)


    def test_readonly_java(self):
        self.assertNotEqual(self._domain, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "Failed to launch device manager")

        dev=filter( lambda c : c._id == 'TestJavaDevice_1', self._domain.devices )[0]
        self.assertNotEqual(dev,None)
        props = dev.query([CF.DataType("readOnly", any.to_any(None))])
        self.assertEqual(props[0].value._v, "set_once")

        readonly_prop=CF.DataType("readOnly", any.to_any("try_again"))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, dev.configure, [ readonly_prop ] )



class TestDevice(scatest.CorbaTestCase):

    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/test_DeviceReadOnlyProps/DeviceManager.dcd.xml")
        self._domain=redhawk.attach(scatest.getTestDomainName())

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)


    def test_readonly_python(self):
        self.assertNotEqual(self._domain, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "Failed to launch device manager")

        dev=filter( lambda c : c._id == 'TestPythonDevice_1', self._domain.devices )[0]
        self.assertNotEqual(dev,None)
        props = dev.query([CF.DataType("readOnly", any.to_any(None))])
        self.assertEqual(dev.readOnly, 'set_once')

        readonly_prop=CF.DataType("readOnly", any.to_any("try_again"))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, dev.configure, [ readonly_prop ] )


    def test_readonly_cpp(self):
        self.assertNotEqual(self._domain, None, "DomainManager not available")
        self.assertNotEqual(self._devMgr, None, "Failed to launch app")

        dev=filter( lambda c : c._id == 'TestCppDevice_1', self._domain.devices )[0]
        self.assertNotEqual(dev,None)
        props = dev.query([CF.DataType("readOnly", any.to_any(None))])
        self.assertEqual(props[0].value._v, "set_once")

        readonly_prop=CF.DataType("readOnly", any.to_any("try_again"))
        self.assertRaises(CF.PropertySet.InvalidConfiguration, dev.configure, [ readonly_prop ] )

