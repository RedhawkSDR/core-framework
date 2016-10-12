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

# Helper classes to deal with threading, especially for CORBA servants
# because default omniORB multi-threads all upcalls
import copy
import threading

def synchronize(f, lock):
    """A old-style decorator to simplify making callables thread-safe.  This
    can be useful for making CORBA servant thread-safe without having to resort
    to the SINGLE_THREAD POA model."""
    def __f__(*args, **kwargs):
        try:
            lock.acquire()
            return f(*args, **kwargs)
        finally:
            lock.release()
    return __f__

class metaSynchronized(type):
    """A meta-class the applies the syncronize tag to all callables"""
    def __new__(cls, classname, bases, classdict):
        newdict = copy.copy(classdict)
        rlock = threading.RLock()
        newdict['__rlock__'] = rlock

        for k, v in classdict.items():
            # Don't syncronize special methods
            if k.startswith("__") and k.endswith("__"):
                continue
            # Only' syncronize callables
            if not callable(v):
                continue
            newdict[k] = synchronize(v, rlock)
        return type.__new__(cls, classname, bases, newdict)

class Synchronized(object):
    """A convient way add the syncronized metaclass via inheritence"""
    __metaclass__ = metaSynchronized
