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
        frontend_scanner_allocation = struct_property(id_="FRONTEND::scanner_allocation",name="frontend_scanner_allocation",structdef=fe_types.frontend_scanner_allocation,configurationkind=("allocation",),mode="writeonly",description="""Frontend Interfaces v2.0 scanner allocation structure""")
        self.assertEquals(frontend_scanner_allocation.fields['FRONTEND::scanner_allocation::min_freq'][1].type_, 'double')
        self.assertEquals(frontend_scanner_allocation.fields['FRONTEND::scanner_allocation::max_freq'][1].type_, 'double')
        self.assertEquals(frontend_scanner_allocation.fields['FRONTEND::scanner_allocation::mode'][1].type_, 'string')
        self.assertEquals(frontend_scanner_allocation.fields['FRONTEND::scanner_allocation::control_mode'][1].type_, 'string')
        self.assertEquals(frontend_scanner_allocation.fields['FRONTEND::scanner_allocation::control_limit'][1].type_, 'double')

if __name__ == '__main__':
    import runtests
    runtests.main()
