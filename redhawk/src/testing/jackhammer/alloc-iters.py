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
    CF.AllocationManager.LOCAL_ALLOCATIONS,
    CF.AllocationManager.ALL_ALLOCATIONS,
]

class AllocListIterators(jackhammer.Jackhammer):
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
        try:
            items, allociter = self.allocMgr.listAllocations(self.next_scope(), 0)
        except:
            import traceback, sys
            traceback.print_exception(*sys.exc_info())
            raise
        if allociter is not None:
            try:
                status = True
                while status:
                    status, item = allociter.next_one()
            finally:
                allociter.destroy()

if __name__ == '__main__':
    jackhammer.run(AllocListIterators)
