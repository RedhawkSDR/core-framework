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
from ossie.utils.testing import main as _ossie_test_main
main=_ossie_test_main

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in Oversized_framedata"""

    def testConsistentSize(self):
        """Test that subsequent timestamps differ by an integer multiple of sri.subsize.
        """
        #######################################################################
        # Launch the resource with the default execparams
        self.launch()

        sink = sb.StreamSink()
        sink.start()
        self.comp.connect(sink)

        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()

        data = sink.read(eos=True)

        ts_first = data.timestamps[0][1]
        ts_last = data.timestamps[-1][1]
        delta = int(ts_last.twsec - ts_first.twsec)
        self.assertEquals(delta % data.sri.subsize, 0)

        #######################################################################
        # Simulate regular resource shutdown
        sink.releaseObject()
        self.comp.releaseObject()

    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.bulkio.bulkio_helpers,
    #   ossie.utils.bluefile.bluefile_helpers
    # for modules that will assist with testing resource with BULKIO ports

if __name__ == "__main__":
    try:
        import sys
        import os
        if '--with-xunit' in sys.argv:
            sys.argv=sys.argv[:]+[ __file__ ]
            import use_nose_test
            main=use_nose_test.NoseTestProgram
    except:
        traceback.print_exc()
        pass

    main("../Oversized_framedata.spd.xml") # By default tests all implementations
