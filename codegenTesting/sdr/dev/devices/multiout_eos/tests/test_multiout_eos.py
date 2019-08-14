#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import frontend, time
from ossie.cf import CF

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../multiout_eos.spd.xml'

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

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        sb.start()
        alloc = frontend.createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id='master',center_frequency=100)
        listen_alloc = frontend.createTunerListenerAllocation('master','slave')
        another_listen_alloc = frontend.createTunerListenerAllocation('master','another_slave')
        self.comp.allocateCapacity(alloc)
        self.comp.allocateCapacity(listen_alloc)
        self.comp.allocateCapacity(another_listen_alloc)

        master = sb.StreamSink()
        slave = sb.StreamSink()
        another_slave = sb.StreamSink()
        self.comp.connect(master,connectionId='master')
        self.comp.connect(slave,connectionId='slave')
        self.comp.connect(another_slave,connectionId='another_slave')

        time.sleep(1)
        fake_master_alloc = frontend.createTunerListenerAllocation('master','master')
        self.assertRaises(CF.Device.InvalidCapacity, self.comp.deallocateCapacity, fake_master_alloc)
        self.assertEquals(self.comp.frontend_tuner_status[0].allocation_id_csv[:6],'master')

        def get_eos(streamSink, **kwargs):
            streamData = streamSink.read(**kwargs)
            if streamData:
                return streamData.eos
            return False

        self.assertEquals(get_eos(master), False)
        self.assertEquals(get_eos(slave), False)
        self.assertEquals(get_eos(another_slave), False)

        self.comp.deallocateCapacity(listen_alloc)
        time.sleep(0.5)
        self.assertEquals(get_eos(master), False)
        # Save result so we dont call read() twice after eos.
        eos_slave = get_eos(slave, timeout=1, eos=True)
        self.assertEquals(eos_slave, True)
        self.assertEquals(get_eos(another_slave), False)

        self.comp.deallocateCapacity(alloc)
        self.assertEquals(get_eos(master, timeout=1, eos=True), True)
        self.assertEquals(eos_slave, True)
        self.assertEquals(get_eos(another_slave, timeout=1, eos=True), True)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
