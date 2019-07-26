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
import threading
import time

from ossie.utils.log4py import logging

import bulkio
from bulkio.bulkioInterfaces import BULKIO

from helpers import *

class SriListener(object):
    def __init__(self):
        self.sri = None

    def __call__(self, sri):
        self.sri = sri

    def reset(self):
        self.sri = None

class InPortTest(object):
    def setUp(self):
        self.port = self.helper.createInPort()
        self.port.startPort()

    def tearDown(self):
        pass

    def testBasicAPI(self):
        ##
        ## test bulkio base class standalone
        ##
        ps = self.port._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        s = self.port._get_state()
        self.assertNotEqual(s,None,"Cannot get Port State")
        self.assertEqual(s, BULKIO.IDLE,"Invalid Port State")

        streams = self.port._get_activeSRIs()
        self.assertNotEqual(streams,None,"Cannot get Streams List")

        qed = self.port.getMaxQueueDepth()
        self.assertEqual(qed,100,"Get Stream Depth Failed")

        self.port.setMaxQueueDepth(22)
        qed = self.port.getMaxQueueDepth()
        self.assertEqual(qed,22,"Set/Get Stream Depth Failed")

    def testGetPacket(self):
        sri = bulkio.sri.create('test_get_packet')
        self.port.pushSRI(sri)

        ts = bulkio.timestamp.now()
        self._pushTestPacket(50, ts, False, sri.streamID)

        # Check result of getPacket
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.failIf(packet.dataBuffer is None, 'packet.dataBuffer is empty')
        self.assertEqual(50, len(packet.dataBuffer))
        if isinstance(self.port, bulkio.InXMLPort):
            # XML does not use timestamp
            self.failUnless(packet.T is None)
        else:
            self.assertEqual(ts, packet.T, 'packet.T is incorrect')
        self.assertEqual(packet.EOS, False, 'packet.EOS is incorrect')
        self.assertEqual(packet.streamID, sri.streamID, 'packet.streamID is incorrect')
        self.failUnless(bulkio.sri.compare(packet.SRI, sri), 'packet.SRI is incorrect')
        self.failIf(packet.sriChanged is None, 'packet.sriChanged is incorrect')
        self.assertEqual(packet.inputQueueFlushed, False, 'packet.inputQueueFlushed is incorrect')

        # Check backwards-compatibility for tuple offsets
        self.assertEqual(7, len(packet), 'packet should be a tuple of length 7')
        self.assertEqual(packet.dataBuffer, packet[bulkio.InPort.DATA_BUFFER], 'packet[DATA_BUFFER] mismatch')
        self.assertEqual(packet.T, packet[bulkio.InPort.TIME_STAMP], 'packet[TIME_STAMP] mismatch')
        self.assertEqual(packet.EOS, packet[bulkio.InPort.END_OF_STREAM], 'packet[END_OF_STREAM] mismatch')
        self.assertEqual(packet.streamID, packet[bulkio.InPort.STREAM_ID], 'packet[STREAM_ID] mismatch')
        self.assertEqual(packet.SRI, packet[bulkio.InPort.SRI], 'packet[SRI] mismatch')
        self.assertEqual(packet.sriChanged, packet[bulkio.InPort.SRI_CHG], 'packet[SRI_CHG] mismatch')
        self.assertEqual(packet.inputQueueFlushed, packet[bulkio.InPort.QUEUE_FLUSH], 'packet[QUEUE_FLUSH] mismatch')

        # No packet, all fields should be None
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failUnless(packet.dataBuffer is None, 'packet.dataBuffer should be None')
        self.failUnless(packet.T is None, 'packet.T should be None')
        self.failUnless(packet.EOS is None, 'packet.EOS should be None')
        self.failUnless(packet.streamID is None, 'packet.streamID should be None')
        self.failUnless(packet.SRI is None, 'packet.SRI should be None')
        self.failUnless(packet.sriChanged is None, 'packet.sriChanged should be None')
        self.failUnless(packet.inputQueueFlushed is None, 'packet.inputQueueFlushed should be None')

        # Change mode to complex and push another packet with EOS set
        sri.mode = 1
        self.port.pushSRI(sri)
        self._pushTestPacket(100, ts, True, sri.streamID)
        packet = self.port.getPacket()
        self.assertEqual(100, len(packet.dataBuffer))
        self.assertEqual(True, packet.EOS, 'packet.EOS should be True')
        self.assertEqual(True, packet.sriChanged, 'packet.sriChanged should be True')
        self.assertEqual(1, packet.SRI.mode, 'packet.SRI should have complex mode')

    def testSriChanged(self):
        """
        Tests that SRI changes are reported correctly from getPacket().
        """
        # Create a default SRI and push it
        sri = bulkio.sri.create('sri_changed')
        self.port.pushSRI(sri)

        # SRI should report changed for first packet
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet.dataBuffer is None)
        self.failUnless(packet.sriChanged)

        # No SRI change for second packet
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet.dataBuffer is None)
        self.failIf(packet.sriChanged)

        # Reduce the queue size so we can force a flush
        self.port.setMaxQueueDepth(2)

        # Push a packet, change the SRI, and push two more packets so that the
        # packet with the associated SRI change gets flushed
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        sri.xdelta /= 2.0
        self.port.pushSRI(sri)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)

        # Get the last packet and verify that the queue has flushed, and the
        # SRI change is still reported
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet.dataBuffer is None)
        self.failUnless(packet.inputQueueFlushed)
        self.failUnless(packet.sriChanged)

    def testStatistics(self):
        """
        Tests that statistics reports the expected information.
        """
        # Push a packet of data to trigger meaningful statistics
        sri = bulkio.sri.create("port_stats")
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID);

        # Check that the statistics report the right element size
        stats = self.port._get_statistics()
        self.failUnless(stats.elementsPerSecond > 0.0)
        bits_per_element = int(round(stats.bitsPerSecond / stats.elementsPerSecond))
        self.assertEqual(self.helper.BITS_PER_ELEMENT, bits_per_element)

    def testStreamIds(self):
        """
        Tests that the same stream ID can be in consecutive streams
        """
        # Create a few streams, push an SRI and packet for each, and test that
        # the statistics report the correct stream IDs
        stream_id = 'hello'
        stream_sri = bulkio.sri.create(stream_id)

        stream_sri.mode = 0
        self.port.pushSRI(stream_sri)
        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)
        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)
        self._pushTestPacket(50, bulkio.timestamp.now(), True, stream_id)

        stream_sri.mode = 1
        self.port.pushSRI(stream_sri)
        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)
        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)
        self._pushTestPacket(50, bulkio.timestamp.now(), True, stream_id)

        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(0, packet.SRI.mode, 'packet.SRI should have real mode')
        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(0, packet.SRI.mode, 'packet.SRI should have real mode')
        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(True, packet.EOS, 'packet.EOS should be True')
        self.assertEqual(0, packet.SRI.mode, 'packet.SRI should have real mode')

        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(1, packet.SRI.mode, 'packet.SRI should have complex mode')
        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(1, packet.SRI.mode, 'packet.SRI should have complex mode')
        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(True, packet.EOS, 'packet.EOS should be True')
        self.assertEqual(1, packet.SRI.mode, 'packet.SRI should have complex mode')

        stream_sri.mode = 0
        self.port.pushSRI(stream_sri)
        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)
        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)
        self._pushTestPacket(50, bulkio.timestamp.now(), True, stream_id)

        stream_sri.mode = 1
        self.port.pushSRI(stream_sri)
        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)

        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(0, packet.SRI.mode, 'packet.SRI should have real mode')
        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(0, packet.SRI.mode, 'packet.SRI should have real mode')
        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(True, packet.EOS, 'packet.EOS should be True')
        self.assertEqual(0, packet.SRI.mode, 'packet.SRI should have real mode')

        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(1, packet.SRI.mode, 'packet.SRI should have complex mode')

        self._pushTestPacket(50, bulkio.timestamp.now(), False, stream_id)
        self._pushTestPacket(50, bulkio.timestamp.now(), True, stream_id)

        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(False, packet.EOS, 'packet.EOS should be False')
        self.assertEqual(1, packet.SRI.mode, 'packet.SRI should have complex mode')
        packet = self.port.getPacket()
        self.assertEqual(50, len(packet.dataBuffer))
        self.assertEqual(True, packet.EOS, 'packet.EOS should be True')
        self.assertEqual(1, packet.SRI.mode, 'packet.SRI should have complex mode')

    def testStatisticsStreamIDs(self):
        """
        Tests that the stream IDs reported in statistics are correct.
        """
        # Create a few streams, push an SRI and packet for each, and test that
        # the statistics report the correct stream IDs
        stream_ids = set('sri%d' % ii for ii in xrange(3))
        for stream in stream_ids:
            stream_sri = bulkio.sri.create(stream)
            self.port.pushSRI(stream_sri)
            self._pushTestPacket(1, bulkio.timestamp.now(), False, stream)
        self.assertEqual(stream_ids, set(self.port._get_statistics().streamIDs))

        # Push an end-of-stream for one of the streams (doesn't matter which),
        # and test that the stream ID has been removed from the stats
        stream = stream_ids.pop()
        self._pushTestPacket(0, bulkio.timestamp.notSet(), True, stream)
        self.assertEqual(stream_ids, set(self.port._get_statistics().streamIDs))

    def testSriChangedInvalidStream(self):
        """
        Tests that the callback is triggered and SRI changes are reported for
        an unknown stream ID.
        """
        stream_id = 'invalid_stream'

        # Turn off the port's logging to avoid dumping a warning to the screen
        self.port.getLogger().setLevel(logging.OFF);

        # Push data without an SRI to check that the sriChanged flag is still
        # set and the SRI callback gets called
        listener = SriListener()
        self.port.setNewSriListener(listener)
        self._pushTestPacket(100, bulkio.timestamp.now(), False, stream_id)
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(not packet)
        self.failUnless(packet.sriChanged)
        self.failIf(listener.sri is None)
        
        # Push again to the same stream ID; sriChanged should now be false and the
        # SRI callback should not get called
        listener.reset()
        self._pushTestPacket(100, bulkio.timestamp.now(), False, stream_id)
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(not packet)
        self.failIf(packet.sriChanged)
        self.failUnless(listener.sri is None)

    def testGetPacketTimeout(self):
        """
        Tests that timeout modes work as expected in getPacket().
        """
        # If non-blocking takes more than a millisecond, something is wrong;
        # however, on VMs, timing is a little unreliable, so try to round out
        # timing spikes by doing it a few times
        results = []
        start = time.time()
        iterations = 10
        for ii in xrange(iterations):
            packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
            if packet.dataBuffer:
                results.append(packet)
        elapsed = time.time() - start
        self.assertEqual(0, len(results))
        self.failIf(elapsed > (iterations * 1e-3))

        # Check that (at least) the timeout period elapses
        timeout = 0.125
        start = time.time()
        packet = self.port.getPacket(timeout)
        elapsed = time.time() - start
        self.failUnless(packet.dataBuffer is None)
        self.failIf(elapsed < timeout)

        # Try a blocking getPacket() on another thread
        results = []
        def get_packet():
            packet = self.port.getPacket(bulkio.const.BLOCKING)
            results.append(packet)
        t = threading.Thread(target=get_packet)
        t.setDaemon(True)
        t.start()

        # Wait for a while to ensure that the thread has had a chance to enter
        # getPacket(), then check that it has not returned
        time.sleep(0.125)
        self.assertEqual(len(results), 0)

        # Stop the port and make sure the thread exits
        self.port.stopPort()
        t.join(timeout=1.0)
        self.failIf(t.isAlive())
        self.failUnless(results[0].dataBuffer is None)

    def testBlockingDeadlock(self):
        """
        Tests that a blocking pushPacket does not prevent other threads from
        interacting with the port.
        """
        sri = bulkio.sri.create('blocking-stream')
        sri.blocking = True
        self.port.pushSRI(sri)

        self.port.setMaxQueueDepth(1)

        # Push enough packets to block in one thread
        def push_packet():
            for ii in range(2):
                self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        push_thread = threading.Thread(target=push_packet)
        push_thread.setDaemon(True)
        push_thread.start()
        
        # Get the queue depth in another thread, which used to lead to deadlock
        # (well, mostly-dead-lock)
        test_thread = threading.Thread(target=self.port.getCurrentQueueDepth)
        test_thread.setDaemon(True)
        test_thread.start()

        # Wait a while for the queue depth query to complete, which should happen
        # quickly. If the thread is still alive, then deadlock must have occurred
        test_thread.join(1.0)
        deadlock = test_thread.isAlive()

        # Get packets to unblock the push thread, allows all threads to finish
        self.port.getPacket()
        self.port.getPacket()
        self.failIf(deadlock)

    def testDiscardEmptyPacket(self):
        # Push an empty, non-EOS packet
        sri = bulkio.sri.create("empty_packet")
        self.port.pushSRI(sri)
        self._pushTestPacket(0, bulkio.timestamp.now(), False, sri.streamID)

        # No packet should be returned
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failUnless(not packet.dataBuffer)

    def testQueueFlushFlags(self):
        """
        Tests that EOS and sriChanged flags are preserved on a per-stream basis
        when a queue flush occurs.
        """
        # Push 1 packet for the normal data stream
        sri_data = bulkio.sri.create('stream_data')
        sri_data.blocking = False
        self.port.pushSRI(sri_data)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri_data.streamID)

        # Push 1 packet for the EOS test stream
        sri_eos = bulkio.sri.create('stream_eos')
        sri_eos.blocking = False
        self.port.pushSRI(sri_eos)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri_eos.streamID)

        # Push 1 packet for the SRI change stream
        sri_change = bulkio.sri.create('stream_sri')
        sri_change.blocking = False
        self.port.pushSRI(sri_change)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri_change.streamID)

        # Grab the packets to ensure the initial conditions are correct
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_data.streamID, packet.streamID)

        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_eos.streamID, packet.streamID)

        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_change.streamID, packet.streamID)

        # Push an EOS packet for the EOS stream
        self._pushTestPacket(0, bulkio.timestamp.notSet(), True, sri_eos.streamID)

        # Modify the SRI for the SRI change stream and push another packet
        sri_change.mode = 1
        self.port.pushSRI(sri_change)
        self._pushTestPacket(2, bulkio.timestamp.now(), False, sri_change.streamID)

        # Cause a queue flush by lowering the ceiling and pushing packets
        self.port.setMaxQueueDepth(3)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri_data.streamID)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri_data.streamID)

        # Push another packet for the SRI change stream
        self._pushTestPacket(2, bulkio.timestamp.now(), False, sri_change.streamID)

        # 1st packet should be for EOS stream, with no data or SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_eos.streamID, packet.streamID)
        self.assertTrue(packet.inputQueueFlushed)
        self.assertTrue(packet.EOS)
        self.assertFalse(packet.sriChanged)
        self.assertEqual(0, len(packet.dataBuffer))

        # 2nd packet should be for data stream, with no EOS or SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_data.streamID, packet.streamID)
        self.assertFalse(packet.inputQueueFlushed)
        self.assertFalse(packet.EOS)
        self.assertFalse(packet.sriChanged)

        # 3rd packet should contain the "lost" SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_change.streamID, packet.streamID)
        self.assertFalse(packet.inputQueueFlushed)
        self.assertFalse(packet.EOS)
        self.assertTrue(packet.sriChanged)

    def testQueueSize(self):
        """
        Tests that the max queue size can be set to a non-default value or
        unlimited (negative)
        """
        sri = bulkio.sri.create('queue_size')
        sri.blocking = False
        self.port.pushSRI(sri)

        # Start with a reasonably small queue depth and check that a flush
        # occurs at the expected time
        self.port.setMaxQueueDepth(10)
        for _ in xrange(10):
            self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        self.assertEqual(10, self.port.getCurrentQueueDepth())
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        self.assertEqual(1, self.port.getCurrentQueueDepth())

        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet.dataBuffer is None)
        self.assertTrue(packet.inputQueueFlushed)

        # Set queue depth to unlimited and push a lot of packets
        self.port.setMaxQueueDepth(-1)
        QUEUE_SIZE = 250
        for _ in xrange(QUEUE_SIZE):
            self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        self.assertEqual(QUEUE_SIZE, self.port.getCurrentQueueDepth())
        for _ in xrange(QUEUE_SIZE):
            packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
            self.failIf(packet.dataBuffer is None)
            self.assertFalse(packet.inputQueueFlushed)

    def testSRIqueueBlock(self):
        """
        Tests that a queue can be flushed and allow packets to refill the queue, post flush.
        """
        sri = bulkio.sri.create('queue_size')
        sri.blocking = True
        self.port.pushSRI(sri)
        self.port.setMaxQueueDepth(-1)

        # Push enough packets to block in one thread
        def push_packet():
            for ii in range(1):
                self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)

        push_thread = threading.Thread(target=push_packet)
        push_thread.setDaemon(True)
        push_thread.start()
        push_thread.join(1.0)
        queue_depth = self.port.getCurrentQueueDepth()
        self.assertEqual(1, queue_depth)

        # Verify  in one thread
        packet = self.port.getPacket()
        self.assertEqual(packet.streamID, sri.streamID)

    def testSRIqueueMax(self):
        """
        Tests that a queue can be flushed and allow packets to refill the queue, post flush.
        """
        # import pdb
        # pdb.set_trace()
        sri = bulkio.sri.create('queue_size')
        sri.blocking = True
        self.port.pushSRI(sri)
        self.port.setMaxQueueDepth(50)

        # Push enough packets to block in one thread
        def push_packet():
            for ii in range(102):
                self._pushTestPacket(ii+1, bulkio.timestamp.now(), False, sri.streamID)
        push_thread = threading.Thread(target=push_packet)
        push_thread.setDaemon(True)
        push_thread.start()
        push_thread.join(1.0)
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertFalse(packet.inputQueueFlushed)
        queue_depth = self.port.getCurrentQueueDepth()
        count = 0
        while queue_depth != 50 and count != 10:
            time.sleep(.5)
            queue_depth = self.port.getCurrentQueueDepth()
            count += 1
        self.assertEqual(queue_depth, 50)

        for ii in range(101):
            packet = self.port.getPacket(bulkio.const.BLOCKING)
            self.failIf(packet is None)
            self.assertFalse(packet.inputQueueFlushed)

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
    import runtests
    runtests.main()
