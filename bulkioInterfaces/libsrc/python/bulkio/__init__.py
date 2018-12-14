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
"""
bulkio 

Is the python interface library data exchange methods between component of the REDHAWK framework. There are 3 main modules
that support this library:

   timestamp : methods used to create simple BULKIO.PrecisionUTCTime object that provide the ability to reference a time stamp 

   sri : meta data that further documents the contents of the data stream being passed between components

   input ports : input port (sinks) objects used by REDHAWK SCA components to receive data streams.

   output ports : output port (source) objects used by REDHAWK SCA components to publish data streams.

  


"""
#
# Import classes for bulkio python library
#

# 
from statistics import *

import timestamp

import sri

import const

from input_ports import *

from output_ports import *
