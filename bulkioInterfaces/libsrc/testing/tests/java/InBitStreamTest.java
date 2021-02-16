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
/*
 * WARNING: This file is generated from InStreamTestImpl.java.template.
 *          Do not modify directly.
 */
import org.junit.*;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import org.apache.log4j.Level;

import org.ossie.component.RHLogger;
import org.ossie.properties.AnyUtils;

import bulkio.InDataPort;
import bulkio.DataTransfer;
import bulkio.InBitStream;
import bulkio.BitDataBlock;
import bulkio.SampleTimestamp;
import bulkio.InBitPort;

import helpers.TestHelper;
import helpers.BitTestHelper;

import java.lang.Math;

@RunWith(JUnit4.class)
public class InBitStreamTest {

    /**
     * Input port being tested
     */
    protected InBitPort port;

    /**
     * External CORBA interface to the tested port.
     */
    protected BULKIO.dataBitOperations corbaPort;

    protected BitTestHelper helper;

    public InBitStreamTest()
    {
        this.helper = new BitTestHelper();
    }

    @Before
    @SuppressWarnings("unchecked")
    public void setUp()
    {
        String name = helper.getName() + "_out";
        port = helper.createInPort(name);
        corbaPort = helper.toCorbaType(port);
    }

    @Test
    public void testLegacyAPI()
    {
        // Test for methods that are technically still supported, but
        // discouraged
        port.enableStats(false);
    }

    @Test
    public void testTimestamp()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("time_stamp");
        corbaPort.pushSRI(sri);

        BULKIO.PrecisionUTCTime ts = bulkio.time.utils.create(1520883276, 0.8045831);
        helper.pushTestPacket(port, 50, ts, false, sri.streamID);

        InBitStream stream = port.getStream("time_stamp");
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read();
        Assert.assertNotNull(block);
        SampleTimestamp[] timestamps = block.getTimestamps();
        Assert.assertEquals(1, timestamps.length);
        Assert.assertEquals(ts, timestamps[0].time);
        Assert.assertEquals(0, timestamps[0].offset);
        Assert.assertEquals(false, timestamps[0].synthetic);

        BULKIO.PrecisionUTCTime new_ts = null;
        try {
            new_ts = block.getStartTime();
        } catch (Exception exc) {
            Assert.assertNotNull(null);
        }
        Assert.assertEquals(ts, new_ts);
    }

    @Test
    public void testGetCurrentStreamEmptyPacket()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("empty_packet");
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), false, sri.streamID);

        // getCurrentStream() should not return any stream
        InBitStream stream = port.getCurrentStream(bulkio.Const.NON_BLOCKING);
        Assert.assertNull(stream);
    }

    @Test
    public void testGetCurrentStreamEmptyEos()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("empty_eos");
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, sri.streamID);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream("empty_eos");
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read();
        Assert.assertNotNull(block);
        Assert.assertEquals(1024, block.buffer().length);
        Assert.assertTrue(!stream.eos());

        // Push an end-of-stream packet with no data and get the stream again
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), true, sri.streamID);
        stream = port.getCurrentStream(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(stream);
        block = stream.read();
        Assert.assertNull(block);

        // There should be no current stream, because the failed read should have
        // removed it
        InBitStream next_stream = port.getCurrentStream(bulkio.Const.NON_BLOCKING);
        Assert.assertNull(next_stream);

        // The original stream should report end-of-stream
        Assert.assertTrue(stream.eos());
    }

    @Test
    public void testGetCurrentStreamDataEos()
    {
        // Create a new stream and push some data to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("empty_eos");
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, sri.streamID);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream("empty_eos");
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read();
        Assert.assertNotNull(block);
        Assert.assertEquals(1024, block.buffer().length);
        Assert.assertFalse(stream.eos());

        // Push an end-of-stream packet with data and get the stream again
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), true, sri.streamID);
        stream = port.getCurrentStream(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(stream);
        block = stream.read();
        Assert.assertNotNull(block);
        Assert.assertEquals(1024, block.buffer().length);

        // Try to get the current stream again; since the end-of-stream has not been
        // checked yet, it should return the existing stream (as with above)
        stream = port.getCurrentStream(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(stream);
        block = stream.read();
        Assert.assertNull(block);

        // There should be no current stream, because the failed read should have
        // removed it
        InBitStream next_stream = port.getCurrentStream(bulkio.Const.NON_BLOCKING);
        Assert.assertNull(next_stream);

        // The original stream should report end-of-stream
        Assert.assertTrue(stream.eos());
    }

    @Test
    public void testSriChanges()
    {
        String stream_id = "sri_changes";

        // Create a new stream and push some data to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        sri.xstart = 0.0;
        sri.xdelta = 1.0;
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, sri.streamID);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read();
        Assert.assertNotNull(block);
        Assert.assertEquals(1024, block.buffer().length);
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(Math.abs(sri.xdelta - block.sri().xdelta) < 0.01);

        // Change xdelta (based on sample rate of 2.5Msps)
        BULKIO.StreamSRI new_sri = bulkio.sri.utils.create(stream_id);
        new_sri.xstart = 0.0;
        new_sri.xdelta = 1.0 / 2.5e6;
        corbaPort.pushSRI(new_sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, new_sri.streamID);
        block = stream.read();
        Assert.assertNotNull(block);
        Assert.assertEquals(1024, block.buffer().length);
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        int flags = bulkio.sri.utils.XDELTA;
        Assert.assertEquals(flags, block.sriChangeFlags());
        Assert.assertTrue(Math.abs(new_sri.xdelta - block.sri().xdelta) < 0.01);

        // Add a keyword, change xdelta back and update xstart
        BULKIO.StreamSRI additional_sri = bulkio.sri.utils.create(stream_id);
        CF.DataType col_rf = new CF.DataType("COL_RF", AnyUtils.stringToAny("101.1e6","double"));
        additional_sri.keywords = new CF.DataType[1];
        additional_sri.keywords[0] = col_rf;
        additional_sri.xstart = 100.0;
        additional_sri.xdelta = 1.0;
        corbaPort.pushSRI(additional_sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, sri.streamID);
        block = stream.read();
        Assert.assertNotNull(block);
        Assert.assertEquals(1024, block.buffer().length);
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        flags = bulkio.sri.utils.XSTART | bulkio.sri.utils.XDELTA | bulkio.sri.utils.KEYWORDS;
        Assert.assertEquals(flags, block.sriChangeFlags());
        Assert.assertTrue(Math.abs(additional_sri.xstart - block.sri().xstart) < 0.01);
        Assert.assertTrue(Math.abs(additional_sri.xdelta - block.sri().xdelta) < 0.01);
    }

    @Test
    public void testDisable()
    {
        String stream_id = "disable";

        // Create a new stream and push some data to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 16, bulkio.time.utils.now(), false, sri.streamID);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);

        BitDataBlock block = stream.read();
        Assert.assertNotNull(block);

        // Push a couple more packets
        helper.pushTestPacket(port, 16, bulkio.time.utils.now(), false, sri.streamID);
        helper.pushTestPacket(port, 16, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(2, port.getCurrentQueueDepth());

        // Disable the stream; this should drop the existing packets
        stream.disable();
        Assert.assertTrue(!stream.enabled());
        Assert.assertEquals(0, port.getCurrentQueueDepth());

        // Push a couple more packets; they should get dropped
        helper.pushTestPacket(port, 16, bulkio.time.utils.now(), false, sri.streamID);
        helper.pushTestPacket(port, 16, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(0, port.getCurrentQueueDepth());

        // Push an end-of-stream packet
        helper.pushTestPacket(port, 16, bulkio.time.utils.now(), true, sri.streamID);

        // Re-enable the stream and read; it should fail with end-of-stream set
        stream.enable();
        block = stream.read();
        Assert.assertNull(block);
        Assert.assertTrue(stream.eos());
    }

    @Test
    public void testSizedReadEmptyEos()
    {
        String stream_id = "read_empty_eos";

        // Create a new stream and push an end-of-stream packet with no data
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), true, stream_id);

        // Try to read a single element; this should return a null block
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read(1);
        Assert.assertNull(block);
        Assert.assertTrue(stream.eos());
    }

    @Test
    public void testSizedTryreadEmptyEos()
    {
        String stream_id = "tryread_empty_eos";

        // Create a new stream and push an end-of-stream packet with no data
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 0, bulkio.time.utils.notSet(), true, stream_id);

        // Try to read a single element; this should return a null block
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.tryread(1, 0);
        Assert.assertNull(block);
        Assert.assertTrue(stream.eos());
    }

    public InBitStream thread_stream;

    public class ReadThread extends Thread {
        protected InBitStream _in_thread_stream;
        public ReadThread(InBitStream in_thread_stream) {
            _in_thread_stream = in_thread_stream;
        }
        public void run() {
            BitDataBlock thread_block = _in_thread_stream.read(800, 1100);
            Assert.assertNull(thread_block);
        }
    }

    @Test
    public void testConsumeMoreThanRead()
    {
        String stream_id = "consume_more_than_read";

        // Create a new stream and push some data to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, stream_id);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), true, stream_id);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read(1000,2000);
        Assert.assertEquals(1000, block.buffer().length);
        block = stream.read();
        Assert.assertEquals(48, block.buffer().length);
        block = stream.read();
        Assert.assertNull(block);

        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1000, bulkio.time.utils.now(), false, stream_id);
        stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        block = stream.read(800, 900);
        Assert.assertEquals(800, block.buffer().length);
        block = stream.read();
        Assert.assertEquals(100, block.buffer().length);
        block = stream.tryread();
        Assert.assertNull(block);

        helper.pushTestPacket(port, 1000, bulkio.time.utils.now(), false, stream_id);
        thread_stream = port.getStream(stream_id);
        Assert.assertNotNull(thread_stream);
        // kick off in thread
        ReadThread read_thread = new ReadThread(thread_stream);
        read_thread.start();
        try {
            Thread.sleep(100); // sleep 100 ms
        } catch (InterruptedException e) {
        }
        read_thread.interrupt();
        try {
            read_thread.join();
        } catch (InterruptedException e) {
        }
        helper.pushTestPacket(port, 1000, bulkio.time.utils.now(), false, stream_id);
        block = thread_stream.read(800, 1100);
        Assert.assertEquals(800, block.buffer().length);
        block = thread_stream.read();
        Assert.assertEquals(900, block.buffer().length);
        block = thread_stream.tryread();
        Assert.assertNull(block);

        helper.pushTestPacket(port, 1000, bulkio.time.utils.now(), false, stream_id);
        stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        block = stream.tryread(800, 1100);
        Assert.assertNull(block);
        block = stream.read();
        Assert.assertEquals(1000, block.buffer().length);
        block = stream.tryread();
        Assert.assertNull(block);
    }

    @Test
    public void testQueueFlushScenarios()
    {
        // Establish all the streams as "active"

        String _stream_a_id = "stream_A";
        String _stream_b_id = "stream_B";
        String _stream_c_id = "stream_C";

        // Push 1 packet for stream_A
        BULKIO.StreamSRI stream_A = bulkio.sri.utils.create(_stream_a_id);
        stream_A.blocking = false;
        corbaPort.pushSRI(stream_A);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_A.streamID);

        // Push 1 packet for stream_B
        BULKIO.StreamSRI stream_B = bulkio.sri.utils.create(_stream_b_id);
        stream_B.blocking = false;
        corbaPort.pushSRI(stream_B);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_B.streamID);

        // Push 1 packet for stream_C
        BULKIO.StreamSRI stream_C = bulkio.sri.utils.create(_stream_c_id);
        stream_C.blocking = false;
        corbaPort.pushSRI(stream_C);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_C.streamID);

        // empty the queue
        InBitStream stream = port.getStream(_stream_a_id);
        BitDataBlock block = stream.read();
        stream = port.getStream(_stream_b_id);
        block = stream.read();
        stream = port.getStream(_stream_c_id);
        block = stream.read();

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        BULKIO.StreamSRI active_sris[] = port.activeSRIs();
        int number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        // Test case 1
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_B.streamID);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_C.streamID);
        helper.pushTestPacket(port, 2, bulkio.time.utils.now(), false, stream_B.streamID);
        helper.pushTestPacket(port, 2, bulkio.time.utils.now(), false, stream_C.streamID);
        port.setMaxQueueDepth(4);
        // flush
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_A.streamID);

        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertEquals(2, block.buffer().length);
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());

        stream = port.getStream(_stream_c_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertEquals(2, block.buffer().length);
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(block.buffer().length!=0);
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        // Test case 2
        port.setMaxQueueDepth(6);
        helper.pushTestPacket(port, 7, bulkio.time.utils.now(), false, stream_A.streamID);
        helper.pushTestPacket(port, 8, bulkio.time.utils.now(), false, stream_B.streamID);
        helper.pushTestPacket(port, 9, bulkio.time.utils.now(), false, stream_C.streamID);
        helper.pushTestPacket(port, 10, bulkio.time.utils.now(), false, stream_A.streamID);
        helper.pushTestPacket(port, 11, bulkio.time.utils.now(), false, stream_B.streamID);
        helper.pushTestPacket(port, 12, bulkio.time.utils.now(), false, stream_C.streamID);
        // flush
        helper.pushTestPacket(port, 13, bulkio.time.utils.now(), false, stream_A.streamID);
        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertEquals(11, block.buffer().length);
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());

        stream = port.getStream(_stream_c_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(12, block.buffer().length);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(13, block.buffer().length);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        port.setMaxQueueDepth(5);
        // Test case 3
        helper.pushTestPacket(port, 11, bulkio.time.utils.now(), false, stream_B.streamID);
        helper.pushTestPacket(port, 12, bulkio.time.utils.now(), false, stream_B.streamID);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), true, stream_B.streamID);
        helper.pushTestPacket(port, 13, bulkio.time.utils.now(), false, stream_C.streamID);
        helper.pushTestPacket(port, 14, bulkio.time.utils.now(), false, stream_C.streamID);
        // flush
        helper.pushTestPacket(port, 15, bulkio.time.utils.now(), false, stream_A.streamID);

        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(12, block.buffer().length);

        stream = port.getStream(_stream_c_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(14, block.buffer().length);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(15, block.buffer().length);

        // Establish all the streams as "active"
        port.setMaxQueueDepth(3);
        // Push 1 packet for stream_A
        stream_A = bulkio.sri.utils.create(_stream_a_id);
        stream_A.blocking = false;
        port.pushSRI(stream_A);
        helper.pushTestPacket(port, 14, bulkio.time.utils.now(), false, stream_A.streamID);

        // Push 1 packet for stream_B
        stream_B = bulkio.sri.utils.create("stream_B");
        stream_B.blocking = false;
        port.pushSRI(stream_B);
        helper.pushTestPacket(port, 15, bulkio.time.utils.now(), false, stream_B.streamID);

        // Push 1 packet for stream_C
        stream_C = bulkio.sri.utils.create("stream_C");
        stream_C.blocking = false;
        port.pushSRI(stream_C);
        helper.pushTestPacket(port, 16, bulkio.time.utils.now(), false, stream_C.streamID);

        // empty the queue
        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertNotNull(block);
        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertNotNull(block);
        stream = port.getStream(_stream_c_id);
        block = stream.read();
        Assert.assertNotNull(block);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        // Test case 4
        port.setMaxQueueDepth(3);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), true, stream_B.streamID);
        helper.pushTestPacket(port, 17, bulkio.time.utils.now(), false, stream_C.streamID);
        helper.pushTestPacket(port, 18, bulkio.time.utils.now(), false, stream_C.streamID);
        // flush
        helper.pushTestPacket(port, 19, bulkio.time.utils.now(), false, stream_A.streamID);

        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertNull(block);
        Assert.assertTrue(stream.eos());

        stream = port.getStream(_stream_c_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(18, block.buffer().length);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(19, block.buffer().length);

        // Establish all the streams as "active"
        port.setMaxQueueDepth(3);
        // Push 1 packet for stream_A
        stream_A = bulkio.sri.utils.create("stream_A");
        stream_A.blocking = false;
        port.pushSRI(stream_A);
        helper.pushTestPacket(port, 19, bulkio.time.utils.now(), false, stream_A.streamID);

        // Push 1 packet for stream_B
        stream_B = bulkio.sri.utils.create("stream_B");
        stream_B.blocking = false;
        port.pushSRI(stream_B);
        helper.pushTestPacket(port, 20, bulkio.time.utils.now(), false, stream_B.streamID);

        // Push 1 packet for stream_C
        stream_C = bulkio.sri.utils.create("stream_C");
        stream_C.blocking = false;
        port.pushSRI(stream_C);
        helper.pushTestPacket(port, 21, bulkio.time.utils.now(), false, stream_C.streamID);

        // empty the queue
        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertNotNull(block);
        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertNotNull(block);
        stream = port.getStream(_stream_c_id);
        block = stream.read();
        Assert.assertNotNull(block);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        // Test case 5
        port.setMaxQueueDepth(3);
        helper.pushTestPacket(port, 22, bulkio.time.utils.now(), false, stream_A.streamID);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), true, stream_A.streamID);
        port.pushSRI(stream_A);
        helper.pushTestPacket(port, 23, bulkio.time.utils.now(), false, stream_A.streamID);
        // flush
        helper.pushTestPacket(port, 24, bulkio.time.utils.now(), false, stream_A.streamID);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(22, block.buffer().length);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        Assert.assertEquals(24, block.buffer().length);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        // Test case 5a
        port.setMaxQueueDepth(4);
        helper.pushTestPacket(port, 21, bulkio.time.utils.now(), false, stream_A.streamID);
        helper.pushTestPacket(port, 22, bulkio.time.utils.now(), false, stream_A.streamID);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), true, stream_A.streamID);
        port.pushSRI(stream_A);
        helper.pushTestPacket(port, 23, bulkio.time.utils.now(), false, stream_A.streamID);
        // flush
        helper.pushTestPacket(port, 24, bulkio.time.utils.now(), false, stream_A.streamID);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(22, block.buffer().length);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        Assert.assertEquals(24, block.buffer().length);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        // Test case 6
        port.setMaxQueueDepth(3);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), true, stream_A.streamID);
        helper.pushTestPacket(port, 25, bulkio.time.utils.now(), false, stream_B.streamID);
        helper.pushTestPacket(port, 26, bulkio.time.utils.now(), false, stream_B.streamID);
        // flush
        port.pushSRI(stream_A);
        helper.pushTestPacket(port, 27, bulkio.time.utils.now(), false, stream_A.streamID);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertNull(block);
        Assert.assertTrue(stream.eos());

        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(26, block.buffer().length);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        Assert.assertEquals(27, block.buffer().length);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        port.setMaxQueueDepth(2);
        BULKIO.StreamSRI new_stream_A = bulkio.sri.utils.create("stream_A");
        new_stream_A.blocking = false;
        if (stream_A.mode == 1) {
            new_stream_A.mode = 0;
        } else {
            new_stream_A.mode = 1;
        }
        port.pushSRI(new_stream_A);
        helper.pushTestPacket(port, 28, bulkio.time.utils.now(), false, stream_A.streamID);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        Assert.assertEquals(28, block.buffer().length);

        // Test case 7
        port.setMaxQueueDepth(4);
        BULKIO.StreamSRI another_stream_A = bulkio.sri.utils.create("stream_A");
        another_stream_A.blocking = false;
        if (new_stream_A.mode == 1) {
            another_stream_A.mode = 0;
        } else {
            another_stream_A.mode = 1;
        }
        port.pushSRI(another_stream_A);
        helper.pushTestPacket(port, 28, bulkio.time.utils.now(), false, stream_A.streamID);
        helper.pushTestPacket(port, 29, bulkio.time.utils.now(), false, stream_C.streamID);
        helper.pushTestPacket(port, 30, bulkio.time.utils.now(), false, stream_A.streamID);
        helper.pushTestPacket(port, 31, bulkio.time.utils.now(), false, stream_C.streamID);
        // flush
        helper.pushTestPacket(port, 32, bulkio.time.utils.now(), false, stream_B.streamID);

        stream = port.getStream(_stream_a_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        Assert.assertEquals(30, block.buffer().length);

        stream = port.getStream(_stream_c_id);
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(31, block.buffer().length);

        stream = port.getStream(_stream_b_id);
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(32, block.buffer().length);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);

        // Test case 8
        port.setMaxQueueDepth(3);
        BULKIO.StreamSRI yet_another_stream_A = bulkio.sri.utils.create("stream_A");
        yet_another_stream_A.blocking = false;
        if (another_stream_A.mode == 1) {
            yet_another_stream_A.mode = 0;
        } else {
            yet_another_stream_A.mode = 1;
        }
        port.pushSRI(yet_another_stream_A);
        BULKIO.PrecisionUTCTime ts = bulkio.time.utils.now();
        helper.pushTestPacket(port, 28, ts, false, stream_A.streamID);
        helper.pushTestPacket(port, 29, bulkio.time.utils.add(ts, 1), false, stream_C.streamID);
        helper.pushTestPacket(port, 30, bulkio.time.utils.add(ts, 2), false, stream_A.streamID);
        // flush
        helper.pushTestPacket(port, 32, bulkio.time.utils.add(ts, 3), false, stream_B.streamID);

        stream = port.getCurrentStream(0);
        Assert.assertTrue(stream.streamID().equals(_stream_c_id));
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(29, block.buffer().length);

        stream = port.getCurrentStream(0);
        Assert.assertTrue(stream.streamID().equals(_stream_a_id));
        block = stream.read();
        Assert.assertTrue(block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(block.sriChanged());
        Assert.assertEquals(30, block.buffer().length);

        stream = port.getCurrentStream(0);
        Assert.assertTrue(stream.streamID().equals(_stream_b_id));
        block = stream.read();
        Assert.assertTrue(!block.inputQueueFlushed());
        Assert.assertTrue(!stream.eos());
        Assert.assertTrue(!block.sriChanged());
        Assert.assertEquals(32, block.buffer().length);

        Assert.assertEquals(port.getCurrentQueueDepth(), 0);
        active_sris = port.activeSRIs();
        number_alive_streams = active_sris.length;
        Assert.assertEquals(number_alive_streams, 3);
    }

    @Test
    public void testTryreadPeek()
    {
        String stream_id = "try_read_peek";

        // Create a new stream and push some data to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), true, stream_id);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.tryread(10000,0);
        Assert.assertEquals(1024, block.buffer().length);
        block = stream.read(10000);
        Assert.assertEquals(1024, block.buffer().length);
        block = stream.read(10000);
        Assert.assertNull(block);
    }

    @Test
    public void testReadPeek()
    {
        String stream_id = "read_peek";

        // Create a new stream and push some data to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), true, stream_id);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);

        BitDataBlock block = stream.read(10000,0);
        Assert.assertEquals(1024, block.buffer().length);

        block = stream.read(10000);
        Assert.assertEquals(1024, block.buffer().length);

        block = stream.read(10000);
        Assert.assertNull(block);
    }

    @Test
    public void testReadPartial()
    {
        String stream_id = "read_partial";

        // Create a new stream and push some data to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), true, stream_id);

        // Get the input stream and read the first packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);

        BitDataBlock block = stream.read(10000,2000);
        Assert.assertEquals(1024, block.buffer().length);

        block = stream.read(10000);
        Assert.assertNull(block);
    }

    @Test
    public void testReadTimestamps()
    {
        String stream_id = "read_timestamps";

        // Create a new stream and push several packets with known timestamps
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        sri.xdelta = 0.0625;
        corbaPort.pushSRI(sri);
        BULKIO.PrecisionUTCTime ts = bulkio.time.utils.create(4000.0, 0.5);
        // Push packets of size 32, which should advance the time by exactly 2
        // seconds each
        helper.pushTestPacket(port, 32, ts, false, sri.streamID);
        helper.pushTestPacket(port, 32, bulkio.time.utils.add(ts,2.0), false, sri.streamID);
        helper.pushTestPacket(port, 32, bulkio.time.utils.add(ts,4.0), false, sri.streamID);
        helper.pushTestPacket(port, 32, bulkio.time.utils.add(ts,6.0), false, sri.streamID);

        // Get the input stream and read several packets as one block, enough to
        // bisect the third packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read(70);
        Assert.assertNotNull(block);
        Assert.assertEquals(70, block.buffer().length);

        // There should be 3 timestamps, all non-synthetic
        bulkio.SampleTimestamp[] timestamps = block.getTimestamps();
        Assert.assertEquals(3, timestamps.length);
        Assert.assertEquals(0, bulkio.time.utils.compare(ts, timestamps[0].time));
        Assert.assertEquals(0, timestamps[0].offset);
        Assert.assertEquals(false, timestamps[0].synthetic);
        try {
            Assert.assertEquals(0, bulkio.time.utils.compare(timestamps[0].time, block.getStartTime()));
        } catch (Exception e) {
        }
        Assert.assertEquals(0, bulkio.time.utils.compare(bulkio.time.utils.add(ts,2.0), timestamps[1].time));
        Assert.assertEquals(32, timestamps[1].offset);
        Assert.assertEquals(false, timestamps[1].synthetic);

        Assert.assertEquals(0, bulkio.time.utils.compare(bulkio.time.utils.add(ts,4.0), timestamps[2].time));
        Assert.assertEquals(64, timestamps[2].offset);
        Assert.assertEquals(false, timestamps[2].synthetic);

        // Read the remaining packet and a half; the first timestamp should be
        // synthetic
        block = stream.read(58);
        Assert.assertNotNull(block);
        Assert.assertEquals(58, block.buffer().length);
        timestamps = block.getTimestamps();
        Assert.assertEquals(2, timestamps.length);
        Assert.assertEquals(true, timestamps[0].synthetic);
        Assert.assertEquals(0, bulkio.time.utils.compare(bulkio.time.utils.add(ts,4.375), timestamps[0].time));
        Assert.assertEquals(0, timestamps[0].offset);
        try {
            Assert.assertEquals(0, bulkio.time.utils.compare(timestamps[0].time, block.getStartTime()));
        } catch (Exception e) {
        }
        Assert.assertEquals(0, bulkio.time.utils.compare(bulkio.time.utils.add(ts,6.0), timestamps[1].time));
        Assert.assertEquals(26, timestamps[1].offset);
        Assert.assertEquals(false, timestamps[1].synthetic);
    }

    @Test
    public void testRepeatStreamIds()
    {
        String stream_id = "repeat_stream_ids";

        // Create a new stream and push several packets with known timestamps
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        sri.xdelta = 0.0625;
        int number_streams = 6;
        int number_packets = 4;
        for (int i=0; i<number_streams; i++) {
            corbaPort.pushSRI(sri);
            for (int j=0; j<number_packets-1; j++) {
                helper.pushTestPacket(port, 32, bulkio.time.utils.now(), false, sri.streamID);
            }
            helper.pushTestPacket(port, 32, bulkio.time.utils.now(), true, sri.streamID);
        }

        int received_streams = 0;
        for (int i=0; i<number_streams; i++) {
            int received_packets = 0;
            InBitStream inputStream = port.getCurrentStream(bulkio.Const.NON_BLOCKING);
            Assert.assertNotNull(inputStream);
            received_streams += 1;
            for (int j=0; j<number_packets; j++) {
                BitDataBlock block = inputStream.read();
                Assert.assertNotNull(block);
                received_packets += 1;
            }
            Assert.assertEquals(inputStream.eos(), true);
            Assert.assertEquals(received_packets, number_packets);
        }
        Assert.assertEquals(received_streams, number_streams);
    }

    @Test
    public void testDisableDiscard()
    {
        String stream_id = "disable_discard";

        // Create a new stream and push a couple of packets to it
        BULKIO.StreamSRI sri = bulkio.sri.utils.create(stream_id);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(1, port.getCurrentQueueDepth());

        // Get the input stream and read half of the first packet
        InBitStream stream = port.getStream(stream_id);
        Assert.assertNotNull(stream);
        BitDataBlock block = stream.read(512);
        Assert.assertNotNull(block);

        // The stream should report samples available, but there should be no
        // packets in the port's queue
        Assert.assertTrue(stream.ready());
        Assert.assertTrue(stream.samplesAvailable() > 0);
        Assert.assertEquals(0, port.getCurrentQueueDepth());

        // Disable the stream; this should discard
        stream.disable();
        Assert.assertFalse(stream.ready());
        Assert.assertEquals(0, stream.samplesAvailable());
    }
}
