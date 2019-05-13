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

import copy
import unittest
import math

from omniORB import CORBA

from ossie import properties

import bulkio
from bulkio.bulkioInterfaces import BULKIO

from helpers import *

class OutStreamTest(object):
    def setUp(self):
        self.port = self.helper.createOutPort()
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
        return self.helper.createInStub()

    def _disconnectPorts(self):
        for connection in self.port._get_connections():
            self.port.disconnectPort(connection.connectionId)

    def _releaseServants(self):
        poa = self.stub._default_POA()
        object_id = poa.servant_to_id(self.stub)
        poa.deactivate_object(object_id)

    def testBasicWrite(self):
        stream = self.port.createStream('test_basic_write')
        self.failUnless(not self.stub.packets)

        time = bulkio.timestamp.now()
        self._writeSinglePacket(stream, 256, time)
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(256, self.helper.packetLength(self.stub.packets[0].data))
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
        sri.keywords = properties.props_from_dict({'string':'value', 'number':100})

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
        self.assertEqual(len(sri.keywords), len(stream.keywords))
        self.assertEqual('value', stream.getKeyword('string'))
        self.assertEqual(100, stream.getKeyword('number'))

    def testSriNoChange(self):
        """Test that the SRI is not pushed when not needed.

        Calls to set attributes of the SRI, or set or erase SRI keywords should
        only cause the SRI to be pushed if a change was made.
        """
        expected_H_size = 0

        stream = self.port.createStream('test_sri_no_change')
        sri = copy.deepcopy(stream.sri)
        sri.streamID = 'changed_sri'
        msg = 'SRI update occurred without call to write data.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        expected_H_size += 1
        self._writeSinglePacket(stream, 10)
        msg = 'No SRI update before first data write.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.sri = sri
        self._writeSinglePacket(stream, 10)
        msg = 'Setting sri with no sri change (except streamID) caused SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        sri.xdelta *= 2
        stream.sri = sri
        expected_H_size += 1
        self._writeSinglePacket(stream, 10)
        msg = 'Setting sri with changed sri (other than streamID) failed to cause SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.xdelta *= 2
        expected_H_size += 1
        self._writeSinglePacket(stream, 10)
        msg = 'Setting stream.xdelta to changed value failed to cause SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.xdelta = stream.xdelta
        self._writeSinglePacket(stream, 10)
        msg = 'Setting stream.xdelta to unchanged value caused SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.keywords = properties.props_from_dict({'string':'value', 'number':100})
        expected_H_size += 1
        self._writeSinglePacket(stream, 10)
        msg = 'Setting stream.keywords to new value failed to cause SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.keywords = properties.props_from_dict({'string':'value', 'number':100})
        self._writeSinglePacket(stream, 10)
        msg = 'Setting stream.keywords to unchanged value caused SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.setKeyword('string', 'changed-value')
        expected_H_size += 1
        self._writeSinglePacket(stream, 10)
        msg = 'Calling stream.setKeyword() with new value failed to cause SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.setKeyword('string', 'changed-value')
        self._writeSinglePacket(stream, 10)
        msg = 'Calling stream.setKeyword() with unchanged value caused SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.eraseKeyword('string')
        expected_H_size += 1
        self._writeSinglePacket(stream, 10)
        msg = 'Calling stream.eraseKeyword(<existing key>) failed to cause SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

        stream.eraseKeyword('string')
        self._writeSinglePacket(stream, 10)
        msg = 'Calling stream.eraseKeyword(<no such key>) caused SRI update.'
        self.assertEqual(len(self.stub.H), expected_H_size, msg)

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
        self.failUnless(bulkio.sri.compare(stream.sri, self.stub.H[-1]))

    def testSriReplace(self):
        # Create initial stream
        stream = self.port.createStream("test_sri_replace")

        # Create a new SRI with a different stream ID
        new_sri = bulkio.sri.create("modified_sri")
        new_sri.mode = 1
        new_sri.blocking = 1
        new_sri.subsize = 16
        new_sri.xstart = -math.pi
        new_sri.xdelta = 2.0 * math.pi / 1024.0
        new_sri.xunits = BULKIO.UNITS_FREQUENCY
        new_sri.ydelta = 1024.0 / 1.25e6
        new_sri.yunits = BULKIO.UNITS_TIME

        # Replace the SRI and ensure that everything *except* the streamID has
        # changed
        stream.sri = new_sri
        self.assertEqual(stream.streamID, "test_sri_replace")
        self.assertEqual(stream.xstart, new_sri.xstart)
        self.assertEqual(stream.xdelta, new_sri.xdelta)
        self.assertEqual(stream.xunits, new_sri.xunits)
        self.assertEqual(stream.subsize, new_sri.subsize)
        self.assertEqual(stream.ystart, new_sri.ystart)
        self.assertEqual(stream.ydelta, new_sri.ydelta)
        self.assertEqual(stream.yunits, new_sri.yunits)
        self.failUnless(stream.complex)
        self.failUnless(stream.blocking)

    def testKeywords(self):
        stream = self.port.createStream("test_keywords")
        self._writeSinglePacket(stream, 1)
        self.assertEqual(1, len(self.stub.H))

        # Set/get keywords
        stream.setKeyword('integer', 250)
        stream.setKeyword('string', "value")
        stream.setKeyword('double', 101.1e6)
        stream.setKeyword('boolean', False)
        self.assertEqual(250, stream.getKeyword('integer'))
        self.assertEqual('value', stream.getKeyword('string'))
        self.assertEqual(101.1e6, stream.getKeyword('double'))
        self.assertEqual(False, stream.getKeyword('boolean'))

        # Set with a specific type
        stream.setKeyword('float', -1.25, 'float')
        self.assertEqual(-1.25, stream.getKeyword('float'))
        any_value = stream.keywords[-1].value
        self.assertEqual(CORBA.TC_float, any_value.typecode())

        # Erase and check for presence of keywords
        stream.eraseKeyword('string')
        self.failUnless(stream.hasKeyword('integer'))
        self.failIf(stream.hasKeyword('string'))
        self.failUnless(stream.hasKeyword('double'))
        self.failUnless(stream.hasKeyword('boolean'))

        # Write a packet to trigger an SRI update
        self.assertEqual(1, len(self.stub.H))
        self._writeSinglePacket(stream, 1)
        self.assertEqual(2, len(self.stub.H))

        keywords = properties.props_to_dict(self.stub.H[-1].keywords)
        self.assertEqual(len(stream.keywords), len(keywords))
        for key, value in keywords.iteritems():
            self.assertEqual(stream.getKeyword(key), value)

        # Replace keywords with a new set
        stream.keywords = properties.props_from_dict({'COL_RF': 100.0e6, 'CHAN_RF': 101.1e6})
        self.assertEqual(2, len(stream.keywords))
        self.assertEqual(100.0e6, stream.getKeyword('COL_RF'))
        self.assertEqual(101.1e6, stream.getKeyword('CHAN_RF'))

        # Trigger another SRI update
        self.assertEqual(2, len(self.stub.H))
        self._writeSinglePacket(stream, 1)
        self.assertEqual(3, len(self.stub.H))

        keywords = properties.props_to_dict(self.stub.H[-1].keywords)
        self.assertEqual(len(stream.keywords), len(keywords))
        for key, value in keywords.iteritems():
            self.assertEqual(stream.getKeyword(key), value)

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
        data = self.helper.createStreamData(length)
        stream.write(data, time)

class BufferedOutStreamTest(OutStreamTest):
    def testBufferedWrite(self):
        # Initial state is unbuffered; turn on buffering
        stream = self.port.createStream("test_buffered_write")
        self.assertEqual(0, stream.bufferSize())
        stream.setBufferSize(128)
        self.assertEqual(128, stream.bufferSize())
        self.assertEqual(0, len(self.stub.packets))

        # First write is below the buffer size
        data = self.helper.createStreamData(48)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(0, len(self.stub.packets))

        # The second write is still below the buffer size
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(0, len(self.stub.packets))

        # The third write goes beyond the buffer size and should trigger a push,
        # but only up to the buffer size (48*3 == 144)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(stream.bufferSize(), self.helper.packetLength(self.stub.packets[-1].data))

        # There should now be 16 samples in the queue; writing another 48 should
        # not trigger a push
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(1, len(self.stub.packets))

        # Flush the stream and make sure we get as many samples as expected
        stream.flush()
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(64, self.helper.packetLength(self.stub.packets[-1].data))

        # Disable buffering; push should happen immediately
        stream.setBufferSize(0)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(3, len(self.stub.packets))
        self.assertEqual(len(data), self.helper.packetLength(self.stub.packets[-1].data))

    def testWriteSkipBuffer(self):
        # Turn on buffering
        stream = self.port.createStream("test_skip_buffer")
        stream.setBufferSize(100)

        # With an empty queue, large write should go right through
        data = self.helper.createStreamData(256)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(len(data), self.helper.packetLength(self.stub.packets[-1].data))

        # Queue up a bit of data
        data = self.helper.createStreamData(16)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(1, len(self.stub.packets))

        # With queued data, the large write should get broken up into a buffer-
        # sized packet
        data = self.helper.createStreamData(128)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(stream.bufferSize(), self.helper.packetLength(self.stub.packets[-1].data))

    def testFlush(self):
        # Turn on buffering
        stream = self.port.createStream("test_flush")
        stream.setBufferSize(64)

        # Queue data (should not flush)
        data = self.helper.createStreamData(48)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(0, len(self.stub.H))
        self.assertEqual(0, len(self.stub.packets))

        # Make sure flush sends a packet
        stream.flush()
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(len(data), self.helper.packetLength(self.stub.packets[-1].data))
        self.assertEqual(False, self.stub.packets[-1].EOS)

    def testFlushOnClose(self):
        stream = self.port.createStream("test_flush_close")
        stream.setBufferSize(64)

        # Queue data (should not flush)
        data = self.helper.createStreamData(48)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(0, len(self.stub.H))
        self.assertEqual(0, len(self.stub.packets))

        # Close the stream; should cause a flush
        stream.close()
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(len(data), self.helper.packetLength(self.stub.packets[-1].data))
        self.assertEqual(True, self.stub.packets[-1].EOS)

    def testFlushOnSriChange(self):
        # Start with known values for important stream metadata
        stream = self.port.createStream("test_flush_sri")
        stream.setBufferSize(64)
        stream.xdelta = 0.125
        stream.complex = False
        stream.blocking = False
        stream.subsize = 0

        # Queue data (should not flush)
        data = self.helper.createStreamData(24)
        stream.write(data, bulkio.timestamp.now())

        # Change the xdelta to cause a flush; the received data should be using
        # the old xdelta
        self.assertEqual(0, len(self.stub.packets))
        stream.xdelta = 0.25
        self.assertEqual(1, len(self.stub.packets), "xdelta change did not flush stream")
        self.assertEqual(0.125, self.stub.H[-1].xdelta)

        # Queue more data (should not flush)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, len(self.stub.packets))

        # Change the mode to complex to cause a flush; the mode shouldn't
        # change yet, but xdelta should be up-to-date now
        stream.complex = True
        self.assertEqual(2, len(self.stub.packets), "Complex mode change did not flush stream")
        self.assertEqual(0, self.stub.H[-1].mode)
        self.assertEqual(stream.xdelta, self.stub.H[-1].xdelta)

        # Queue more data (should not flush)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(2, len(self.stub.H))
        self.assertEqual(2, len(self.stub.packets))

        # Change the blocking mode to cause a flush; the blocking flag
        # shouldn't change yet, but mode should be up-to-date now
        stream.blocking = True
        self.assertEqual(3, len(self.stub.packets), "Blocking change did not flush stream")
        self.assertEqual(0, self.stub.H[-1].blocking)
        self.assertNotEqual(0, self.stub.H[-1].mode)

        # Queue more data (should not flush)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(3, len(self.stub.H))
        self.assertEqual(3, len(self.stub.packets))

        # Change the subsize to cause a flush; the subsize shouldn't change
        # yet, but blocking should be up-to-date now
        stream.subsize = 16
        self.assertEqual(4, len(self.stub.packets), "Subsize change did not flush stream")
        self.assertEqual(0, self.stub.H[-1].subsize)
        self.assertNotEqual(0, self.stub.H[-1].blocking)

    def testFlushOnBufferSizeChange(self):
        stream = self.port.createStream("test_flush_buffer_size")
        stream.setBufferSize(64)

        # Queue data (should not flush)
        data = self.helper.createStreamData(48)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(0, len(self.stub.packets))

        # Reduce the buffer size smaller than the current queue, should trigger
        # a flush
        stream.setBufferSize(32)
        self.assertEqual(1, len(self.stub.packets), "Reducing buffer size below queue size did not flush")

        # Reduce the buffer size again, but not down to the queue size, should
        # not trigger a flush
        data = self.helper.createStreamData(16)
        stream.write(data, bulkio.timestamp.now())
        stream.setBufferSize(24)
        self.assertEqual(1, len(self.stub.packets), "Reducing buffer size above queue size flushed")

        # Boundary condition: exact size
        stream.setBufferSize(16)
        self.assertEqual(2, len(self.stub.packets), "Reducing buffer size to exact size did not flush")

        # Increasing the buffer size should not trigger a flush
        data = self.helper.createStreamData(8)
        stream.write(data, bulkio.timestamp.now())
        stream.setBufferSize(128)
        self.assertEqual(2, len(self.stub.packets), "Increasing buffer size flushed")

        # Disabling buffering must flush
        stream.setBufferSize(0)
        self.assertEqual(3, len(self.stub.packets), "Disabling buffering did not flush")

class NumericOutStreamTest(BufferedOutStreamTest):
    def testWriteComplex(self):
        stream = self.port.createStream("test_write_complex")
        stream.complex = True

        # Write a list of complex values, which should get turned into a list
        # of real values that is twice as long
        data = [complex(ii,0) for ii in xrange(100)]
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(1, len(self.stub.packets))
        result = self.helper.unpack(self.stub.packets[-1].data)
        self.assertEqual(200, len(result))
        self.assertEqual([ii.real for ii in data], result[::2])
        self.assertEqual([ii.imag for ii in data], result[1::2])

        # Write a list of real values, each of which is interpreted as a
        # complex value (with an imaginary component of 0)
        data = range(100)
        stream.write(data, bulkio.timestamp.now())
        self.assertEqual(2, len(self.stub.packets))
        result = self.helper.unpack(self.stub.packets[-1].data)
        self.assertEqual(200, len(result))
        self.assertEqual(data, result[::2])
        self.assertEqual([0] * 100, result[1::2])

        # Write pre-interleaved data; no conversion should occur
        data = self.helper.pack(range(100))
        stream.write(data, bulkio.timestamp.now(), interleaved=True)
        self.assertEqual(3, len(self.stub.packets))
        self.assertEqual(100, len(self.stub.packets[-1].data))
        self.assertEqual(data, self.stub.packets[-1].data)


class OutXMLStreamTest(OutStreamTest, unittest.TestCase):
    helper = XMLTestHelper()

    def _writeSinglePacket(self, stream, length, time=None):
        data = self.helper.createStreamData(length)
        stream.write(data)

class OutBitStreamTest(BufferedOutStreamTest, unittest.TestCase):
    helper = BitTestHelper()

    def testWriteLiteral(self):
        stream = self.port.createStream("test_write_literal")

        literal = '101101011101011010101'
        stream.write(literal, bulkio.timestamp.now())

        self.assertEqual(1, len(self.stub.packets))
        result = self.helper.unpack(self.stub.packets[-1].data)
        self.assertEqual(literal, result)

def register_test(name, testbase, **kwargs):
    globals()[name] = type(name, (testbase, unittest.TestCase), kwargs)

register_test('OutFileStreamTest', OutStreamTest, helper=FileTestHelper())
register_test('OutCharStreamTest', NumericOutStreamTest, helper=CharTestHelper())
register_test('OutOctetStreamTest', NumericOutStreamTest, helper=OctetTestHelper())
register_test('OutShortStreamTest', NumericOutStreamTest, helper=ShortTestHelper())
register_test('OutUShortStreamTest', NumericOutStreamTest, helper=UShortTestHelper())
register_test('OutLongStreamTest', NumericOutStreamTest, helper=LongTestHelper())
register_test('OutULongStreamTest', NumericOutStreamTest, helper=ULongTestHelper())
register_test('OutLongLongStreamTest', NumericOutStreamTest, helper=LongLongTestHelper())
register_test('OutULongLongStreamTest', NumericOutStreamTest, helper=ULongLongTestHelper())
register_test('OutFloatStreamTest', NumericOutStreamTest, helper=FloatTestHelper())
register_test('OutDoubleStreamTest', NumericOutStreamTest, helper=DoubleTestHelper())

if __name__ == '__main__':
    import runtests
    runtests.main()
