#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
from ossie.cf import CF
from omniORB import any
from omniORB import CORBA
from ossie.utils.bulkio.bulkio_data_helpers import SDDSSink
from redhawk.frontendInterfaces import FRONTEND
from ossie.utils import uuid
from ossie import properties
import time
from ossie.utils.bluefile.bluefile_helpers import sri_to_hdr
import frontend

DEBUG_LEVEL = 2

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../RX_Digitizer_Sim.spd.xml'

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
        self.comp = sb.launch(self.spd_file, impl=self.impl,execparams={'DEBUG_LEVEL': DEBUG_LEVEL})
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()

    def testSingleTunerAllocation(self):
        
        #self.comp.start()
        
        sink = sb.DataSink()
        
               
        alloc = self._generateAlloc(cf=110e6,sr=2.5e6,bw=2e6)
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()
        
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
            print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
            print "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%"
            print str(e)
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if not retval:
            self.assertFalse("Allocation Failed")
            
        time.sleep(1)
        
        sri = sink.sri()
        self.assertEqual(sri.streamID, allocationID)
        self.assertAlmostEqual(sri.xdelta, 1.0/2.5e6,5)
        #time.sleep(1)

        data= sink.getData() 
        self.assertTrue(len(data)>0)
        
        self.comp.deallocateCapacity(alloc)
        time.sleep(1)
        
        sri = sink.sri()
        self.assertTrue( sink.eos())

    def testDoubleTunerAllocation(self):
        
        #self.comp.start()
        
        #Create Allocation and Sink for Tuner 1
        sink = sb.DataSink()               
        alloc = self._generateAlloc(cf=110e6,sr=2.5e6,bw=2e6)
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()


        
        #Allocate a Tuner
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if not retval:
            self.assertFalse("Allocation Failed")

        #Sleep and let data and SRI flow    
        time.sleep(1)
        
        #Check Tuner 1 SRI
        sri = sink.sri()
        print "SRI 1 1st Time" , sri
        self.assertEqual(sri.streamID, allocationID, "SRI 1 Did not Match")
        self.assertAlmostEqual(sri.xdelta, 1.0/2.5e6,5)

        #Create Allocation and Sink for Tuner 2
        sink2 = sb.DataSink()               
        alloc2 = self._generateAlloc(cf=110e6,sr=5e6,bw=4e6)
        allocationID2 = properties.props_to_dict(alloc2)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink2,connectionId=allocationID2)
        sink2.start()

        #Allocate a Second Tuner
        try:
            retval = self.comp.allocateCapacity(alloc2)
        except Exception, e:
            print str(e)
            self.assertFalse("Exception thrown on allocateCapactiy on second Tuner %s" % str(e))
        if not retval:
            self.assertFalse("Allocation Failed on second Tuner")
        
        #Sleep and let data and SRI flow    
        time.sleep(1)

        #Check Tuner 1 SRI Again (should not change)
        sri = sink.sri()
        print "SRI 1 2nd Time" , sri
        self.assertEqual(sri.streamID, allocationID, "SRI 1 Did not Match Second Time")
        self.assertAlmostEqual(sri.xdelta, 1.0/2.5e6,5)

        #Check Tuner 2 SRI
        sri2 = sink2.sri()
        print "SRI 2 " , sri2
        print "allocationID2",  allocationID2
        self.assertEqual(sri2.streamID, allocationID2,"SRI 2 Did not MAtch")
        self.assertAlmostEqual(sri2.xdelta, 1.0/5.0e6,5)

        #Check Tuner 1 Data
        data= sink.getData() 
        self.assertTrue(len(data)>0)
        
        #Check Tuner 2 Data
        data2= sink2.getData() 
        self.assertTrue(len(data2)>0)
        
        #Deallocate Tuners
        self.comp.deallocateCapacity(alloc)
        self.comp.deallocateCapacity(alloc2)
        time.sleep(1)
        
        #Check that they sent EOS
        self.assertTrue(sink.eos())
        self.assertTrue(sink2.eos())

    def testValidRFInfoPacket(self):
        
        #Create Allocation and Sink for Tuner 1
        sink = sb.DataSink()               
        alloc = self._generateAlloc(cf=110e6,sr=2.5e6,bw=2e6,rf_flow_id="testRFInfoPacket_FlowID")
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()

        #Send an RF Info Packet
        rfInfo_port = self.comp.getPort("RFInfo_in")
        rf_info_pkt = self._generateRFInfoPkt(rf_freq=100e6,rf_bw=100e6,if_freq=0,rf_flow_id="testRFInfoPacket_FlowID")
        rfInfo_port._set_rfinfo_pkt(rf_info_pkt)

        
        #Allocate a Tuner
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if not retval:
            self.assertFalse("Allocation Failed")
        
        self.comp.deallocateCapacity(alloc)

    def testInValidRFInfoPacket(self):
        
        #Create Allocation and Sink for Tuner 1
        sink = sb.DataSink()               
        alloc = self._generateAlloc(cf=110e6,sr=2.5e6,bw=2e6,rf_flow_id="invalid_FlowID")
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()

        #Send an RF Info Packet
        rfInfo_port = self.comp.getPort("RFInfo_in")
        rf_info_pkt = self._generateRFInfoPkt(rf_freq=100e6,rf_bw=100e6,if_freq=0,rf_flow_id="testRFInfoPacket_FlowID")
        rfInfo_port._set_rfinfo_pkt(rf_info_pkt)

        
        #Allocate a Tuner
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if retval:
            self.assertFalse("Allocation Succeeded but should have failed")
        
        try:
            self.comp.deallocateCapacity(alloc)      
        except CF.Device.InvalidCapacity, e:
            # Deallocating shouldn't be required if the allocation failed so we would expect this deallocation to be invalid
            pass

    def testValidRFInfoFreq(self):
        """ Ask for a Allocation outside the range of the REceiver but should still work because the RFInfo packet tells the receiver a frequency down conversation has already occured"""
        
        #Create Allocation and Sink for Tuner 1
        sink = sb.DataSink()               
        alloc = self._generateAlloc(cf=9001e6,sr=2.5e6,bw=2e6,rf_flow_id="testRFInfoPacket_FlowID")
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()

        #Send an RF Info Packet
        rfInfo_port = self.comp.getPort("RFInfo_in")
        rf_info_pkt = self._generateRFInfoPkt(rf_freq=9000e6,rf_bw=100e6,if_freq=100e6,rf_flow_id="testRFInfoPacket_FlowID")
        rfInfo_port._set_rfinfo_pkt(rf_info_pkt)

        
        #Allocate a Tuner
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if not retval:
            self.assertFalse("Allocation Failed")
        
        try:
            self.comp.deallocateCapacity(alloc)      
        except CF.Device.InvalidCapacity, e:
            # Deallocating shouldn't be required if the allocation failed so we would expect this deallocation to be invalid
            pass              

    def testInValidRFInfoFreq(self):
        """ Ask for a Allocation outside the range of the REceiver but should still work because the RFInfo packet tells the receiver a frequency down conversation has already occured"""
        
        #Create Allocation and Sink for Tuner 1
        sink = sb.DataSink()               
        alloc = self._generateAlloc(cf=100e6,sr=2.5e6,bw=2e6,rf_flow_id="testRFInfoPacket_FlowID")
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()

        #Send an RF Info Packet
        rfInfo_port = self.comp.getPort("RFInfo_in")
        rf_info_pkt = self._generateRFInfoPkt(rf_freq=9000e6,rf_bw=100e6,if_freq=100e6,rf_flow_id="testRFInfoPacket_FlowID")
        rfInfo_port._set_rfinfo_pkt(rf_info_pkt)

        
        #Allocate a Tuner
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if retval:
            self.assertFalse("Allocation Succeeded but should have failed")
        
        try:
            self.comp.deallocateCapacity(alloc)      
        except CF.Device.InvalidCapacity, e:
            # Deallocating shouldn't be required if the allocation failed so we would expect this deallocation to be invalid
            pass        

    def testInValidRFInfoBW(self):
        """ Ask for a Allocation outside the range of the REceiver but should still work because the RFInfo packet tells the receiver a frequency down conversation has already occured"""
        
        #Create Allocation and Sink for Tuner 1
        sink = sb.DataSink()               
        alloc = self._generateAlloc(cf=9049.1e6,sr=0,bw=2e6,rf_flow_id="testRFInfoPacket_FlowID")
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()

        #Send an RF Info Packet
        rfInfo_port = self.comp.getPort("RFInfo_in")
        rf_info_pkt = self._generateRFInfoPkt(rf_freq=9000e6,rf_bw=100e6,if_freq=100e6,rf_flow_id="testRFInfoPacket_FlowID")
        rfInfo_port._set_rfinfo_pkt(rf_info_pkt)

        
        #Allocate a Tuner
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if retval:
            self.assertFalse("Allocation Succeeded but should have failed")
        
        try:
            self.comp.deallocateCapacity(alloc)      
        except CF.Device.InvalidCapacity, e:
            # Deallocating shouldn't be required if the allocation failed so we would expect this deallocation to be invalid
            pass   

    def testInValidRFInfoSR(self):
        """ Ask for a Allocation outside the range of the REceiver but should still work because the RFInfo packet tells the receiver a frequency down conversation has already occured"""
        
        #Create Allocation and Sink for Tuner 1
        sink = sb.DataSink()               
        alloc = self._generateAlloc(cf=9049.1e6,sr=2.5e6,bw=0,rf_flow_id="testRFInfoPacket_FlowID")
        allocationID = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        self.comp.connect(sink,connectionId=allocationID)
        sink.start()

        #Send an RF Info Packet
        rfInfo_port = self.comp.getPort("RFInfo_in")
        rf_info_pkt = self._generateRFInfoPkt(rf_freq=9000e6,rf_bw=100e6,if_freq=100e6,rf_flow_id="testRFInfoPacket_FlowID")
        rfInfo_port._set_rfinfo_pkt(rf_info_pkt)

        
        #Allocate a Tuner
        try:
            retval = self.comp.allocateCapacity(alloc)
        except Exception, e:
            self.assertFalse("Exception thrown on allocateCapactiy %s" % str(e))
        if retval:
            self.assertFalse("Allocation Succeeded but should have failed")
        
        try:
            self.comp.deallocateCapacity(alloc)      
        except CF.Device.InvalidCapacity, e:
            # Deallocating shouldn't be required if the allocation failed so we would expect this deallocation to be invalid
            pass   

    def testInvalidAllocation(self):
        alloc = self._generateAlloc(cf=110e6,sr=2.5e6,bw=2e6)
        retval = self.comp.allocateCapacity(alloc)
        self.assertRaises(CF.Device.InvalidCapacity, self.comp.allocateCapacity,alloc)

    def testTuningException(self):
        alloc = self._generateAlloc(cf=111e6,sr=2.5e6,bw=2e6)
        self.assertEquals(self.comp.allocateCapacity(alloc), False)

    def testFalseControl(self):
        center_frequency = 110e6
        sample_rate = 2.5e6
        bandwidth = 2e6
        alloc = self._generateAlloc(cf=center_frequency,sr=sample_rate,bw=bandwidth)
        retval = self.comp.allocateCapacity(alloc)
        self.assertEquals(retval, True)
        _type = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::tuner_type']
        _alloc_id = properties.props_to_dict(alloc)['FRONTEND::tuner_allocation']['FRONTEND::tuner_allocation::allocation_id']
        listen_alloc = [frontend.createTunerGenericListenerAllocation(_type, allocation_id='foo', center_frequency=center_frequency, bandwidth=bandwidth, sample_rate=sample_rate,returnDict=False)]
        retval = self.comp.allocateCapacity(listen_alloc)
        self.assertEquals(retval, True)
        self.comp.deallocateCapacity(listen_alloc)
        self.comp.deallocateCapacity(alloc)

    def _generateAlloc(self,cf=100e6,sr=25e6,bw=20e6,rf_flow_id=''):
        
        value = {}
        value['ALLOC_ID'] = str(uuid.uuid4())
        value['TYPE'] = 'RX_DIGITIZER'
        value['BW_TOLERANCE'] = 100.0
        value['SR_TOLERANCE'] = 100.0
        value['RF_FLOW_ID'] = rf_flow_id
        value['GROUP_ID'] = ''
        value['CONTROL'] = True
        value['CF'] = cf
        value['SR'] = sr
        value['BW'] = bw
        
        #generate the allocation
        allocationPropDict = {'FRONTEND::tuner_allocation':{
                    'FRONTEND::tuner_allocation::tuner_type': value['TYPE'],
                    'FRONTEND::tuner_allocation::allocation_id': value['ALLOC_ID'],
                    'FRONTEND::tuner_allocation::center_frequency': float(value['CF']),
                    'FRONTEND::tuner_allocation::bandwidth': float(value['BW']),
                    'FRONTEND::tuner_allocation::bandwidth_tolerance': float(value['BW_TOLERANCE']),
                    'FRONTEND::tuner_allocation::sample_rate': float(value['SR']),
                    'FRONTEND::tuner_allocation::sample_rate_tolerance': float(value['SR_TOLERANCE']),
                    'FRONTEND::tuner_allocation::device_control': value['CONTROL'],
                    'FRONTEND::tuner_allocation::group_id': value['GROUP_ID'],
                    'FRONTEND::tuner_allocation::rf_flow_id': value['RF_FLOW_ID'],
                    }}
        return properties.props_from_dict(allocationPropDict)

    def _generateRFInfoPkt(self,rf_freq=1e9,rf_bw=1e9,if_freq=0,spec_inverted=False,rf_flow_id="testflowID"):
        antenna_info = FRONTEND.AntennaInfo("antenna_name","antenna_type","antenna.size","description")
        freqRange= FRONTEND.FreqRange(0,1e12,[] )
        feed_info = FRONTEND.FeedInfo("feed_name", "polarization",freqRange)
        sensor_info = FRONTEND.SensorInfo("msn_name", "collector_name", "receiver_name",antenna_info,feed_info)
        delays = [];
        cap = FRONTEND.RFCapabilities(freqRange,freqRange);
        add_props = [];
        rf_info_pkt = FRONTEND.RFInfoPkt(rf_flow_id,rf_freq, rf_bw, if_freq, spec_inverted, sensor_info, delays, cap, add_props)         
        
        return rf_info_pkt


if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
