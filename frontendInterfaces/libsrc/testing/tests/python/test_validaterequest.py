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
import fe_types

class ValidateRequestTest(unittest.TestCase):

    def testSRI(self):
        request = fe_types.frontend_tuner_allocation()
        upstream_sri = BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=1/2e6,
                              xunits=BULKIO.UNITS_TIME, subsize=0, ystart=0.0, ydelta=0.0,
                              yunits=BULKIO.UNITS_NONE, mode=0, streamID="", blocking=False, keywords=[])
        request.center_frequency = 100e6
        request.bandwidth = 1e6
        request.sample_rate = 2e6
        cf = 100e6
        bw = 1e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertTrue(tuner_device.validateRequestVsSRI(request, upstream_sri, False))
        cf = 100.49e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertTrue(tuner_device.validateRequestVsSRI(request, upstream_sri, False))
        cf = 99.51e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertTrue(tuner_device.validateRequestVsSRI(request, upstream_sri, False))
        cf = 100.51e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsSRI, request, upstream_sri, False)
        cf = 99.49e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsSRI, request, upstream_sri, False)

    def testDeviceSRI(self):
        request = fe_types.frontend_tuner_allocation()
        upstream_sri = BULKIO.StreamSRI(hversion=1, xstart=0.0, xdelta=1/2e6,
                              xunits=BULKIO.UNITS_TIME, subsize=0, ystart=0.0, ydelta=0.0,
                              yunits=BULKIO.UNITS_NONE, mode=0, streamID="", blocking=False, keywords=[])
        request.center_frequency = 100e6
        request.bandwidth = 1e6
        request.sample_rate = 2e6
        cf = 100e6
        bw = 1e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        min_dev_cf = 99.5e6
        max_dev_cf = 100.5e6
        max_dev_bw = 3e6
        max_dev_sr = 6e6
        self.assertTrue(tuner_device.validateRequestVsDeviceStream(request, upstream_sri, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr))
        cf = 100.49e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertTrue(tuner_device.validateRequestVsDeviceStream(request, upstream_sri, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr))
        cf = 99.51e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertTrue(tuner_device.validateRequestVsDeviceStream(request, upstream_sri, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr))
        cf = 100.51e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsDeviceStream, request, upstream_sri, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr)
        cf = 99.49e6
        _keywords=[CF.DataType(id="CHAN_RF", value=_any.to_any(cf)), CF.DataType(id="FRONTEND::BANDWIDTH", value=_any.to_any(bw))]
        upstream_sri.keywords = _keywords
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsDeviceStream, request, upstream_sri, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr)

    def testRFInfo(self):
        request = fe_types.frontend_tuner_allocation()
        rfinfo = fe_types.RFInfoPkt()
        request.center_frequency = 100e6
        request.bandwidth = 1e6
        request.sample_rate = 2e6
        cf = 100e6
        bw = 1e6
        rfinfo.rf_center_freq = cf
        rfinfo.rf_bandwidth = bw
        self.assertTrue(tuner_device.validateRequestVsRFInfo(request, rfinfo, False))
        cf = 100.49e6
        rfinfo.rf_center_freq = cf
        self.assertTrue(tuner_device.validateRequestVsRFInfo(request, rfinfo, False))
        cf = 99.51e6
        rfinfo.rf_center_freq = cf
        self.assertTrue(tuner_device.validateRequestVsRFInfo(request, rfinfo, False))
        cf = 100.51e6
        rfinfo.rf_center_freq = cf
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsRFInfo, request, rfinfo, False)
        cf = 99.49e6
        rfinfo.rf_center_freq = cf
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsRFInfo, request, rfinfo, False)

    def testDeviceRFInfo(self):
        request = fe_types.frontend_tuner_allocation()
        rfinfo = fe_types.RFInfoPkt()
        request.center_frequency = 100e6
        request.bandwidth = 1e6
        request.sample_rate = 2e6
        cf = 100e6
        bw = 1e6
        rfinfo.rf_center_freq = cf
        rfinfo.rf_bandwidth = bw
        min_dev_cf = 99.5e6
        max_dev_cf = 100.5e6
        max_dev_bw = 3e6
        max_dev_sr = 6e6
        self.assertTrue(tuner_device.validateRequestVsDevice(request, rfinfo, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr))
        cf = 100.49e6
        rfinfo.rf_center_freq = cf
        self.assertTrue(tuner_device.validateRequestVsDevice(request, rfinfo, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr))
        cf = 99.51e6
        rfinfo.rf_center_freq = cf
        self.assertTrue(tuner_device.validateRequestVsDevice(request, rfinfo, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr))
        cf = 100.51e6
        rfinfo.rf_center_freq = cf
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsDevice, request, rfinfo, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr)
        cf = 99.49e6
        rfinfo.rf_center_freq = cf
        self.assertRaises(FRONTEND.BadParameterException, tuner_device.validateRequestVsDevice, request, rfinfo, False, min_dev_cf, max_dev_cf, max_dev_bw, max_dev_sr)

if __name__ == '__main__':
    import runtests
    runtests.main()
