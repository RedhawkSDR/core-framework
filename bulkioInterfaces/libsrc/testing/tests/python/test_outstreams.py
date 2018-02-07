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
import math

from omniORB import CORBA
from omniORB.any import to_any

from ossie.cf import CF
from ossie.utils.log4py import logging

import bulkio
from bulkio.bulkioInterfaces import BULKIO

from inport_stubs import *

class OutStreamTest(object):
    def setUp(self):
        self.port = self._createPort()
        self.stub = self._createStub()

        objref = self.stub._this()
        self.port.connectPort(objref, 'test_connection')

    def tearDown(self):
        try:
            self._disconnectPorts()
        except:
            # Ignore disconnection errors
            pass

        self._releaseServants()

    def _createStub(self):
        return self.StubType()

    def _disconnectPorts(self):
        for connection in self.port._get_connections():
            self.port.disconnectPort(connection.connectionId)

    def _releaseServants(self):
        poa = self.stub._default_POA()
        object_id = poa.servant_to_id(self.stub)
        poa.deactivate_object(object_id)

    def _dataLength(self, data):
        return len(data)

    def testBasicWrite(self):
        stream = self.port.createStream('test_basic_write')
        self.failUnless(not self.stub.packets)

        time = bulkio.timestamp.now()
        self._writeSinglePacket(stream, 256, time)
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(256, self._dataLength(self.stub.packets[0].data))
        self.failIf(self.stub.packets[0].EOS)
        self.assertEqual(stream.streamID, self.stub.packets[0].streamID)

        # Check timestamp, but only if it's supported (i.e., not dataXML)
        if hasattr(self.stub.Packet, 'T'):
            self.assertEqual(time, self.stub.packets[0].T, msg='Received incorrect time stamp')

    def testSriFields(self):
        sri = bulkio.sri.create("test_sri")
        sri.xstart = -2.5
        sri.xdelta = 0.125
        sri.xunits = BULKIO.UNITS_FREQUENCY
        sri.subsize = 1024
        sri.ystart = 2.5
        sri.ydelta = 1.0
        sri.yunits = BULKIO.UNITS_TIME
        sri.mode = 1
        sri.blocking = 1
        # TODO: keywords

        # Create a stream from the SRI and compare accessors
        stream = self.port.createStream(sri)
        self.assertEqual(stream.streamID, sri.streamID)
        self.assertEqual(stream.xstart, sri.xstart)
        self.assertEqual(stream.xdelta, sri.xdelta)
        self.assertEqual(stream.xunits, sri.xunits)
        self.assertEqual(stream.subsize, sri.subsize)
        self.assertEqual(stream.ystart, sri.ystart)
        self.assertEqual(stream.ydelta, sri.ydelta)
        self.assertEqual(stream.yunits, sri.yunits)
        self.failUnless(stream.complex)
        self.failUnless(stream.blocking)
        # TODO: keywords

    def testSriUpdate(self):
        # Create initial stream; all changes should be queued up for the first
        # write
        stream = self.port.createStream("test_sri_update")
        xdelta = 1.0 / 1.25e6
        stream.xdelta = xdelta
        stream.blocking = True
        self.failUnless(not self.stub.H)

        # Write data to trigger initial SRI update
        self._writeSinglePacket(stream, 10)
        self.assertEqual(len(self.stub.H), 1)
        self.failUnless(self.stub.H[-1].blocking)
        self.assertEqual(xdelta, self.stub.H[-1].xdelta)

        # Update xdelta; no SRI update should occur
        new_xdelta = 1.0/2.5e6
        stream.xdelta = new_xdelta
        self.assertEqual(len(self.stub.H), 1)
        self.assertEqual(xdelta, self.stub.H[-1].xdelta)

        # Write data to trigger SRI update
        self._writeSinglePacket(stream, 25)
        self.assertEqual(len(self.stub.H), 2)
        self.assertEqual(new_xdelta, self.stub.H[-1].xdelta)

        # Change blocking flag, then trigger an SRI update
        stream.blocking = False
        self.assertEqual(len(self.stub.H), 2)
        self.failUnless(self.stub.H[-1].blocking)
        self._writeSinglePacket(stream, 25)
        self.assertEqual(len(self.stub.H), 3)
        self.failIf(self.stub.H[-1].blocking)

        # Change multiple fields, but only one SRI update should occur (after the
        # next write)
        stream.complex = True
        stream.subsize = 16
        stream.xstart = -math.pi
        stream.xdelta = 2.0 * math.pi / 1024.0
        stream.xunits = BULKIO.UNITS_FREQUENCY
        stream.ydelta = 1024.0 / 1.25e6
        stream.yunits = BULKIO.UNITS_TIME
        self.assertEqual(len(self.stub.H), 3)

        # Trigger SRI update and verify that it matches
        self._writeSinglePacket(stream, 1024)
        self.assertEqual(len(self.stub.H), 4)
        self.failUnless(bulkio.sri.compare(stream.sri(), self.stub.H[-1]))

    def testKeywords(self):
        # TODO
        pass

    def testSendEosOnClose(self):
        stream = self.port.createStream("close_eos")

        self.assertEqual(len(self.stub.H), 0)
        self.assertEqual(len(self.stub.packets), 0)

        self._writeSinglePacket(stream, 16)

        self.assertEqual(len(self.stub.H), 1)
        self.assertEqual(len(self.stub.packets), 1)
        self.failIf(self.stub.packets[-1].EOS)

        stream.close()
        self.assertEqual(len(self.stub.packets), 2)
        self.failUnless(self.stub.packets[-1].EOS)

    def _writeSinglePacket(self, stream, length, time=None):
        if time is None:
            time = bulkio.timestamp.now()
        data = self._createData(length)
        stream.write(data, time)

    def _createData(self, length):
        return range(length)

class OutCharStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InCharPortStub

    def _createPort(self):
        return bulkio.OutCharPort('dataChar_out')

    def _createData(self, length):
        return '\x00'*length

class OutOctetStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InOctetPortStub

    def _createPort(self):
        return bulkio.OutOctetPort('dataOctet_out')

    def _createData(self, length):
        return '\x00'*length

class OutBitStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InBitPortStub

    def _createPort(self):
        return bulkio.OutBitPort('dataBit_out')

    def _createData(self, length):
        data = '\x00' * int((length+7)/8)
        return BULKIO.BitSequence(data, length)

    def _dataLength(self, data):
        return data.bits

class OutShortStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InShortPortStub

    def _createPort(self):
        return bulkio.OutShortPort('dataShort_out')

class OutUShortStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InUshortPortStub

    def _createPort(self):
        return bulkio.OutUShortPort('dataUShort_out')

class OutLongStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InLongPortStub

    def _createPort(self):
        return bulkio.OutLongPort('dataLong_out')

class OutULongStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InUlongPortStub

    def _createPort(self):
        return bulkio.OutULongPort('dataULong_out')

class OutLongLongStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InLongLongPortStub

    def _createPort(self):
        return bulkio.OutLongLongPort('dataLongLong_out')

class OutULongLongStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InUlongLongPortStub

    def _createPort(self):
        return bulkio.OutULongLongPort('dataULongLong_out')

class OutFloatStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InFloatPortStub

    def _createPort(self):
        return bulkio.OutFloatPort('dataFloat_out')

class OutDoubleStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InDoublePortStub

    def _createPort(self):
        return bulkio.OutDoublePort('dataDouble_out')

class OutXMLStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InXMLPortStub

    def _createPort(self):
        return bulkio.OutXMLPort('dataXML_out')

    def _createData(self, length):
        return ' '*length

class OutFileStreamTest(OutStreamTest, unittest.TestCase):
    StubType = InFilePortStub

    def _createPort(self):
        return bulkio.OutFilePort('dataFile_out')

    def _createData(self, length):
        return ' '*length

if __name__ == '__main__':
    logging.basicConfig()
    #logging.getLogger().setLevel(logging.TRACE)

    orb = CORBA.ORB_init()
    root_poa = orb.resolve_initial_references("RootPOA")
    manager = root_poa._get_the_POAManager()
    manager.activate()

    unittest.main()

    orb.shutdown(True)
