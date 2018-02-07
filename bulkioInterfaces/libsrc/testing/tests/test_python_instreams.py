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

class InStreamTest(object):
    def setUp(self):
        self.port = self._createPort()

    def tearDown(self):
        pass

    def _pushTestPacket(self, length, time, eos, streamID):
        self.port.pushPacket(range(length), time, eos, streamID)

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
        self.assertEqual(1024, block.size())
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
        self.assertEqual(1024, block.size())
        self.failIf(stream.eos())

        # Push an end-of-stream packet with data and get the stream again
        self._pushTestPacket(1024, bulkio.timestamp.now(), True, sri.streamID)
        stream = self.port.getCurrentStream(bulkio.const.NON_BLOCKING)
        self.failIf(not stream)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, block.size())

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
        self.assertEqual(1024, block.size())
        self.failIf(stream.eos())
        self.assertEqual(sri.xdelta, block.sri().xdelta)

        # Change xdelta (based on sample rate of 2.5Msps)
        sri.xdelta = 1.0 / 2.5e6
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, block.size())
        self.failIf(stream.eos())
        self.failUnless(block.sriChanged())
        flags = bulkio.sri.XDELTA
        self.assertEqual(flags, block.sriChangeFlags, 'SRI change flags incorrect')
        self.assertEqual(sri.xdelta, block.sri().xdelta, 'SRI xdelta incorrect')

        # Add a keyword, change xdelta back and update xstart
        sri.keywords.append(CF.DataType('COL_RF', to_any(101.1e6)))
        sri.xstart = 100.0
        sri.xdelta = 1.0
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.assertEqual(1024, block.size())
        self.failIf(stream.eos())
        self.failUnless(block.sriChanged())
        flags = bulkio.sri.XSTART | bulkio.sri.XDELTA | bulkio.sri.KEYWORDS
        self.assertEqual(flags, block.sriChangeFlags, 'SRI change flags incorrect')
        self.assertEqual(sri.xstart, block.sri().xstart, 'SRI xstart incorrect')
        self.assertEqual(sri.xdelta, block.sri().xdelta, 'SRI xdelta incorrect')

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
        self.failIf(stream.enabled())
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
        self.assertEqual(1024, block.size())
        block = stream.read(10000)
        self.assertEqual(1024, block.size())
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
        self.assertEqual(1024, block.size())
        block = stream.read(10000)
        self.assertEqual(1024, block.size())
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
        self.assertEqual(1024, block.size())
        block = stream.read(10000)
        self.failUnless(not block)

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
        self.failUnless(block.sriChanged())

        # Change the mode to complex and push more data
        sri.mode = 1
        self.port.pushSRI(sri)
        self._pushTestPacket(200, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.failUnless(block.complex())
        self.failUnless(block.sriChanged())
        self.failUnless(block.sriChangeFlags & bulkio.sri.MODE)

        # Next push should report no SRI changes
        self._pushTestPacket(200, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.failUnless(block.complex())
        self.failIf(block.sriChanged())

        # Change back to scalar
        sri.mode = 0
        self.port.pushSRI(sri)
        self._pushTestPacket(100, bulkio.timestamp.now(), False, sri.streamID)
        block = stream.read()
        self.failIf(not block)
        self.failIf(block.complex())
        self.failUnless(block.sriChanged())
        self.failUnless(block.sriChangeFlags & bulkio.sri.MODE)


class InFileStreamTest(InStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InFilePort('dataFile_in')

    def _pushTestPacket(self, length, time, eos, streamID):
        data = ' '*length
        self.port.pushPacket(data, time, eos, streamID)

class InXMLStreamTest(InStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InXMLPort('dataXML_in')

    def _pushTestPacket(self, length, time, eos, streamID):
        data = ' '*length
        self.port.pushPacket(data, eos, streamID)

class InBitStreamTest(InStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InBitPort('dataBit_in')

    def _pushTestPacket(self, length, time, eos, streamID):
        data = '\x00' * int((length + 7)/8)
        data = BULKIO.BitSequence(data, length)
        self.port.pushPacket(data, time, eos, streamID)

class InCharStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InCharPort('dataChar_in')

class InOctetStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InOctetPort('dataOctet_in')

class InShortStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InShortPort('dataShort_in')

class InUShortStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InUShortPort('dataUShort_in')

class InLongStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InLongPort('dataLong_in')

class InULongStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InULongPort('dataULong_in')

class InLongLongStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InLongLongPort('dataLongLong_in')

class InULongLongStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InULongLongPort('dataULongLong_in')

class InFloatStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InFloatPort('dataFloat_in')

class InDoubleStreamTest(NumericInStreamTest, unittest.TestCase):
    def _createPort(self):
        return bulkio.InDoublePort('dataDouble_in')

if __name__ == '__main__':
    unittest.main()
