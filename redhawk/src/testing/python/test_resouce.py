#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
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

import threading
import time
import unittest

from omniORB.any import to_any, from_any
from omniORB import CORBA
import omniORB

from ossie import resource

class ResourceTest(unittest.TestCase):
    def setUp(self):
        pass

    def tearDown(self):
        pass

    def testGetPOA(self):
        orb = CORBA.ORB_init()
        root_poa = orb.resolve_initial_references("RootPOA")
        poa_mgr = root_poa._get_the_POAManager()
        name = 'hello'
        self.assertRaises(omniORB.PortableServer.POA.AdapterNonExistent, root_poa.find_POA, name, True)
        poa = resource.getPOA(orb, None, None)
        self.assertEqual(poa._get_the_name(), 'RootPOA')
        poa = resource.getPOA(orb, None, name)
        self.assertEqual(poa._get_the_name(), name)
        poa = resource.getPOA(orb, None, None)
        self.assertEqual(poa._get_the_name(), 'RootPOA')

if __name__ == '__main__':
    import runtests
    runtests.main()
