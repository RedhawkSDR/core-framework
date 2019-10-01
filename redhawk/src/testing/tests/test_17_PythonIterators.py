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
        fp=open('/proc/'+str(self.svc._pid)+'/statm', 'r')
        mem_content = fp.read()
        fp.close()
        mem = mem_content.split(' ')[0]
        return int(mem)

    def test_iterators(self):
        print self.get_memory_size(self.svc._pid)
        devlist, deviter = self.svc.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        print devlist, deviter
        print self.get_memory_size(self.svc._pid)
        time.sleep(2)
        print self.get_memory_size(self.svc._pid)
        devlist, deviter = self.svc.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        print self.get_memory_size(self.svc._pid)
        time.sleep(2)
        print self.get_memory_size(self.svc._pid)
        devlist, deviter = self.svc.listDevices(CF.AllocationManager.LOCAL_DEVICES, 10)
        print self.get_memory_size(self.svc._pid)
        time.sleep(2)
        print self.get_memory_size(self.svc._pid)
