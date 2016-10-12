#!/usr/bin/env python
import unittest
import ossie.utils.testing
import os
from omniORB import any
from ossie.utils import sb

class ResourceTests(ossie.utils.testing.RHComponentTestCase):
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
        self.comp = sb.launch("../dev_src.spd.xml")
        self.comp_snk = sb.launch("../../dev_snk/dev_snk.spd.xml")
    
    def tearDown(self):
        self.comp.releaseObject()
        self.comp_snk.releaseObject()

    def testRHBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.connect(self.comp_snk)
        self.assertTrue(self.comp.ports[0]._get_connections()[0].port._is_equivalent(self.comp_snk.ports[0].ref))

if __name__ == "__main__":
    ossie.utils.testing.main("../dev_src.spd.xml") # By default tests all implementations
