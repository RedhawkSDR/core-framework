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
 * WARNING: This file is generated from NumericOutPortTestImpl.java.template.
 *          Do not modify directly.
 */

package impl;

import java.util.Arrays;

import org.junit.*;
import org.junit.runner.RunWith;

import helpers.TestHelper;

public class NumericOutPortTestImpl<E extends BULKIO.updateSRIOperations,A> extends ChunkingOutPortTestImpl<E,A> {

    public NumericOutPortTestImpl(TestHelper<E,A> helper)
    {
        super(helper);
    }

    @Test
    public void testPushChunkingComplex()
    {
        // Set up a complex stream
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("push_chunking_complex");
        sri.mode = 1;
        sri.xdelta = 0.0625;
        port.pushSRI(sri);

        // Test that the push is properly chunked
        BULKIO.PrecisionUTCTime time = bulkio.time.utils.create(0.0, 0.0);
        _testPushOversizedPacket(time, false, sri.streamID);

        // Check that each packet contains an even number of samples (i.e., no
        // complex value was split)
        for (int index = 0; index < stub.packets.size(); index++) {
            int packet_length = helper.dataLength(stub.packets.get(index).data);
            Assert.assertEquals("Packet contains a partial complex value", 0, packet_length % 2);
        }

        // Check that the synthesized time stamp(s) advanced by the expected
        // time
        for (int index = 1; index < stub.packets.size(); index++) {
            double expected = helper.dataLength(stub.packets.get(index-1).data) * 0.5 * sri.xdelta;
            BULKIO.PrecisionUTCTime prev = stub.packets.get(index-1).T;
            BULKIO.PrecisionUTCTime curr = stub.packets.get(index).T;
            double elapsed = bulkio.time.utils.difference(curr, prev);
            Assert.assertEquals("Incorrect time stamp delta", expected, elapsed, 0.0);
        }
    }

    @Test
    public void testPushChunkingSubsizeComplex()
    {
        // Set up a 2-dimensional complex stream
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("push_chunking_subsize_complex");
        sri.subsize = 2048;
        sri.mode = 1;
        port.pushSRI(sri);

        _testPushOversizedPacket(bulkio.time.utils.now(), false, sri.streamID);

        // Check that each packet is a multiple of the subsize (except the
        // last, because the oversized packet was not explicitly quantized to
        // be an exact multiple)
        int frame_size = sri.subsize * 2;
        for (int index = 0; index < (stub.packets.size() - 1); index++) {
            int packet_length = helper.dataLength(stub.packets.get(index).data);
            Assert.assertEquals("Packet size is not a multiple of subsize", 0, packet_length % frame_size);
        }
    }
}
