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

#include "Ports.h"

CPPUNIT_TEST_SUITE_REGISTRATION(PortsTest);

void PortsTest::testGPSGetter()
{
    gps_port_sample input_parent;
    frontend::InGPSPort *input_port_1 = new frontend::InGPSPort("input_1", &input_parent);
    frontend::OutGPSPort *output_port = new frontend::OutGPSPort("output");

    input_parent.set_source_id("newvalue");

    CPPUNIT_ASSERT_THROW(output_port->gps_info(), redhawk::PortCallError);
    CPPUNIT_ASSERT_THROW(output_port->_get_gps_info("hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    frontend::GPSInfo gpsinfo = output_port->gps_info();
    CPPUNIT_ASSERT(gpsinfo.source_id == "newvalue");
    gpsinfo = output_port->_get_gps_info("hello");
    CPPUNIT_ASSERT(gpsinfo.source_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_gps_info("foo"), redhawk::PortCallError);

    gps_port_sample input_parent_2;
    input_parent_2.set_source_id("newvalue_2");
    frontend::InGPSPort *input_port_2 = new frontend::InGPSPort("input_2", &input_parent_2);

    output_port->connectPort(input_port_2->_this(), "foo");
    CPPUNIT_ASSERT_THROW(output_port->gps_info(), redhawk::PortCallError);
    gpsinfo = output_port->_get_gps_info("hello");
    CPPUNIT_ASSERT(gpsinfo.source_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_gps_info("something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testGPSSetter()
{
    gps_port_sample input_parent;
    gps_port_sample input_parent_2;
    frontend::InGPSPort *input_port_1 = new frontend::InGPSPort("input_1", &input_parent);
    frontend::OutGPSPort *output_port = new frontend::OutGPSPort("output");

    CPPUNIT_ASSERT(input_parent.get_source_id()=="original");

    frontend::GPSInfo gpsinfo;
    gpsinfo.source_id = "newvalue";

    output_port->gps_info(gpsinfo);
    CPPUNIT_ASSERT(input_parent.get_source_id() == "original");
    CPPUNIT_ASSERT_THROW(output_port->gps_info(gpsinfo, "hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    output_port->gps_info(gpsinfo);
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue");

    gpsinfo.source_id = "newvalue_2";
    output_port->gps_info(gpsinfo, "hello");
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue_2");

    CPPUNIT_ASSERT_THROW(output_port->gps_info(gpsinfo, "foo"), redhawk::PortCallError);

    gpsinfo.source_id = "newvalue_3";
    frontend::InGPSPort *input_port_2 = new frontend::InGPSPort("input_2", &input_parent_2);
    output_port->connectPort(input_port_2->_this(), "foo");

    output_port->gps_info(gpsinfo);
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue_3");
    CPPUNIT_ASSERT(input_parent_2.get_source_id() == "newvalue_3");

    gpsinfo.source_id = "newvalue_4";
    output_port->gps_info(gpsinfo, "hello");
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue_4");
    CPPUNIT_ASSERT(input_parent_2.get_source_id() == "newvalue_3");

    CPPUNIT_ASSERT_THROW(output_port->gps_info(gpsinfo, "something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testNavGetter()
{
    nav_port_sample input_parent;
    frontend::InNavDataPort *input_port_1 = new frontend::InNavDataPort("input_1", &input_parent);
    frontend::OutNavDataPort *output_port = new frontend::OutNavDataPort("output");

    input_parent.set_source_id("newvalue");

    CPPUNIT_ASSERT_THROW(output_port->nav_packet(), redhawk::PortCallError);
    CPPUNIT_ASSERT_THROW(output_port->_get_nav_packet("hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    frontend::NavigationPacket navinfo = output_port->nav_packet();
    CPPUNIT_ASSERT(navinfo.source_id == "newvalue");
    navinfo = output_port->_get_nav_packet("hello");
    CPPUNIT_ASSERT(navinfo.source_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_nav_packet("foo"), redhawk::PortCallError);

    nav_port_sample input_parent_2;
    input_parent_2.set_source_id("newvalue_2");
    frontend::InNavDataPort *input_port_2 = new frontend::InNavDataPort ("input_2", &input_parent_2);

    output_port->connectPort(input_port_2->_this(), "foo");
    CPPUNIT_ASSERT_THROW(output_port->nav_packet(), redhawk::PortCallError);
    navinfo = output_port->_get_nav_packet("hello");
    CPPUNIT_ASSERT(navinfo.source_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_nav_packet("something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testNavSetter()
{
    nav_port_sample input_parent;
    nav_port_sample input_parent_2;
    frontend::InNavDataPort *input_port_1 = new frontend::InNavDataPort("input_1", &input_parent);
    frontend::OutNavDataPort *output_port = new frontend::OutNavDataPort("output");

    CPPUNIT_ASSERT(input_parent.get_source_id()=="original");

    frontend::NavigationPacket navinfo;
    navinfo.source_id = "newvalue";

    output_port->nav_packet(navinfo);
    CPPUNIT_ASSERT(input_parent.get_source_id() == "original");
    CPPUNIT_ASSERT_THROW(output_port->nav_packet(navinfo, "hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    output_port->nav_packet(navinfo);
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue");

    navinfo.source_id = "newvalue_2";
    output_port->nav_packet(navinfo, "hello");
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue_2");

    CPPUNIT_ASSERT_THROW(output_port->nav_packet(navinfo, "foo"), redhawk::PortCallError);

    navinfo.source_id = "newvalue_3";
    frontend::InNavDataPort *input_port_2 = new frontend::InNavDataPort("input_2", &input_parent_2);
    output_port->connectPort(input_port_2->_this(), "foo");

    output_port->nav_packet(navinfo);
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue_3");
    CPPUNIT_ASSERT(input_parent_2.get_source_id() == "newvalue_3");

    navinfo.source_id = "newvalue_4";
    output_port->nav_packet(navinfo, "hello");
    CPPUNIT_ASSERT(input_parent.get_source_id() == "newvalue_4");
    CPPUNIT_ASSERT(input_parent_2.get_source_id() == "newvalue_3");

    CPPUNIT_ASSERT_THROW(output_port->nav_packet(navinfo, "something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testRFInfoGetter()
{
    rfinfo_port_sample input_parent;
    frontend::InRFInfoPort *input_port_1 = new frontend::InRFInfoPort("input_1", &input_parent);
    frontend::OutRFInfoPort *output_port = new frontend::OutRFInfoPort("output");

    input_parent.set_rf_flow_id("newvalue");

    CPPUNIT_ASSERT_THROW(output_port->rfinfo_pkt(), redhawk::PortCallError);
    CPPUNIT_ASSERT_THROW(output_port->_get_rfinfo_pkt("hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    frontend::RFInfoPkt rfinfo = output_port->rfinfo_pkt();
    CPPUNIT_ASSERT(rfinfo.rf_flow_id == "newvalue");
    rfinfo = output_port->_get_rfinfo_pkt("hello");
    CPPUNIT_ASSERT(rfinfo.rf_flow_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_rfinfo_pkt("foo"), redhawk::PortCallError);

    rfinfo_port_sample input_parent_2;
    input_parent_2.set_rf_flow_id("newvalue_2");
    frontend::InRFInfoPort *input_port_2 = new frontend::InRFInfoPort ("input_2", &input_parent_2);

    output_port->connectPort(input_port_2->_this(), "foo");
    CPPUNIT_ASSERT_THROW(output_port->rfinfo_pkt(), redhawk::PortCallError);
    rfinfo = output_port->_get_rfinfo_pkt("hello");
    CPPUNIT_ASSERT(rfinfo.rf_flow_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_rfinfo_pkt("something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testRFInfoSetter()
{
    rfinfo_port_sample input_parent;
    rfinfo_port_sample input_parent_2;
    frontend::InRFInfoPort *input_port_1 = new frontend::InRFInfoPort("input_1", &input_parent);
    frontend::OutRFInfoPort *output_port = new frontend::OutRFInfoPort("output");

    CPPUNIT_ASSERT(input_parent.get_rf_flow_id()=="original");

    frontend::RFInfoPkt rfinfo;
    rfinfo.rf_flow_id = "newvalue";

    output_port->rfinfo_pkt(rfinfo);
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "original");
    CPPUNIT_ASSERT_THROW(output_port->rfinfo_pkt(rfinfo, "hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    output_port->rfinfo_pkt(rfinfo);
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue");

    rfinfo.rf_flow_id = "newvalue_2";
    output_port->rfinfo_pkt(rfinfo, "hello");
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue_2");

    CPPUNIT_ASSERT_THROW(output_port->rfinfo_pkt(rfinfo, "foo"), redhawk::PortCallError);

    rfinfo.rf_flow_id = "newvalue_3";
    frontend::InRFInfoPort *input_port_2 = new frontend::InRFInfoPort("input_2", &input_parent_2);
    output_port->connectPort(input_port_2->_this(), "foo");

    output_port->rfinfo_pkt(rfinfo);
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue_3");
    CPPUNIT_ASSERT(input_parent_2.get_rf_flow_id() == "newvalue_3");

    rfinfo.rf_flow_id = "newvalue_4";
    output_port->rfinfo_pkt(rfinfo, "hello");
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue_4");
    CPPUNIT_ASSERT(input_parent_2.get_rf_flow_id() == "newvalue_3");

    CPPUNIT_ASSERT_THROW(output_port->rfinfo_pkt(rfinfo, "something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testRFSourceGetter()
{
    rfsource_port_sample input_parent;
    frontend::InRFSourcePort *input_port_1 = new frontend::InRFSourcePort("input_1", &input_parent);
    frontend::OutRFSourcePort *output_port = new frontend::OutRFSourcePort("output");

    input_parent.set_rf_flow_id("newvalue");

    CPPUNIT_ASSERT_THROW(output_port->current_rf_input(), redhawk::PortCallError);
    CPPUNIT_ASSERT_THROW(output_port->_get_current_rf_input("hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    frontend::RFInfoPkt* rfsource = output_port->current_rf_input();
    CPPUNIT_ASSERT(rfsource->rf_flow_id == "newvalue");
    rfsource = output_port->_get_current_rf_input("hello");
    CPPUNIT_ASSERT(rfsource->rf_flow_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_current_rf_input("foo"), redhawk::PortCallError);

    rfsource_port_sample input_parent_2;
    input_parent_2.set_rf_flow_id("newvalue_2");
    frontend::InRFSourcePort *input_port_2 = new frontend::InRFSourcePort ("input_2", &input_parent_2);

    output_port->connectPort(input_port_2->_this(), "foo");
    CPPUNIT_ASSERT_THROW(output_port->current_rf_input(), redhawk::PortCallError);
    rfsource = output_port->_get_current_rf_input("hello");
    CPPUNIT_ASSERT(rfsource->rf_flow_id == "newvalue");
    CPPUNIT_ASSERT_THROW(output_port->_get_current_rf_input("something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testRFSourceSetter()
{
    rfsource_port_sample input_parent;
    rfsource_port_sample input_parent_2;
    frontend::InRFSourcePort *input_port_1 = new frontend::InRFSourcePort("input_1", &input_parent);
    frontend::OutRFSourcePort *output_port = new frontend::OutRFSourcePort("output");

    CPPUNIT_ASSERT(input_parent.get_rf_flow_id()=="original");

    frontend::RFInfoPkt rfsource;
    rfsource.rf_flow_id = "newvalue";

    output_port->current_rf_input(rfsource);
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "original");
    CPPUNIT_ASSERT_THROW(output_port->current_rf_input(rfsource, "hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    output_port->current_rf_input(rfsource);
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue");

    rfsource.rf_flow_id = "newvalue_2";
    output_port->current_rf_input(rfsource, "hello");
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue_2");

    CPPUNIT_ASSERT_THROW(output_port->current_rf_input(rfsource, "foo"), redhawk::PortCallError);

    rfsource.rf_flow_id = "newvalue_3";
    frontend::InRFSourcePort *input_port_2 = new frontend::InRFSourcePort("input_2", &input_parent_2);
    output_port->connectPort(input_port_2->_this(), "foo");

    output_port->current_rf_input(rfsource);
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue_3");
    CPPUNIT_ASSERT(input_parent_2.get_rf_flow_id() == "newvalue_3");

    rfsource.rf_flow_id = "newvalue_4";
    output_port->current_rf_input(rfsource, "hello");
    CPPUNIT_ASSERT(input_parent.get_rf_flow_id() == "newvalue_4");
    CPPUNIT_ASSERT(input_parent_2.get_rf_flow_id() == "newvalue_3");

    CPPUNIT_ASSERT_THROW(output_port->current_rf_input(rfsource, "something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testTunerGetter()
{
    tuner_port_sample input_parent;
    frontend::InDigitalScanningTunerPort *input_port_1 = new frontend::InDigitalScanningTunerPort("input_1", &input_parent);
    frontend::OutDigitalTunerPort *output_port = new frontend::OutDigitalTunerPort("output");

    input_parent.set_bw(1);

    CPPUNIT_ASSERT_THROW(output_port->getTunerBandwidth("first_tuner"), redhawk::PortCallError);
    CPPUNIT_ASSERT_THROW(output_port->getTunerBandwidth("first_tuner", "hello"), redhawk::PortCallError);

    output_port->connectPort(input_port_1->_this(), "hello");

    double bw = output_port->getTunerBandwidth("first_tuner");
    CPPUNIT_ASSERT(bw == 1);
    bw = output_port->getTunerBandwidth("first_tuner", "hello");
    CPPUNIT_ASSERT(bw == 1);
    CPPUNIT_ASSERT_THROW(output_port->getTunerBandwidth("first_tuner", "foo"), redhawk::PortCallError);

    tuner_port_sample input_parent_2;
    input_parent_2.set_bw(2);
    frontend::InDigitalScanningTunerPort *input_port_2 = new frontend::InDigitalScanningTunerPort ("input_2", &input_parent_2);

    output_port->connectPort(input_port_2->_this(), "foo");
    CPPUNIT_ASSERT_THROW(output_port->getTunerBandwidth("first_tuner"), redhawk::PortCallError);
    bw = output_port->getTunerBandwidth("first_tuner", "hello");
    CPPUNIT_ASSERT(bw == 1);
    CPPUNIT_ASSERT_THROW(output_port->getTunerBandwidth("first_tuner", "something"), redhawk::PortCallError);

    output_port->disconnectPort("hello");

    input_port_1->_remove_ref();
}

void PortsTest::testTunerSetter()
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

void PortsTest::setUp()
{
}

void PortsTest::tearDown()
{
}
