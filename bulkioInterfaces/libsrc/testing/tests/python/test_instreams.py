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

from omniORB.any import to_any

from ossie.cf import CF

import bulkio
from bulkio.bulkioInterfaces import BULKIO

from helpers import *

class InStreamTest(object):
    def setUp(self):
        self.port = self.helper.createInPort()

    def tearDown(self):
        pass

    def _pushTestPacket(self, length, time, eos, streamID):
        data = self.helper.createData(length)
        self.helper.pushPacket(self.port, data, time, eos, streamID)

    def testTimestamp(self):
        # Create a new stream and push data with a known timestamp to it
        sri = bulkio.sri.create("time_stamp")
        self.port.pushSRI(sri)
        ts = bulkio.timestamp.create(1520883276.8045831)
        self._pushTestPacket(16, ts, False, sri.streamID)

        # Get the input stream and read the packet as a data block; it should
        # contain exactly 1 timestamp, equal to the one that was pushed
        stream = self.port.getStream("time_stamp")
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)
        timestamps = block.getTimestamps()
        self.assertEqual(1, len(timestamps))
        self.assertEqual(ts, timestamps[0].time)
        self.assertEqual(0, timestamps[0].offset)
        self.assertEqual(False, timestamps[0].synthetic)

        # getStartTime() should always return the first timestamp
        self.assertEqual(ts, block.getStartTime())

    def testGetCurrentStreamEmptyPacket(self):
        # Create a new stream and push some data to it
        sri = bulkio.sri.create("empty_packet")
        self.port.pushSRI(sri)
        self._pushTestPacket(0, bulkio.timestamp.now(), False, sri.streamID)

        # getCurrentStream() should not return any stream
        stream = self.port.getCurrentStream(bulkio.const.NON_BLOCKING)
        self.failUnless(not stream)

    def testGetCurrentStreamEmptyEos(self):
        # Create a new stream and push some data to it
        sri = bulkio.sri.create("empty_eos")
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream and read the first packet
        stream = self.port.getStream("empty_eos")
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, len(block.buffer))
        self.failIf(stream.eos())

        # Push an end-of-stream packet with no data and get the stream again
        self._pushTestPacket(0, bulkio.timestamp.notSet(), True, sri.streamID)
        stream = self.port.getCurrentStream(bulkio.const.NON_BLOCKING)
        self.failIf(not stream)
        block = stream.read()
        self.failUnless(not block)

        # There should be no current stream, because the failed read should have
        # removed it
        next_stream = self.port.getCurrentStream(bulkio.const.NON_BLOCKING)
        self.failUnless(not next_stream)

        # The original stream should report end-of-stream
        self.failUnless(stream.eos())

    def testGetCurrentStreamDataEos(self):
        # Create a new stream and push some data to it
        sri = bulkio.sri.create("empty_eos")
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream and read the first packet
        stream = self.port.getStream("empty_eos")
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, len(block.buffer))
        self.failIf(stream.eos())

        # Push an end-of-stream packet with data and get the stream again
        self._pushTestPacket(1024, bulkio.timestamp.now(), True, sri.streamID)
        stream = self.port.getCurrentStream(bulkio.const.NON_BLOCKING)
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, len(block.buffer))

        # Try to get the current stream again since the end-of-stream has not been
        # checked yet, it should return the existing stream (as with above)
        stream = self.port.getCurrentStream(bulkio.const.NON_BLOCKING)
        self.failIf(not stream)
        block = stream.read()
        self.failUnless(not block)

        # There should be no current stream, because the failed read should have
        # removed it
        next_stream = self.port.getCurrentStream(bulkio.const.NON_BLOCKING)
        self.failUnless(not next_stream)

        # The original stream should report end-of-stream
        self.failUnless(stream.eos())

    def testSriChanges(self):
        stream_id = 'sri_changes'

        # Create a new stream and push some data to it
        sri = bulkio.sri.create(stream_id)
        sri.xstart = 0.0
        sri.xdelta = 1.0
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream and read the first packet
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, len(block.buffer))
        self.failIf(stream.eos())
        self.assertEqual(sri.xdelta, block.xdelta)

        # Change xdelta (based on sample rate of 2.5Msps)
        sri.xdelta = 1.0 / 2.5e6
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, len(block.buffer))
        self.failIf(stream.eos())
        self.failUnless(block.sriChanged)
        flags = bulkio.sri.XDELTA
        self.assertEqual(flags, block.sriChangeFlags, 'SRI change flags incorrect')
        self.assertEqual(sri.xdelta, block.xdelta, 'SRI xdelta incorrect')

        # Add a keyword, change xdelta back and update xstart
        sri.keywords.append(CF.DataType('COL_RF', to_any(101.1e6)))
        sri.xstart = 100.0
        sri.xdelta = 1.0
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, len(block.buffer))
        self.failIf(stream.eos())
        self.failUnless(block.sriChanged)
        flags = bulkio.sri.XSTART | bulkio.sri.XDELTA | bulkio.sri.KEYWORDS
        self.assertEqual(flags, block.sriChangeFlags, 'SRI change flags incorrect')
        self.assertEqual(sri.xstart, block.sri.xstart, 'SRI xstart incorrect')
        self.assertEqual(sri.xdelta, block.sri.xdelta, 'SRI xdelta incorrect')

    def testDisable(self):
        stream_id = "disable"

        # Create a new stream and push some data to it
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(16, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream and read the first packet
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)

        block = stream.read()
        self.failIf(not block)

        # Push a couple more packets
        self._pushTestPacket(16, bulkio.timestamp.now(), False, sri.streamID)
        self._pushTestPacket(16, bulkio.timestamp.now(), False, sri.streamID)
        self.assertEqual(2, self.port.getCurrentQueueDepth())

        # Disable the stream this should drop the existing packets
        stream.disable()
        self.failIf(stream.enabled)
        self.assertEqual(0, self.port.getCurrentQueueDepth(), 'Queued packets for disabled stream were not discarded')

        # Push a couple more packets they should get dropped
        self._pushTestPacket(16, bulkio.timestamp.now(), False, sri.streamID)
        self._pushTestPacket(16, bulkio.timestamp.now(), False, sri.streamID)
        self.assertEqual(0, self.port.getCurrentQueueDepth(), 'New packets for disabled stream were not dropped')

        # Push an end-of-stream packet
        self._pushTestPacket(16, bulkio.timestamp.notSet(), True, sri.streamID)

        # Re-enable the stream and read it should fail with end-of-stream set
        stream.enable()
        block = stream.read()
        self.failUnless(block is None)
        self.failUnless(stream.eos())

class BufferedInStreamTest(InStreamTest):
    def testSizedReadEmptyEos(self):
        stream_id = "read_empty_eos"

        # Create a new stream and push an end-of-stream packet with no data
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(0, bulkio.timestamp.notSet(), True, stream_id)

        # Try to read a single element this should return a null block
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.read(1)
        self.failUnless(not block)
        self.failUnless(stream.eos())

    def testStreamAPIMultiStream(self):
        sri = bulkio.sri.create('my_stream')
        self.port.pushSRI(sri)
        self._pushTestPacket(10, bulkio.timestamp.now(), False, sri.streamID)
        self._pushTestPacket(10, bulkio.timestamp.now(), True, sri.streamID)

        self.port.pushSRI(sri)
        self._pushTestPacket(10, bulkio.timestamp.now(), False, sri.streamID)

        stream = self.port.getStream(sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(10, len(block.buffer))
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(10, len(block.buffer))
        self.assertEqual(stream.eos(), True)

        self._pushTestPacket(10, bulkio.timestamp.now(), True, sri.streamID)

        stream = self.port.getStream(sri.streamID)
        block = stream.read(10)
        self.failIf(not block)
        self.assertEqual(10, len(block.buffer))

    def testSizedTryreadEmptyEos(self):
        stream_id = "tryread_empty_eos"

        # Create a new stream and push an end-of-stream packet with no data
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(0, bulkio.timestamp.notSet(), True, stream_id)

        # Try to read a single element this should return a null block
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.tryread(1)
        self.failUnless(not block)
        self.failUnless(stream.eos())

    def testTryreadPeek(self):
        stream_id = "tryread_peek"

        # Create a new stream and push some data to it
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), True, stream_id)

        # Get the input stream and read the first packet
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.tryread(10000, 0)
        self.assertEqual(1024, len(block.buffer))
        block = stream.read(10000)
        self.assertEqual(1024, len(block.buffer))
        block = stream.read(10000)
        self.failUnless(not block)
        self.failUnless(stream.eos())

    def testReadPeek(self):
        stream_id = "read_peek"

        # Create a new stream and push some data to it
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), True, stream_id)

        # Get the input stream and read the first packet
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.read(10000, 0)
        self.assertEqual(1024, len(block.buffer))
        block = stream.read(10000)
        self.assertEqual(1024, len(block.buffer))
        block = stream.read(10000)
        self.failUnless(not block)
        self.failUnless(stream.eos())

    def testReadPartial(self):
        stream_id = "read_partial"

        # Create a new stream and push some data to it
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), True, stream_id)

        # Get the input stream and read the first packet
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.read(10000, 2000)
        self.assertEqual(1024, len(block.buffer))
        block = stream.read(10000)
        self.failUnless(not block)

    def testReadMultiplePackets(self):
        sri = bulkio.sri.create('multiple_packets')
        self.port.pushSRI(sri)
        for _ in xrange(4):
            self._pushTestPacket(100, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream
        stream = self.port.getStream(sri.streamID)
        self.failIf(not stream)

        # Read a block that spans two packets but does not consume the entire
        # second packet
        block = stream.read(150)
        self.failIf(not block)
        self.assertEqual(150, len(block.buffer))

        # Read a block that spans the remainder of the prior packet, an entire
        # middle packet, and part of the next
        block = stream.read(200)
        self.failIf(not block)
        self.assertEqual(200, len(block.buffer))

    def testReadSubPacket(self):
        sri = bulkio.sri.create('sub_packet')
        self.port.pushSRI(sri)
        self._pushTestPacket(400, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream and read the packet
        stream = self.port.getStream(sri.streamID)
        self.failIf(not stream)

        # Read half
        block = stream.read(200)
        self.failIf(not block)
        self.assertEqual(200, len(block.buffer))

        # Read a smaller packet
        block = stream.read(100)
        self.failIf(not block)
        self.assertEqual(100, len(block.buffer))

        # Read the remainder of the packet
        block = stream.tryread()
        self.failIf(not block)
        self.assertEqual(100, len(block.buffer))

    def testReadTimestamps(self):
        # Create a new stream and push several packets with known timestamps
        sri = bulkio.sri.create('read_timestamps')
        sri.xdelta = 0.0625;
        self.port.pushSRI(sri)

        # Push packets of size 32, which should advance the time by exactly 2
        # seconds each
        ts = bulkio.timestamp.create(4000.0, 0.5)
        self._pushTestPacket(32, ts, False, sri.streamID)
        self._pushTestPacket(32, ts+2.0, False, sri.streamID)
        self._pushTestPacket(32, ts+4.0, False, sri.streamID)
        self._pushTestPacket(32, ts+6.0, False, sri.streamID)

        # Get the input stream and read several packets as one block, enough to
        # bisect the third packet
        stream = self.port.getStream(sri.streamID)
        self.failIf(not stream)
        block = stream.read(70)
        self.failIf(not block)
        self.assertEqual(70, len(block.buffer))

        # There should be 3 timestamps, all non-synthetic
        timestamps = block.getTimestamps()
        self.assertEqual(3, len(timestamps))
        self.assertEqual(ts, timestamps[0].time)
        self.assertEqual(0, timestamps[0].offset)
        self.assertEqual(False, timestamps[0].synthetic)
        self.assertEqual(timestamps[0].time, block.getStartTime(), "getStartTime() doesn't match first timestamp")
        self.assertEqual(ts+2.0, timestamps[1].time)
        self.assertEqual(32, timestamps[1].offset)
        self.assertEqual(False, timestamps[1].synthetic)
        self.assertEqual(ts+4.0, timestamps[2].time)
        self.assertEqual(64, timestamps[2].offset)
        self.assertEqual(False, timestamps[2].synthetic)

        # Read the remaining packet and a half; the first timestamp should be
        # synthetic
        block = stream.read(58)
        self.failIf(not block)
        self.assertEqual(58, len(block.buffer))
        timestamps = block.getTimestamps()
        self.assertEqual(2, len(timestamps))
        self.assertEqual(True, timestamps[0].synthetic, "First timestamp should by synthesized")
        self.assertEqual(ts+4.375, timestamps[0].time, "Synthesized timestamp is incorrect")
        self.assertEqual(0, timestamps[0].offset)
        self.assertEqual(timestamps[0].time, block.getStartTime(), "getStartTime() doesn't match first timestamp")
        self.assertEqual(ts+6.0, timestamps[1].time)
        self.assertEqual(26, timestamps[1].offset)
        self.assertEqual(False, timestamps[1].synthetic)

    def testDisableDiscard(self):
        stream_id = "disable_discard"

        # Create a new stream and push a couple of packets to it
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)
        self.assertEqual(1, self.port.getCurrentQueueDepth())

        # Get the input stream and read half of the first packet
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.read(512)
        self.failIf(not block)

        # There should be no packets in the port's queue, but a peek should
        # still still return valid data; the data is not consumed so that we
        # can be sure that it's discarded later
        self.assertEqual(0, self.port.getCurrentQueueDepth())
        block = stream.read(512, 0)
        self.failIf(not block)

        # Disable the stream--this should discard
        stream.disable()

        # Re-enable the stream and try to read
        stream.enable()
        block = stream.tryread(512)
        self.failUnless(not block)

class NumericInStreamTest(BufferedInStreamTest):
    def testSriModeChanges(self):
        stream_id = "sri_mode_changes"

        # Create a new stream and push some data to it
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)
        self._pushTestPacket(100, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream and read the first packet
        stream = self.port.getStream(stream_id)
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)

        # First block from a new stream reports SRI change
        self.failUnless(block.sriChanged)

        # Change the mode to complex and push more data
        sri.mode = 1
        self.port.pushSRI(sri)
        self._pushTestPacket(200, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.failUnless(block.complex)
        self.failUnless(block.sriChanged)
        self.failUnless(block.sriChangeFlags & bulkio.sri.MODE)

        # Next push should report no SRI changes
        self._pushTestPacket(200, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.failUnless(block.complex)
        self.failIf(block.sriChanged)

        # Change back to scalar
        sri.mode = 0
        self.port.pushSRI(sri)
        self._pushTestPacket(100, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.failIf(block.complex)
        self.failUnless(block.sriChanged)
        self.failUnless(block.sriChangeFlags & bulkio.sri.MODE)

    def testReadComplex(self):
        sri = bulkio.sri.create('read_complex')
        sri.mode = 1
        self.port.pushSRI(sri)
        self._pushTestPacket(128, bulkio.timestamp.now(), False, sri.streamID)

        # Get the input stream and read the packet
        stream = self.port.getStream(sri.streamID)
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)

        self.failUnless(block.complex)
        self.assertEqual(64, block.cxsize)

    def testReadTimestampsComplex(self):
        # Create a new complex stream and push several packets with known
        # timestamps
        sri = bulkio.sri.create('read_timestamps_cx')
        sri.mode = 1
        sri.xdelta = 0.125
        self.port.pushSRI(sri)

        # Push 8 complex values (16 real), which should advance the time by
        # exactly 1 second each time
        ts = bulkio.timestamp.create(100.0, 0.0)
        self._pushTestPacket(16, ts, False, sri.streamID)
        self._pushTestPacket(16, ts+1.0, False, sri.streamID)
        self._pushTestPacket(16, ts+2.0, False, sri.streamID)
        self._pushTestPacket(16, ts+3.0, False, sri.streamID)

        # Get the input stream and read several packets as one block, enough to
        # bisect the third packet
        stream = self.port.getStream(sri.streamID)
        self.failIf(not stream)
        block = stream.read(20)
        self.failIf(not block)
        self.failUnless(block.complex)
        self.assertEqual(20, block.cxsize)

        # There should be 3 timestamps, all non-synthetic, with sample offsets
        # based on the complex type
        timestamps = block.getTimestamps()
        self.assertEqual(3, len(timestamps))
        self.assertEqual(ts, timestamps[0].time)
        self.assertEqual(0, timestamps[0].offset)
        self.assertEqual(False, timestamps[0].synthetic)
        self.assertEqual(timestamps[0].time, block.getStartTime(), "getStartTime() doesn't match first timestamp")
        self.assertEqual(ts+1.0, timestamps[1].time)
        self.assertEqual(8, timestamps[1].offset)
        self.assertEqual(False, timestamps[1].synthetic)
        self.assertEqual(ts+2.0, timestamps[2].time)
        self.assertEqual(16, timestamps[2].offset)
        self.assertEqual(False, timestamps[2].synthetic)

        # Read the remaining packet and a half; the first timestamp should be
        # synthetic
        block = stream.read(12)
        self.failIf(not block)
        self.failUnless(block.complex)
        self.assertEqual(12, block.cxsize)
        timestamps = block.getTimestamps()
        self.assertEqual(2, len(timestamps))
        self.assertEqual(True, timestamps[0].synthetic, "First timestamp should by synthesized")
        self.assertEqual(ts+2.5, timestamps[0].time, "Synthesized timestamp is incorrect")
        self.assertEqual(0, timestamps[0].offset)
        self.assertEqual(timestamps[0].time, block.getStartTime(), "getStartTime() doesn't match first timestamp")
        self.assertEqual(ts+3.0, timestamps[1].time)
        self.assertEqual(4, timestamps[1].offset)
        self.assertEqual(False, timestamps[1].synthetic)


class InXMLStreamTest(InStreamTest, unittest.TestCase):
    helper = XMLTestHelper()

    def testTimestamp(self):
        # Override for XML ports, which do not pass timestamp information
        # Create a new stream and push some data to it
        sri = bulkio.sri.create("time_stamp")
        self.port.pushSRI(sri)
        self._pushTestPacket(16, None, False, sri.streamID)

        # Get the input stream and read the packet as a data block; it should
        # not contain any timestamps
        stream = self.port.getStream("time_stamp")
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)
        timestamps = block.getTimestamps()
        self.assertEqual(0, len(timestamps))

        # Calling getStartTime() will throw an IndexError

def register_test(name, testbase, **kwargs):
    globals()[name] = type(name, (testbase, unittest.TestCase), kwargs)

register_test('InBitStreamTest', BufferedInStreamTest, helper=BitTestHelper())
register_test('InFileStreamTest', InStreamTest, helper=FileTestHelper())
register_test('InCharStreamTest', NumericInStreamTest, helper=CharTestHelper())
register_test('InOctetStreamTest', NumericInStreamTest, helper=OctetTestHelper())
register_test('InShortStreamTest', NumericInStreamTest, helper=ShortTestHelper())
register_test('InUShortStreamTest', NumericInStreamTest, helper=UShortTestHelper())
register_test('InLongStreamTest', NumericInStreamTest, helper=LongTestHelper())
register_test('InULongStreamTest', NumericInStreamTest, helper=ULongTestHelper())
register_test('InLongLongStreamTest', NumericInStreamTest, helper=LongLongTestHelper())
register_test('InULongLongStreamTest', NumericInStreamTest, helper=ULongLongTestHelper())
register_test('InFloatStreamTest', NumericInStreamTest, helper=FloatTestHelper())
register_test('InDoubleStreamTest', NumericInStreamTest, helper=DoubleTestHelper())

if __name__ == '__main__':
    import runtests
    runtests.main()
