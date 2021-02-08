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
