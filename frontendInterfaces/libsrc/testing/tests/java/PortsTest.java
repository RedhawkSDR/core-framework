/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

import utils.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import frontend.*;
import FRONTEND.*;
/*import FRONTEND.GPSInfo;
import FRONTEND.GpsTimePos;
import FRONTEND.FrontendException;
import FRONTEND.NotSupportedException;
import FRONTEND.BadParameterException;*/
import java.util.ArrayList;
import org.omg.CORBA.ORB;
import org.ossie.redhawk.PortCallError;
import CF.PortPackage.InvalidPort;
import CF.PortPackage.OccupiedPort;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.omg.PortableServer.POAPackage.ObjectNotActive;

@RunWith(JUnit4.class)
public class PortsTest
{
    public class gps_port_sample implements frontend.GPSDelegate {

        public String id;

        public gps_port_sample() {
            id = "original";
        };
        public void set_source_id(String in_id) {
            id = in_id;
        };
        public String get_source_id() {
            return id;
        };
        public GPSInfo get_gps_info(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            GPSInfo _gps = new GPSInfo();
            _gps.additional_info = new CF.DataType[0];
            _gps.mode = new String("");
            _gps.rf_flow_id = new String("");
            _gps.source_id = new String(id);
            _gps.status_message = new String("");
            _gps.timestamp = new BULKIO.PrecisionUTCTime();
            return _gps;
        };
        public void set_gps_info(String port_name, GPSInfo data) throws FrontendException, BadParameterException, NotSupportedException {
            id = data.source_id;
        };
        public GpsTimePos get_gps_time_pos(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            return new GpsTimePos();
        };
        public void set_gps_time_pos(String port_name, GpsTimePos data) throws FrontendException, BadParameterException, NotSupportedException {
        };
    }

    public class nav_port_sample implements frontend.NavDataDelegate {

        public String id;

        public nav_port_sample() {
            id = "original";
        };
        public void set_source_id(String in_id) {
            id = in_id;
        };
        public String get_source_id() {
            return id;
        };
        public NavigationPacket get_nav_packet(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            NavigationPacket _nav = new NavigationPacket();
            _nav.acceleration = new AccelerationInfo();
            _nav.acceleration.coordinate_system = new String("");
            _nav.acceleration.datum = new String("");
            _nav.additional_info = new CF.DataType[0];
            _nav.attitude = new AttitudeInfo();
            _nav.cposition = new CartesianPositionInfo();
            _nav.cposition.datum = new String("");
            _nav.position = new PositionInfo();
            _nav.position.datum = new String("");
            _nav.rf_flow_id = new String("");
            _nav.source_id = new String(id);
            _nav.timestamp = new BULKIO.PrecisionUTCTime();
            _nav.velocity = new VelocityInfo();
            _nav.velocity.coordinate_system = new String("");
            _nav.velocity.datum = new String("");
            return _nav;
        };
        public void set_nav_packet(String port_name, NavigationPacket data) throws FrontendException, BadParameterException, NotSupportedException {
            id = data.source_id;
        };
    }

    public class rfinfo_port_sample implements frontend.RFInfoDelegate {

        public String id;

        public rfinfo_port_sample() {
            id = "original";
        };
        public void set_source_id(String in_id) {
            id = in_id;
        };
        public String get_source_id() {
            return id;
        };
        public String get_rf_flow_id(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            return new String("");
        };
        public void set_rf_flow_id(String port_name, String data) throws FrontendException, BadParameterException, NotSupportedException {
        };
        public RFInfoPkt get_rfinfo_pkt(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            RFInfoPkt foo = new RFInfoPkt();
            foo.rf_flow_id = new String(id);
            foo.sensor = new SensorInfo();
            foo.sensor.collector = new String("");
            foo.sensor.antenna = new AntennaInfo();
            foo.sensor.antenna.description = new String("");
            foo.sensor.antenna.name = new String("");
            foo.sensor.antenna.size = new String("");
            foo.sensor.antenna.type = new String("");
            foo.sensor.feed = new FeedInfo();
            foo.sensor.feed.name = new String("");
            foo.sensor.feed.polarization = new String("");
            foo.sensor.feed.freq_range = new FreqRange();
            foo.sensor.feed.freq_range.values = new double[0];
            foo.sensor.mission = new String("");
            foo.sensor.rx = new String("");
            foo.ext_path_delays = new PathDelay[0];
            foo.capabilities = new RFCapabilities();
            foo.capabilities.freq_range = new FreqRange();
            foo.capabilities.freq_range.values = new double[0];
            foo.capabilities.bw_range = new FreqRange();
            foo.capabilities.bw_range.values = new double[0];
            foo.additional_info = new CF.DataType[0];
            return foo;
        };
        public void set_rfinfo_pkt(String port_name, RFInfoPkt data) throws FrontendException, BadParameterException, NotSupportedException {
            id = data.rf_flow_id;
        };
    }
    public class rfsource_port_sample implements frontend.RFSourceDelegate {

        public String id;

        public rfsource_port_sample() {
            id = "original";
        };
        public void set_source_id(String in_id) {
            id = in_id;
        };
        public String get_source_id() {
            return id;
        };
        public RFInfoPkt[] get_available_rf_inputs(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            return new RFInfoPkt[0];
        };
        public void set_available_rf_inputs(String port_name, RFInfoPkt[] data) throws FrontendException, BadParameterException, NotSupportedException {
        };
        public RFInfoPkt get_current_rf_input(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            RFInfoPkt foo = new RFInfoPkt();
            foo.rf_flow_id = new String(id);
            foo.sensor = new SensorInfo();
            foo.sensor.collector = new String("");
            foo.sensor.antenna = new AntennaInfo();
            foo.sensor.antenna.description = new String("");
            foo.sensor.antenna.name = new String("");
            foo.sensor.antenna.size = new String("");
            foo.sensor.antenna.type = new String("");
            foo.sensor.feed = new FeedInfo();
            foo.sensor.feed.name = new String("");
            foo.sensor.feed.polarization = new String("");
            foo.sensor.feed.freq_range = new FreqRange();
            foo.sensor.feed.freq_range.values = new double[0];
            foo.sensor.mission = new String("");
            foo.sensor.rx = new String("");
            foo.ext_path_delays = new PathDelay[0];
            foo.capabilities = new RFCapabilities();
            foo.capabilities.freq_range = new FreqRange();
            foo.capabilities.freq_range.values = new double[0];
            foo.capabilities.bw_range = new FreqRange();
            foo.capabilities.bw_range.values = new double[0];
            foo.additional_info = new CF.DataType[0];
            return foo;
        };
        public void set_current_rf_input(String port_name, RFInfoPkt data) throws FrontendException, BadParameterException, NotSupportedException {
            id = data.rf_flow_id;
        };
    }
    public class digitaltuner_port_sample implements frontend.DigitalTunerDelegate {

        public double bw;

        public digitaltuner_port_sample() {
            bw = 0;
        };
        public void set_bw(double in_bw) {
            bw = in_bw;
        };
        public double get_bw() {
            return bw;
        };
        public String getTunerType(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return "";
        };

        public boolean getTunerDeviceControl(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return false;
        };

        public String getTunerGroupId(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return "";
        };

        public String getTunerRfFlowId(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return "";
        };

        public CF.DataType[] getTunerStatus(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return null;
        };

        public void setTunerCenterFrequency(String id, double freq) throws FrontendException, BadParameterException, NotSupportedException {};

        public double getTunerCenterFrequency(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return 0.0;
        }

        public void setTunerBandwidth(String id, double _bw) throws FrontendException, BadParameterException, NotSupportedException {
            bw = _bw;
        };

        public double getTunerBandwidth(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return bw;
        }

        public void setTunerAgcEnable(String id, boolean enable) throws FrontendException, BadParameterException, NotSupportedException {};

        public boolean getTunerAgcEnable(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return true;
        }

        public void setTunerGain(String id, float gain) throws FrontendException, BadParameterException, NotSupportedException {};

        public float getTunerGain(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return (float)0.0;
        }

        public void setTunerReferenceSource(String id, int source) throws FrontendException, BadParameterException, NotSupportedException {};

        public int getTunerReferenceSource(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return 0;
        }

        public void setTunerEnable(String id, boolean enable) throws FrontendException, BadParameterException, NotSupportedException {};

        public boolean getTunerEnable(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return true;
        }

        public void setTunerOutputSampleRate(String id, double sr) throws FrontendException, BadParameterException, NotSupportedException {};

        public double getTunerOutputSampleRate(String id) throws FrontendException, BadParameterException, NotSupportedException {
            return 0.0;
        }
    }

    @Test
    public void testGPSGetter() {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        gps_port_sample input_parent = new gps_port_sample();
        InGPSPort input_port_1 = new InGPSPort("input_1", input_parent);
        OutGPSPort output_port = new OutGPSPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        input_parent.set_source_id("newvalue");

        Assert.assertThrows(PortCallError.class, () -> output_port.gps_info());
        Assert.assertThrows(PortCallError.class, () -> output_port._get_gps_info("hello"));

        try {
            output_port.connectPort(input_port_1._this(), "hello");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };

        GPSInfo gpsinfo = null;
        try {
            gpsinfo = output_port.gps_info();
            Assert.assertEquals(gpsinfo.source_id, "newvalue");
            gpsinfo = output_port._get_gps_info("hello");
            Assert.assertEquals(gpsinfo.source_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_gps_info("foo"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };

        gps_port_sample input_parent_2 = new gps_port_sample();
        input_parent_2.set_source_id("newvalue_2");
        InGPSPort input_port_2 = new InGPSPort("input_2", input_parent);
        try {
            byte[] oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        try {
            output_port.connectPort(input_port_2._this(), "foo");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };
        Assert.assertThrows(PortCallError.class, () -> output_port.gps_info());
        try {
            gpsinfo = output_port._get_gps_info("hello");
            Assert.assertEquals(gpsinfo.source_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_gps_info("something"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };
        output_port.disconnectPort("hello");
    }

    @Test
    public void testGPSSetter()
    {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        gps_port_sample input_parent = new gps_port_sample();
        gps_port_sample input_parent_2 = new gps_port_sample();
        InGPSPort input_port_1 = new InGPSPort("input_1", input_parent);
        InGPSPort input_port_2 = new InGPSPort("input_2", input_parent_2);
        OutGPSPort output_port = new OutGPSPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
            oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        Assert.assertEquals(input_parent.get_source_id(), "original");

        GPSInfo gpsinfo = new GPSInfo();
        gpsinfo.additional_info = new CF.DataType[0];
        gpsinfo.mode = new String("");
        gpsinfo.rf_flow_id = new String("");
        gpsinfo.source_id = new String("newvalue");
        gpsinfo.status_message = new String("");
        gpsinfo.timestamp = new BULKIO.PrecisionUTCTime();

        try {
            output_port.gps_info(gpsinfo);
            Assert.assertEquals(input_parent.get_source_id(), "original");
            Assert.assertThrows(PortCallError.class, () -> output_port.gps_info(gpsinfo, "hello"));

            output_port.connectPort(input_port_1._this(), "hello");

            output_port.gps_info(gpsinfo);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue");

            gpsinfo.source_id = new String("newvalue_2");
            output_port.gps_info(gpsinfo, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_2");

            Assert.assertThrows(PortCallError.class, () -> output_port.gps_info(gpsinfo, "foo"));

            gpsinfo.source_id = new String("newvalue_3");
            output_port.connectPort(input_port_2._this(), "foo");

            output_port.gps_info(gpsinfo);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_3");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
            gpsinfo.source_id = new String("newvalue_4");
            output_port.gps_info(gpsinfo, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_4");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
        } catch (InvalidPort | OccupiedPort | PortCallError e) {
            Assert.assertTrue(false);
        };
    }

    @Test
    public void testNavGetter() {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        nav_port_sample input_parent = new nav_port_sample();
        InNavDataPort input_port_1 = new InNavDataPort("input_1", input_parent);
        OutNavDataPort output_port = new OutNavDataPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        input_parent.set_source_id("newvalue");

        Assert.assertThrows(PortCallError.class, () -> output_port.nav_packet());
        Assert.assertThrows(PortCallError.class, () -> output_port._get_nav_packet("hello"));

        try {
            output_port.connectPort(input_port_1._this(), "hello");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };

        NavigationPacket navdata = null;
        try {
            navdata = output_port.nav_packet();
            Assert.assertEquals(navdata.source_id, "newvalue");
            navdata = output_port._get_nav_packet("hello");
            Assert.assertEquals(navdata.source_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_nav_packet("foo"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };

        nav_port_sample input_parent_2 = new nav_port_sample();
        input_parent_2.set_source_id("newvalue_2");
        InNavDataPort input_port_2 = new InNavDataPort("input_2", input_parent);
        try {
            byte[] oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        try {
            output_port.connectPort(input_port_2._this(), "foo");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };
        Assert.assertThrows(PortCallError.class, () -> output_port.nav_packet());
        try {
            navdata = output_port._get_nav_packet("hello");
            Assert.assertEquals(navdata.source_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_nav_packet("something"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };
        output_port.disconnectPort("hello");
    }

    @Test
    public void testNavSetter()
    {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        nav_port_sample input_parent = new nav_port_sample();
        nav_port_sample input_parent_2 = new nav_port_sample();
        InNavDataPort input_port_1 = new InNavDataPort("input_1", input_parent);
        InNavDataPort input_port_2 = new InNavDataPort("input_2", input_parent_2);
        OutNavDataPort output_port = new OutNavDataPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
            oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        Assert.assertEquals(input_parent.get_source_id(), "original");

        NavigationPacket navdata = new NavigationPacket();
        navdata.acceleration = new AccelerationInfo();
        navdata.acceleration.coordinate_system = new String("");
        navdata.acceleration.datum = new String("");
        navdata.additional_info = new CF.DataType[0];
        navdata.attitude = new AttitudeInfo();
        navdata.cposition = new CartesianPositionInfo();
        navdata.cposition.datum = new String("");
        navdata.position = new PositionInfo();
        navdata.position.datum = new String("");
        navdata.rf_flow_id = new String("");
        navdata.source_id = new String("newvalue");
        navdata.timestamp = new BULKIO.PrecisionUTCTime();
        navdata.velocity = new VelocityInfo();
        navdata.velocity.coordinate_system = new String("");
        navdata.velocity.datum = new String("");

        try {
            output_port.nav_packet(navdata);
            Assert.assertEquals(input_parent.get_source_id(), "original");
            Assert.assertThrows(PortCallError.class, () -> output_port.nav_packet(navdata, "hello"));

            output_port.connectPort(input_port_1._this(), "hello");

            output_port.nav_packet(navdata);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue");

            navdata.source_id = new String("newvalue_2");
            output_port.nav_packet(navdata, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_2");

            Assert.assertThrows(PortCallError.class, () -> output_port.nav_packet(navdata, "foo"));

            navdata.source_id = new String("newvalue_3");
            output_port.connectPort(input_port_2._this(), "foo");

            output_port.nav_packet(navdata);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_3");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
            navdata.source_id = new String("newvalue_4");
            output_port.nav_packet(navdata, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_4");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
        } catch (InvalidPort | OccupiedPort | PortCallError e) {
            Assert.assertTrue(false);
        };
    }

    @Test
    public void testRFInfoGetter() {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        rfinfo_port_sample input_parent = new rfinfo_port_sample();
        InRFInfoPort input_port_1 = new InRFInfoPort("input_1", input_parent);
        OutRFInfoPort output_port = new OutRFInfoPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        input_parent.set_source_id("newvalue");

        Assert.assertThrows(PortCallError.class, () -> output_port.rfinfo_pkt());
        Assert.assertThrows(PortCallError.class, () -> output_port._get_rfinfo_pkt("hello"));

        try {
            output_port.connectPort(input_port_1._this(), "hello");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };

        RFInfoPkt rfinfo = null;
        try {
            rfinfo = output_port.rfinfo_pkt();
            Assert.assertEquals(rfinfo.rf_flow_id, "newvalue");
            rfinfo = output_port._get_rfinfo_pkt("hello");
            Assert.assertEquals(rfinfo.rf_flow_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_rfinfo_pkt("foo"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };

        rfinfo_port_sample input_parent_2 = new rfinfo_port_sample();
        input_parent_2.set_source_id("newvalue_2");
        InRFInfoPort input_port_2 = new InRFInfoPort("input_2", input_parent);
        try {
            byte[] oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        try {
            output_port.connectPort(input_port_2._this(), "foo");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };
        Assert.assertThrows(PortCallError.class, () -> output_port.rfinfo_pkt());
        try {
            rfinfo = output_port._get_rfinfo_pkt("hello");
            Assert.assertEquals(rfinfo.rf_flow_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_rfinfo_pkt("something"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };
        output_port.disconnectPort("hello");
    }

    @Test
    public void testRFInfoSetter()
    {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        rfinfo_port_sample input_parent = new rfinfo_port_sample();
        rfinfo_port_sample input_parent_2 = new rfinfo_port_sample();
        InRFInfoPort input_port_1 = new InRFInfoPort("input_1", input_parent);
        InRFInfoPort input_port_2 = new InRFInfoPort("input_2", input_parent_2);
        OutRFInfoPort output_port = new OutRFInfoPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
            oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        Assert.assertEquals(input_parent.get_source_id(), "original");

            RFInfoPkt rfinfo = new RFInfoPkt();
            rfinfo.rf_flow_id = new String("newvalue");
            rfinfo.sensor = new SensorInfo();
            rfinfo.sensor.collector = new String("");
            rfinfo.sensor.antenna = new AntennaInfo();
            rfinfo.sensor.antenna.description = new String("");
            rfinfo.sensor.antenna.name = new String("");
            rfinfo.sensor.antenna.size = new String("");
            rfinfo.sensor.antenna.type = new String("");
            rfinfo.sensor.feed = new FeedInfo();
            rfinfo.sensor.feed.name = new String("");
            rfinfo.sensor.feed.polarization = new String("");
            rfinfo.sensor.feed.freq_range = new FreqRange();
            rfinfo.sensor.feed.freq_range.values = new double[0];
            rfinfo.sensor.mission = new String("");
            rfinfo.sensor.rx = new String("");
            rfinfo.ext_path_delays = new PathDelay[0];
            rfinfo.capabilities = new RFCapabilities();
            rfinfo.capabilities.freq_range = new FreqRange();
            rfinfo.capabilities.freq_range.values = new double[0];
            rfinfo.capabilities.bw_range = new FreqRange();
            rfinfo.capabilities.bw_range.values = new double[0];
            rfinfo.additional_info = new CF.DataType[0];

        try {
            output_port.rfinfo_pkt(rfinfo);
            Assert.assertEquals(input_parent.get_source_id(), "original");
            Assert.assertThrows(PortCallError.class, () -> output_port.rfinfo_pkt(rfinfo, "hello"));

            output_port.connectPort(input_port_1._this(), "hello");

            output_port.rfinfo_pkt(rfinfo);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue");

            rfinfo.rf_flow_id = new String("newvalue_2");
            output_port.rfinfo_pkt(rfinfo, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_2");

            Assert.assertThrows(PortCallError.class, () -> output_port.rfinfo_pkt(rfinfo, "foo"));

            rfinfo.rf_flow_id = new String("newvalue_3");
            output_port.connectPort(input_port_2._this(), "foo");

            output_port.rfinfo_pkt(rfinfo);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_3");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
            rfinfo.rf_flow_id = new String("newvalue_4");
            output_port.rfinfo_pkt(rfinfo, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_4");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
        } catch (InvalidPort | OccupiedPort | PortCallError e) {
            Assert.assertTrue(false);
        };
    }

    @Test
    public void testRFSourceGetter() {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        rfsource_port_sample input_parent = new rfsource_port_sample();
        InRFSourcePort input_port_1 = new InRFSourcePort("input_1", input_parent);
        OutRFSourcePort output_port = new OutRFSourcePort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        input_parent.set_source_id("newvalue");

        Assert.assertThrows(PortCallError.class, () -> output_port.current_rf_input());
        Assert.assertThrows(PortCallError.class, () -> output_port._get_current_rf_input("hello"));

        try {
            output_port.connectPort(input_port_1._this(), "hello");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };

        RFInfoPkt rfinfo = null;
        try {
            rfinfo = output_port.current_rf_input();
            Assert.assertEquals(rfinfo.rf_flow_id, "newvalue");
            rfinfo = output_port._get_current_rf_input("hello");
            Assert.assertEquals(rfinfo.rf_flow_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_current_rf_input("foo"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };

        rfsource_port_sample input_parent_2 = new rfsource_port_sample();
        input_parent_2.set_source_id("newvalue_2");
        InRFSourcePort input_port_2 = new InRFSourcePort("input_2", input_parent);
        try {
            byte[] oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        try {
            output_port.connectPort(input_port_2._this(), "foo");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };
        Assert.assertThrows(PortCallError.class, () -> output_port.current_rf_input());
        try {
            rfinfo = output_port._get_current_rf_input("hello");
            Assert.assertEquals(rfinfo.rf_flow_id, "newvalue");
            Assert.assertThrows(PortCallError.class, () -> output_port._get_current_rf_input("something"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };
        output_port.disconnectPort("hello");
    }

    @Test
    public void testRFSourceSetter()
    {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        rfsource_port_sample input_parent = new rfsource_port_sample();
        rfsource_port_sample input_parent_2 = new rfsource_port_sample();
        InRFSourcePort input_port_1 = new InRFSourcePort("input_1", input_parent);
        InRFSourcePort input_port_2 = new InRFSourcePort("input_2", input_parent_2);
        OutRFSourcePort output_port = new OutRFSourcePort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
            oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        Assert.assertEquals(input_parent.get_source_id(), "original");

            RFInfoPkt rfinfo = new RFInfoPkt();
            rfinfo.rf_flow_id = new String("newvalue");
            rfinfo.sensor = new SensorInfo();
            rfinfo.sensor.collector = new String("");
            rfinfo.sensor.antenna = new AntennaInfo();
            rfinfo.sensor.antenna.description = new String("");
            rfinfo.sensor.antenna.name = new String("");
            rfinfo.sensor.antenna.size = new String("");
            rfinfo.sensor.antenna.type = new String("");
            rfinfo.sensor.feed = new FeedInfo();
            rfinfo.sensor.feed.name = new String("");
            rfinfo.sensor.feed.polarization = new String("");
            rfinfo.sensor.feed.freq_range = new FreqRange();
            rfinfo.sensor.feed.freq_range.values = new double[0];
            rfinfo.sensor.mission = new String("");
            rfinfo.sensor.rx = new String("");
            rfinfo.ext_path_delays = new PathDelay[0];
            rfinfo.capabilities = new RFCapabilities();
            rfinfo.capabilities.freq_range = new FreqRange();
            rfinfo.capabilities.freq_range.values = new double[0];
            rfinfo.capabilities.bw_range = new FreqRange();
            rfinfo.capabilities.bw_range.values = new double[0];
            rfinfo.additional_info = new CF.DataType[0];

        try {
            output_port.current_rf_input(rfinfo);
            Assert.assertEquals(input_parent.get_source_id(), "original");
            Assert.assertThrows(PortCallError.class, () -> output_port.current_rf_input(rfinfo, "hello"));

            output_port.connectPort(input_port_1._this(), "hello");

            output_port.current_rf_input(rfinfo);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue");

            rfinfo.rf_flow_id = new String("newvalue_2");
            output_port.current_rf_input(rfinfo, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_2");

            Assert.assertThrows(PortCallError.class, () -> output_port.current_rf_input(rfinfo, "foo"));

            rfinfo.rf_flow_id = new String("newvalue_3");
            output_port.connectPort(input_port_2._this(), "foo");

            output_port.current_rf_input(rfinfo);
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_3");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
            rfinfo.rf_flow_id = new String("newvalue_4");
            output_port.current_rf_input(rfinfo, "hello");
            Assert.assertEquals(input_parent.get_source_id(), "newvalue_4");
            Assert.assertEquals(input_parent_2.get_source_id(), "newvalue_3");
        } catch (InvalidPort | OccupiedPort | PortCallError e) {
            Assert.assertTrue(false);
        };
    }

    @Test
    public void testTunerGetter() {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        digitaltuner_port_sample input_parent = new digitaltuner_port_sample();
        InDigitalTunerPort input_port_1 = new InDigitalTunerPort("input_1", input_parent);
        OutDigitalTunerPort output_port = new OutDigitalTunerPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        input_parent.set_bw(1);

        Assert.assertThrows(PortCallError.class, () -> output_port.getTunerBandwidth("first_tuner"));
        Assert.assertThrows(PortCallError.class, () -> output_port.getTunerBandwidth("first_tuner", "hello"));

        try {
            output_port.connectPort(input_port_1._this(), "hello");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };

        double bw = 0;
        try {
            bw = output_port.getTunerBandwidth("first_tuner");
            Assert.assertTrue(bw==1);
            bw = output_port.getTunerBandwidth("first_tuner", "hello");
            Assert.assertTrue(bw==1);
            Assert.assertThrows(PortCallError.class, () -> output_port.getTunerBandwidth("first_tuner", "foo"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };

        digitaltuner_port_sample input_parent_2 = new digitaltuner_port_sample();
        input_parent_2.set_bw(2);
        InDigitalTunerPort input_port_2 = new InDigitalTunerPort("input_2", input_parent);
        try {
            byte[] oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        try {
            output_port.connectPort(input_port_2._this(), "foo");
        } catch (InvalidPort | OccupiedPort e) {
            Assert.assertTrue(false);
        };
        Assert.assertThrows(PortCallError.class, () -> output_port.getTunerBandwidth("first_tuner"));
        try {
            bw = output_port.getTunerBandwidth("first_tuner", "hello");
            Assert.assertTrue(bw==1);
            Assert.assertThrows(PortCallError.class, () -> output_port.getTunerBandwidth("first_tuner", "something"));
        } catch (PortCallError e) {
            Assert.assertTrue(false);
        };
        output_port.disconnectPort("hello");
    }

    @Test
    public void testTunerSetter()
    {
        String[] args = null;
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, null );
        final POA rootpoa = org.ossie.corba.utils.RootPOA();

        digitaltuner_port_sample input_parent = new digitaltuner_port_sample();
        digitaltuner_port_sample input_parent_2 = new digitaltuner_port_sample();
        InDigitalTunerPort input_port_1 = new InDigitalTunerPort("input_1", input_parent);
        InDigitalTunerPort input_port_2 = new InDigitalTunerPort("input_2", input_parent_2);
        OutDigitalTunerPort output_port = new OutDigitalTunerPort("output");
        try {
            byte[] oid = rootpoa.activate_object(input_port_1);
            oid = rootpoa.activate_object(input_port_2);
        } catch (ServantAlreadyActive | WrongPolicy e) {
        }

        Assert.assertTrue(input_parent.get_bw()==0);
        double bw = 1;

        try {
            output_port.setTunerBandwidth("first_tuner", bw);
            Assert.assertTrue(input_parent.get_bw()==0);
            final double testbw = bw;
            Assert.assertThrows(PortCallError.class, () -> output_port.setTunerBandwidth("first_tuner", testbw, "hello"));

            output_port.connectPort(input_port_1._this(), "hello");

            output_port.setTunerBandwidth("first_tuner", bw);
            Assert.assertTrue(input_parent.get_bw() == bw);

            bw = 2;
            output_port.setTunerBandwidth("first_tuner", bw, "hello");
            Assert.assertTrue(input_parent.get_bw() == bw);

            final double testbw_2 = bw;
            Assert.assertThrows(PortCallError.class, () -> output_port.setTunerBandwidth("first_tuner", testbw_2, "foo"));

            bw = 3;
            output_port.connectPort(input_port_2._this(), "foo");

            output_port.setTunerBandwidth("first_tuner", bw);
            Assert.assertTrue(input_parent.get_bw() == bw);
            Assert.assertTrue(input_parent_2.get_bw() == bw);
            bw = 4;
            output_port.setTunerBandwidth("first_tuner", bw, "hello");
            Assert.assertTrue(input_parent.get_bw() == bw);
            Assert.assertTrue(input_parent_2.get_bw() == 3);
        } catch (InvalidPort | OccupiedPort | PortCallError e) {
            Assert.assertTrue(false);
        };
    }

/*void PortsTest::testTunerSetter()
{
    tuner_port_sample input_parent;
    tuner_port_sample input_parent_2;
    frontend::InDigitalScanningTunerPort *input_port_1 = new frontend::InDigitalScanningTunerPort("input_1", &input_parent);
    frontend::OutDigitalTunerPort *output_port = new frontend::OutDigitalTunerPort("output");

    CPPUNIT_ASSERT(input_parent.get_bw()==0);

    double bw = 1;
    output_port->setTunerBandwidth("first_tuner", bw);
    CPPUNIT_ASSERT(input_parent.get_bw() == 0);
    CPPUNIT_ASSERT_THROW(output_port->setTunerBandwidth("first_tuner", bw, "hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    output_port->setTunerBandwidth("first_tuner", bw);
    CPPUNIT_ASSERT(input_parent.get_bw() == bw);

    bw = 2;
    output_port->setTunerBandwidth("first_tuner", bw, "hello");
    CPPUNIT_ASSERT(input_parent.get_bw() == bw);

    CPPUNIT_ASSERT_THROW(output_port->setTunerBandwidth("first_tuner", bw, "foo"), redhawk::PortCallError);

    bw = 3;
    frontend::InDigitalScanningTunerPort *input_port_2 = new frontend::InDigitalScanningTunerPort("input_2", &input_parent_2);
    output_port->connectPort(input_port_2->_this(), "foo");

    output_port->setTunerBandwidth("first_tuner", bw);
    CPPUNIT_ASSERT(input_parent.get_bw() == bw);
    CPPUNIT_ASSERT(input_parent_2.get_bw() == bw);

    bw = 4;
    output_port->setTunerBandwidth("first_tuner", bw, "hello");
    CPPUNIT_ASSERT(input_parent.get_bw() == 4);
    CPPUNIT_ASSERT(input_parent_2.get_bw() == 3);

    CPPUNIT_ASSERT_THROW(output_port->setTunerBandwidth("first_tuner", bw, "something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}
*/

}
