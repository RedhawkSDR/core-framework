#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import time
from ossie.utils.testing import main as _ossie_test_main
main=_ossie_test_main

class ComponentTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the component.
    SPD_FILE = '../src.spd.xml'

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
        # Launch the component, using the selected implementation
        self.comp = sb.launch('../../src/src.spd.xml')
        self.snk = sb.launch('../../snk_slow/snk_slow.spd.xml')
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testSrcSnkBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.connect(self.snk)
        self.comp.start()
        self.snk.start()
        time.sleep(1)
        try:
            self.comp.stop()
        except:
            pass
        self.snk.stop()
        self.comp.releaseObject()
        self.snk.releaseObject()

    def testSnkSrcBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.connect(self.snk)
        self.comp.start()
        self.snk.start()
        time.sleep(1)
        self.snk.stop()
        try:
            self.comp.stop()
        except:
            pass
        self.comp.releaseObject()
        self.snk.releaseObject()

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

    main("../src.spd.xml") # By default tests all implementations

