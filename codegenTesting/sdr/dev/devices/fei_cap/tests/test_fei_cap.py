#!/usr/bin/env python3

import ossie.utils.testing
from ossie.utils import sb
import frontend
from ossie.cf import CF
from omniORB import any
from redhawk.frontendInterfaces import FRONTEND

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../fei_cap.spd.xml'

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
        self.comp.stop()
        a=frontend.createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id='1',returnDict=False)
        b=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='2',returnDict=False)
        c=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='3',returnDict=False)
        d=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='4',returnDict=False)
        a_bad=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='1',returnDict=False)
        a_bad.id = 'foo'
        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)
        ret_a = self.comp.allocate([a])
        self.assertEqual(len(ret_a), 1)
        #self.assertEqual(self.comp.allocateCapacity([a]), True)
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        ret_b = self.comp.allocate([b])
        self.assertEqual(len(ret_b), 1)
        #self.assertEqual(self.comp.allocateCapacity([b]), True)
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        ret_c = self.comp.allocate([c])
        self.assertEqual(len(ret_c), 1)
        #self.assertEqual(self.comp.allocateCapacity([b]), True)
        self.assertEqual(self.comp._get_usageState(), CF.Device.BUSY)
        ret_d = self.comp.allocate([d])
        self.assertEqual(len(ret_d), 0)
        #self.assertEqual(self.comp.allocateCapacity([c]), False)
        self.assertEqual(self.comp._get_usageState(), CF.Device.BUSY)
        try:
            self.comp.deallocate('bad_id')
        except Exception as e:
            pass
        try:
            self.comp.deallocate('2')
        except Exception as e:
            pass
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        self.comp.deallocateCapacity([a])
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        self.comp.deallocateCapacity([c])
        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)

    def testListenerBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()
        b=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='2',returnDict=False)
        c=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='3',returnDict=False)
        d=frontend.createTunerListenerAllocation('2','4',returnDict=False)

        fts=self.comp.frontend_tuner_status
        self.assertEqual(len(fts), 3)
        for f in fts:
            self.assertEqual(f['FRONTEND::tuner_status::allocation_id_csv'], '')

        ret_b = self.comp.allocate([b])
        self.assertEqual(len(ret_b), 1)
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        fts=self.comp.frontend_tuner_status
        found_id=False
        for f in fts:
            if f['FRONTEND::tuner_status::allocation_id_csv'] == '2':
                found_id = True
                break
        self.assertTrue(found_id)

        ret_c = self.comp.allocate([c])
        self.assertEqual(len(ret_c), 1)
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        fts=self.comp.frontend_tuner_status
        found_id=False
        for f in fts:
            if f['FRONTEND::tuner_status::allocation_id_csv'] == '3':
                found_id = True
                break
        self.assertTrue(found_id)

        ret_d = self.comp.allocate([d])
        self.assertEqual(len(ret_d), 1)
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        fts=self.comp.frontend_tuner_status
        found_id=False
        for f in fts:
            if f['FRONTEND::tuner_status::allocation_id_csv'] == '2,4':
                found_id = True
                break
        self.assertTrue(found_id)

        try:
            self.comp.deallocate('bad_id')
        except Exception as e:
            pass
        try:
            self.comp.deallocate('2')
        except Exception as e:
            pass
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        fts=self.comp.frontend_tuner_status
        found_id=False
        for f in fts:
            if f['FRONTEND::tuner_status::allocation_id_csv'] == '2' or f['FRONTEND::tuner_status::allocation_id_csv'] == '2,4':
                found_id = True
                break
        self.assertFalse(found_id)

        self.comp.deallocate('3')
        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)
        fts=self.comp.frontend_tuner_status
        self.assertEqual(len(fts), 3)
        for f in fts:
            self.assertEqual(f['FRONTEND::tuner_status::allocation_id_csv'], '')

    def testOldAllocationCall(self):
        #######################################################################
        # test the allocate api over allocateCapacity
        self.comp.start()
        self.comp.stop()
        a=frontend.createTunerAllocation(tuner_type='RX_DIGITIZER',allocation_id='1',returnDict=False)
        b=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='2',returnDict=False)
        c=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='3',returnDict=False)
        d=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='4',returnDict=False)
        a_bad=frontend.createTunerAllocation(tuner_type='RDC',allocation_id='1',returnDict=False)
        a_bad.id = 'foo'
        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)
        retval_a = self.comp.allocateCapacity([a])
        self.assertEqual(retval_a, True)
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        retval_b = self.comp.allocateCapacity([b])
        self.assertEqual(retval_b, True)
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        retval_c = self.comp.allocateCapacity([c])
        self.assertEqual(retval_c, True)
        self.assertEqual(self.comp._get_usageState(), CF.Device.BUSY)
        retval_d = self.comp.allocateCapacity([d])
        self.assertEqual(retval_d, False)
        self.assertEqual(self.comp._get_usageState(), CF.Device.BUSY)
        self.comp.deallocateCapacity([b])
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        self.comp.deallocateCapacity([a])
        self.assertEqual(self.comp._get_usageState(), CF.Device.ACTIVE)
        self.comp.deallocateCapacity([c])
        self.assertEqual(self.comp._get_usageState(), CF.Device.IDLE)

    def testRFInfo(self):
        _port = None
        for port in self.comp.ports:
            if port.name == 'RFInfo_in':
                _port = port
                break
        _antennainfo=FRONTEND.AntennaInfo('testAnt','testType','testSize','testDescription')
        _freqrange=FRONTEND.FreqRange(0,100,[1])
        _feedinfo=FRONTEND.FeedInfo('testFeed','testPol',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('testMission','testCollector','testRX',_antennainfo,_feedinfo)
        _pathdelays=[FRONTEND.PathDelay(100,200), FRONTEND.PathDelay(300,400)]
        _rfcapabilities=FRONTEND.RFCapabilities(FRONTEND.FreqRange(1,2,[]),FRONTEND.FreqRange(3,4,[]), FRONTEND.Range(1,2,[]), FRONTEND.Range(4,5,[]))
        _additionalinfo = [CF.DataType(id='a',value=any.to_any(1)), CF.DataType(id='b',value=any.to_any(2)), CF.DataType(id='c',value=any.to_any(3))]
        _rfinfopkt=FRONTEND.RFInfoPkt('TestID',256.0,101.0,412.0,False,_sensorinfo,_pathdelays,[],[],_rfcapabilities,_additionalinfo)
        _port.ref._set_rfinfo_pkt(_rfinfopkt)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
