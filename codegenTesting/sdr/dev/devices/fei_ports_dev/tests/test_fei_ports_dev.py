#!/usr/bin/env python

import ossie.utils.testing
from ossie.utils import sb
from redhawk.frontendInterfaces import FRONTEND
import bulkio

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../fei_ports_dev.spd.xml'

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

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        self.comp.stop()

        gps_port = self.comp.getPort('gps_in')
        tuner_port = self.comp.getPort('tuner_in')
        navdata_port = self.comp.getPort('navdata_in')
        rfsource_port = self.comp.getPort('rfsource_in')
        rfinfo_port = self.comp.getPort('rfinfo_in')

        allocation_id = "hello"
        rightnow = bulkio.timestamp.now()

        # tuner
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerType, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerDeviceControl, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerGroupId, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerRfFlowId, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setTunerCenterFrequency, "hello", 1.0)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerCenterFrequency, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setTunerBandwidth, "hello", 1.0)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerBandwidth, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setTunerAgcEnable, "hello", True)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerAgcEnable, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setTunerGain, "hello", 1.0)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerGain, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setTunerReferenceSource, "hello", 1L)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerReferenceSource, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setTunerEnable, "hello", True)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerEnable, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setTunerOutputSampleRate, "hello", 1.0)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getTunerOutputSampleRate, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.getScanStatus, "hello")
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setScanStartTime, "hello", rightnow)
        _scan_strategy=FRONTEND.ScanningTuner.ScanStrategy(FRONTEND.ScanningTuner.MANUAL_SCAN, FRONTEND.ScanningTuner.ScanModeDefinition(center_frequency=1.0), FRONTEND.ScanningTuner.TIME_BASED, 0.0)
        self.assertRaises(FRONTEND.NotSupportedException, tuner_port.setScanStrategy, "hello", _scan_strategy)

        # gps
        _gpsinfo = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',rightnow,[])
        self.assertEquals(gps_port._get_gps_info().status_message, '')
        gps_port._set_gps_info(_gpsinfo)
        self.assertEquals(gps_port._get_gps_time_pos().position.datum, 'DATUM_WGS84')
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _gpstimepos = FRONTEND.GpsTimePos(_positioninfo, rightnow)
        gps_port._set_gps_time_pos(_gpstimepos)

        # navdata
        _cartesianpos = FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _velocityinfo = FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _accelerationinfo = FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _attitudeinfo = FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
        _navpacket = FRONTEND.NavigationPacket('','',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,rightnow,[])
        navdata_port._set_nav_packet(_navpacket)
        new_navpacket = navdata_port._get_nav_packet()
        self.assertEquals(new_navpacket.position.datum, 'DATUM_WGS84')

        # rfinfo
        self.assertEquals(rfinfo_port._get_rf_flow_id(), '')
        rfinfo_port._set_rf_flow_id('hello')
        self.assertEquals(rfinfo_port._get_rfinfo_pkt().rf_flow_id, '')
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        rfinfo_port._set_rfinfo_pkt(_rfinfopkt)

        # rfsource
        self.assertEquals(len(rfsource_port._get_available_rf_inputs()), 0)
        rfsource_port._set_available_rf_inputs([])
        self.assertEquals(rfsource_port._get_current_rf_input().rf_flow_id, '')
        rfsource_port._set_current_rf_input(_rfinfopkt)

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
