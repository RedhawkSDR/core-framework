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

import org.junit.Assert;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import frontend.*;
import FRONTEND.BadParameterException;
import java.util.ArrayList;
import org.omg.CORBA.ORB;

@RunWith(JUnit4.class)
public class ValidateRequestTest
{

    @Test
    public void testSRI() {
        FETypes.frontend_tuner_allocation_struct request = new FETypes.frontend_tuner_allocation_struct();
        BULKIO.StreamSRI upstream_sri = new BULKIO.StreamSRI();
        request.center_frequency.setValue(100e6);
        request.bandwidth.setValue(1e6);
        request.sample_rate.setValue(2e6);
        upstream_sri.xdelta = 1/2e6;
        upstream_sri.mode = 0;
        upstream_sri.keywords = new CF.DataType[2];
        CF.DataType chanrf = new CF.DataType();
        CF.DataType chanbw = new CF.DataType();
        chanrf.id = "CHAN_RF";
        chanrf.value = ORB.init().create_any();
        chanbw.id = "FRONTEND::BANDWIDTH";
        chanbw.value = ORB.init().create_any();
        int cf_idx = 0;
        int bw_idx = 1;
        double cf, bw;
        cf = 100e6;
        chanrf.value.insert_double(cf);
        upstream_sri.keywords[cf_idx] = chanrf;
        bw = 1e6;
        chanbw.value.insert_double(bw);
        upstream_sri.keywords[bw_idx] = chanbw;
        boolean retval = false;
        try {
            retval = FrontendTunerDevice.validateRequestVsSRI(request, upstream_sri, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.49e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsSRI(request, upstream_sri, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 99.51e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsSRI(request, upstream_sri, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.51e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsSRI(request, upstream_sri, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);

        cf = 99.49e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsSRI(request, upstream_sri, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);
    }

    @Test
    public void testDeviceSRI() {
        FETypes.frontend_tuner_allocation_struct request = new FETypes.frontend_tuner_allocation_struct();
        BULKIO.StreamSRI upstream_sri = new BULKIO.StreamSRI();
        request.center_frequency.setValue(100e6);
        request.bandwidth.setValue(1e6);
        request.sample_rate.setValue(2e6);
        request.tuner_type.setValue("RX");
        upstream_sri.xdelta = 1/2e6;
        upstream_sri.mode = 0;
        upstream_sri.keywords = new CF.DataType[2];
        CF.DataType chanrf = new CF.DataType();
        CF.DataType chanbw = new CF.DataType();
        chanrf.id = "CHAN_RF";
        chanrf.value = ORB.init().create_any();
        chanbw.id = "FRONTEND::BANDWIDTH";
        chanbw.value = ORB.init().create_any();
        int cf_idx = 0;
        int bw_idx = 1;
        double cf, bw;
        cf = 100e6;
        chanrf.value.insert_double(cf);
        upstream_sri.keywords[cf_idx] = chanrf;
        bw = 1e6;
        chanbw.value.insert_double(bw);
        upstream_sri.keywords[bw_idx] = chanbw;
        double min_dev_cf = 99e6;
        double max_dev_cf = 101e6;
        double max_dev_bw = 3e6;
        double max_dev_sr = 6e6;
        boolean retval = false;
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.49e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 99.51e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.51e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);

        cf = 99.49e6;
        upstream_sri.keywords[cf_idx].value.insert_double(cf);
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, upstream_sri, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);
    }

    @Test
    public void testRFInfo() {
        FETypes.frontend_tuner_allocation_struct request = new FETypes.frontend_tuner_allocation_struct();
        FRONTEND.RFInfoPkt rfinfo = new FRONTEND.RFInfoPkt();
        request.center_frequency.setValue(100e6);
        request.bandwidth.setValue(1e6);
        request.sample_rate.setValue(2e6);
        double cf, bw;
        cf = 100e6;
        bw = 1e6;
        rfinfo.rf_center_freq = cf;
        rfinfo.rf_bandwidth = bw;
        boolean retval = false;
        try {
            retval = FrontendTunerDevice.validateRequestVsRFInfo(request, rfinfo, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.49e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsRFInfo(request, rfinfo, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 99.51e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsRFInfo(request, rfinfo, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.51e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsRFInfo(request, rfinfo, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);

        cf = 99.49e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsRFInfo(request, rfinfo, false);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);
    }

    @Test
    public void testDeviceRFInfo() {
        FETypes.frontend_tuner_allocation_struct request = new FETypes.frontend_tuner_allocation_struct();
        FRONTEND.RFInfoPkt rfinfo = new FRONTEND.RFInfoPkt();
        request.center_frequency.setValue(100e6);
        request.bandwidth.setValue(1e6);
        request.sample_rate.setValue(2e6);
        request.tuner_type.setValue("RX");
        double cf, bw;
        cf = 100e6;
        bw = 1e6;
        rfinfo.rf_center_freq = cf;
        rfinfo.rf_bandwidth = bw;
        double min_dev_cf = 99e6;
        double max_dev_cf = 101e6;
        double max_dev_bw = 3e6;
        double max_dev_sr = 6e6;
        boolean retval = false;
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.49e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 99.51e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertTrue(retval);

        cf = 100.51e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);

        cf = 99.49e6;
        rfinfo.rf_center_freq = cf;
        try {
            retval = FrontendTunerDevice.validateRequestVsDevice(request, rfinfo, false, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr);
        } catch (FRONTEND.BadParameterException e) {
            retval = false;
        }
        Assert.assertFalse(retval);
    }
}
