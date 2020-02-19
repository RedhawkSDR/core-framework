#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import time, frontend

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../fei_src.spd.xml'

    # setUp is run before every function preceded by "test" is executed
    # tearDown is run after every function preceded by "test" is executed
    
    # self.comp is a device using the sandbox API
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
        # Launch the device, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testDefinedConnectionId(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        alloc=frontend.createTunerAllocation()
        snk_1 = sb.StreamSink()
        snk_2 = sb.StreamSink()
        self.comp.allocateCapacity(alloc)
        self.comp.connect(snk_1, connectionId='some_connection')
        tuner_status = self.comp.frontend_tuner_status[0]
        alloc_id = alloc['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.connect(snk_2, connectionId=alloc_id)
        self.assertEquals(len(self.comp.connectionTable), 1)
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        sb.start()
        time.sleep(0.1)
        time.sleep(0.25)
        self.comp.deallocateCapacity(alloc)
        time.sleep(0.1)
        data_1 = snk_1.read(timeout=1)
        self.assertEquals(data_1, None)
        data_2 = snk_2.read(timeout=1)
        self.assertEquals(data_2.streamID, 'my_data')
        self.assertEquals(len(data_2.data), 25)
        self.assertEquals(len(self.comp.connectionTable), 0)
        sb.stop()

    def testConnectBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        alloc=frontend.createTunerAllocation()
        snk_1 = sb.StreamSink()
        snk_2 = sb.StreamSink()
        self.comp.allocateCapacity(alloc)
        self.comp.connect(snk_1, connectionId='some_connection')
        tuner_status = self.comp.frontend_tuner_status[0]
        alloc_id = alloc['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.connect(snk_2)
        self.assertEquals(len(self.comp.connectionTable), 2)
        alloc_id = alloc_id + ',' + self.comp.connectionTable[1].connection_id
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        sb.start()
        time.sleep(0.1)
        time.sleep(0.25)
        self.comp.deallocateCapacity(alloc)
        time.sleep(0.1)
        data_1 = snk_1.read(timeout=1)
        self.assertEquals(data_1, None)
        data_2 = snk_2.read(timeout=1)
        self.assertEquals(data_2.streamID, 'my_data')
        self.assertEquals(len(data_2.data), 25)
        self.assertEquals(len(self.comp.connectionTable), 0)
        sb.stop()

    def testDisconnectBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        alloc=frontend.createTunerAllocation()
        snk_1 = sb.StreamSink()
        snk_2 = sb.StreamSink()
        self.comp.allocateCapacity(alloc)
        self.comp.connect(snk_1, connectionId='some_connection')
        tuner_status = self.comp.frontend_tuner_status[0]
        alloc_id = alloc['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.connect(snk_2)
        sb.start()
        time.sleep(0.1)
        time.sleep(0.25)
        self.comp.disconnect(snk_2)
        time.sleep(0.1)
        data_1 = snk_1.read(timeout=1)
        self.assertEquals(data_1, None)
        data_2 = snk_2.read(timeout=1)
        self.assertEquals(data_2.streamID, 'my_data')
        self.assertEquals(len(data_2.data), 25)
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        sb.stop()

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
