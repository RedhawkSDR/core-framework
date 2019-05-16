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

import time
from _unitTestHelpers import scatest
from ossie.utils import sb

class ComponentThreadTest(scatest.CorbaTestCase):
    def setUp(self):
        if sb.domainless._sandbox:
            sb.domainless._sandbox.shutdown()
            sb.domainless._sandbox = None
        self.timeout = 10

    def tearDown(self):
        sb.release()
        scatest.CorbaTestCase.tearDown(self)

    # check that the default delay is 100ms
    def test_defaultDelay(self):
        self.comp = sb.launch('check_noop')
        self.comp.start()
        self.assertEquals(self.comp.evaluate, 'done')
        self.assertEquals(self.comp.average_delay, 0.0)
        self.comp.evaluate = 'go'
        begin_time = time.time()
        while time.time()-begin_time < self.timeout and self.comp.evaluate != 'done':
            time.sleep(0.1)
        self.assertAlmostEquals(self.comp.average_delay, 0.1, places=2)

    # check that the delay can change to something other than the default
    def test_updateDelay(self):
        self.comp = sb.launch('check_noop')
        self.comp.start()
        self.assertEquals(self.comp.evaluate, 'done')
        self.assertEquals(self.comp.average_delay, 0.0)
        self.comp.evaluate = 'go'
        begin_time = time.time()
        while time.time()-begin_time < self.timeout and self.comp.evaluate != 'done':
            time.sleep(0.1)
        self.assertAlmostEquals(self.comp.average_delay, 0.1, places=2)
        new_delay = 0.05
        self.comp.noop_delay = new_delay
        self.comp.evaluate = 'go'
        begin_time = time.time()
        while time.time()-begin_time < self.timeout and self.comp.evaluate != 'done':
            time.sleep(0.1)
        self.assertAlmostEquals(self.comp.average_delay, new_delay, places=2)

    # check that the delay can change multiple times
    # also check that delays can be shorted or longer than the default delay
    def test_changeDelay(self):
        self.comp = sb.launch('check_noop')
        self.comp.start()
        self.assertEquals(self.comp.evaluate, 'done')
        self.assertEquals(self.comp.average_delay, 0.0)
        self.comp.evaluate = 'go'
        begin_time = time.time()
        while time.time()-begin_time < self.timeout and self.comp.evaluate != 'done':
            time.sleep(0.1)
        self.assertAlmostEquals(self.comp.average_delay, 0.1, places=2)
        new_delay = 0.05
        self.comp.noop_delay = new_delay
        self.comp.evaluate = 'go'
        begin_time = time.time()
        while time.time()-begin_time < self.timeout and self.comp.evaluate != 'done':
            time.sleep(0.1)
        self.assertAlmostEquals(self.comp.average_delay, new_delay, places=2)
        new_delay = 0.15
        self.comp.noop_delay = new_delay
        self.comp.evaluate = 'go'
        begin_time = time.time()
        while time.time()-begin_time < self.timeout and self.comp.evaluate != 'done':
            time.sleep(0.1)
        self.assertAlmostEquals(self.comp.average_delay, new_delay, places=2)
