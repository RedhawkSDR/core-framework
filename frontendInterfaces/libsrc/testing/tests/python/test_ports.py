#!/usr/bin/python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest
import copy

import bulkio
from bulkio.bulkioInterfaces import BULKIO

import sys
sys.path.insert(0, '../../../../src/python')
sys.path.insert(0, '../../../../libsrc/python')

from omniORB import any as _any
from ossie.cf import CF

from redhawk.frontendInterfaces import FRONTEND
import tuner_device
from input_ports import *
from output_ports import *
import fe_types
from ossie.resource import PortCallError

class PortsTest(unittest.TestCase):
    class gps_port_sample(gps_delegation):
        def __init__(self):
            self._id = "original"
        def set_source_id(self, in_id):
            self._id = in_id
        def get_source_id(self):
            return self._id;
        def get_gps_info(self, port_name):
            _gpsinfo = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0),[])
            _gpsinfo.source_id = self._id
            return _gpsinfo
        def set_gps_info(self, port_name, gps_info):
            self._id = gps_info.source_id;
        def get_gps_time_pos(self, port_name):
            _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
            _gpstimepos = FRONTEND.GpsTimePos(_positioninfo,BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0))
            return _gpstimepos
        def set_gps_time_pos(self, port_name, gps_time_pos):
            pass

    class nav_port_sample(nav_delegation):
        def __init__(self):
            self._id = "original"
        def set_source_id(self, in_id):
            self._id = in_id
        def get_source_id(self):
            return self._id;
        def get_nav_packet(self, port_name):
            _time = BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0)
            _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
            _cartesianpos=FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
            _velocityinfo=FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
            _accelerationinfo=FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
            _attitudeinfo=FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
            _navpacket=FRONTEND.NavigationPacket(self._id,'',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,_time,[])
            return _navpacket
        def set_nav_packet(self, port_name, nav_info):
            self._id = nav_info.source_id

    class rfinfo_port_sample(rfinfo_delegation):
        def __init__(self):
            self._id = "original"
        def set_source_id(self, in_id):
            self._id = in_id
        def get_source_id(self):
            return self._id;
        def get_rf_flow_id(self, port_name):
            return ""
        def set_rf_flow_id(self, port_name, id):
            pass
        def get_rfinfo_pkt(self, port_name):
            _antennainfo=FRONTEND.AntennaInfo('','','','')
            _freqrange=FRONTEND.FreqRange(0,0,[])
            _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
            _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
            _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
            _rfinfopkt=FRONTEND.RFInfoPkt(self._id,0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
            return _rfinfopkt
        def set_rfinfo_pkt(self, port_name, pkt):
            self._id = pkt.rf_flow_id

    class rfsource_port_sample(rfsource_delegation):
        def __init__(self):
            self._id = "original"
        def set_rf_flow_id(self, in_id):
            self._id = in_id
        def get_rf_flow_id(self):
            return self._id;
        def get_available_rf_inputs(self, port_name):
            return []
        def set_available_rf_inputs(self, port_name, inputs):
            pass
        def get_current_rf_input(self, port_name):
            _antennainfo=FRONTEND.AntennaInfo('','','','')
            _freqrange=FRONTEND.FreqRange(0,0,[])
            _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
            _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
            _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
            _rfinfopkt=FRONTEND.RFInfoPkt(self._id,0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
            return _rfinfopkt
        def set_current_rf_input(self, port_name, _input):
            self._id = _input.rf_flow_id

    class tuner_port_sample(digital_scanning_tuner_delegation):
        def __init__(self):
            self._bw = 0
        def set_bw(self, in_bw):
            self._bw= in_bw
        def get_bw(self):
            return self._bw
        def setTunerBandwidth(self, id, in_bw):
            self._bw = in_bw
        def getTunerBandwidth(self, id):
            return self._bw;

    def testGPSGetter(self):
        input_parent = self.gps_port_sample()
        input_port_1 = InGPSPort("input_1", input_parent)
        output_port = OutGPSPort("output")

        input_parent.set_source_id("newvalue");

        self.assertRaises(PortCallError, output_port._get_gps_info);
        self.assertRaises(PortCallError, output_port._get_gps_info, 'hello')

        output_port.connectPort(input_port_1._this(), "hello")

        gpsinfo = output_port._get_gps_info()
        self.assertEquals(gpsinfo.source_id, "newvalue")
        gpsinfo = output_port._get_gps_info("hello")
        self.assertEquals(gpsinfo.source_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_gps_info, 'foo')

        input_parent_2 = self.gps_port_sample()
        input_parent_2.set_source_id("newvalue_2")
        input_port_2 = InGPSPort("input_2", input_parent_2)

        output_port.connectPort(input_port_2._this(), "foo")
        self.assertRaises(PortCallError, output_port._get_gps_info)
        gpsinfo = output_port._get_gps_info("hello")
        self.assertEquals(gpsinfo.source_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_gps_info, "something")

        output_port.disconnectPort("hello")

    def testGPSSetter(self):
        input_parent = self.gps_port_sample()
        input_parent_2 = self.gps_port_sample()
        input_port_1 = InGPSPort("input_1", input_parent)
        output_port = OutGPSPort("output")

        self.assertEquals(input_parent.get_source_id(),"original")

        gpsinfo = FRONTEND.GPSInfo('','','',1L,1L,1L,1.0,1.0,1.0,1.0,1,1.0,'',BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0),[])
        gpsinfo.source_id = "newvalue"

        output_port._set_gps_info(gpsinfo)
        self.assertEquals(input_parent.get_source_id(), "original")
        self.assertRaises(PortCallError, output_port._set_gps_info, gpsinfo, "hello")

        output_port.connectPort(input_port_1._this(), "hello")

        output_port._set_gps_info(gpsinfo)
        self.assertEquals(input_parent.get_source_id(), "newvalue")

        gpsinfo.source_id = "newvalue_2";
        output_port._set_gps_info(gpsinfo, "hello")
        self.assertEquals(input_parent.get_source_id(), "newvalue_2")

        self.assertRaises(PortCallError, output_port._set_gps_info, gpsinfo, "foo")

        gpsinfo.source_id = "newvalue_3"
        input_port_2 = InGPSPort("input_2", input_parent_2)
        output_port.connectPort(input_port_2._this(), "foo")

        output_port._set_gps_info(gpsinfo);
        self.assertEquals(input_parent.get_source_id(), "newvalue_3")
        self.assertEquals(input_parent_2.get_source_id(), "newvalue_3")

        gpsinfo.source_id = "newvalue_4";
        output_port._set_gps_info(gpsinfo, "hello")
        self.assertEquals(input_parent.get_source_id(), "newvalue_4")
        self.assertEquals(input_parent_2.get_source_id(), "newvalue_3")

        self.assertRaises(PortCallError, output_port._set_gps_info, gpsinfo, "something")

        output_port.disconnectPort("hello")

    def testNavGetter(self):
        input_parent = self.nav_port_sample()
        input_port_1 = InNavDataPort("input_1", input_parent)
        output_port = OutNavDataPort("output")

        input_parent.set_source_id("newvalue");

        self.assertRaises(PortCallError, output_port._get_nav_packet);
        self.assertRaises(PortCallError, output_port._get_nav_packet, 'hello')

        output_port.connectPort(input_port_1._this(), "hello")

        navinfo = output_port._get_nav_packet()
        self.assertEquals(navinfo.source_id, "newvalue")
        navinfo = output_port._get_nav_packet("hello")
        self.assertEquals(navinfo.source_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_nav_packet, 'foo')

        input_parent_2 = self.nav_port_sample()
        input_parent_2.set_source_id("newvalue_2")
        input_port_2 = InNavDataPort("input_2", input_parent_2)

        output_port.connectPort(input_port_2._this(), "foo")
        self.assertRaises(PortCallError, output_port._get_nav_packet)
        navinfo = output_port._get_nav_packet("hello")
        self.assertEquals(navinfo.source_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_nav_packet, "something")

        output_port.disconnectPort("hello")

    def testNavSetter(self):
        input_parent = self.nav_port_sample()
        input_parent_2 = self.nav_port_sample()
        input_port_1 = InNavDataPort("input_1", input_parent)
        output_port = OutNavDataPort("output")

        self.assertEquals(input_parent.get_source_id(),"original")

        _time = BULKIO.PrecisionUTCTime(1,1,1.0,1.0,1.0)
        _positioninfo = FRONTEND.PositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _cartesianpos=FRONTEND.CartesianPositionInfo(False,'DATUM_WGS84',0.0,0.0,0.0)
        _velocityinfo=FRONTEND.VelocityInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _accelerationinfo=FRONTEND.AccelerationInfo(False,'DATUM_WGS84','',0.0,0.0,0.0)
        _attitudeinfo=FRONTEND.AttitudeInfo(False,0.0,0.0,0.0)
        navpacket=FRONTEND.NavigationPacket('','',_positioninfo,_cartesianpos,_velocityinfo,_accelerationinfo,_attitudeinfo,_time,[])
        navpacket.source_id = "newvalue"

        output_port._set_nav_packet(navpacket)
        self.assertEquals(input_parent.get_source_id(), "original")
        self.assertRaises(PortCallError, output_port._set_nav_packet, navpacket, "hello")

        output_port.connectPort(input_port_1._this(), "hello")

        output_port._set_nav_packet(navpacket)
        self.assertEquals(input_parent.get_source_id(), "newvalue")

        navpacket.source_id = "newvalue_2";
        output_port._set_nav_packet(navpacket, "hello")
        self.assertEquals(input_parent.get_source_id(), "newvalue_2")

        self.assertRaises(PortCallError, output_port._set_nav_packet, navpacket, "foo")

        navpacket.source_id = "newvalue_3"
        input_port_2 = InNavDataPort("input_2", input_parent_2)
        output_port.connectPort(input_port_2._this(), "foo")

        output_port._set_nav_packet(navpacket);
        self.assertEquals(input_parent.get_source_id(), "newvalue_3")
        self.assertEquals(input_parent_2.get_source_id(), "newvalue_3")

        navpacket.source_id = "newvalue_4";
        output_port._set_nav_packet(navpacket, "hello")
        self.assertEquals(input_parent.get_source_id(), "newvalue_4")
        self.assertEquals(input_parent_2.get_source_id(), "newvalue_3")

        self.assertRaises(PortCallError, output_port._set_nav_packet, navpacket, "something")

        output_port.disconnectPort("hello")

    def testRFInfoGetter(self):
        input_parent = self.rfinfo_port_sample()
        input_port_1 = InRFInfoPort("input_1", input_parent)
        output_port = OutRFInfoPort("output")

        input_parent.set_source_id("newvalue");

        self.assertRaises(PortCallError, output_port._get_rfinfo_pkt);
        self.assertRaises(PortCallError, output_port._get_rfinfo_pkt, 'hello')

        output_port.connectPort(input_port_1._this(), "hello")

        rfinfo = output_port._get_rfinfo_pkt()
        self.assertEquals(rfinfo.rf_flow_id, "newvalue")
        rfinfo = output_port._get_rfinfo_pkt("hello")
        self.assertEquals(rfinfo.rf_flow_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_rfinfo_pkt, 'foo')

        input_parent_2 = self.rfinfo_port_sample()
        input_parent_2.set_source_id("newvalue_2")
        input_port_2 = InRFInfoPort("input_2", input_parent_2)

        output_port.connectPort(input_port_2._this(), "foo")
        self.assertRaises(PortCallError, output_port._get_rfinfo_pkt)
        rfinfo = output_port._get_rfinfo_pkt("hello")
        self.assertEquals(rfinfo.rf_flow_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_rfinfo_pkt, "something")

        output_port.disconnectPort("hello")

    def testRFInfoSetter(self):
        input_parent = self.rfinfo_port_sample()
        input_parent_2 = self.rfinfo_port_sample()
        input_port_1 = InRFInfoPort("input_1", input_parent)
        output_port = OutRFInfoPort("output")

        self.assertEquals(input_parent.get_source_id(),"original")

        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        rfinfo=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        rfinfo.rf_flow_id = "newvalue"

        output_port._set_rfinfo_pkt(rfinfo)
        self.assertEquals(input_parent.get_source_id(), "original")
        self.assertRaises(PortCallError, output_port._set_rfinfo_pkt, rfinfo, "hello")

        output_port.connectPort(input_port_1._this(), "hello")

        output_port._set_rfinfo_pkt(rfinfo)
        self.assertEquals(input_parent.get_source_id(), "newvalue")

        rfinfo.rf_flow_id = "newvalue_2";
        output_port._set_rfinfo_pkt(rfinfo, "hello")
        self.assertEquals(input_parent.get_source_id(), "newvalue_2")

        self.assertRaises(PortCallError, output_port._set_rfinfo_pkt, rfinfo, "foo")

        rfinfo.rf_flow_id = "newvalue_3"
        input_port_2 = InRFInfoPort("input_2", input_parent_2)
        output_port.connectPort(input_port_2._this(), "foo")

        output_port._set_rfinfo_pkt(rfinfo);
        self.assertEquals(input_parent.get_source_id(), "newvalue_3")
        self.assertEquals(input_parent_2.get_source_id(), "newvalue_3")

        rfinfo.rf_flow_id = "newvalue_4";
        output_port._set_rfinfo_pkt(rfinfo, "hello")
        self.assertEquals(input_parent.get_source_id(), "newvalue_4")
        self.assertEquals(input_parent_2.get_source_id(), "newvalue_3")

        self.assertRaises(PortCallError, output_port._set_rfinfo_pkt, rfinfo, "something")

        output_port.disconnectPort("hello")

    def testRFSourceGetter(self):
        input_parent = self.rfsource_port_sample()
        input_port_1 = InRFSourcePort("input_1", input_parent)
        output_port = OutRFSourcePort("output")

        input_parent.set_rf_flow_id("newvalue");

        self.assertRaises(PortCallError, output_port._get_current_rf_input);
        self.assertRaises(PortCallError, output_port._get_current_rf_input, 'hello')

        output_port.connectPort(input_port_1._this(), "hello")

        rfsource = output_port._get_current_rf_input()
        self.assertEquals(rfsource.rf_flow_id, "newvalue")
        rfsource = output_port._get_current_rf_input("hello")
        self.assertEquals(rfsource.rf_flow_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_current_rf_input, 'foo')

        input_parent_2 = self.rfsource_port_sample()
        input_parent_2.set_rf_flow_id("newvalue_2")
        input_port_2 = InRFSourcePort("input_2", input_parent_2)

        output_port.connectPort(input_port_2._this(), "foo")
        self.assertRaises(PortCallError, output_port._get_current_rf_input)
        rfsource = output_port._get_current_rf_input("hello")
        self.assertEquals(rfsource.rf_flow_id, "newvalue")
        self.assertRaises(PortCallError, output_port._get_current_rf_input, "something")

        output_port.disconnectPort("hello")

    def testRFSourceSetter(self):
        input_parent = self.rfsource_port_sample()
        input_parent_2 = self.rfsource_port_sample()
        input_port_1 = InRFSourcePort("input_1", input_parent)
        output_port = OutRFSourcePort("output")

        self.assertEquals(input_parent.get_rf_flow_id(),"original")

        _antennainfo=FRONTEND.AntennaInfo('','','','')
        _freqrange=FRONTEND.FreqRange(0,0,[])
        _feedinfo=FRONTEND.FeedInfo('','',_freqrange)
        _sensorinfo=FRONTEND.SensorInfo('','','',_antennainfo,_feedinfo)
        _rfcapabilities=FRONTEND.RFCapabilities(_freqrange,_freqrange)
        rfsource=FRONTEND.RFInfoPkt('',0.0,0.0,0.0,False,_sensorinfo,[],_rfcapabilities,[])
        rfsource.rf_flow_id = "newvalue"

        output_port._set_current_rf_input(rfsource)
        self.assertEquals(input_parent.get_rf_flow_id(), "original")
        self.assertRaises(PortCallError, output_port._set_current_rf_input, rfsource, "hello")

        output_port.connectPort(input_port_1._this(), "hello")

        output_port._set_current_rf_input(rfsource)
        self.assertEquals(input_parent.get_rf_flow_id(), "newvalue")

        rfsource.rf_flow_id = "newvalue_2";
        output_port._set_current_rf_input(rfsource, "hello")
        self.assertEquals(input_parent.get_rf_flow_id(), "newvalue_2")

        self.assertRaises(PortCallError, output_port._set_current_rf_input, rfsource, "foo")

        rfsource.rf_flow_id = "newvalue_3"
        input_port_2 = InRFSourcePort("input_2", input_parent_2)
        output_port.connectPort(input_port_2._this(), "foo")

        output_port._set_current_rf_input(rfsource);
        self.assertEquals(input_parent.get_rf_flow_id(), "newvalue_3")
        self.assertEquals(input_parent_2.get_rf_flow_id(), "newvalue_3")

        rfsource.rf_flow_id = "newvalue_4";
        output_port._set_current_rf_input(rfsource, "hello")
        self.assertEquals(input_parent.get_rf_flow_id(), "newvalue_4")
        self.assertEquals(input_parent_2.get_rf_flow_id(), "newvalue_3")

        self.assertRaises(PortCallError, output_port._set_current_rf_input, rfsource, "something")

        output_port.disconnectPort("hello")

    def testTunerGetter(self):
        input_parent = self.tuner_port_sample()
        input_port_1 = InDigitalScanningTunerPort("input_1", input_parent)
        output_port = OutDigitalTunerPort("output")

        input_parent.set_bw(1);

        self.assertRaises(PortCallError, output_port.getTunerBandwidth, 'first_tuner');
        self.assertRaises(PortCallError, output_port.getTunerBandwidth, 'first_tuner', 'hello')

        output_port.connectPort(input_port_1._this(), "hello")

        bw = output_port.getTunerBandwidth('first_tuner')
        self.assertEquals(bw, 1)
        bw = output_port.getTunerBandwidth('first_tuner', "hello")
        self.assertEquals(bw, 1)
        self.assertRaises(PortCallError, output_port.getTunerBandwidth, 'first_tuner', 'foo')

        input_parent_2 = self.tuner_port_sample()
        input_parent_2.set_bw(2)
        input_port_2 = InDigitalScanningTunerPort("input_2", input_parent_2)

        output_port.connectPort(input_port_2._this(), "foo")
        self.assertRaises(PortCallError, output_port.getTunerBandwidth, 'first_tuner')
        bw = output_port.getTunerBandwidth('first_tuner', "hello")
        self.assertEquals(bw, 1)
        self.assertRaises(PortCallError, output_port.getTunerBandwidth, 'first_tuner', "something")

        output_port.disconnectPort("hello")

    def testTunerSetter(self):
        input_parent = self.tuner_port_sample()
        input_parent_2 = self.tuner_port_sample()
        input_port_1 = InDigitalScanningTunerPort("input_1", input_parent)
        output_port = OutDigitalTunerPort("output")

        self.assertEquals(input_parent.get_bw(),0)

        bw = 1
        output_port.setTunerBandwidth("first_tuner", bw)
        self.assertEquals(input_parent.get_bw(), 0)
        self.assertRaises(PortCallError, output_port.setTunerBandwidth, "first_tuner", bw, "hello")

        output_port.connectPort(input_port_1._this(), "hello")

        output_port.setTunerBandwidth("first_tuner", bw)
        self.assertEquals(input_parent.get_bw(), bw)

        bw = 2
        output_port.setTunerBandwidth("first_tuner", bw, "hello")
        self.assertEquals(input_parent.get_bw(), bw)

        self.assertRaises(PortCallError, output_port.setTunerBandwidth, "first_tuner", bw, "foo")

        bw = 3
        input_port_2 = InDigitalScanningTunerPort("input_2", input_parent_2)
        output_port.connectPort(input_port_2._this(), "foo")

        output_port.setTunerBandwidth("first_tuner", bw);
        self.assertEquals(input_parent.get_bw(), bw)
        self.assertEquals(input_parent_2.get_bw(), bw)

        bw = 4
        output_port.setTunerBandwidth("first_tuner", bw, "hello")
        self.assertEquals(input_parent.get_bw(), bw)
        self.assertEquals(input_parent_2.get_bw(), 3)

        self.assertRaises(PortCallError, output_port.setTunerBandwidth, "first_tuner", bw, "something")

        output_port.disconnectPort("hello")

if __name__ == '__main__':
    import runtests
    runtests.main()
