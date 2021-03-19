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
import BULKIO.dataFileOperations;
import bulkio.OutFileStream;

/**
 * BulkIO output port implementation for dataFile.
 */
public class OutFilePort extends OutDataPort<dataFileOperations,String> {

    protected Map<String, OutFileStream> streams;
    public Object streamsMutex;

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
        this.streams = new HashMap<String, OutFileStream>();
        this.streamsMutex = new Object();
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

    public OutFileStream getStream(String streamID)
    {
        synchronized (this.updatingPortsLock) {
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
        }
        return null;
    }
  
    public OutFileStream[] getStreams()
    {
        OutFileStream[] retval = null;
        Iterator<OutFileStream> streams_iter = streams.values().iterator();
        synchronized (this.streamsMutex) {
            retval = new OutFileStream[streams.size()];
            int streams_idx = 0;
            while (streams_iter.hasNext()) {
                retval[streams_idx] = streams_iter.next();
                streams_idx++;
            }
        }
        return retval;
    }
  
    public OutFileStream createStream(String streamID)
    {
        OutFileStream stream = null;
        synchronized (this.updatingPortsLock) {
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
            stream = new OutFileStream(bulkio.sri.utils.create(streamID), this);
            streams.put(streamID, stream);
        }
        return stream;
    }
  
    public OutFileStream createStream(BULKIO.StreamSRI sri)
    {
        OutFileStream stream = null;
        synchronized (this.updatingPortsLock) {
            String streamID = sri.streamID;
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
            stream = new OutFileStream(sri, this);
            streams.put(streamID, stream);
        }
        return stream;
    }
}
