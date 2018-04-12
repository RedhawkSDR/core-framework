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

import omniORB.any

from ossie.cf import CF
import ossie.properties

from bulkio.bulkioInterfaces import BULKIO

# Bit flags for SRI fields
NONE     = 0
HVERSION = (1<<0)
XSTART   = (1<<1)
XDELTA   = (1<<2)
XUNITS   = (1<<3)
SUBSIZE  = (1<<4)
YSTART   = (1<<5)
YDELTA   = (1<<6)
YUNITS   = (1<<7)
MODE     = (1<<8)
STREAMID = (1<<9)
BLOCKING = (1<<10)
KEYWORDS = (1<<11)

def _compareKeywords(keywordsA, keywordsB):
    if len(keywordsA) != len(keywordsB):
        return False
    for keyA, keyB in zip(keywordsA, keywordsB):
        if keyA.value._t != keyB.value._t:
            return False
        if keyA.value._v != keyB.value._v:
            return False
    return True

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

def compareFields(sriA, sriB):
    """
    Field-by-field comparison of two SRIs, returning a combination of bit flags
    indicating the fields that are different.
    """
    result = NONE
    if sriA.hversion != sriB.hversion:
        result = result | HVERSION
    if sriA.xstart != sriB.xstart:
        result = result | XSTART
    if sriA.xdelta != sriB.xdelta:
        result = result | XDELTA
    if sriA.xunits != sriB.xunits:
        result = result | XUNITS
    if sriA.subsize != sriB.subsize:
        result = result | SUBSIZE
    if sriA.ystart != sriB.ystart:
        result = result | YSTART
    if sriA.ydelta != sriB.ydelta:
        result = result | YDELTA
    if sriA.yunits != sriB.yunits:
        result = result | YUNITS
    if sriA.mode != sriB.mode:
        result = result | MODE
    if sriA.streamID != sriB.streamID:
        result = result | STREAMID
    if sriA.blocking != sriB.blocking:
        result = result | BLOCKING
    if not _compareKeywords(sriA.keywords, sriB.keywords):
        result = result | KEYWORDS
    return result

def create( sid='defStream', srate=1.0, xunits=1 ):
     return BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=1.0/srate, 
                              xunits=xunits, subsize=0, ystart=0.0, ydelta=0.0, 
                              yunits=0, mode=0, streamID=sid, blocking=False, keywords=[])
    
def hasKeyword(sri, name):
    """
    Checks for the presence of a keyword in the SRI.
    """
    for dt in sri.keywords:
        if dt.id == name:
            return True
    return False

def getKeyword(sri, name):
    """
    Gets the current value of a keyword in the SRI.

    Allows for easy lookup of keyword values in the SRI. To avoid exceptions on
    missing keywords, the presence of a keyword can be checked with
    hasKeyword().
    """
    for dt in sri.keywords:
        if dt.id == name:
            return omniORB.any.from_any(dt.value)
    raise KeyError(name)

def setKeyword(sri, name, value, format=None):
    """
    Sets the current value of a keyword in the SRI.
    
    If the keyword "name" already exists, its value is updated to "value".  If
    the keyword "name" does not exist, the new keyword is appended.

    If the optional 'format' argument is given, it must be the name of the
    desired CORBA type. Otherwise, the CORBA type is determined based on the
    Python type of 'value'.
    """
    if format is None:
        value = omniORB.any.to_any(value)
    else:
        value = ossie.properties.to_tc_value(value, format)

    for dt in sri.keywords:
        if dt.id == name:
            dt.value = value
            return
    sri.keywords.append(CF.DataType(name, value))

def eraseKeyword(sri, name):
    """
    Erases the first instance of the keyword "name" from the SRI keywords. If
    no keyword "name" is found, the keywords are not modified.
    """
    for index in xrange(len(sri.keywords)):
        if sri.keywords[index].id == name:
            del sri.keywords[index]
            return
    
