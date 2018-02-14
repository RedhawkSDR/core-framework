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

import org.apache.log4j.Logger;

import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import BULKIO.dataFileOperations;

/**
 * BulkIO output port implementation for dataFile.
 */
public class OutFilePort extends OutDataPort<dataFileOperations,String> {

    public OutFilePort(String portName) {
        this(portName, null, null);
    }

    public OutFilePort(String portName, Logger logger) {
        this(portName, logger, null);
    }

    public OutFilePort(String portName, Logger logger, ConnectionEventListener eventCB) {
        super(portName, logger, eventCB, new FileDataHelper());
        if (this.logger != null) {
            this.logger.debug("bulkio.OutPort CTOR port: " + portName);
        }
    }

    public String getRepid() {
        return BULKIO.dataFileHelper.id();
    }

    protected dataFileOperations narrow(org.omg.CORBA.Object obj) {
        return BULKIO.dataFileHelper.narrow(obj);
    }

    protected void sendPacket(dataFileOperations port, String data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        port.pushPacket(data, time, endOfStream, streamID);
    }
}
