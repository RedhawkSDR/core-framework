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

"""
Private module to provide equivalents to commonly used functions from Python
2.5+ 'uuid' module to older versions of Python. This module should not be
imported directly.
"""

import commands as _commands

def uuid1(node=None, clock_seq=None):
    """
    Generate a UUID from a host ID, sequence number, and the cuurent time.
    The 'node' and 'clock_seq' arguments are ignored.
       Attempt to use 'uuidgen'
       Attempt to use 'uuid' (for debian)
    """
    (result, output) = _commands.getstatusoutput('uuidgen -t')
    if (result == 0): return output
    return _commands.getoutput('uuid -t')

def uuid4():
    """
    Generate a random UUID.
       Attempt to use 'uuidgen'
       Attempt to use 'uuid' (for debian)
    """
    (result, output) = _commands.getstatusoutput('uuidgen -r')
    if (result == 0): return output
    return _commands.getoutput('uuid -r')
