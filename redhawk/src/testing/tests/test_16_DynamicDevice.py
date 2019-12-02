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
import time
import contextlib
import cStringIO
import tempfile
import re
import resource
import sys as _sys
from omniORB import CORBA
from omniORB import any as _any
from xml.dom import minidom
import os as _os
import Queue
import StringIO
from ossie.cf import CF
from ossie.utils import redhawk
from ossie.utils import type_helpers
from ossie.utils import rhconnection
from ossie.utils import allocations
from ossie.utils import sb
from ossie.utils.model import NoMatchingPorts
from ossie.events import Subscriber, Publisher
from ossie.cf import CF
import traceback

class DynamicDeviceLaunchTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/wb_receiver_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels...... 
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        redhawk.setTrackApps(False)

    def test_launch(self):
        self.assertEquals(len(self._rhDom.devices), 8)
        devices = ['wb_receiver_1:supersimple_1:anothersimple_1', 
                   'wb_receiver_1:supersimple_1:anothersimple_2', 
                   'wb_receiver_1:supersimple_1', 
                   'wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1', 
                   'wb_receiver_1:anothersimple_1', 
                   'wb_receiver_1:anothersimple_1:anothersimple_2', 
                   'wb_receiver_1:anothersimple_1:anothersimple_1', 
                   'wb_receiver_1']

        for dev in self._rhDom.devices:
            self.assertTrue(dev.label in devices)
            devices.pop(devices.index(dev.label))

        for dev in self._rhDom.devices:
            print '++++++++++++++++++++', dev._get_identifier(), dev._get_label()


class DynamicCppDeviceLaunchTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, self._domMgr = self.launchDomainManager()
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/cpp_wb_receiver_node/DeviceManager.dcd.xml")
        self._rhDom = redhawk.attach(scatest.getTestDomainName())

    def tearDown(self):
        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        redhawk.core._cleanUpLaunchedApps()
        scatest.CorbaTestCase.tearDown(self)
        # need to let event service clean up event channels...... 
        # cycle period is 10 milliseconds
        time.sleep(0.1)
        redhawk.setTrackApps(False)

    def test_cpp_launch(self):
        print self._rhDom.devices
        print self._rhDom.devMgrs[0]._get_registeredDevices()

        for dev in self._rhDom.devices:
            print '++++++++++++++++++++', dev._get_identifier(),'*******', dev._get_label()

        self.assertEquals(len(self._rhDom.devices), 8)
        devices = ['cpp_wb_receiver_1:supersimple_1:anothersimple_1', 
                   'cpp_wb_receiver_1:supersimple_1:anothersimple_2', 
                   'cpp_wb_receiver_1:supersimple_1', 
                   'cpp_wb_receiver_1:anothersimple_1:anothersimple_2:anothersimple_1', 
                   'cpp_wb_receiver_1:anothersimple_1', 
                   'cpp_wb_receiver_1:anothersimple_1:anothersimple_2', 
                   'cpp_wb_receiver_1:anothersimple_1:anothersimple_1', 
                   'cpp_wb_receiver_1']

        for dev in self._rhDom.devices:
            self.assertTrue(dev.label in devices)
            devices.pop(devices.index(dev.label))

        for dev in self._rhDom.devices:
            dev.start()

        raw_input('press any key')

