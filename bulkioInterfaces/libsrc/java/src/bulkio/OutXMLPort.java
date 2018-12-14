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
import BULKIO.dataXMLOperations;

/**
 * BulkIO output port implementation for dataXML.
 */
public class OutXMLPort extends OutDataPort<dataXMLOperations,String> {

    public OutXMLPort(String portName) {
        this(portName, null, null);
    }

    public OutXMLPort(String portName, Logger logger) {
        this(portName, logger, null);
    }

    public OutXMLPort(String portName, Logger logger, ConnectionEventListener eventCB) {
        super(portName, logger, eventCB, new XMLDataHelper());
        if (this.logger != null) {
            this.logger.debug("bulkio.OutPort CTOR port: " + portName); 
        }
    }

    public String getRepid() {
        return BULKIO.dataXMLHelper.id();
    }

    public void pushPacket(String data, boolean endOfStream, String streamID)
    {
        // Pass a null timestamp; it will never be referenced in the base class
        // and can be safely dropped in sendPacket().
        super.pushPacket(data, null, endOfStream, streamID);
    }

    protected dataXMLOperations narrow(org.omg.CORBA.Object obj) {
        return BULKIO.dataXMLHelper.narrow(obj);
    }

    protected void sendPacket(dataXMLOperations port, String data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        port.pushPacket(data, endOfStream, streamID);
    }
}

