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
 * WARNING: This file is generated from InPortTestImpl.java.template.
 *          Do not modify directly.
 */

package impl;

import org.junit.*;
import org.junit.runner.RunWith;

import org.apache.log4j.Level;

import org.ossie.component.RHLogger;

import bulkio.InDataPort;
import bulkio.DataTransfer;

import helpers.TestHelper;

public class InPortTestImpl<E extends BULKIO.updateSRIOperations & BULKIO.ProvidesPortStatisticsProviderOperations,A> {

    /**
     * Input port being tested (using generic interface).
     */
    protected InDataPort<E,A> port;

    /**
     * External CORBA interface to the tested port.
     */
    protected E corbaPort;

    protected TestHelper<E,A> helper;

    public InPortTestImpl(TestHelper<E,A> helper)
    {
        this.helper = helper;
    }

    @Before
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
    public void testGetPacket()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("test_get_packet");
        corbaPort.pushSRI(sri);

        BULKIO.PrecisionUTCTime ts = bulkio.time.utils.now();
        helper.pushTestPacket(port, 50, ts, false, sri.streamID);

        // Check result of getPacket
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(50, helper.dataLength(packet.dataBuffer));
        // For all types except XML, the timestamp should be preserved
        if (!(corbaPort instanceof BULKIO.dataXMLOperations)) {
            Assert.assertEquals(0, bulkio.time.utils.compare(ts, packet.T));
        }
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(sri.streamID, packet.streamID);
        Assert.assertTrue(bulkio.sri.utils.compare(packet.SRI, sri));
        Assert.assertEquals(true, packet.sriChanged);
        Assert.assertEquals(false, packet.inputQueueFlushed);

        // No packet, should return null
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNull(packet);

        // Change mode to complex and push another packet with EOS set
        // NB: Have to create a new instance because the input port doesn't
        //     copy the SRI, it just shares the reference
        sri = bulkio.sri.utils.create(sri.streamID);
        sri.mode = 1;
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 100, ts, true, sri.streamID);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertEquals(100, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(true, packet.EOS);
        Assert.assertEquals(true, packet.sriChanged);
        Assert.assertEquals(1, packet.SRI.mode);
    }

    @Test
    public void testActiveSRIs()
    {
        BULKIO.StreamSRI[] active_sris = corbaPort.activeSRIs();
        Assert.assertEquals(0, active_sris.length);

        // Push a new SRI, and make sure that it is immediately visible and
        // correct in activeSRIs
        BULKIO.StreamSRI sri_1 = bulkio.sri.utils.create("active_sri_1");
        corbaPort.pushSRI(sri_1);
        active_sris = corbaPort.activeSRIs();
        Assert.assertEquals(1, active_sris.length);
        Assert.assertTrue(bulkio.sri.utils.compare(active_sris[0], sri_1));

        // Push a second SRI, and make sure that activeSRIs is up-to-date
        BULKIO.StreamSRI sri_2 = bulkio.sri.utils.create("active_sri_2");
        corbaPort.pushSRI(sri_2);
        active_sris = corbaPort.activeSRIs();
        Assert.assertEquals(2, active_sris.length);
        for (BULKIO.StreamSRI current_sri : active_sris) {
            if (current_sri.streamID.equals("active_sri_2")) {
                Assert.assertTrue(bulkio.sri.utils.compare(current_sri, sri_2));
            } else if (!current_sri.streamID.equals("active_sri_1")) {
                Assert.fail("unexpected SRI '" + current_sri.streamID +"'");
            }
        }

        // Push an end-of-stream, retrieve the packet, and verify that the
        // stream is no longer in activeSRIs
        helper.pushTestPacket(port, 0, bulkio.time.utils.notSet(), true, sri_1.streamID);
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertTrue(packet.EOS);
        active_sris = corbaPort.activeSRIs();
        Assert.assertEquals(1, active_sris.length);
        Assert.assertEquals(active_sris[0].streamID, sri_2.streamID);
    }

    @Test
    public void testStreamIds()
    {
        BULKIO.StreamSRI sri_1 = bulkio.sri.utils.create("test_get_packet");
        DataTransfer<A> packet = null;
        sri_1.mode = 0;
        corbaPort.pushSRI(sri_1);

        helper.pushTestPacket(port, 50, bulkio.time.utils.now(), false, sri_1.streamID);
        helper.pushTestPacket(port, 50, bulkio.time.utils.now(), false, sri_1.streamID);
        helper.pushTestPacket(port, 50, bulkio.time.utils.now(), true, sri_1.streamID);

        BULKIO.StreamSRI sri_2 = bulkio.sri.utils.create("test_get_packet");
        sri_2.mode = 1;
        corbaPort.pushSRI(sri_2);

        helper.pushTestPacket(port, 50, bulkio.time.utils.now(), false, sri_2.streamID);
        helper.pushTestPacket(port, 50, bulkio.time.utils.now(), false, sri_2.streamID);
        helper.pushTestPacket(port, 50, bulkio.time.utils.now(), true, sri_2.streamID);

        // Check result of getPacket
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(50, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(0, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(50, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(0, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(50, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(true, packet.EOS);
        Assert.assertEquals(0, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(50, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(1, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(50, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(1, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(50, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(true, packet.EOS);
        Assert.assertEquals(1, packet.SRI.mode);

        corbaPort.pushSRI(sri_1);

        helper.pushTestPacket(port, 100, bulkio.time.utils.now(), false, sri_1.streamID);
        helper.pushTestPacket(port, 100, bulkio.time.utils.now(), false, sri_1.streamID);
        helper.pushTestPacket(port, 100, bulkio.time.utils.now(), true, sri_1.streamID);

        corbaPort.pushSRI(sri_2);

        helper.pushTestPacket(port, 100, bulkio.time.utils.now(), false, sri_2.streamID);

        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(100, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(0, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(100, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(0, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(100, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(true, packet.EOS);
        Assert.assertEquals(0, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(100, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(1, packet.SRI.mode);

        helper.pushTestPacket(port, 100, bulkio.time.utils.now(), false, sri_2.streamID);
        helper.pushTestPacket(port, 100, bulkio.time.utils.now(), true, sri_2.streamID);

        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(100, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(false, packet.EOS);
        Assert.assertEquals(1, packet.SRI.mode);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertNotNull(packet.dataBuffer);
        Assert.assertEquals(100, helper.dataLength(packet.dataBuffer));
        Assert.assertEquals(true, packet.EOS);
        Assert.assertEquals(1, packet.SRI.mode);
    }

    @Test
    public void testQueueDepth()
    {
        // The port had better start with an empty queue
        Assert.assertEquals(0, port.getCurrentQueueDepth());

        // Use a non-blocking stream to allow queue flushing
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("queue_depth");
        sri.blocking = false;
        corbaPort.pushSRI(sri);

        // Push some test packets, the queue should start growing
        for (int ii = 0; ii < 4; ii++) {
            helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        }
        Assert.assertEquals(4, port.getCurrentQueueDepth());

        // Read a packet and make sure the current depth drops
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(3, port.getCurrentQueueDepth());

        // Reduce the max queue size and push another packet, causing a flush
        port.setMaxQueueDepth(3);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(1, port.getCurrentQueueDepth());

        // Read the packet and make sure the flush is reported
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertTrue(packet.inputQueueFlushed);

        // One more packet, should not report a flush
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertFalse(packet.inputQueueFlushed);
    }

    @Test
    public void testState()
    {
        // Port starts out idle
        Assert.assertEquals(BULKIO.PortUsageType.IDLE, corbaPort.state());

        // Push one test packet, state goes to active
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("test_state");
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(BULKIO.PortUsageType.ACTIVE, corbaPort.state());

        // Full queue should report busy
        port.setMaxQueueDepth(2);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(BULKIO.PortUsageType.BUSY, corbaPort.state());

        // Drop below max, back to active
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(BULKIO.PortUsageType.ACTIVE, corbaPort.state());

        // Empty queue, back to idle
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(BULKIO.PortUsageType.IDLE, corbaPort.state());
    }

    /**
     * Tests that SRI changes are reported correctly from getPacket().
     */
    @Test
    public void testSriChanged()
    {
        helpers.SriListener listener = new helpers.SriListener();
        port.setSriListener(listener);

        // Create a default SRI and push it, which should trigger the callback
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("sri_changed");
        corbaPort.pushSRI(sri);
        Assert.assertEquals(1, listener.newSRIs.size());

        // SRI should report changed for first packet
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertTrue(packet.sriChanged);

        // No SRI change for second packet
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(1, listener.newSRIs.size());
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertFalse(packet.sriChanged);

        // Change the SRI, should call the change listener and flag the packet
        sri = bulkio.sri.utils.create("sri_changed");
        sri.mode = 1;
        corbaPort.pushSRI(sri);
        Assert.assertEquals(1, listener.newSRIs.size());
        Assert.assertEquals(1, listener.changedSRIs.size());
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertTrue(packet.sriChanged);
    }

    @Test
    public void testSriChangedFlush()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("sri_changed_flush");
        corbaPort.pushSRI(sri);

        // Reduce the queue size so we can force a flush
        port.setMaxQueueDepth(2);

        // Push a packet, change the SRI (using a new SRI to avoid accidental
        // "same object" issues), and push two more packets so that the packet
        // with the associated SRI change gets flushed
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        sri = bulkio.sri.utils.create(sri.streamID, 2.0);
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);

        // Get the last packet and verify that the queue has flushed, and the
        // SRI change is still reported
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertTrue(packet.inputQueueFlushed);
        Assert.assertTrue(packet.sriChanged);
    }

    /**
     * Tests that the callback is triggered and SRI changes are reported for an
     * unknown stream ID.
     */
    @Test
    public void testSriChangedInvalidStream()
    {
        final String stream_id = "invalid_stream";

        // Turn off the port's logging to avoid dumping a warning to the screen
        port.getLogger().setLevel(Level.OFF);

        // Push data without an SRI to check that the sriChanged flag is still
        // set and the SRI callback gets called
        helpers.SriListener listener = new helpers.SriListener();
        port.setSriListener(listener);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_id);
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertTrue(packet.sriChanged);
        Assert.assertEquals(1, listener.newSRIs.size());
        Assert.assertEquals(0, listener.changedSRIs.size());
        Assert.assertEquals(stream_id, listener.newSRIs.get(0).streamID);

        // Push again to the same stream ID; sriChanged should now be false and the
        // SRI callback should not get called
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, stream_id);
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertFalse(packet.sriChanged);
        Assert.assertEquals(1, listener.newSRIs.size());
        Assert.assertEquals(0, listener.changedSRIs.size());

        // Push to an invalid stream with no logger, ensure that nothing fails
        port.setLogger((RHLogger) null);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, "null_logger");
    }

    @Test
    public void testStatistics()
    {
        // Push a packet of data to trigger meaningful statistics
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("port_stats");
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 1024, bulkio.time.utils.now(), false, sri.streamID);
       
        // Check that the statistics report the right element size
        BULKIO.PortStatistics stats = corbaPort.statistics();
        Assert.assertTrue(stats.elementsPerSecond > 0.0);
        int bits_per_element = Math.round(stats.bitsPerSecond / stats.elementsPerSecond);
        Assert.assertEquals(helper.bitsPerElement(), bits_per_element);
    }

    @Test
    public void testDiscardEmptyPacket()
    {
        // Push an empty, non-EOS packet
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("empty_packet");
        corbaPort.pushSRI(sri);
        helper.pushTestPacket(port, 0, bulkio.time.utils.now(), false, sri.streamID);

        // No packet should be returned
        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNull(packet);
    }

    @Test
    public void testQueueFlushFlags()
    {
        // Push 1 packet for the normal data stream
        BULKIO.StreamSRI sri_data = bulkio.sri.utils.create("stream_data", 1.0, BULKIO.UNITS_TIME.value, false);
        corbaPort.pushSRI(sri_data);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri_data.streamID);

        // Push 1 packet for the EOS test stream
        BULKIO.StreamSRI sri_eos = bulkio.sri.utils.create("stream_eos", 1.0, BULKIO.UNITS_TIME.value, false);
        corbaPort.pushSRI(sri_eos);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri_eos.streamID);

        // Push 1 packet for the SRI change stream
        BULKIO.StreamSRI sri_change = bulkio.sri.utils.create("stream_change", 1.0, BULKIO.UNITS_TIME.value, false);
        sri_change.mode = 0;
        corbaPort.pushSRI(sri_change);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri_change.streamID);

        // Grab the packets to ensure the initial conditions are correct
	DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(sri_data.streamID, packet.streamID);

        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(sri_eos.streamID, packet.streamID);

        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(sri_change.streamID, packet.streamID);

        // Push an EOS packet for the EOS stream
        helper.pushTestPacket(port, 0, bulkio.time.utils.notSet(), true, sri_eos.streamID);

        // Modify the SRI for the SRI change stream and push another packet
        // (note that we need to create a new StreamSRI object, otherwise the
        // change won't be registered)
        sri_change = bulkio.sri.utils.create("stream_change", 1.0, BULKIO.UNITS_TIME.value, false);
        sri_change.mode = 1;
        corbaPort.pushSRI(sri_change);
        helper.pushTestPacket(port, 2, bulkio.time.utils.now(), false, sri_change.streamID);

        // Cause a queue flush by lowering the ceiling and pushing packets
        port.setMaxQueueDepth(3);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri_data.streamID);
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri_data.streamID);

        // Push another packet for the SRI change stream
        helper.pushTestPacket(port, 2, bulkio.time.utils.now(), false, sri_change.streamID);

        // 1st packet should be for EOS stream, with no data or SRI change flag
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(sri_eos.streamID, packet.streamID);
        Assert.assertTrue("Input queue flush should be reported", packet.inputQueueFlushed);
        Assert.assertTrue("EOS should be reported", packet.EOS);
        Assert.assertFalse("SRI change should not be reported", packet.sriChanged);
        Assert.assertEquals("EOS packet should contain no data", 0, helper.dataLength(packet.dataBuffer));

        // 2nd packet should be for data stream, with no EOS or SRI change flag
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(sri_data.streamID, packet.streamID);
        Assert.assertFalse("Input queue flush should not be reported", packet.inputQueueFlushed);
        Assert.assertFalse("EOS should not be reported", packet.EOS);
        Assert.assertFalse("SRI change should not be reported", packet.sriChanged);

        // 3rd packet should contain the "lost" SRI change flag
        packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertEquals(sri_change.streamID, packet.streamID);
        Assert.assertFalse("Input queue flush should not be reported", packet.inputQueueFlushed);
        Assert.assertFalse("EOS should not be reported", packet.EOS);
        Assert.assertTrue("SRI change should be reported", packet.sriChanged);
    }

    @Test
    public void testQueueSize()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("queue_size", 1.0, BULKIO.UNITS_TIME.value, false);
        corbaPort.pushSRI(sri);

        // Start with a reasonably small queue depth and check that a flush
        // occurs at the expected time
        port.setMaxQueueDepth(10);
        for (int ii = 0; ii < 10; ++ii) {
            helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        }
        Assert.assertEquals(10, port.getCurrentQueueDepth());
        helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(1, port.getCurrentQueueDepth());

        DataTransfer<A> packet = port.getPacket(bulkio.Const.NON_BLOCKING);
        Assert.assertNotNull(packet);
        Assert.assertTrue("Input queue flush not reported", packet.inputQueueFlushed);

        // Set queue depth to unlimited and push a lot of packets
        port.setMaxQueueDepth(-1);       
        final int QUEUE_SIZE = 250;
        for (int ii = 0; ii < QUEUE_SIZE; ++ii) {
            helper.pushTestPacket(port, 1, bulkio.time.utils.now(), false, sri.streamID);
        }
        Assert.assertEquals(QUEUE_SIZE, port.getCurrentQueueDepth());
        for (int ii = 0; ii < QUEUE_SIZE; ++ii) {
            packet = port.getPacket(bulkio.Const.NON_BLOCKING);
            Assert.assertNotNull(packet);
            Assert.assertFalse("Input queue flush reported with unlimited queue size", packet.inputQueueFlushed);
        }
    }
}
