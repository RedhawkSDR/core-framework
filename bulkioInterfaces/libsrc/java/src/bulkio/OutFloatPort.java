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
import BULKIO.dataFloatOperations;

/**
 * 
 */
public class OutFloatPort extends OutDataPort<dataFloatOperations,float[]> {

    public OutFloatPort(String portName) {
        this(portName, null, null);
    }

    public OutFloatPort(String portName, Logger logger) {
        this(portName, logger, null);
    }

    public OutFloatPort(String portName, Logger logger, ConnectionEventListener eventCB) {
        super(portName, logger, eventCB, new FloatSize());
        if (this.logger != null) {
            this.logger.debug("bulkio.OutPort CTOR port: " + portName); 
        }
    }

    protected dataFloatOperations narrow(final org.omg.CORBA.Object obj) {
        return BULKIO.jni.dataFloatHelper.narrow(obj);
    }

    protected void sendPacket(dataFloatOperations port, float[] data, PrecisionUTCTime time,
                              boolean endOfStream, String streamID) {
        port.pushPacket(data, time, endOfStream, streamID);
    }

    protected float[] copyOfRange(float[] array, int start, int end) {
        return Arrays.copyOfRange(array, start, end);
    }

    protected int arraySize(float[] array) {
        return array.length;
    }

    protected float[] emptyArray() {
        return new float[0];
    }

	public String getRepid() {
		return BULKIO.dataFloatHelper.id();
	}
}

