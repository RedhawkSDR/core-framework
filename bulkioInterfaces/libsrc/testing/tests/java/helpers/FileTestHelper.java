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
import bulkio.InFilePort;
import bulkio.OutDataPort;
import bulkio.OutFilePort;

import stubs.Stub;
import stubs.InFilePortStub;

public class FileTestHelper implements TestHelper<BULKIO.dataFileOperations,String> {

    public static final int BITS_PER_ELEMENT = 8;

    public int bitsPerElement()
    {
        return FileTestHelper.BITS_PER_ELEMENT;
    }

    public InFilePort createInPort(String name)
    {
        return new InFilePort(name);
    }

    public OutFilePort createOutPort(String name)
    {
        return new OutFilePort(name);
    }

    public Stub<String> createStub()
    {
        return new InFilePortStub();
    }

    public String getName()
    {
        return "dataFile";
    }

    public BULKIO.dataFileOperations toCorbaType(InDataPort<BULKIO.dataFileOperations,String> port)
    {
        return (BULKIO.dataFileOperations) port;
    }

    public void pushTestPacket(InDataPort<BULKIO.dataFileOperations,String> port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        String data = makeData(length);
        toCorbaType(port).pushPacket(data, time, eos, streamID);
    }

    public void pushTestPacket(OutDataPort<BULKIO.dataFileOperations,String> port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
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
