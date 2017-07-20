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

import zlib
try:
    from hashlib import md5
except ImportError:
    from md5 import md5

def strenum(*values):
    return type('StringEnum', (), dict([(v.upper(), v) for v in values]))

def parseBoolean(value):
    if isinstance(value, basestring):
        if value.lower() in ('true', 'yes', '1'):
            return True
        elif value.lower() in ('false', 'no', '0'):
            return False
        else:
            raise ValueError, "Invalid literal '%s' for boolean value" % value
    else:
        return value

def fileCRC(filename, stripnewlines=False):
    value = 0
    for line in open(filename, 'r'):
        # If requested, strip newlines (for backwards-compatibility with the IDE).
        if stripnewlines and line.endswith('\n'):
            line = line[:-1]
        value = zlib.crc32(line, value)
    # Return an unsigned value; zlib.crc32 typically returns a signed value, but
    # this may differ across Python versions or platforms. Note that this may
    # cause promotion to 'long' on 32-bit systems.
    return value & 0xffffffff

def fileMD5(filename):
    # On FIPS-enabled systems, MD5 is disabled because it's not cryptologically
    # secure; the "usedforsecurity" flag assures the library that it's not used
    # in that way, since in this case it's just a hash for tracking when a file
    # has changed. 
    m = md5(usedforsecurity=False)
    for line in open(filename, 'r'):
        m.update(line)
    return m.hexdigest()
