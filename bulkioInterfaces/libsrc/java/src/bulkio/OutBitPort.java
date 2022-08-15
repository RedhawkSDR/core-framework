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
import org.ossie.buffer.bitbuffer;

import java.util.Map;
import java.util.HashMap;
import java.util.Iterator;

import BULKIO.PrecisionUTCTime;
import BULKIO.dataBitOperations;
import BULKIO.StreamSRI;
import bulkio.OutBitStream;

/**
 * BulkIO output port implementation for dataBit.
 */
public class OutBitPort extends ChunkingOutPort<dataBitOperations,BULKIO.BitSequence> {

    protected Map<String, OutBitStream> streams;
    public Object streamsMutex;

    public OutBitPort(String portName) {
        this(portName, null, null);
    }

    public OutBitPort(String portName, Logger logger) {
        this(portName, logger, null);
    }

    public OutBitPort(String portName, Logger logger, ConnectionEventListener eventCB) {
        super(portName, logger, eventCB, new BitSequenceDataHelper());
        if (this.logger != null) {
            this.logger.debug("bulkio.OutPort CTOR port: " + portName);
        }
        this.streams = new HashMap<String, OutBitStream>();
        this.streamsMutex = new Object();
    }

    protected dataBitOperations narrow(org.omg.CORBA.Object obj) {
        return BULKIO.jni.dataBitHelper.narrow(obj);
    }

    protected void sendPacket(dataBitOperations port, BULKIO.BitSequence data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        port.pushPacket(data, time, endOfStream, streamID);
    }

    public String getRepid() {
        return BULKIO.dataBitHelper.id();
    }

    public OutBitStream getStream(String streamID)
    {
        synchronized (this.updatingPortsLock) {
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
        }
        return null;
    }
  
    public OutBitStream[] getStreams()
    {
        OutBitStream[] retval = null;
        Iterator<OutBitStream> streams_iter = streams.values().iterator();
        synchronized (this.streamsMutex) {
            retval = new OutBitStream[streams.size()];
            int streams_idx = 0;
            while (streams_iter.hasNext()) {
                retval[streams_idx] = streams_iter.next();
                streams_idx++;
            }
        }
        return retval;
    }
  
    public OutBitStream createStream(String streamID)
    {
        OutBitStream stream = null;
        synchronized (this.updatingPortsLock) {
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
            stream = new OutBitStream(bulkio.sri.utils.create(streamID), this);
            streams.put(streamID, stream);
        }
        return stream;
    }
  
    public OutBitStream createStream(BULKIO.StreamSRI sri)
    {
        if (sri == null) {
            return null;
        }
        OutBitStream stream = null;
        synchronized (this.updatingPortsLock) {
            String streamID = sri.streamID;
            if (streams.containsKey(streamID)) {
                return streams.get(streamID);
            }
            stream = new OutBitStream(sri, this);
            streams.put(streamID, stream);
        }
        return stream;
    }
}
