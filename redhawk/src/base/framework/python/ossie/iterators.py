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
import copy

def construct(typeobj):
    if isinstance(typeobj, CORBA.TypeCode):
        tc = typeobj
    else:
        tc = CORBA.TypeCode(CORBA.id(typeobj))
    kind = tc.kind()
    if kind == CORBA.tk_alias:
        return construct(tc.content_type())
    if kind == CORBA.tk_struct:
        # This is sort-of-dependent on omniORB implementation details, but
        # findType() is public by Python convention. It returns a tuple with
        # details of the type, but here we only need the class object.
        typeobj = omniORB.findType(tc.id())[1]
        args = [construct(tc.member_type(idx)) for idx in range(tc.member_count())]
        return typeobj(*args)
    if kind == CORBA.tk_enum:
        # As with above, an implementation detail. The last element of the
        # type details is the list values; return the first.
        return omniORB.findType(tc.id())[-1][0]
    if kind == CORBA.tk_union:
        # See comment in struct branch.
        typeobj = omniORB.findType(tc.id())[1]
        index = tc.default_index()
        if index < 0:
            index = tc.member_count() + index
        kwds = {}
        kwds[tc.member_name(index)] = construct(tc.member_type(index))
        return typeobj(**kwds)
    if kind == CORBA.tk_string:
        return str()
    if kind in (CORBA.tk_float, CORBA.tk_double):
        return float()
    if kind in (CORBA.tk_octet, CORBA.tk_short, CORBA.tk_ushort, CORBA.tk_long, CORBA.tk_boolean):
        return int()
    if kind in (CORBA.tk_ulong, CORBA.tk_longlong, CORBA.tk_ulonglong):
        return long()
    if kind == CORBA.tk_char:
        return '\x00'
    if kind == CORBA.tk_sequence:
        return []
    if kind == CORBA.tk_any:
        return CORBA.Any(CORBA.TC_null, None)
    if kind == CORBA.tk_objref:
        return None
    print 'not implemented', tc

class Iterator:
    def __init__(self, object_type, _list=[]):
        self.list_ = copy.deepcopy(_list)
        self.offset_ = 0
        self.empty_object_ = construct(object_type)

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
            return False, []

        how_many = min(how_many, len(self.list_)-self.offset_)
        ret_list = self.list_[self.offset_:self.offset_+how_many]
        self.offset_ += how_many

        return True, ret_list

    def destroy():
        pass

def get_list_iterator(items, clazz, ttl=60):
    if len(items) == 0:
        return None

    _iter = clazz(items)

    # Activate the iterator into the garbage-collected POA
    orb = CORBA.ORB_init(sys.argv, CORBA.ORB_ID)
    obj_poa = orb.resolve_initial_references("RootPOA")
    poa = obj_poa.find_POA("Iterators", 1)

    obj = gcpoa.activateGCObject(poa, _iter, ttl)

    return obj
