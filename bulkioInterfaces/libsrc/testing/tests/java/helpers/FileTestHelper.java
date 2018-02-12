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

import bulkio.InFilePort;
import bulkio.OutFilePort;

public class FileTestHelper {

    public static final int BITS_PER_ELEMENT = 8;

    public FileTestHelper()
    {
    }

    public String getName()
    {
        return "dataFile";
    }

    public void pushTestPacket(InFilePort port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        String data = makeData(length);
        port.pushPacket(data, time, eos, streamID);
    }

    public void pushTestPacket(OutFilePort port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        String data = makeData(length);
        port.pushPacket(data, time, eos, streamID);
    }

    public int dataLength(String data)
    {
        return data.length();
    }

    public String makeData(int length)
    {
        return new String(new char[length]);
    }
}
