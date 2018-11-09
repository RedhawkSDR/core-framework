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
#ifndef FRONTEND_PORTSTTEST_H
#define FRONTEND_PORTSTTEST_H

#include <cppunit/extensions/HelperMacros.h>
#include <frontend.h>
#include <bulkio/bulkio.h>
#include <ossie/PropertyMap.h>

class gps_port_sample: public frontend::gps_delegation {
    public:
        gps_port_sample() {
            id = "original";
        }
        void set_source_id(std::string in_id) {
            id = in_id;
        }
        std::string get_source_id() {
            return id;
        }
        frontend::GPSInfo get_gps_info(const std::string& port_name) {
            frontend::GPSInfo gpsinfo;
            gpsinfo.source_id = id;
            return gpsinfo;
        }
        void set_gps_info(const std::string& port_name, const frontend::GPSInfo &gps_info) {
            id = gps_info.source_id;
        }
        frontend::GpsTimePos get_gps_time_pos(const std::string& port_name) {
            return frontend::GpsTimePos();
        }
        void set_gps_time_pos(const std::string& port_name, const frontend::GpsTimePos &gps_time_pos) {
        }
        std::string id;
};

class nav_port_sample: public frontend::nav_delegation {
    public:
        nav_port_sample() {
            id = "original";
        }
        void set_source_id(std::string in_id) {
            id = in_id;
        }
        std::string get_source_id() {
            return id;
        }
        frontend::NavigationPacket get_nav_packet(const std::string& port_name) {
            frontend::NavigationPacket nav_info;
            nav_info.source_id = id;
            return nav_info;
        }
        void set_nav_packet(const std::string& port_name, const frontend::NavigationPacket &nav_info) {
            id = nav_info.source_id;
        }
        std::string id;
};

class rfinfo_port_sample: public frontend::rfinfo_delegation {
    public:
        rfinfo_port_sample() {
            id = "original";
        }
        void set_rf_flow_id(std::string in_id) {
            id = in_id;
        }
        std::string get_rf_flow_id() {
            return id;
        }
        std::string get_rf_flow_id(const std::string& port_name) {
            return std::string("none");
        }
        void set_rf_flow_id(const std::string& port_name, const std::string& id) {
        }
        frontend::RFInfoPkt get_rfinfo_pkt(const std::string& port_name) {
            frontend::RFInfoPkt rfinfo;
            rfinfo.rf_flow_id = id;
            return rfinfo;
        }
        void set_rfinfo_pkt(const std::string& port_name, const frontend::RFInfoPkt &pkt) {
            id = pkt.rf_flow_id;
        }
        std::string id;
};
class rfsource_port_sample: public frontend::rfsource_delegation {
    public:
        rfsource_port_sample() {
            id = "original";
        }
        void set_rf_flow_id(std::string in_id) {
            id = in_id;
        }
        std::string get_rf_flow_id() {
            return id;
        }
        std::vector<frontend::RFInfoPkt> get_available_rf_inputs(const std::string& port_name) {
            return std::vector<frontend::RFInfoPkt>();
        }
        void set_available_rf_inputs(const std::string& port_name, std::vector<frontend::RFInfoPkt> &inputs) {
        }
        frontend::RFInfoPkt get_current_rf_input(const std::string& port_name) {
            frontend::RFInfoPkt rfinfo;
            rfinfo.rf_flow_id = id;
            return rfinfo;
        }
        void set_current_rf_input(const std::string& port_name, const frontend::RFInfoPkt &input) {
            id = input.rf_flow_id;
        }
        std::string id;
};

class tuner_port_sample: public frontend::digital_scanning_tuner_delegation {
    public:
        tuner_port_sample() {
            bw = 0;
        }
        void set_bw(double in_bw) {
            bw = in_bw;
        }
        double get_bw() {
            return bw;
        }
        void setTunerBandwidth(const std::string& id, double in_bw) {
            bw = in_bw;
        }
        double getTunerBandwidth(const std::string& id) {
            return bw;
        }
        CF::Properties* getTunerStatus(const std::string &id) {};
        double bw;
};

class PortsTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(PortsTest);
    CPPUNIT_TEST(testGPSGetter);
    CPPUNIT_TEST(testGPSSetter);
    CPPUNIT_TEST(testNavGetter);
    CPPUNIT_TEST(testNavSetter);
    CPPUNIT_TEST(testRFInfoGetter);
    CPPUNIT_TEST(testRFInfoSetter);
    CPPUNIT_TEST(testRFSourceGetter);
    CPPUNIT_TEST(testRFSourceSetter);
    CPPUNIT_TEST(testTunerGetter);
    CPPUNIT_TEST(testTunerSetter);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp();
    void tearDown();

    void testGPSGetter();
    void testGPSSetter();
    void testNavGetter();
    void testNavSetter();
    void testRFInfoGetter();
    void testRFInfoSetter();
    void testRFSourceGetter();
    void testRFSourceSetter();
    void testTunerGetter();
    void testTunerSetter();
};

#endif  // FRONTEND_PORTSTTEST_H
