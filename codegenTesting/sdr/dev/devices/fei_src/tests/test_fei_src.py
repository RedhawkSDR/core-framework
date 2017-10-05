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
        # Launch the device, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testDefinedConnectionId(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        alloc=frontend.createTunerAllocation()
        snk_1 = sb.DataSink()
        snk_2 = sb.DataSink()
        self.comp.allocateCapacity(alloc)
        self.comp.connect(snk_1, connectionId='some_connection')
        tuner_status = self.comp.frontend_tuner_status[0]
        alloc_id = alloc['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.connect(snk_2, connectionId=alloc_id)
        self.assertEquals(len(self.comp.connectionTable), 1)
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.start()
        time.sleep(0.1)
        time.sleep(0.25)
        self.comp.deallocateCapacity(alloc)
        time.sleep(0.1)
        stream_1 = snk_1.getCurrentStream(timeout=1)
        self.assertEquals(stream_1, None)
        stream_2 = snk_2.getCurrentStream()
        self.assertEquals(stream_2.streamID(), 'my_data')
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(stream_2.read(), None)
        self.assertEquals(len(self.comp.connectionTable), 0)
        self.comp.stop()

    def testConnectBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        alloc=frontend.createTunerAllocation()
        snk_1 = sb.DataSink()
        snk_2 = sb.DataSink()
        self.comp.allocateCapacity(alloc)
        self.comp.connect(snk_1, connectionId='some_connection')
        tuner_status = self.comp.frontend_tuner_status[0]
        alloc_id = alloc['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.connect(snk_2)
        self.assertEquals(len(self.comp.connectionTable), 2)
        alloc_id = alloc_id + ',' + self.comp.connectionTable[1].connection_id
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.start()
        time.sleep(0.1)
        time.sleep(0.25)
        self.comp.deallocateCapacity(alloc)
        time.sleep(0.1)
        stream_1 = snk_1.getCurrentStream(timeout=1)
        self.assertEquals(stream_1, None)
        stream_2 = snk_2.getCurrentStream()
        self.assertEquals(stream_2.streamID(), 'my_data')
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(stream_2.read(), None)
        self.assertEquals(len(self.comp.connectionTable), 0)
        self.comp.stop()

    def testDisconnectBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        alloc=frontend.createTunerAllocation()
        snk_1 = sb.DataSink()
        snk_2 = sb.DataSink()
        self.comp.allocateCapacity(alloc)
        self.comp.connect(snk_1, connectionId='some_connection')
        tuner_status = self.comp.frontend_tuner_status[0]
        alloc_id = alloc['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.connect(snk_2)
        self.comp.start()
        time.sleep(0.1)
        time.sleep(0.25)
        self.comp.disconnect(snk_2)
        time.sleep(0.1)
        stream_1 = snk_1.getCurrentStream(timeout=1)
        self.assertEquals(stream_1, None)
        stream_2 = snk_2.getCurrentStream()
        self.assertEquals(stream_2.streamID(), 'my_data')
        #self.assertEquals(len(stream_2.read().data()), 25)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(len(stream_2.read().data()), 5)
        self.assertEquals(stream_2.read(), None)
        self.assertEquals(tuner_status.allocation_id_csv, alloc_id)
        self.comp.stop()

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
