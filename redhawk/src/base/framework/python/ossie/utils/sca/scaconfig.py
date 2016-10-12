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

import os

def sdrRoot():
    if 'SDRROOT' in os.environ and os.path.exists(os.environ['SDRROOT']):
        sdrroot = os.path.normpath(os.environ['SDRROOT'])
    elif os.path.exists('/sdr/sca'):
        sdrroot = '/sdr/sca'
    elif os.path.exists('/sdr'):
        sdrroot = '/sdr'
    else:
        print "Cannot find SDR root directory"
        return False
        
    return sdrroot

def ossieRoot():
    if 'OSSIEHOME' in os.environ and os.path.exists(os.environ['OSSIEHOME']):
        ossieroot = os.path.normpath(os.environ['OSSIEHOME'])
    elif os.path.exists('/usr/local/include/ossie') and os.path.exists('/usr/local/share/ossie'):
        ossieroot = '/usr/local'
    elif os.path.exists('/usr/include/ossie') and os.path.exists('/usr/share/ossie'):
        ossieroot = '/usr'
    else:
        print "Cannot find OSSIE installation location."
        return False

    return ossieroot

def idlRoot():
    ossieroot = ossieRoot()
    if not ossieRoot: return False

    if os.path.exists(os.path.join(ossieroot,'share/ossie/idl')):
        idlroot = os.path.join(ossieroot,'share/ossie/idl')
    else:
        print "Cannot find OSSIE IDL location."
        return False

    return idlroot

def ossieInclude():
    ossieroot = ossieRoot()
    if not ossieRoot: return False

    if os.path.exists(os.path.join(ossieroot,'include/ossie')):
        ossieinclude = os.path.join(ossieroot,'include/ossie')
    else:
        print "Cannot find OSSIE IDL location."
        return False
        
    return ossieinclude

def ossieShare():
    ossieroot = ossieRoot()
    if not ossieRoot: return False

    if os.path.exists(os.path.join(ossieroot,'share/ossie')):
        ossieshare = os.path.join(ossieroot,'share/ossie')
    else:
        print "Cannot find OSSIE share location."
        return False
        
    return ossieshare
