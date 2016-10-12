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

class DeviceLifeCycleTest(scatest.CorbaTestCase):
    def setUp(self):
        domBooter, domMgr = self.launchDomainManager()
        devBooter, devMgr = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")

    def test_DeviceLifeCycle(self):
        q=os.walk('/proc')
        for root,dirs,files in q:
            all_dirs = root.split('/')
            num_slash = len(all_dirs)
            if num_slash>3:
                continue
            process_number = all_dirs[-1]
            if 'status' in files:
                fp=open(root+'/status','r')
                stuff=fp.read()
                fp.close()
                if 'GPP' in stuff:
                    print "Killing process "+process_number+" (presumably a GPP)"
                    os.kill(int(process_number),9)

    def test_DeviceLifeCycleNoKill(self):
        pass

