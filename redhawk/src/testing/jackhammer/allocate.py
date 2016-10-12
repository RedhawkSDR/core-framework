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

import sys

from omniORB import CORBA

from ossie.cf import CF

import jackhammer

class AllocDealloc(jackhammer.Jackhammer):
    def initialize (self):
        bogomipsId = "DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8"
        self.devMgr = self.domMgr._get_deviceManagers()[0]
        self.device = self.devMgr._get_registeredDevices()[0]
        self.props = [CF.DataType(bogomipsId, CORBA.Any(CORBA.TC_long, 100000000/100))]

    def test (self):
        self.device.allocateCapacity(self.props)
        self.device.deallocateCapacity(self.props)

if __name__ == '__main__':
    jackhammer.run(AllocDealloc)
