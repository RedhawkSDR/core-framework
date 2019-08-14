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
import frontend

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

    def testFailedAllocation(self):
        # Check the that tuner status exists and contains the extra "agc" field
        frontend_allocation_1 = frontend.tuner_device.createTunerAllocation(tuner_type="RX_DIGITIZER",bandwidth=24.576, center_frequency=30000000, sample_rate=123, bandwidth_tolerance=100, allocation_id='hello')
        retval = self.comp.allocateCapacity(frontend_allocation_1)
        self.assertEqual(retval, True)
        self.assertEqual(self.comp.frontend_tuner_status[0].bandwidth, 24.576)
        self.assertEqual(self.comp.frontend_tuner_status[0].sample_rate, 123)
        self.assertEqual(self.comp.frontend_tuner_status[0].center_frequency, 30000000)
        frontend_allocation_2 = frontend.tuner_device.createTunerAllocation(tuner_type="RX_DIGITIZER",bandwidth=20.576, center_frequency=20000000, sample_rate=456, bandwidth_tolerance=100, allocation_id='hello_2')
        retval = self.comp.allocateCapacity(frontend_allocation_2)
        self.assertEqual(retval, False)
        self.assertEqual(self.comp.frontend_tuner_status[0].bandwidth, 24.576)
        self.assertEqual(self.comp.frontend_tuner_status[0].sample_rate, 123)
        self.assertEqual(self.comp.frontend_tuner_status[0].center_frequency, 30000000)

if __name__ == "__main__":
    ossie.utils.testing.main("../basic_fei_device.spd.xml") # By default tests all implementations
