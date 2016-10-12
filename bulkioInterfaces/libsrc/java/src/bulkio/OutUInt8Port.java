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
package bulkio;

import java.util.Arrays;
import org.apache.log4j.Logger;
import BULKIO.PrecisionUTCTime;
import BULKIO.dataOctetOperations;

/**
 * 
 */
public class OutUInt8Port extends OutDataPort<dataOctetOperations,byte[]> {

    public OutUInt8Port(String portName) {
        this(portName, null, null);
    }

    public OutUInt8Port(String portName, Logger logger) {
        this(portName, logger, null);
    }

    public OutUInt8Port(String portName, Logger logger, ConnectionEventListener eventCB) {
        super(portName, logger, eventCB, new UInt8Size());
        if (this.logger != null) {
            this.logger.debug("bulkio.OutPort CTOR port: " + portName); 
        }
    }

    protected dataOctetOperations narrow(final org.omg.CORBA.Object obj) {
        return BULKIO.jni.dataOctetHelper.narrow(obj);
    }

    protected void sendPacket(dataOctetOperations port, byte[] data, PrecisionUTCTime time,
                              boolean endOfStream, String streamID) {
        port.pushPacket(data, time, endOfStream, streamID);
    }

    protected byte[] copyOfRange(byte[] array, int start, int end) {
        return Arrays.copyOfRange(array, start, end);
    }

    protected int arraySize(byte[] array) {
        return array.length;
    }

    protected byte[] emptyArray() {
        return new byte[0];
    }

	public String getRepid() {
		return BULKIO.dataOctetHelper.id();
	}
}

