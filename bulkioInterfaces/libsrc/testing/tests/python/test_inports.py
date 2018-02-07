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

from ossie.utils.log4py import logging

import bulkio
from bulkio.bulkioInterfaces import BULKIO

from helpers import *

class SriListener(object):
    def __init__(self):
        self.reset()

    def __call__(self, *args):
        self.sriChanged = True

    def reset(self):
        self.sriChanged = False

class InPortTest(object):
    def setUp(self):
        self.port = self.helper.createInPort()

    def tearDown(self):
        pass

    def testSriChange(self):
        stream_id = 'invalid_stream'

        # Push data without an SRI to check that the sriChanged flag is still
        # set and the SRI callback gets called
        listener = SriListener()
        self.port.setNewSriListener(listener)
        self._pushTestPacket(0, bulkio.timestamp.notSet(), False, stream_id)
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(not packet)
        self.failUnless(packet.sriChanged)
        self.failUnless(listener.sriChanged)
        
        # Push again to the same stream ID; sriChanged should now be false and the
        # SRI callback should not get called
        listener.reset()
        self._pushTestPacket(0, bulkio.timestamp.notSet(), False, stream_id)
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(not packet)
        self.failIf(packet.sriChanged)
        self.failIf(listener.sriChanged)

    def _pushTestPacket(self, length, time, eos, streamID):
        data = self.helper.createData(length)
        self.helper.pushPacket(self.port, data, time, eos, streamID)

def register_test(name, testbase, **kwargs):
    globals()[name] = type(name, (testbase, unittest.TestCase), kwargs)

register_test('InBitPortTest', InPortTest, helper=BitTestHelper())
register_test('InXMLPortTest', InPortTest, helper=XMLTestHelper())
register_test('InFilePortTest', InPortTest, helper=FileTestHelper())
register_test('InCharPortTest', InPortTest, helper=CharTestHelper())
register_test('InOctetPortTest', InPortTest, helper=OctetTestHelper())
register_test('InShortPortTest', InPortTest, helper=ShortTestHelper())
register_test('InUShortPortTest', InPortTest, helper=UShortTestHelper())
register_test('InLongPortTest', InPortTest, helper=LongTestHelper())
register_test('InULongPortTest', InPortTest, helper=ULongTestHelper())
register_test('InLongLongPortTest', InPortTest, helper=LongLongTestHelper())
register_test('InULongLongPortTest', InPortTest, helper=ULongLongTestHelper())
register_test('InFloatPortTest', InPortTest, helper=FloatTestHelper())
register_test('InDoublePortTest', InPortTest, helper=DoubleTestHelper())

if __name__ == '__main__':
    logging.basicConfig()
    unittest.main()
