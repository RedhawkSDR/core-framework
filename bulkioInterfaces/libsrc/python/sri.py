#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

try:
    from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA
except:
    pass

def compare(sriA, sriB):
    """
    Will compare two BULKIO.StreamSRI objects and return True
    if they are both equal, and false otherwise
    """
    if not sriA or not sriB:
        return False

    if sriA.hversion != sriB.hversion:
        return False
    if sriA.xstart != sriB.xstart:
        return False
    if sriA.xdelta != sriB.xdelta:
        return False
    if sriA.xunits != sriB.xunits:
        return False
    if sriA.subsize != sriB.subsize:
        return False
    if sriA.ystart != sriB.ystart:
        return False
    if sriA.ydelta != sriB.ydelta:
        return False
    if sriA.yunits != sriB.yunits:
        return False
    if sriA.mode != sriB.mode:
        return False
    if sriA.streamID != sriB.streamID:
        return False
    if sriA.blocking != sriB.blocking:
        return False
    if len(sriA.keywords) != len(sriB.keywords):
        return False
    for keyA, keyB in zip(sriA.keywords, sriB.keywords):
        if keyA.value._t != keyB.value._t:
            return False
        if keyA.value._v != keyB.value._v:
            return False
    return True

def create( sid='defStream', srate=1.0, xunits=1 ):
     return BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=1.0/srate, 
                              xunits=xunits, subsize=0, ystart=0.0, ydelta=0.0, 
                              yunits=0, mode=0, streamID=sid, blocking=False, keywords=[])
    
