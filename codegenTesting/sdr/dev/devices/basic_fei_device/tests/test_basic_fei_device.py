#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK codegenTesting.
#
# REDHAWK codegenTesting is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
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
from ossie.cf import CF

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
        tuner_alloc = CF.DataType(id='FRONTEND::tuner_allocation', value=any.to_any(None))
        listen_alloc = CF.DataType(id='FRONTEND::listener_allocation', value=any.to_any(None))
        self.assertRaises(CF.UnknownProperties, self.comp.query, [tuner_alloc])
        self.assertRaises(CF.UnknownProperties, self.comp.query, [listen_alloc])

if __name__ == "__main__":
    ossie.utils.testing.main("../basic_fei_device.spd.xml") # By default tests all implementations
