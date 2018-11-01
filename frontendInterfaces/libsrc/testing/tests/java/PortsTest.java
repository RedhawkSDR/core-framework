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
import FRONTEND.GPSInfo;
import FRONTEND.GpsTimePos;
import FRONTEND.FrontendException;
import FRONTEND.NotSupportedException;
import FRONTEND.BadParameterException;
import java.util.ArrayList;
import org.omg.CORBA.ORB;
import org.ossie.redhawk.PortCallError;

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
            return new GPSInfo();
        };
        public void set_gps_info(String port_name, GPSInfo data) throws FrontendException, BadParameterException, NotSupportedException {
        };
        public GpsTimePos get_gps_time_pos(String port_name) throws FrontendException, BadParameterException, NotSupportedException {
            return new GpsTimePos();
        };
        public void set_gps_time_pos(String port_name, GpsTimePos data) throws FrontendException, BadParameterException, NotSupportedException {
        };
    }

    @Test
    public void testGPSGetter() {
        gps_port_sample input_parent = new gps_port_sample();
        InGPSPort input_port_1 = new InGPSPort("input_1", input_parent);
        OutGPSPort output_port = new OutGPSPort("output");

        input_parent.set_source_id("newvalue");

        Assert.assertThrows(PortCallError.class, () -> output_port.gps_info());
        /*CPPUNIT_ASSERT_THROW(output_port->gps_info(), redhawk::PortCallError);*/
        Assert.assertThrows(PortCallError.class, () -> output_port._get_gps_info("hello"));
        //CPPUNIT_ASSERT_THROW(output_port->_get_gps_info("hello"), redhawk::PortCallError);

        /*output_port->connectPort(input_port_1->_this(), "hello");

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

        output_port->disconnectPort("hello");*/
    }

    /*void PortsTest::testGPSSetter()
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
    }*/
}
