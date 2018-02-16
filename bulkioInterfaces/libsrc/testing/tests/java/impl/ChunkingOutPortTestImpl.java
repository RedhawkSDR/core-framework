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
 * WARNING: This file is generated from ChunkingOutPortTestImpl.java.template.
 *          Do not modify directly.
 */

package impl;

import org.junit.*;
import org.junit.runner.RunWith;

import helpers.TestHelper;

public class ChunkingOutPortTestImpl<E extends BULKIO.updateSRIOperations,A> extends OutPortTestImpl<E,A> {

    public ChunkingOutPortTestImpl(TestHelper<E,A> helper)
    {
        super(helper);
    }

    protected void _testPushOversizedPacket(BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        // Pick a sufficiently large number of samples that the packet has to span
        // multiple packets
        final int max_bits = 8 * (int) bulkio.Const.MAX_TRANSFER_BYTES;
        int count = 2 * max_bits / helper.bitsPerElement();
        helper.pushTestPacket(port, count, time, eos, streamID);

        // More than one packet must have been received, and no packet can
        // exceed the max transfer size
        Assert.assertTrue(stub.packets.size() > 1);
        for (int index = 0; index < stub.packets.size(); ++index) {
            int packet_bits = helper.dataLength(stub.packets.get(index).data) * helper.bitsPerElement();
            Assert.assertTrue("Packet too large", packet_bits < max_bits);
        }
    }

    @Test
    public void testPushChunking()
    {
        // Set up a basic stream
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("push_chunking");
        sri.xdelta = 0.125;
        port.pushSRI(sri);

        // Test that the push is properly chunked
        BULKIO.PrecisionUTCTime time = bulkio.time.utils.create(0.0, 0.0);
        _testPushOversizedPacket(time, false, sri.streamID);

        // Check that the synthesized time stamp(s) advanced by the expected time
        for (int index = 1; index < stub.packets.size(); index++) {
            double expected = helper.dataLength(stub.packets.get(index-1).data) * sri.xdelta;
            BULKIO.PrecisionUTCTime prev = stub.packets.get(index-1).T;
            BULKIO.PrecisionUTCTime curr = stub.packets.get(index).T;
            double elapsed = bulkio.time.utils.difference(curr, prev);
            Assert.assertEquals("Incorrect time stamp delta", expected, elapsed, 0.0);
        }
    }

    @Test
    public void testPushChunkingEOS()
    {
        // Set up a basic stream
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("push_chunking_eos");
        port.pushSRI(sri);

        // Test that the push is properly chunked
        _testPushOversizedPacket(bulkio.time.utils.now(), true, sri.streamID);

        // Check that only the final packet has end-of-stream-set
        int packets = stub.packets.size();
        Assert.assertTrue("Last packet does not have EOS set", stub.packets.get(packets-1).EOS);
        for (int index = 0; index < (packets - 1); index++) {
            Assert.assertFalse("Intermediate packet has EOS set", stub.packets.get(index).EOS);
        }
    }

    @Test
    public void testPushChunkingSubsize()
    {
        // Set up a 2-dimensional stream
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("push_chunking_subsize");
        sri.subsize = 1023;
        port.pushSRI(sri);

        _testPushOversizedPacket(bulkio.time.utils.now(), false, sri.streamID);

        // Check that each packet is a multiple of the subsize (except the
        // last, because the oversized packet was not explicitly quantized to
        // be an exact multiple)
        for (int index = 0; index < (stub.packets.size() - 1); index++) {
            int packet_length = helper.dataLength(stub.packets.get(index).data);
            Assert.assertEquals("Packet size is not a multiple of subsize", 0, packet_length % sri.subsize);
        }
    }
}
