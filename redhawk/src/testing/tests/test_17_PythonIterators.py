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
import tempfile
import time
from ossie.cf import CF, CF__POA
from omniORB import CORBA

from ossie.utils import sb

class Iterators(unittest.TestCase):

    def setUp(self):
        self.svc = sb.launch('allocmgr_svc')

    def tearDown(self):
        sb.release()
    
    def get_memory_size(self, _pid):
        fp=open('/proc/'+str(_pid)+'/statm', 'r')
        mem_content = fp.read()
        fp.close()
        mem = mem_content.split(' ')[0]
        return int(mem)

    def test_iter_count(self):
        # the list of allocations in the service is 20 allocations long
        alloclist, allociter = self.svc.listAllocations(CF.AllocationManager.LOCAL_ALLOCATIONS, 1)
        self.assertEquals(len(alloclist), 1)
        self.assertEquals(alloclist[0].sourceID, "1")
        allocstatus, alloclist = allociter.next_n(10)
        self.assertEquals(len(alloclist), 10)
        self.assertEquals(alloclist[9].sourceID, "11")
        self.assertEquals(allocstatus, True)
        allocstatus, allocvalue = allociter.next_one()
        self.assertEquals(allocvalue.allocationID, "sample_id")
        self.assertEquals(allocvalue.sourceID, "12")
        self.assertEquals(allocstatus, True)
        allocstatus, alloclist = allociter.next_n(10)
        self.assertEquals(len(alloclist), 8)
        self.assertEquals(alloclist[-1].sourceID, "20")
        self.assertEquals(allocstatus, True)
        allocstatus, alloclist = allociter.next_n(10)
        self.assertEquals(len(alloclist), 0)
        self.assertEquals(allocstatus, False)
        allocstatus, allocvalue = allociter.next_one()
        self.assertEquals(allocvalue.allocationID, "")
        self.assertEquals(allocstatus, False)

    def test_object_not_exist(self):
        # the list of allocations in the service is 50000 devices long
        mem_1 = self.get_memory_size(self.svc._pid)
        devlist, deviter = self.svc.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        self.assertEquals(len(devlist), 10)
        mem_2 = self.get_memory_size(self.svc._pid)
        self.assertEquals(mem_1<mem_2, True)

        devstatus, devlist = deviter.next_n(20)
        self.assertEquals(len(devlist), 20)
        self.assertEquals(devstatus, True)

        time.sleep(1.5)

        mem_3 = self.get_memory_size(self.svc._pid)
        self.assertEquals(mem_3<mem_2, True)

        self.assertRaises(CORBA.OBJECT_NOT_EXIST, deviter.next_n, 10)

    def test_mem_dealloc(self):
        # the list of allocations in the service is 50000 devices long
        # mem_1: initial memory
        mem_1 = self.get_memory_size(self.svc._pid)
        devlist, deviter = self.svc.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        self.assertEquals(len(devlist), 10)
        # mem_2: memory after an iterator is created
        mem_2 = self.get_memory_size(self.svc._pid)
        # check that the iterator required new memory
        self.assertTrue(mem_2 > mem_1)

        time.sleep(1) # wait for the iterator to expire
        devlist, deviter = self.svc.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        mem_4 = self.get_memory_size(self.svc._pid)
        # mem_4: memory after an iterator is created
        # verify that the new iterator did not require more memory beyond the first iterator
        # if the iterator did not expire, new memory would have been allocated
        self.assertEquals(mem_2, mem_4)

        time.sleep(1) # wait for the iterator to expire
        devlist, deviter = self.svc.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        # mem_6: memory after an iterator is created
        mem_6 = self.get_memory_size(self.svc._pid)
        # verify that the new iterator did not require more memory beyond the first or second iterator
        self.assertEquals(mem_4, mem_6)
