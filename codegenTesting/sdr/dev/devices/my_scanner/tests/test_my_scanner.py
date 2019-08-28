#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
import frontend
from redhawk.frontendInterfaces import FRONTEND

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../my_scanner.spd.xml'

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
        self.comp.start()
        bad_tuner=frontend.createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id='1', bandwidth=0.0,returnDict=False)
        good_tuner=frontend.createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id='1', bandwidth=1000.0,returnDict=False)
        bad_scanner=frontend.createScannerAllocation(returnDict=False)
        good_scanner=frontend.createScannerAllocation(min_freq=10000.0,returnDict=False)
        self.assertFalse(self.comp.allocateCapacity([bad_tuner, bad_scanner]))
        self.assertFalse(self.comp.allocateCapacity([good_tuner, bad_scanner]))
        self.assertFalse(self.comp.allocateCapacity([bad_tuner, good_scanner]))
        self.assertTrue(self.comp.allocateCapacity([good_tuner, good_scanner]))
        self.comp.deallocateCapacity([good_tuner, good_scanner])
        self.assertTrue(self.comp.allocateCapacity([good_scanner, good_tuner]))
        ref = None
        for port in self.comp.ports:
            if port.name == 'DigitalScanningTuner_in':
                ref = port.ref
                break
        self.assertEquals(self.comp.strategy_request, 'initial')
        scan_strategy=FRONTEND.ScanningTuner.ScanStrategy(FRONTEND.ScanningTuner.MANUAL_SCAN, FRONTEND.ScanningTuner.ScanModeDefinition(center_frequency=1.0), FRONTEND.ScanningTuner.TIME_BASED, 0.0)
        ref.setScanStrategy('1', scan_strategy)
        self.assertEquals(self.comp.strategy_request, 'manual')
        scan_strategy=FRONTEND.ScanningTuner.ScanStrategy(FRONTEND.ScanningTuner.DISCRETE_SCAN, FRONTEND.ScanningTuner.ScanModeDefinition(discrete_freq_list=[]), FRONTEND.ScanningTuner.TIME_BASED, 0.0)
        ref.setScanStrategy('1', scan_strategy)
        self.assertEquals(self.comp.strategy_request, 'discrete')
        scan_strategy=FRONTEND.ScanningTuner.ScanStrategy(FRONTEND.ScanningTuner.SPAN_SCAN, FRONTEND.ScanningTuner.ScanModeDefinition(freq_scan_list=[]), FRONTEND.ScanningTuner.TIME_BASED, 0.0)
        ref.setScanStrategy('1', scan_strategy)
        self.assertEquals(self.comp.strategy_request, 'span')
        status = ref.getScanStatus('1')
        self.assertEquals(status.strategy.control_value, 123)
        self.comp.stop()

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
