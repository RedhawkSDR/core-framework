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
        super(ResourceTests,self).setUp()
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        self.comp.releaseObject()

    def testRHBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()

    def testBasicProperties(self):
        self.assertEqual(self.comp.device_kind, "FRONTEND::TUNER")
        self.assertEqual(self.comp.device_model, "FEI codegen test device")

        # Check the that tuner status exists and contains the extra "agc" field
        self.assertEqual(len(self.comp.frontend_tuner_status), 1)
        self.assertTrue('FRONTEND::tuner_status::agc' in self.comp.frontend_tuner_status[0])

if __name__ == "__main__":
    ossie.utils.testing.main("../basic_fei_device.spd.xml") # By default tests all implementations
