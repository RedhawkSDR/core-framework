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

import threading

from ossie.cf import CF

import jackhammer

SCOPES = [
    CF.AllocationManager.LOCAL_DEVICES,
    CF.AllocationManager.ALL_DEVICES,
    CF.AllocationManager.AUTHORIZED_DEVICES
]

class DevListIterators(jackhammer.Jackhammer):
    def initialize (self):
        self.allocMgr = self.domMgr._get_allocationMgr()
        self.lock = threading.Lock()
        self.index = 0

    def next_scope(self):
        self.lock.acquire()
        try:
            index = self.index
            self.index = (self.index + 1) % len(SCOPES)
            return SCOPES[index]
        finally:
            self.lock.release()

    def test (self):
        items, deviter = self.allocMgr.listDevices(self.next_scope(), 0)
        if deviter is not None:
            try:
                status = True
                while status:
                    status, item = deviter.next_one()
            finally:
                deviter.destroy()

if __name__ == '__main__':
    jackhammer.run(DevListIterators)
