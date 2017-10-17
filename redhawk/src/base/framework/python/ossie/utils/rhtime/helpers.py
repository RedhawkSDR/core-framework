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
import time
import copy
import math

from ossie.cf import CF
from omniORB import any as _any

def now():
    """
    Generates a CF.UTCTime object using the current 
    CPU time that you can use in the pushPacket call
    """
    ts = time.time()
    return CF.UTCTime(1, int(ts), ts - int(ts))

def notSet():
    """
    Generates a CF.UTCTime object with zero time 
    and an invalid flag. This is used by the automatic EOS
    """
    return CF.UTCTime(1, 0.0, 0.0)

def queryTimestamp():
    return CF.DataType(id='QUERY_TIMESTAMP',value=_any.to_any(None))

def cpuTimeStamp():
    return now()

def create( whole_secs=-1.0, fractional_secs=-1.0 ):
    """
    Generates a CF.UTCTime object using the current 
    CPU time that you can use in the pushPacket call
    """
    wsec = whole_secs;
    fsec = fractional_secs;
    if wsec < 0.0 and fsec < 0.0 :
        ts=time.time()
        wsec=int(ts)
        fsec = ts-int(ts)

    return CF.UTCTime(1, wsec, fsec )

def compare(T1, T2):
    """
    Will compare two CF.UTCTime objects and return True
    if they are both equal, and false otherwise
    """
    if not T1 or not T2:
        return False

    if T1.tcstatus != T2.tcstatus:
        return False
    if T1.tfsec != T2.tfsec:
        return False
    if T1.twsec != T2.twsec:
        return False
    return True

def addSampleOffset(T, numSamples=0, xdelta=0.0):
    tstamp = copy.deepcopy(T)
    tstamp.twsec += int(numSamples*xdelta)
    tstamp.tfsec += numSamples*xdelta - int(numSamples*xdelta)
    if tstamp.tfsec >= 1.0:
        tstamp.twsec += 1
        tstamp.tfsec -= 1.0
    return tstamp

def normalize(tstamp):
    # Get fractional adjustment from whole seconds
    fadj, tstamp.twsec = math.modf(tstamp.twsec)

    # Adjust fractional seconds and get whole seconds adjustment
    tstamp.tfsec, wadj = math.modf(tstamp.tfsec + fadj)

    # If fractional seconds are negative, borrow a second from the whole
    # seconds to make it positive, normalizing to [0,1)
    if (tstamp.tfsec < 0.0):
        tstamp.tfsec += 1.0;
        wadj -= 1.0;

    tstamp.twsec += wadj;

def difference(t1, t2):
    return (t1.twsec - t2.twsec) + (t1.tfsec - t2.tfsec)

def add(t1, offset):
    return iadd(copy.copy(t1), offset)

def iadd(t1, offset):
    fractional, whole = math.modf(offset)
    t1.twsec += whole
    t1.tfsec += fractional
    normalize(t1)
    return t1

def sub(t1, other):
    if isinstance(other, CF.UTCTime):
        return difference(t1, other)
    else:
        return isub(copy.copy(t1), other)

def isub(t1, offset):
    return iadd(t1, -offset)

def compare(t1, t2):
    if not isinstance(t2, CF.UTCTime):
        return -1
    if (t1.twsec == t2.twsec):
        return cmp(t1.tfsec, t2.tfsec)
    else:
        return cmp(t1.twsec, t2.twsec)

def toString(tstamp):
    # Break out the whole seconds into a GMT time
    gmt = time.gmtime(tstamp.twsec)
    # Append the fractional seconds down to microsecond precision
    fractional = int(round(tstamp.tfsec * 1e6))
    return '%04d:%02d:%02d::%02d:%02d:%02d.%06d' % (gmt.tm_year, gmt.tm_mon, gmt.tm_mday, gmt.tm_hour,
                                                    gmt.tm_min, gmt.tm_sec, fractional)

def convert(timeString):
    if timeString == 'now':
        return now()
    _sets = timeString.split(':')
    if len(_sets) != 7:
        return CF.UTCTime(0,0,0)
    _year, _month, _day, _blank, _hours, _minutes, _seconds = timeString.split(':')
    _full_seconds = float(_seconds)
    _time = time.mktime((int(_year),int(_month),int(_day),int(_hours),int(_minutes),int(_full_seconds),0,0,0))-time.timezone
    return CF.UTCTime(1, _time, _full_seconds - int(_full_seconds))
    # Break out the whole seconds into a GMT time

# Insert the arithmetic functions as operators on the PrecisionUTCTime class
CF.UTCTime.__add__ = add
CF.UTCTime.__iadd__ = iadd
CF.UTCTime.__sub__ = sub
CF.UTCTime.__isub__ = isub
CF.UTCTime.__str__ = toString
CF.UTCTime.__cmp__ = compare
