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

import bulkio.InBitPort;
import bulkio.OutBitPort;

public class BitTestHelper {
    public BitTestHelper()
    {
    }

    public InBitPort createInPort()
    {
        return new InBitPort("dataBit_in");
    }

    void pushTestPacket(InBitPort port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        int bytes = (length + 7) / 8;
        BULKIO.BitSequence data = new BULKIO.BitSequence(new byte[bytes], length);
        port.pushPacket(data, time, eos, streamID);
    }

    int dataLength(BULKIO.BitSequence data)
    {
        return data.bits;
    }
}
