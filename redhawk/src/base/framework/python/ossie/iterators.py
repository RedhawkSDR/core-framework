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
import omniORB
from omniORB import CORBA
from utils import corba
import copy

class Iterator:
    def __init__(self, object_type, _list=[]):
        self.list_ = copy.deepcopy(_list)
        self._offset = 0
        self._empty_object = corba.constructDefaultType(object_type)

    def next_one(self):
        if self._offset >= len(self.list_):
            return False, self._empty_object

        item = self.list_[self._offset]
        self._offset+=1
        return True, item

    def next_n(self, how_many):
        if self._offset >= len(self.list_):
            return False, []

        how_many = min(how_many, len(self.list_)-self._offset)
        ret_list = self.list_[self._offset:self._offset+how_many]
        self._offset += how_many

        return True, ret_list

    def destroy():
        pass

    @classmethod
    def get_list_iterator(clazz, items, ttl=60):
        if len(items) == 0:
            return None

        _iter = clazz(items)

        # Activate the iterator into the garbage-collected POA
        orb = CORBA.ORB_init()
        obj_poa = orb.resolve_initial_references("RootPOA")
        poa = obj_poa.find_POA("Iterators", 1)

        obj = gcpoa.activateGCObject(poa, _iter, ttl)

        return obj

def construct(object_type):
    return corba.constructDefaultType(object_type)
