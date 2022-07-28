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

import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;

import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import BULKIO.dataXMLOperations;
import bulkio.OutXMLStream;

/**
 * BulkIO output port implementation for dataXML.
 */
public class OutXMLPort extends OutDataPort<dataXMLOperations,String> {

    protected Map<String, OutXMLStream> streams;
    public Object streamsMutex;

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
        this.streams = new HashMap<String, OutXMLStream>();
        this.streamsMutex = new Object();
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

    public OutXMLStream getStream(String streamID)
    {
        synchronized (this.updatingPortsLock) {
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
        }
        return null;
    }
  
    public OutXMLStream[] getStreams()
    {
        OutXMLStream[] retval = null;
        Iterator<OutXMLStream> streams_iter = streams.values().iterator();
        synchronized (this.streamsMutex) {
            retval = new OutXMLStream[streams.size()];
            int streams_idx = 0;
            while (streams_iter.hasNext()) {
                retval[streams_idx] = streams_iter.next();
                streams_idx++;
            }
        }
        return retval;
    }
  
    public OutXMLStream createStream(String streamID)
    {
        OutXMLStream stream = null;
        synchronized (this.updatingPortsLock) {
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
            stream = new OutXMLStream(bulkio.sri.utils.create(streamID), this);
            streams.put(streamID, stream);
        }
        return stream;
    }
  
    public OutXMLStream createStream(BULKIO.StreamSRI sri)
    {
        if (sri == null) {
            return null;
        }
        OutXMLStream stream = null;
        synchronized (this.updatingPortsLock) {
            String streamID = sri.streamID;
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
            stream = new OutXMLStream(sri, this);
            streams.put(streamID, stream);
        }
        return stream;
    }
}

