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

#include "ValidateRequest.h"

#include <frontend.h>
#include <bulkio/bulkio.h>
#include <ossie/PropertyMap.h>

CPPUNIT_TEST_SUITE_REGISTRATION(ValidateRequestTest);

void ValidateRequestTest::testSRI()
{
    frontend::frontend_tuner_allocation_struct request;
    BULKIO::StreamSRI upstream_sri;
    request.center_frequency = 100e6;
    request.bandwidth = 1e6;
    request.sample_rate = 2e6;
    upstream_sri.xdelta = 1/2e6;
    upstream_sri.mode = 0;
    redhawk::PropertyMap& keywords = redhawk::PropertyMap::cast(upstream_sri.keywords);
    double cf, bw;
    cf = 100e6;
    bw = 1e6;
    keywords["CHAN_RF"] = cf;
    keywords["FRONTEND::BANDWIDTH"] = bw;
    CPPUNIT_ASSERT(validateRequestVsSRI(request, upstream_sri, false));
    cf = 100.49e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT(validateRequestVsSRI(request, upstream_sri, false));
    cf = 99.51e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT(validateRequestVsSRI(request, upstream_sri, false));
    cf = 100.51e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsSRI(request, upstream_sri, false), FRONTEND::BadParameterException);
    cf = 99.49e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsSRI(request, upstream_sri, false), FRONTEND::BadParameterException);
}

void ValidateRequestTest::testDeviceSRI()
{
    frontend::frontend_tuner_allocation_struct request;
    BULKIO::StreamSRI upstream_sri;
    request.center_frequency = 100e6;
    request.bandwidth = 1e6;
    request.sample_rate = 2e6;
    upstream_sri.xdelta = 1/2e6;
    upstream_sri.mode = 0;
    redhawk::PropertyMap& keywords = redhawk::PropertyMap::cast(upstream_sri.keywords);
    double cf, bw;
    cf = 100e6;
    bw = 1e6;
    keywords["CHAN_RF"] = cf;
    keywords["FRONTEND::BANDWIDTH"] = bw;
    double min_dev_cf = 99e6;
    double max_dev_cf = 100e6;
    double max_dev_bw = 3e6;
    double max_dev_sr = 6e6;
    CPPUNIT_ASSERT(validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr));
    cf = 100.49e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT(validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr));
    cf = 99.51e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT(validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr));
    cf = 100.51e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr), FRONTEND::BadParameterException);
    cf = 99.49e6;
    keywords["CHAN_RF"] = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr), FRONTEND::BadParameterException);
}

void ValidateRequestTest::testRFInfo()
{
    frontend::frontend_tuner_allocation_struct request;
    frontend::RFInfoPkt rfinfo;
    request.center_frequency = 100e6;
    request.bandwidth = 1e6;
    request.sample_rate = 2e6;
    double cf, bw;
    cf = 100e6;
    bw = 1e6;
    rfinfo.rf_center_freq = cf;
    rfinfo.rf_bandwidth = bw;
    CPPUNIT_ASSERT(validateRequestVsRFInfo(request, rfinfo, false));
    cf = 100.49e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT(validateRequestVsRFInfo(request, rfinfo, false));
    cf = 99.51e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT(validateRequestVsRFInfo(request, rfinfo, false));
    cf = 100.51e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsRFInfo(request, rfinfo, false), FRONTEND::BadParameterException);
    cf = 99.49e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsRFInfo(request, rfinfo, false), FRONTEND::BadParameterException);
}

void ValidateRequestTest::testDeviceRFInfo()
{
    frontend::frontend_tuner_allocation_struct request;
    frontend::RFInfoPkt rfinfo;
    request.center_frequency = 100e6;
    request.bandwidth = 1e6;
    request.sample_rate = 2e6;
    rfinfo.rf_center_freq = 100e6;
    rfinfo.rf_bandwidth = 1e6;
    double cf, bw;
    cf = 100e6;
    bw = 1e6;
    rfinfo.rf_center_freq = cf;
    rfinfo.rf_bandwidth = bw;
    double min_dev_cf = 99e6;
    double max_dev_cf = 100e6;
    double max_dev_bw = 3e6;
    double max_dev_sr = 6e6;
    CPPUNIT_ASSERT(validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr));
    cf = 100.49e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT(validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr));
    cf = 99.51e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT(validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr));
    cf = 100.51e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr), FRONTEND::BadParameterException);
    cf = 99.49e6;
    rfinfo.rf_center_freq = cf;
    CPPUNIT_ASSERT_THROW(validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr), FRONTEND::BadParameterException);
}

void ValidateRequestTest::setUp()
{
}

void ValidateRequestTest::tearDown()
{
}
