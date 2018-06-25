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

package helpers;

import bulkio.InDataPort;
import bulkio.InBitPort;
import bulkio.OutDataPort;
import bulkio.OutBitPort;

import stubs.Stub;
import stubs.InBitPortStub;

public class BitTestHelper implements TestHelper<BULKIO.dataBitOperations, BULKIO.BitSequence> {

    public static final int BITS_PER_ELEMENT = 1;

    public int bitsPerElement()
    {
        return BitTestHelper.BITS_PER_ELEMENT;
    }

    public InBitPort createInPort(String name)
    {
        return new InBitPort(name);
    }

    public OutBitPort createOutPort(String name)
    {
        return new OutBitPort(name);
    }

    public Stub<BULKIO.BitSequence> createStub()
    {
        return new InBitPortStub();
    }

    public String getName()
    {
        return "dataBit";
    }

    public BULKIO.dataBitOperations toCorbaType(InDataPort<BULKIO.dataBitOperations,BULKIO.BitSequence> port)
    {
        return (BULKIO.dataBitOperations) port;
    }

    public void pushTestPacket(InDataPort<BULKIO.dataBitOperations,BULKIO.BitSequence> port,
                               int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        BULKIO.BitSequence data = makeData(length);
        toCorbaType(port).pushPacket(data, time, eos, streamID);
    }

    public void pushTestPacket(OutDataPort<BULKIO.dataBitOperations, BULKIO.BitSequence> port,
                               int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        BULKIO.BitSequence data = makeData(length);
        port.pushPacket(data, time, eos, streamID);
    }

    public int dataLength(BULKIO.BitSequence data)
    {
        return data.bits;
    }

    public BULKIO.BitSequence makeData(int length)
    {
        int bytes = (length + 7) / 8;
        return new BULKIO.BitSequence(new byte[bytes], length);
    }
}
