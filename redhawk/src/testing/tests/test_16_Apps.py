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
from ossie import parsers
import commands


class AppsTest(scatest.CorbaTestCase):

    def test_DeviceConnections(self):
        status,output = commands.getstatusoutput('../base/framework/python/ossie/apps/py2prf app_input/py_prf_check_base.py')
        self.assertEquals(status, 0)
        prf = parsers.PRFParser.parseString(output)
        self.assertNotEquals(prf,None)
        self.assertEquals(len(prf.get_simple()),1)
        self.assertEquals(prf.get_simple()[0].get_id(),'some_simple')
        self.assertEquals(len(prf.get_simplesequence()),1)
        self.assertEquals(prf.get_simplesequence()[0].get_id(),'some_sequence')
        self.assertEquals(len(prf.get_struct()),1)
        self.assertEquals(prf.get_struct()[0].get_id(),'some_struct')
        self.assertEquals(len(prf.get_structsequence()),1)
        self.assertEquals(prf.get_structsequence()[0].get_id(),'some_struct_seq')
