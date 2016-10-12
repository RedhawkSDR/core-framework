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

import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;

import org.apache.log4j.Logger;

import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;

public abstract class OutDataPort<E extends BULKIO.updateSRIOperations,A> extends OutPortBase<E> {
    /**
     * CORBA transfer limit in bytes
     */
    // Multiply by some number < 1 to leave some margin for the CORBA header
    protected static final int MAX_PAYLOAD_SIZE = (int)(Const.MAX_TRANSFER_BYTES * 0.9);

    /**
     * Size of a single element
     */
    protected final SizeOf sizeof;

    /**
     * CORBA transfer limit in samples
     */
    protected int maxSamplesPerPush;

    protected List<connection_descriptor_struct> filterTable = null;

    protected OutDataPort(String portName, Logger logger, ConnectionEventListener connectionListener, SizeOf size) {
        super(portName, logger, connectionListener);
        this.sizeof = size;
        // Make sure max samples per push is even so that complex data case is
        // handled properly
        this.maxSamplesPerPush = (MAX_PAYLOAD_SIZE/this.sizeof.sizeof()) & 0xFFFFFFFE;
    }

    /**
     * Connects a port to receive data from this port.
     */
    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort
    {
        if (logger != null) {
            logger.trace("bulkio.OutPort connectPort ENTER (port=" + name +")");
        }

        synchronized (this.updatingPortsLock) {
            final E port;
            try {
                port = this.narrow(connection);
            } catch (final Exception ex) {
                if (logger != null) {
                    logger.error("bulkio.OutPort CONNECT PORT: " + name + " PORT NARROW FAILED");
                }
                throw new CF.PortPackage.InvalidPort((short)1, "Invalid port for connection '" + connectionId + "'");
            }
            this.outConnections.put(connectionId, port);
            this.active = true;
            this.stats.put(connectionId, new linkStatistics(this.name, this.sizeof));

            if (logger != null) {
                logger.debug("bulkio.OutPort CONNECT PORT: " + name + " CONNECTION '" + connectionId + "'");
            }
        }

        if (callback != null) {
            callback.connect(connectionId);
        }

        if (logger != null) {
            logger.trace("bulkio.OutPort connectPort EXIT (port=" + name +")");
        }
    }

    /**
     * Breaks a connection.
     */
    public void disconnectPort(String connectionId) {
        if (logger != null) {
            logger.trace("bulkio.OutPort disconnectPort ENTER (port=" + name +")");
        }

        synchronized (this.updatingPortsLock) {
            final E port = this.outConnections.remove(connectionId);
            if (port != null)
            {
                // Create an empty data packet with an invalid timestamp to
                // send with the end-of-stream
                final A data = emptyArray();
                final BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.notSet();
                for (Map.Entry<String, SriMapStruct> entry: this.currentSRIs.entrySet()) {
                    final String streamID = entry.getKey();
                    if (!isStreamRoutedToConnection(streamID, connectionId)) {
                        continue;
                    }

                    final SriMapStruct sriMap = entry.getValue();
                    if (sriMap.connections.contains(connectionId)) {
                        try {
                            sendPacket(port, data, tstamp, true, streamID);
                        } catch(Exception e) {
                            if (logger != null) {
                                logger.error("Call to pushPacket failed on port " + name + " connection " + connectionId);
                            }
                        }
                    }
                }
            }

            this.stats.remove(connectionId);
            this.active = (this.outConnections.size() != 0);

            // Remove connectionId from any sets in the currentSRIs.connections values
            for (Map.Entry<String,SriMapStruct> entry :  this.currentSRIs.entrySet()) {
                entry.getValue().connections.remove(connectionId);
            }

            if (logger != null) {
                logger.trace("bulkio.OutPort DISCONNECT PORT:" + name + " CONNECTION '" + connectionId + "'");
                for(Map.Entry<String,SriMapStruct> entry: this.currentSRIs.entrySet()) {
                    logger.trace("bulkio.OutPort updated currentSRIs key=" + entry.getKey() + ", value.sri=" + entry.getValue().sri + ", value.connections=" + entry.getValue().connections);
                }
            }
        }

        if (callback != null) {
            callback.disconnect(connectionId);
        }

        if (logger != null) {
            logger.trace("bulkio.OutPort disconnectPort EXIT (port=" + name +")");
        }
    }

    /**
     * Sends an array of samples.
     */
    public void pushPacket(A data, PrecisionUTCTime time, boolean endOfStream, String streamID)
    {
        if (logger != null) {
            logger.trace("bulkio.OutPort pushPacket  ENTER (port=" + this.name +")");
        }

        synchronized(this.updatingPortsLock) {
            if (!this.currentSRIs.containsKey(streamID)) {
                StreamSRI header = bulkio.sri.utils.create();
                header.streamID = streamID;
                this.pushSRI(header);
            }

            pushOversizedPacket(data, time, endOfStream, streamID);
        }

        if (logger != null) {
            logger.trace("bulkio.OutPort pushPacket  EXIT (port=" + this.name +")");
        }
    }

    /**
     * Sends out SRI describing the data payload.
     *
     *  H: structure of type BULKIO.StreamSRI with the SRI for this stream
     *    hversion
     *    xstart: start time of the stream
     *    xdelta: delta between two samples
     *    xunits: unit types from Platinum specification
     *    subsize: 0 if the data is one-dimensional
     *    ystart
     *    ydelta
     *    yunits: unit types from Platinum specification
     *    mode: 0-scalar, 1-complex
     *    streamID: stream identifier
     *    sequence<CF::DataType> keywords: unconstrained sequence of key-value pairs for additional description
     */
    public void pushSRI(StreamSRI header)
    {
        if (logger != null) {
            logger.trace("bulkio.OutPort pushSRI  ENTER (port=" + name +")");
        }

        // Header cannot be null
        if (header == null) {
            if (logger != null) {
                logger.trace("bulkio.OutPort pushSRI  EXIT (port=" + name +")");
            }
            return;
        }

        // Null streamID is a usage error
        if (header.streamID == null) {
            throw new NullPointerException("SRI streamID cannot be null");
        }

        // Header cannot have null keywords
        if (header.keywords == null) {
            header.keywords = new CF.DataType[0];
        }

        synchronized (this.updatingPortsLock) {
            this.currentSRIs.put(header.streamID, new SriMapStruct(header));
            if (this.active) {
                for (Entry<String,E> entry : this.outConnections.entrySet()) {
                    final String connectionID = entry.getKey();

                    // Check filter for route
                    if (!isStreamRoutedToConnection(header.streamID, connectionID)) {
                        continue;
                    }

                    final E port = entry.getValue();
                    try {
                        port.pushSRI(header);

                        // Update entry in currentSRIs
                        this.currentSRIs.get(header.streamID).connections.add(connectionID);
                    } catch (Exception e) {
                        if (logger != null) {
                            logger.error("Call to pushSRI failed on port " + name + " connection " + connectionID);
                        }
                    }
                }
            }
        }

        if (logger != null) {
            logger.trace("bulkio.OutPort pushSRI  EXIT (port=" + name +")");
        }
    }

    public void updateConnectionFilter(List<connection_descriptor_struct> _filterTable) {
        this.filterTable = _filterTable;
    }

    protected boolean isStreamRoutedToConnection(final String streamID, final String connectionID)
    {
        // Is this port listed in the filter table?
        boolean portListed = false;

        // Check the filter table for this stream/connection pair.
        for (connection_descriptor_struct filter : bulkio.utils.emptyIfNull(this.filterTable)) {
            // Ignore filters for other ports
            if (!this.name.equals(filter.port_name.getValue())) {
                continue;
            }
            // Filtering is in effect for this port
            portListed = true;

            if (connectionID.equals(filter.connection_id.getValue()) &&
                streamID.equals(filter.stream_id.getValue())) {
                if (logger != null) {
                    logger.trace("OutPort FilterMatch port:" + this.name + " connection:" + connectionID +
                                 " streamID:" + streamID);
                }
                return true;
            }
        }

        // If the port was not listed and we made it to here, there is no
        // filter in effect, so send the packet or SRI; otherwise, it was
        // listed and there is no route.
        if (!portListed) {
            if (logger != null) {
                logger.trace("OutPort NO Filter port:" + this.name + " connection:" + connectionID +
                             " streamID:" + streamID);
            }
            return true;
        } else {
            return false;
        }
    }

    private void pushOversizedPacket(A data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        final int length = arraySize(data);

        // If there is no need to break data into smaller packets, skip
        // straight to the pushPacket call and return.
        SriMapStruct sriStruct = this.currentSRIs.get(streamID);
        if (sriStruct.sri.subsize != 0) {
            if (this.maxSamplesPerPush%sriStruct.sri.subsize != 0) {
                this.maxSamplesPerPush = (MAX_PAYLOAD_SIZE/this.sizeof.sizeof()) & 0xFFFFFFFE;
                while (this.maxSamplesPerPush%sriStruct.sri.subsize != 0) {
                    this.maxSamplesPerPush -= this.maxSamplesPerPush%sriStruct.sri.subsize;
                    if (this.maxSamplesPerPush%2 != 0){
                        this.maxSamplesPerPush--;
                    }
                }
            }
        }
        if (length <= this.maxSamplesPerPush) {
            this.pushSinglePacket(data, time, endOfStream, streamID);
            return;
        }

        // Determine xdelta for this streamID to be used for time increment for subpackets
        SriMapStruct sriMap = this.currentSRIs.get(streamID);
        double xdelta = 0.0;
        if (sriMap != null){
            xdelta = sriMap.sri.xdelta;
        }

        // Initialize time of first subpacket
        PrecisionUTCTime packetTime = time;
        for (int offset = 0; offset < length;) {
            // Don't send more samples than are remaining
            final int pushSize = java.lang.Math.min(length-offset, this.maxSamplesPerPush);

            // Copy the range for this sub-packet and advance the offset
            A subPacket = copyOfRange(data, offset, offset+pushSize);
            offset += pushSize;

            // Send end-of-stream as false for all sub-packets except for the
            // last one (when there are no samples remaining after this push),
            // which gets the input EOS.
            boolean packetEOS = false;
            if (offset == length) {
                packetEOS = endOfStream;
            }

            if (logger != null) {
                logger.trace("bulkio.OutPort pushOversizedPacket() calling pushPacket with pushSize " + pushSize + " and packetTime twsec: " + packetTime.twsec + " tfsec: " + packetTime.tfsec);
            }
            this.pushSinglePacket(subPacket, packetTime, packetEOS, streamID);
            int data_xfer_len = pushSize;
            if (sriMap != null){
                if (sriMap.sri.mode == 1) {
                    data_xfer_len = data_xfer_len / 2;
                }
            }
            packetTime = bulkio.time.utils.addSampleOffset(packetTime, data_xfer_len, xdelta);
        }
    }

    private void pushSinglePacket(A data, PrecisionUTCTime time, boolean endOfStream, String streamID)
    {
        final int length = arraySize(data);
        SriMapStruct sriStruct = this.currentSRIs.get(streamID);
        if (this.active) {
            for (Entry<String,E> entry : this.outConnections.entrySet()) {
                final String connectionID = entry.getKey();

                // Check filter for route
                if (!isStreamRoutedToConnection(streamID, connectionID)) {
                    continue;
                }

                final E port = entry.getValue();
                try {
                    // If SRI for given streamID has not been pushed to this connection, push it
                    if (!sriStruct.connections.contains(connectionID)) {
                        port.pushSRI(sriStruct.sri);
                        sriStruct.connections.add(connectionID);
                    }

                    this.sendPacket(port, data, time, endOfStream, streamID);
                    this.stats.get(connectionID).update(length, (float)0.0, endOfStream, streamID, false);
                } catch (Exception e) {
                    if (logger != null) {
                        logger.error("Call to pushPacket failed on port " + name + " connection " + connectionID);
                    }
                }
            }
	}
	if (endOfStream) {
	    if (this.currentSRIs.containsKey(streamID)) {
		this.currentSRIs.remove(streamID);
	    }
	}
	return;
    }

    protected abstract E narrow(org.omg.CORBA.Object obj);
    protected abstract void sendPacket(E port, A data, PrecisionUTCTime time, boolean endOfStream, String streamID);
    protected abstract A copyOfRange(A array, int start, int end);
    protected abstract int arraySize(A array);
    protected abstract A emptyArray();
}
