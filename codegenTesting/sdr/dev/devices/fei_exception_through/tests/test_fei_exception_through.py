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

import ossie.utils.testing
from ossie.utils import sb
from bulkio.bulkioInterfaces import BULKIO
from redhawk.frontendInterfaces import FRONTEND
import os, time
from omniORB import CORBA
import ossie.utils.sandbox
import logging
import frontend

    
class varLog(logging.Handler):
    def __init__(self, parent):
        logging.Handler.__init__(self)
        self.parent = parent
        self.parent.got_logmsg = False
            
    def emit(self, record):
        self.parent.got_logmsg = True

class DeviceTests(ossie.utils.testing.RHTestCase):
    # Path to the SPD file, relative to this file. This must be set in order to
    # launch the device.
    SPD_FILE = '../fei_exception_through.spd.xml'
    got_logmsg = False

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
        self.var=varLog(self)
        ossie.utils.sandbox.local.log.addHandler(self.var)
        self.comp = sb.launch(self.spd_file, impl=self.impl)
    
    def tearDown(self):
        # Clean up all sandbox artifacts created during test
        ossie.utils.sandbox.local.log.removeHandler(self.var)
        sb.release()

    def testAllocation(self):
        frontend_alloc = frontend.createTunerAllocation(returnDict=False)
        retval = self.comp.allocateCapacity([frontend_alloc])
        self.assertEquals(retval, True)

    def testBasicBehavior(self):
        self.assertEquals(self.got_logmsg, False)
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        exc_src = sb.launch('./build/fei_exception_through/tests/fei_exc_src/fei_exc_src.spd.xml')
        self.comp.connect(exc_src,usesPortName='DigitalTuner_out')
        self.comp.connect(exc_src,usesPortName='RFInfo_out')
        self.comp.connect(exc_src,usesPortName='GPS_out')
        self.comp.connect(exc_src,usesPortName='NavData_out')
        self.comp.connect(exc_src,usesPortName='RFSource_out')
        for port in self.comp.ports:
            if port.name == 'DigitalTuner_in':
                DigitalTuner_in = port
            if port.name == 'RFInfo_in':
                RFInfo_in = port
            if port.name == 'GPS_in':
                GPS_in = port
            if port.name == 'NavData_in':
                NavData_in = port
            if port.name == 'RFSource_in':
                RFSource_in = port
                
        _time = BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0)
        _gpsinfo = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',_time,[])
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _gpstimepos = FRONTEND.GpsTimePos(_positioninfo,_time)
        _cartesianpos=FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _velocityinfo=FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _accelerationinfo=FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _attitudeinfo=FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
        _navpacket=FRONTEND.NavigationPacket('','',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,_time,[])
        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        _rfinfopkt=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        _strat = FRONTEND.ScanningTuner.ScanStrategy(FRONTEND.ScanningTuner.DISCRETE_SCAN, FRONTEND.ScanningTuner.ScanModeDefinition(discrete_freq_list=[]), FRONTEND.ScanningTuner.TIME_BASED, 0.1)

        DigitalTuner_in.ref.getScanStatus('hello')
        DigitalTuner_in.ref.setScanStartTime('hello', _time)
        DigitalTuner_in.ref.setScanStrategy('hello', _strat)
        DigitalTuner_in.ref.getTunerType('hello')
        DigitalTuner_in.ref.getTunerDeviceControl('hello')
        DigitalTuner_in.ref.getTunerGroupId('hello')
        DigitalTuner_in.ref.getTunerRfFlowId('hello')
        DigitalTuner_in.ref.setTunerCenterFrequency('hello', 1.0)
        DigitalTuner_in.ref.getTunerCenterFrequency('hello')
        DigitalTuner_in.ref.setTunerBandwidth('hello', 1.0)
        DigitalTuner_in.ref.getTunerBandwidth('hello')
        DigitalTuner_in.ref.setTunerAgcEnable('hello', True)
        DigitalTuner_in.ref.getTunerAgcEnable('hello')
        DigitalTuner_in.ref.setTunerGain('hello', 1.0)
        DigitalTuner_in.ref.getTunerGain('hello')
        DigitalTuner_in.ref.setTunerReferenceSource('hello', 1L)
        DigitalTuner_in.ref.getTunerReferenceSource('hello')
        DigitalTuner_in.ref.setTunerEnable('hello', True)
        DigitalTuner_in.ref.getTunerEnable('hello')
        DigitalTuner_in.ref.setTunerOutputSampleRate('hello', 1.0)
        DigitalTuner_in.ref.getTunerOutputSampleRate('hello')
        GPS_in.ref._get_gps_info()
        GPS_in.ref._set_gps_info(_gpsinfo)
        GPS_in.ref._get_gps_time_pos()
        GPS_in.ref._set_gps_time_pos(_gpstimepos)
        NavData_in.ref._get_nav_packet()
        NavData_in.ref._set_nav_packet(_navpacket)
        RFInfo_in.ref._get_rf_flow_id()
        RFInfo_in.ref._set_rf_flow_id('rf_flow')
        RFInfo_in.ref._get_rfinfo_pkt()
        RFInfo_in.ref._set_rfinfo_pkt(_rfinfopkt)
        
        os.killpg(exc_src._pid, 9)
        
        while True:
            try:
                os.kill(exc_src._pid, 0)
                time.sleep(0.1)
            except:
                break
        
        exception = (CORBA.COMM_FAILURE, CORBA.TRANSIENT)

        self.assertRaises(exception, DigitalTuner_in.ref.getScanStatus, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setScanStartTime, 'hello', _time)
        self.assertRaises(exception, DigitalTuner_in.ref.setScanStrategy, 'hello', _strat)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerType, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerDeviceControl, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerGroupId, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerRfFlowId, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setTunerCenterFrequency, 'hello', 1.0)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerCenterFrequency, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setTunerBandwidth, 'hello', 1.0)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerBandwidth, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setTunerAgcEnable, 'hello', True)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerAgcEnable, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setTunerGain, 'hello', 1.0)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerGain, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setTunerReferenceSource, 'hello', 1L)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerReferenceSource, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setTunerEnable, 'hello', True)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerEnable, 'hello')
        self.assertRaises(exception, DigitalTuner_in.ref.setTunerOutputSampleRate, 'hello', 1.0)
        self.assertRaises(exception, DigitalTuner_in.ref.getTunerOutputSampleRate, 'hello')
        self.assertRaises(exception, GPS_in.ref._get_gps_info)
        self.assertRaises(exception, GPS_in.ref._set_gps_info, _gpsinfo)
        self.assertRaises(exception, GPS_in.ref._get_gps_time_pos)
        self.assertRaises(exception, GPS_in.ref._set_gps_time_pos, _gpstimepos)
        self.assertRaises(exception, NavData_in.ref._get_nav_packet)
        self.assertRaises(exception, NavData_in.ref._set_nav_packet, _navpacket)
        self.assertRaises(exception, RFInfo_in.ref._get_rf_flow_id)
        self.assertRaises(exception, RFInfo_in.ref._set_rf_flow_id, 'rf_flow')
        self.assertRaises(exception, RFInfo_in.ref._get_rfinfo_pkt)
        self.assertRaises(exception, RFInfo_in.ref._set_rfinfo_pkt, _rfinfopkt)


    def test_RFSource_Out(self):
        self.assertEquals(self.got_logmsg, False)
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        exc_src = sb.launch('./build/fei_exception_through/tests/fei_exc_src/fei_exc_src.spd.xml')
        self.comp.connect(exc_src,usesPortName='RFSource_out')
        self.comp.start();
        self.comp.stop();
        sb.release()


if __name__ == "__main__":
    ossie.utils.testing.main() # By default tests all implementations
