#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb

class ComponentTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the component.
    SPD_FILE = '../init_props.spd.xml'

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
        self.comp = sb.launch(self.spd_file, impl=self.impl, initialize=False, configure=None)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testStructSequenceDefaulValues(self):
        # Quick sanity check on the sandbox component's default value
        self.assertEqual(len(self.comp.initseq.defValue), 2)
        self.assertEqual(self.comp.initseq.defValue[0]['initseq::longval'], -1)
        self.assertEqual(self.comp.initseq.defValue[1]['initseq::stringseq'], ['d', 'e'])

        # Compare the queried value with the default value as a dictionary
        self.assertEqual(self.comp.initseq, self.comp.initseq.defValue)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
