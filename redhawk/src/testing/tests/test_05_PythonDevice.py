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
from _unitTestHelpers import buildconfig
from ossie.cf import CF
from omniORB import any, CORBA
from ossie.utils import sb

class PythonDeviceTest(scatest.CorbaTestCase):

    def test_BasicDevice(self):
        dev = sb.launch('sdr/dev/devices/alloc_test/alloc_test.spd.xml')
        self.assertNotEqual(dev, None)
        alloc = dev.allocateCapacity({'callback_test':5})
        self.assertEqual(alloc, True)
        self.assertEqual(dev.callback_value, 5)
        dev.deallocateCapacity({'callback_test':7})
        self.assertEqual(dev.callback_value, 7)

    def test_AllocReturn(self):
        dev = sb.launch('sdr/dev/devices/alloc_test/alloc_test.spd.xml')
        self.assertNotEqual(dev, None)
        allocation = CF.DataType(id='callback_test',value=any.to_any(5))
        allocation.value._t=CORBA.TC_short
        alloc = dev.ref.allocate([allocation])
        self.assertEqual(len(alloc), 1)
        self.assertEqual(dev.callback_value, 5)
        self.assertEqual(alloc[0].allocated[0].id, 'callback_test')
        self.assertEqual(alloc[0].allocated[0].value._v, 5)
        dev.ref.deallocate(alloc[0].alloc_id)
        self.assertEqual(dev.callback_value, 7)
