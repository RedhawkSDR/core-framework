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

import org.junit.*;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

import stubs.Packet;
import helpers.BitTestHelper;

@RunWith(JUnit4.class)
public class OutBitPortTest extends impl.ChunkingOutPortTestImpl<BULKIO.dataBitOperations,BULKIO.BitSequence>
{
    public OutBitPortTest()
    {
        super(new BitTestHelper());
    }

    @Test
    public void testPushPacketData()
    {
        BULKIO.StreamSRI sri = bulkio.sri.utils.create("push_packet");
        port.pushSRI(sri);

        // Create a byte array and fill it with alternating bits
        BULKIO.BitSequence data = new BULKIO.BitSequence();
        data.data = new byte[128];
        for (int ii = 0; ii < data.data.length; ++ii) {
            data.data[ii] = (byte) 0x55;
        }
        data.bits = data.data.length * 8;

        // Check the received data is consistent
        port.pushPacket(data, bulkio.time.utils.now(), false, sri.streamID);
        Assert.assertEquals(1, stub.packets.size());
        Packet<BULKIO.BitSequence> packet = stub.packets.get(0);
        Assert.assertEquals(data.bits, packet.data.bits);
        Assert.assertArrayEquals(data.data, packet.data.data);
    }
}
