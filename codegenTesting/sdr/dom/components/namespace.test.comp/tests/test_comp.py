#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb

class ComponentTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the component.
    SPD_FILE = '../comp.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a component using the sandbox API
    # to create a data source, the package sb contains data sources like DataSource or FileSource
    # to create a data sink, there are sinks like DataSink and FileSink
    # to connect the component to get data from a file, process it, and write the output to a file, use the following syntax:
    #  src = sb.FileSource('myfile.dat')
    #  snk = sb.DataSink()
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
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()

    def testSpecFile(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        fp = open('../namespace.test.comp.spec','r')
        text = fp.read()
        fp.close()
        self.assertNotEqual(text.find('%dir %{_sdrroot}/dom/components/namespace\n'), -1)
        self.assertNotEqual(text.find('%dir %{_sdrroot}/dom/components/namespace/test\n'), -1)
        self.assertNotEqual(text.find('%dir %{_sdrroot}/dom/components/namespace/test/comp\n'), -1)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
