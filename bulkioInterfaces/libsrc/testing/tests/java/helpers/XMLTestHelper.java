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
import bulkio.InXMLPort;
import bulkio.OutDataPort;
import bulkio.OutXMLPort;

import stubs.Stub;
import stubs.InXMLPortStub;

public class XMLTestHelper implements TestHelper<BULKIO.dataXMLOperations,String> {

    public static final int BITS_PER_ELEMENT = 8;

    public int bitsPerElement()
    {
        return XMLTestHelper.BITS_PER_ELEMENT;
    }

    public InXMLPort createInPort(String name)
    {
        return new InXMLPort(name);
    }

    public OutXMLPort createOutPort(String name)
    {
        return new OutXMLPort(name);
    }

    public Stub<String> createStub()
    {
        return new InXMLPortStub();
    }

    public String getName()
    {
        return "dataXML";
    }

    public BULKIO.dataXMLOperations toCorbaType(InDataPort<BULKIO.dataXMLOperations,String> port)
    {
        return (BULKIO.dataXMLOperations) port;
    }

    public void pushTestPacket(InDataPort<BULKIO.dataXMLOperations,String> port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        String data = makeData(length);
        toCorbaType(port).pushPacket(data, eos, streamID);
    }

    public void pushTestPacket(OutDataPort<BULKIO.dataXMLOperations,String> port, int length, BULKIO.PrecisionUTCTime time, boolean eos, String streamID)
    {
        String data = makeData(length);
        port.pushPacket(data, null, eos, streamID);
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
