#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK core.
#
# REDHAWK core is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import unittest, time
from _unitTestHelpers import scatest
from ossie.cf import CF, ExtendedCF
from omniORB import any, CORBA
from ossie import properties
from ossie.utils import redhawk
import traceback

class ApplicationMetrics(scatest.CorbaTestCase):
    def setUp(self):
        self._app = None

    def tearDown(self):
        if self._app:
            self._app.ref.stop()
            self._app.ref.releaseObject()

        # Do all application shutdown before calling the base class tearDown,
        # or failures will probably occur.
        scatest.CorbaTestCase.tearDown(self)

    def test_AppAllMetrics(self):
        domBooter, self._domMgr = self.launchDomainManager()
        dommgr = redhawk.attach(self._domMgr._get_name())
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        dommgr.devices[0].threshold_cycle_time = 100

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._plainnode, None)

        time.sleep(1)

        self._app = dommgr.createApplication('/waveforms/busycomp_w/busycomp_w.sad.xml', 'busycomp_w', [], [])
        self.assertNotEqual(self._app, None)
        time.sleep(1)
        self.assertRaises(CF.Application.InvalidMetric, self._app.metrics, ['utilization'], [])

        bc=self._app.metrics(['busycomp_1'], [])[0].value._v
        value = -1
        for _val in bc:
            if _val.id == 'cores':
                value = _val.value._v
        self.assertTrue(value<0.1)
        bc=self._app.metrics(['busycomp_2'], [])[0].value._v
        value = -1
        for _val in bc:
            if _val.id == 'cores':
                value = _val.value._v
        self.assertTrue(value<0.1)
        bc=self._app.metrics(['msg_through_1'], [])[0].value._v
        value = -1
        for _val in bc:
            if _val.id == 'cores':
                value = _val.value._v
        self.assertTrue(value<0.1)

        self._app.start()
        for comp in self._app.comps:
            if comp.name == 'msg_through':
                comp.stop()
                break
        time.sleep(2)

        bc=self._app.metrics(['busycomp_1'], [])[0].value._v
        value = -1
        for _val in bc:
            if _val.id == 'cores':
                value = _val.value._v
        self.assertAlmostEquals(value, 2, places=1)

        bc=self._app.metrics(['busycomp_2'], [])[0].value._v
        value = -1
        for _val in bc:
            if _val.id == 'cores':
                value = _val.value._v
        self.assertAlmostEquals(value, 2, places=1)
        bc=self._app.metrics(['msg_through_1'], [])[0].value._v
        value = -1
        for _val in bc:
            if _val.id == 'cores':
                value = _val.value._v
        self.assertTrue(value<0.1)

        bc = self._app.metrics([], [])
        util_total = {}
        moving_total = {}
        for _i in bc:
            if _i.id == 'busycomp_2':
                continue
            if _i.id == 'application utilization':
                for v in _i.value._v:
                    if v.id == 'valid':
                        continue
                    util_total[v.id] = v.value._v
                continue
            for v in _i.value._v:
                if v.id == 'componenthost':
                    continue
                if v.id == 'valid':
                    continue
                if moving_total.has_key(v.id):
                    moving_total[v.id] += v.value._v
                else:
                    moving_total[v.id] = v.value._v
        for key in util_total:
            self.assertEquals(util_total[key],moving_total[key])

    def test_AppIndividualMetrics(self):
        domBooter, self._domMgr = self.launchDomainManager()
        dommgr = redhawk.attach(self._domMgr._get_name())
        plainBooter, self._plainnode = self.launchDeviceManager("/nodes/test_GPP_node/DeviceManager.dcd.xml")
        dommgr.devices[0].threshold_cycle_time = 100

        self.assertNotEqual(self._domMgr, None)
        self.assertNotEqual(self._plainnode, None)

        time.sleep(1)

        self._app = dommgr.createApplication('/waveforms/busycomp_w/busycomp_w.sad.xml', 'busycomp_w', [], [])
        self.assertNotEqual(self._app, None)
        time.sleep(1)
        self.assertRaises(CF.Application.InvalidMetric, self._app.metrics, ['utilization'], [])

        bc=self._app.metrics([], ['memory'])
        self.assertEquals(len(bc), 4)
        self.assertEquals(len(bc[0].value._v), 1)
        self.assertEquals(bc[0].value._v[0].id, 'memory')
        self.assertEquals(bc[3].value._v[0].id, 'memory')
        bc=self._app.metrics(['busycomp_1'], ['memory'])
        self.assertEquals(len(bc), 1)
        self.assertEquals(bc[0].id, 'busycomp_1')
        self.assertEquals(len(bc[0].value._v), 1)
        self.assertEquals(bc[0].value._v[0].id, 'memory')
        bc=self._app.metrics(['application utilization'], ['memory'])
        self.assertEquals(len(bc), 1)
        self.assertEquals(bc[0].id, 'application utilization')
        self.assertEquals(len(bc[0].value._v), 1)
        self.assertEquals(bc[0].value._v[0].id, 'memory')
        bc=self._app.metrics(['msg_through_1','busycomp_1'], ['memory'])
        self.assertEquals(len(bc), 2)
        self.assertEquals(bc[0].id, 'msg_through_1')
        self.assertEquals(len(bc[0].value._v), 1)
        self.assertEquals(bc[0].value._v[0].id, 'memory')
        self.assertEquals(bc[1].id, 'busycomp_1')
        self.assertEquals(len(bc[1].value._v), 1)
        self.assertEquals(bc[1].value._v[0].id, 'memory')

        bc=self._app.metrics([], ['cores', 'memory'])
        self.assertEquals(len(bc), 4)
        self.assertEquals(len(bc[0].value._v), 2)
        self.assertEquals(bc[0].value._v[0].id, 'cores')
        self.assertEquals(bc[0].value._v[1].id, 'memory')
        self.assertEquals(bc[3].value._v[0].id, 'cores')
        self.assertEquals(bc[3].value._v[1].id, 'memory')
        bc=self._app.metrics(['busycomp_1'], ['cores', 'memory'])
        self.assertEquals(len(bc), 1)
        self.assertEquals(bc[0].id, 'busycomp_1')
        self.assertEquals(len(bc[0].value._v), 2)
        self.assertEquals(bc[0].value._v[0].id, 'cores')
        self.assertEquals(bc[0].value._v[1].id, 'memory')
        bc=self._app.metrics(['msg_through_1','busycomp_1'], ['cores', 'memory'])
        self.assertEquals(len(bc), 2)
        self.assertEquals(bc[0].id, 'msg_through_1')
        self.assertEquals(len(bc[0].value._v), 2)
        self.assertEquals(bc[0].value._v[0].id, 'cores')
        self.assertEquals(bc[0].value._v[1].id, 'memory')
        self.assertEquals(bc[1].id, 'busycomp_1')
        self.assertEquals(len(bc[1].value._v), 2)
        self.assertEquals(bc[1].value._v[0].id, 'cores')
        self.assertEquals(bc[1].value._v[1].id, 'memory')

        self.assertRaises(CF.Application.InvalidMetric, self._app.metrics, [], ['cord', 'memory'])
