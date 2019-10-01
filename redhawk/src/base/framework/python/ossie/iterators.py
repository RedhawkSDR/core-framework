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

import sys, gcpoa
from omniORB import CORBA
import copy

class Iterator:
    def __init__(self, _list=[], empty_object=None):
        self.list_ = copy.deepcopy(_list)
        self.offset_ = 0
        self.empty_object_ = empty_object

    def __del__(self):
        pass

    def next_one(self):
        if self.offset_ >= len(self.list_):
            return False, self.empty_object_

        item = self.list_[self.offset_]
        self.offset_+=1
        return True, item

    def next_n(self, how_many):
        if self.offset_ >= len(self.list_):
            return False, self.empty_object_

        how_many = min(how_many, len(self.list_)-self.offset_)
        ret_list = self.list_[self.offset_:self.offset_+how_many]
        self.offset_ += how_many

        return True, ret_list

    def destroy():
        pass

def get_list(count, items, clazz, empty_object=None):
    if count >= len(items):
        return clazz()._this()

    _iter = clazz(items)

    _list = _iter.next_n(count)

    # Activate the iterator into the garbage-collected POA
    orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
    obj_poa = orb.resolve_initial_references("RootPOA")
    poa = obj_poa.find_POA("Iterators", 1)

    obj = gcpoa.activateGCObject(poa, _iter, 1)

    return obj
