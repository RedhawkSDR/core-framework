#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb

class ServiceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the service.
    SPD_FILE = '../allocmgr_svc.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a service using the sandbox API
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
        # Launch the service, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Check that service launched as expected
        self.assertNotEqual(self.comp, None)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
