#!/usr/bin/env python

from redhawk.frontendInterfaces import FRONTEND, FRONTEND__POA
from bulkio.bulkioInterfaces import BULKIO
import ossie.utils.testing
from ossie.utils import sb
import time

class RFInfoPort(FRONTEND__POA.RFInfo):
    def __init__(self):
        pass
    def _get_rf_flow_id(self):
        return 'hello'
    def _set_rf_flow_id(self, rfinfo_pkt):
        pass
    def _get_rfinfo_pkt(self):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt
    def _set_rfinfo_pkt(self, rf_inputs):
        pass

class RFSourcePort(FRONTEND__POA.RFSource):
    def __init__(self):
        pass
    def _get_available_rf_inputs(self):
        return []
    def _set_available_rf_inputs(self, rf_inputs):
        pass
    def _get_current_rf_input(self):
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        return _rfinfopkt
    def _set_current_rf_input(self, rf_inputs):
        pass

class DigitalTunerPort(FRONTEND__POA.DigitalTuner):
    def __init__(self):
        pass
    def getTunerType(self, _id):
        return ''
    def getTunerDeviceControl(self, _id):
        return False
    def getTunerGroupId(self, _id):
        return ''
    def getTunerRfFlowId(self, _id):
        return ''
    def getTunerStatus(self, _id):
        return []
    def setTunerCenterFrequency(self, _id, freq):
        pass
    def getTunerCenterFrequency(self, _id):
        return 0.0
    def setTunerBandwidth(self, _id, bw):
        pass
    def getTunerBandwidth(self, _id):
        return 0.0
    def setTunerAgcEnable(self, _id, enable):
        pass
    def getTunerAgcEnable(self, _id):
        return False
    def setTunerGain(self, _id, gain):
        pass
    def getTunerGain(self, _id):
        return 0.0
    def setTunerReferenceSource(self, _id, source):
        pass
    def getTunerReferenceSource(self, _id):
        return 0
    def setTunerEnable(self, _id, enable):
        pass
    def getTunerEnable(self, _id):
        return False
    def setTunerOutputSampleRate(self, _id, sr):
        pass
    def getTunerOutputSampleRate(self, _id):
        return 0

class GPSPort(FRONTEND__POA.GPS):
    def __init__(self):
        pass
    def _get_gps_info(self):
        _gpsinfo = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0),[])
        return _gpsinfo
    def _set_gps_info(self, gi):
        pass
    def _get_gps_time_pos(self):
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _gpstimepos = FRONTEND.GpsTimePos(_positioninfo,BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0))
        return _gpstimepos
    def _set_gps_time_pos(self, gtp):
        pass

class NavDataPort(FRONTEND__POA.NavData):
    def __init__(self):
        pass
    def _get_nav_packet(self):
        _time = BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0)
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _cartesianpos=FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _velocityinfo=FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _accelerationinfo=FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _attitudeinfo=FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
        _navpacket=FRONTEND.NavigationPacket('','',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,_time,[])
        return _navpacket
    def _set_nav_packet(self, rf_inputs):
        pass

class ComponentTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the component.
    SPD_FILE = '../rf_ctrl.spd.xml'

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
        # Launch the component, using the selected implementation
        self.comp = sb.launch(self.spd_file, impl=self.impl)
        self._rfinfoPort = RFInfoPort()
        self._rfsourcePort = RFSourcePort()
        self._digitaltunerPort = DigitalTunerPort()
        self._gpsPort = GPSPort()
        self._navdataPort = NavDataPort()
        self._rfinfoPort_2 = RFInfoPort()
        self._rfsourcePort_2 = RFSourcePort()
        self._digitaltunerPort_2 = DigitalTunerPort()
        self._gpsPort_2 = GPSPort()
        self._navdataPort_2 = NavDataPort()

    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        sb.release()

    def testBasicBehavior(self):
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp.start()
        time.sleep(0.5)
        self.comp.stop()
        self.assertEquals(self.comp.get_rfinfo, "No connections available.")
        self.assertEquals(self.comp.set_rfinfo, "ok")
        self.assertEquals(self.comp.get_available_rf, "No connections available.")
        self.assertEquals(self.comp.set_available_rf, "ok")
        self.assertEquals(self.comp.get_current_rf, "No connections available.")
        self.assertEquals(self.comp.set_current_rf, "ok")

        self.assertEquals(self.comp.get_tunertype, "No connections available.")
        self.assertEquals(self.comp.get_tunerdevicecontrol, "No connections available.")
        self.assertEquals(self.comp.get_tunergroupid, "No connections available.")
        self.assertEquals(self.comp.get_tunerrfflowid, "No connections available.")
        self.assertEquals(self.comp.get_tunerstatus, "No connections available.")
        self.assertEquals(self.comp.get_tunercenterfrequency, "No connections available.")
        self.assertEquals(self.comp.set_tunercenterfrequency, "ok")
        self.assertEquals(self.comp.get_tunerbandwidth, "No connections available.")
        self.assertEquals(self.comp.set_tunerbandwidth, "ok")
        self.assertEquals(self.comp.get_tuneragcenable, "No connections available.")
        self.assertEquals(self.comp.set_tuneragcenable, "ok")
        self.assertEquals(self.comp.get_tunergain, "No connections available.")
        self.assertEquals(self.comp.set_tunergain, "ok")
        self.assertEquals(self.comp.get_tunerreferencesource, "No connections available.")
        self.assertEquals(self.comp.set_tunerreferencesource, "ok")
        self.assertEquals(self.comp.get_tunerenable, "No connections available.")
        self.assertEquals(self.comp.set_tunerenable, "ok")
        self.assertEquals(self.comp.get_tuneroutputsamplerate, "No connections available.")
        self.assertEquals(self.comp.set_tuneroutputsamplerate, "ok")

        self.assertEquals(self.comp.get_gpsinfo, "No connections available.")
        self.assertEquals(self.comp.set_gpsinfo, "ok")
        self.assertEquals(self.comp.get_gps_timepos, "No connections available.")
        self.assertEquals(self.comp.set_gps_timepos, "ok")
        self.assertEquals(self.comp.get_nav_packet, "No connections available.")
        self.assertEquals(self.comp.set_nav_packet, "ok")

        self.assertEquals(self.comp.bad_connection, "No connections available.")
        rfinfo_port = self.comp.getPort('rfinfo_out')
        rfsource_port = self.comp.getPort('rfsource_out')
        digitaltuner_port = self.comp.getPort('digitaltuner_out')
        gps_port = self.comp.getPort('gps_out')
        navdata_port = self.comp.getPort('navdata_out')
        rfinfo_port.connectPort(self._rfinfoPort._this(), 'abc')
        rfsource_port.connectPort(self._rfsourcePort._this(), 'abc')
        digitaltuner_port.connectPort(self._digitaltunerPort._this(), 'abc')
        gps_port.connectPort(self._gpsPort._this(), 'abc')
        navdata_port.connectPort(self._navdataPort._this(), 'abc')
        time.sleep(0.5)
        self.comp.start()
        time.sleep(0.5)
        self.comp.stop()
        self.assertEquals(self.comp.get_rfinfo, "ok")
        self.assertEquals(self.comp.set_rfinfo, "ok")
        self.assertEquals(self.comp.get_available_rf, "ok")
        self.assertEquals(self.comp.set_available_rf, "ok")
        self.assertEquals(self.comp.get_current_rf, "ok")
        self.assertEquals(self.comp.set_current_rf, "ok")

        self.assertEquals(self.comp.get_tunertype, "ok")
        self.assertEquals(self.comp.get_tunerdevicecontrol, "ok")
        self.assertEquals(self.comp.get_tunergroupid, "ok")
        self.assertEquals(self.comp.get_tunerrfflowid, "ok")
        self.assertEquals(self.comp.get_tunerstatus, "ok")
        self.assertEquals(self.comp.get_tunercenterfrequency, "ok")
        self.assertEquals(self.comp.set_tunercenterfrequency, "ok")
        self.assertEquals(self.comp.get_tunerbandwidth, "ok")
        self.assertEquals(self.comp.set_tunerbandwidth, "ok")
        self.assertEquals(self.comp.get_tuneragcenable, "ok")
        self.assertEquals(self.comp.set_tuneragcenable, "ok")
        self.assertEquals(self.comp.get_tunergain, "ok")
        self.assertEquals(self.comp.set_tunergain, "ok")
        self.assertEquals(self.comp.get_tunerreferencesource, "ok")
        self.assertEquals(self.comp.set_tunerreferencesource, "ok")
        self.assertEquals(self.comp.get_tunerenable, "ok")
        self.assertEquals(self.comp.set_tunerenable, "ok")
        self.assertEquals(self.comp.get_tuneroutputsamplerate, "ok")
        self.assertEquals(self.comp.set_tuneroutputsamplerate, "ok")

        self.assertEquals(self.comp.get_gpsinfo, "ok")
        self.assertEquals(self.comp.set_gpsinfo, "ok")
        self.assertEquals(self.comp.get_gps_timepos, "ok")
        self.assertEquals(self.comp.set_gps_timepos, "ok")
        self.assertEquals(self.comp.get_nav_packet, "ok")
        self.assertEquals(self.comp.set_nav_packet, "ok")

        self.assertEquals(self.comp.bad_connection, "The requested connection id (invalid_connectionid) does not exist.Connections available: abc")
        rfinfo_port.connectPort(self._rfinfoPort_2._this(), 'def')
        rfsource_port.connectPort(self._rfsourcePort_2._this(), 'def')
        digitaltuner_port.connectPort(self._digitaltunerPort_2._this(), 'def')
        gps_port.connectPort(self._gpsPort_2._this(), 'def')
        navdata_port.connectPort(self._navdataPort_2._this(), 'def')
        time.sleep(0.5)
        self.comp.start()
        time.sleep(0.5)
        self.comp.stop()
        self.assertEquals(self.comp.get_rfinfo, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_rfinfo, "ok")
        self.assertEquals(self.comp.get_available_rf, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_available_rf, "ok")
        self.assertEquals(self.comp.get_current_rf, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_current_rf, "ok")

        self.assertEquals(self.comp.get_tunertype, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.get_tunerdevicecontrol, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.get_tunergroupid, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.get_tunerrfflowid, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.get_tunerstatus, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.get_tunercenterfrequency, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_tunercenterfrequency, "ok")
        self.assertEquals(self.comp.get_tunerbandwidth, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_tunerbandwidth, "ok")
        self.assertEquals(self.comp.get_tuneragcenable, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_tuneragcenable, "ok")
        self.assertEquals(self.comp.get_tunergain, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_tunergain, "ok")
        self.assertEquals(self.comp.get_tunerreferencesource, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_tunerreferencesource, "ok")
        self.assertEquals(self.comp.get_tunerenable, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_tunerenable, "ok")
        self.assertEquals(self.comp.get_tuneroutputsamplerate, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_tuneroutputsamplerate, "ok")

        self.assertEquals(self.comp.get_gpsinfo, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_gpsinfo, "ok")
        self.assertEquals(self.comp.get_gps_timepos, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_gps_timepos, "ok")
        self.assertEquals(self.comp.get_nav_packet, "Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.Connections available: abc, def")
        self.assertEquals(self.comp.set_nav_packet, "ok")

        self.assertEquals(self.comp.bad_connection, "The requested connection id (invalid_connectionid) does not exist.Connections available: abc, def")

if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
