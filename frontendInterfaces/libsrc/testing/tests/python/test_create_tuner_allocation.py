#!/usr/bin/python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest
import copy

import bulkio
from bulkio.bulkioInterfaces import BULKIO

import sys
sys.path.insert(0, '../../../../src/python')
sys.path.insert(0, '../../../../libsrc/python')

from omniORB import any as _any
from ossie.cf import CF

from redhawk.frontendInterfaces import FRONTEND
import tuner_device
import fe_types
from ossie.properties import struct_property

class ValidateRequestTest(unittest.TestCase):

    def testFrontendAllocationStruct(self):
        frontend_allocation = tuner_device.createTunerAllocation(tuner_type="RX_DIGITIZER",bandwidth=24.576, center_frequency=30000000, bandwidth_tolerance=100, allocation_id='hello')
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::sample_rate_tolerance'], 0.0)
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::group_id'], '')
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::tuner_type'], 'RX_DIGITIZER')
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::bandwidth'], 24.576)
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::rf_flow_id'], '')
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::sample_rate'], 0.0)
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id'], 'hello')
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::device_control'], True)
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::center_frequency'], 30000000)
        self.assertEquals(frontend_allocation['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::bandwidth_tolerance'], 100)

if __name__ == '__main__':
    import runtests
    runtests.main()
