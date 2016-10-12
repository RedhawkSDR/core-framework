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

class LoadUnload(jackhammer.Jackhammer):
    def initialize (self, filename):
        self.fileMgr = self.domMgr._get_fileMgr()
        self.filename = filename
        for devMgr in self.domMgr._get_deviceManagers():
            for device in devMgr._get_registeredDevices():
                if device._is_a("IDL:CF/LoadableDevice:1.0"):
                    self.device = device
                    return

        raise RuntimeError, "No LoadableDevice available"
        
    def test (self):
        self.device.load(self.fileMgr, self.filename, CF.LoadableDevice.EXECUTABLE)
        self.device.unload(self.filename)

if __name__ == '__main__':
    jackhammer.run(LoadUnload)
