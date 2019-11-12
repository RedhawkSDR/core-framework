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
        devBooter, self._devMgr = self.launchDeviceManager("/nodes/single_dev/DeviceManager.dcd.xml")
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
        dom=redhawk.attach()
        print dom
        for dev in dom.devices:
            print dev.label
