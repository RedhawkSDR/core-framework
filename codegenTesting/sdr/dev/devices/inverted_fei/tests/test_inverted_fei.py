#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import frontend
from redhawk.frontendInterfaces import FRONTEND

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../inverted_fei.spd.xml'

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
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        rf_center_freq = 1e7
        rf_bandwidth = 3e5
        if_center_freq = 1e6
        pkt_str=FRONTEND.RFInfoPkt('my_flow',rf_center_freq,rf_bandwidth,if_center_freq,False,_sensorinfo,[],_rfcapabilities,[])
        request_cf = 9.95e6
        request_bw = 2e5
        alloc = frontend.createTunerAllocation(allocation_id='foo', center_frequency=request_cf, bandwidth=request_bw, sample_rate=0.0)
        port=self.comp.ports[1]
        port.ref._set_rfinfo_pkt(pkt_str)
        self.comp.max_dev_if_cf = 1e6
        self.assertTrue(self.comp.allocateCapacity(alloc))
        self.comp.deallocateCapacity(alloc)
        pkt_inv=FRONTEND.RFInfoPkt('my_flow',rf_center_freq,rf_bandwidth,if_center_freq,True,_sensorinfo,[],_rfcapabilities,[])
        port.ref._set_rfinfo_pkt(pkt_inv)
        self.assertFalse(self.comp.allocateCapacity(alloc))
        self.comp.max_dev_if_cf = 1.15e6
        self.assertTrue(self.comp.allocateCapacity(alloc))
        self.comp.deallocateCapacity(alloc)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
