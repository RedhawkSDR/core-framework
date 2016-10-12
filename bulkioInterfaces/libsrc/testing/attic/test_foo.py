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

import sys
from test_cntl import *


nchan=1
wname = "Bwave"
print "ARGS(" + str(len(sys.argv)) + ")" + str(sys.argv)

#
# attach do domain first,  this affects sys.argv default REDHAWK_DEV
#
attach('REDHAWK_DEV')

if len(sys.argv) > 1 :
    nchan=int(sys.argv[1])

if len(sys.argv) > 2 :
    wname=sys.argv[2]

def track_datain():
    track_stats('TestJava', 'dataShortIn' )



start_waveforms(wname, nchan)


