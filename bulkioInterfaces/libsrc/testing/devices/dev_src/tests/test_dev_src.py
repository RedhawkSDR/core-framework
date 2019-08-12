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

class ResourceTests(ossie.utils.testing.RHComponentTestCase):
    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a component using the sandbox API
    # to create a data source, the package sb contains sources like StreamSource or FileSource
    # to create a data sink, there are sinks like StreamSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.StreamSink()
    #  src.connect(self.comp)
    #  self.comp.connect(snk)
    #  sb.start()
    #
    # components/sources/sinks need to be started. Individual components or elements can be started
    #  src.start()
    #  self.comp.start()
    #
    # every component/elements in the sandbox can be started
    #  sb.start()

    def setUp(self):
        self.comp = sb.launch("../dev_src.spd.xml")
        self.comp_snk = sb.launch("../../dev_snk/dev_snk.spd.xml")
    
    def tearDown(self):
        self.comp.releaseObject()
        self.comp_snk.releaseObject()

    def testRHBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.connect(self.comp_snk)
        # Explicitly get the CORBA port references for comparison because the
        # IDL may not be installed, throwing exceptions on comp.ports
        src = self.comp.getPort('dataFloat_out')
        sink = self.comp_snk.getPort('dataFloat_in')
        self.assertTrue(src._get_connections()[0].port._is_equivalent(sink))

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

    main("../dev_src.spd.xml") # By default tests all implementations
