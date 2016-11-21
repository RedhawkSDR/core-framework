#!/usr/bin/env python
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
import unittest
import ossie.utils.testing
import os
from omniORB import any
from ossie.utils import sb

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in Oversized_framedata"""

    def testConsistentSize(self):
        #######################################################################
        # Launch the resource with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        snk=sb.DataSink()
        snk.start()
        self.comp.connect(snk)

        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        
        (retval, timestamps) = snk._sink.retrieveData(20000000)
        self.assertEquals(timestamps[1][0]%1024,0) 

        #######################################################################
        # Simulate regular resource shutdown
        snk.releaseObject()
        self.comp.releaseObject()
        
    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.bulkio.bulkio_helpers,
    #   ossie.utils.bluefile.bluefile_helpers
    # for modules that will assist with testing resource with BULKIO ports

if __name__ == "__main__":
    ossie.utils.testing.main("../Oversized_framedata.spd.xml") # By default tests all implementations
