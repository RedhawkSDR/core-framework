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

"""Utilities for common CORBA operations"""
import omniORB
from omniORB import CORBA

def objectExists(obj):
    try:
        return not obj._non_existent()
    except CORBA.Exception:
        return False
    except Exception as e:
        return False

def constructDefaultType(typeobj):
    if isinstance(typeobj, CORBA.TypeCode):
        tc = typeobj
    else:
        tc = CORBA.TypeCode(CORBA.id(typeobj))
    kind = tc.kind()
    if kind == CORBA.tk_alias:
        return constructDefaultType(tc.content_type())
    if kind == CORBA.tk_struct:
        # This is sort-of-dependent on omniORB implementation details, but
        # findType() is public by Python convention. It returns a tuple with
        # details of the type, but here we only need the class object.
        typeobj = omniORB.findType(tc.id())[1]
        args = [constructDefaultType(tc.member_type(idx)) for idx in range(tc.member_count())]
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
        kwds[tc.member_name(index)] = constructDefaultType(tc.member_type(index))
        return typeobj(**kwds)
    if kind == CORBA.tk_string:
        return str()
    if kind in (CORBA.tk_float, CORBA.tk_double):
        return float()
    if kind in (CORBA.tk_octet, CORBA.tk_short, CORBA.tk_ushort, CORBA.tk_long, CORBA.tk_boolean):
        return int()
    if kind in (CORBA.tk_ulong, CORBA.tk_longlong, CORBA.tk_ulonglong):
        return int()
    if kind == CORBA.tk_char:
        return '\x00'
    if kind == CORBA.tk_sequence:
        return []
    if kind == CORBA.tk_any:
        return CORBA.Any(CORBA.TC_null, None)
    if kind == CORBA.tk_objref:
        return None
    raise NotImplementedError(tc)
