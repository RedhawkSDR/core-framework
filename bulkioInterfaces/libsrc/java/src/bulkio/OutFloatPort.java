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

import java.util.Collections;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.List;
import java.util.Iterator;
import java.util.Map;
import java.util.Map.Entry;
import org.omg.CORBA.TCKind;
import org.ossie.properties.AnyUtils;
import org.apache.log4j.Logger;
import CF.DataType;
import java.util.ArrayDeque;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;
import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import BULKIO.UsesPortStatistics;
import ExtendedCF.UsesConnection;
import BULKIO.PortUsageType;
import BULKIO.dataFloatOperations;

import bulkio.linkStatistics;
import bulkio.FloatSize;
import bulkio.ConnectionEventListener;
import bulkio.SizeOf;
import bulkio.connection_descriptor_struct;
import bulkio.SriMapStruct;
import org.ossie.properties.*;


/**
 * 
 */
public class OutFloatPort extends BULKIO.UsesPortStatisticsProviderPOA {


    /**
     * @generated
     */
    protected String name;

    /**
     * @generated
     */
    protected Object updatingPortsLock;

    /**
     * @generated
     */
    protected boolean active;

    /**
     * Map of connection Ids to port objects
     * @generated
     */
    protected Map<String, dataFloatOperations> outConnections = null;

    /**
     * Map of connection ID to statistics
     * @generated
     */
    protected Map<String, linkStatistics > stats;

    /**
     * Map of stream IDs to streamSRI's
     * @generated
     */
    protected Map<String, SriMapStruct > currentSRIs;

    /**
     *
     */
    protected Logger   logger = null;


    /**
     * Event listener when connect/disconnet events happen
     */
    protected ConnectionEventListener   callback = null;

    protected List<connection_descriptor_struct> filterTable = null;

    /**
     * CORBA transfer limit in bytes
     */
    // Multiply by some number < 1 to leave some margin for the CORBA header
    protected static final int MAX_PAYLOAD_SIZE = (int)(Const.MAX_TRANSFER_BYTES * 0.9);

    /**
     * CORBA transfer limit in samples
     */
    protected static int MAX_SAMPLES_PER_PUSH = MAX_PAYLOAD_SIZE/FloatSize.bytes();


    public OutFloatPort(String portName ){
        this( portName, null, null );
    }

    public OutFloatPort(String portName,
                       Logger logger ) {
        this( portName, logger, null );
    }

    /**
     * @generated
     */
    public OutFloatPort(String portName,
                       Logger logger,
                       ConnectionEventListener  eventCB ) {
        name = portName;
        updatingPortsLock = new Object();
        active = false;
        outConnections = new HashMap<String, dataFloatOperations>();
        stats = new HashMap<String, linkStatistics >();
        currentSRIs = new HashMap<String, SriMapStruct>();
        callback = eventCB;
        this.logger = logger;
        filterTable = null;
        // make sure MAX_SAMPLES_PER_PUSH is even so that complex data case is handled properly 
        if (MAX_SAMPLES_PER_PUSH%2 != 0){
            MAX_SAMPLES_PER_PUSH--;
        }
        if ( this.logger != null ) {
            this.logger.debug( "bulkio.OutPort CTOR port: " + portName ); 
        }
    }

    public void setLogger( Logger newlogger ){
        synchronized (this.updatingPortsLock) {
	    logger = newlogger;
	}
    }

    public void setConnectionEventListener( ConnectionEventListener newListener ){
        synchronized (this.updatingPortsLock) {
	    callback = newListener;
	}
    }

    /**
     * @generated
     */
    public PortUsageType state() {
        PortUsageType state = PortUsageType.IDLE;

        if (this.outConnections.size() > 0) {
            state = PortUsageType.ACTIVE;
        }

        return state;
    }

    /**
     * @generated
     */
    public void enableStats(final boolean enable)
    {
        for (String connId : outConnections.keySet()) {
            stats.get(connId).setEnabled(enable);
        }
    };

    /**
     * @generated
     */
    public UsesPortStatistics[] statistics() {
        List<UsesPortStatistics> portStats = new ArrayList<UsesPortStatistics>();
        synchronized (this.updatingPortsLock) {
            for (String connId : this.outConnections.keySet()) {
                portStats.add(new UsesPortStatistics(connId, this.stats.get(connId).retrieve()));
            }
        }
        return portStats.toArray(new UsesPortStatistics[portStats.size()]);
    }

    /**
     * @generated
     */
    public StreamSRI[] activeSRIs()
    {
        ArrayList<StreamSRI> sriList = new ArrayList<StreamSRI>();
        for(Map.Entry<String, SriMapStruct > entry: this.currentSRIs.entrySet()) {
            SriMapStruct srimap = entry.getValue();
            sriList.add(srimap.sri);
        }
        return sriList.toArray(new StreamSRI[0]);
    }

    /**
     * @generated
     */
    public boolean isActive() {
        return this.active;
    }

    /**
     * @generated
     */
    public void setActive(final boolean active) {
        this.active = active;
    }

    /**
     * @generated
     */
    public String getName() {
        return this.name;
    }

    /**
     * @generated
     */
    public HashMap<String, dataFloatOperations> getPorts() {
        return new HashMap<String, dataFloatOperations>();
    }

    /**
     * pushSRI
     *     description: send out SRI describing the data payload
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
     * @generated
     */
    public void pushSRI(StreamSRI header)
    {
	if ( logger != null ) {
	    logger.trace("bulkio.OutPort pushSRI  ENTER (port=" + name +")" );
	}

        // Header cannot be null
        if (header == null) {
	    if ( logger != null ) {
		logger.trace("bulkio.OutPort pushSRI  EXIT (port=" + name +")" );
	    }
	    return;
	}

        if (header.streamID == null) {
            throw new NullPointerException("SRI streamID cannot be null");
        }

        // Header cannot have null keywords
        if (header.keywords == null) header.keywords = new DataType[0];

        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in
            this.currentSRIs.put(header.streamID, new SriMapStruct(header));
            if (this.active) {
		// state if this port is not listed in the filter table... then pushSRI down stream
		boolean portListed = false;

		// for each connection
                for (Entry<String, dataFloatOperations> p : this.outConnections.entrySet()) {
		    
		    // if connection is in the filter table
                    for (connection_descriptor_struct ftPtr : bulkio.utils.emptyIfNull(this.filterTable) ) {

			// if there is an entry for this port in the filter table....so save that state
			if (ftPtr.port_name.getValue().equals(this.name)) {
			    portListed = true;		    
			}
			if ( logger != null ) {
			    logger.trace( "pushSRI - FilterMatch port:" + this.name + " connection:" + p.getKey() + 
					    " streamID:" + header.streamID ); 
			}
			if ( (ftPtr.port_name.getValue().equals(this.name)) &&
			     (ftPtr.connection_id.getValue().equals(p.getKey())) &&
			     (ftPtr.stream_id.getValue().equals(header.streamID))) {
                            try {
				if ( logger != null ) {
				    logger.trace( "pushSRI - FilterMatch port:" + this.name + " connection:" + p.getKey() + 
						  " streamID:" + header.streamID ); 
				}
				p.getValue().pushSRI(header);
                                //Update entry in currentSRIs
                                this.currentSRIs.get(header.streamID).connections.add(p.getKey());

                            } catch(Exception e) {
                                if ( logger != null ) {
				    logger.error("Call to pushSRI failed on port " + name + " connection " + p.getKey() );
                                }
                            }
                        }
                    }
		}

		// no entry exists for this port in the filter table so all connections get SRI data
		if (!portListed ) {
		    for (Entry<String, dataFloatOperations> p : this.outConnections.entrySet()) {
                        try {
			    if ( logger != null ) {
				logger.trace( "pushSRI - NO Filter port:" + this.name + " connection:" + p.getKey() + 
					      " streamID:" + header.streamID ); 
			    }
			    p.getValue().pushSRI(header);
                            //Update entry in currentSRIs
                            this.currentSRIs.get(header.streamID).connections.add(p.getKey());
			} catch(Exception e) {
			    if ( logger != null ) {
				logger.error("Call to pushSRI failed on port " + name + " connection " + p.getKey() );
			    }
                        }
                    }
                }


            }


        }    // don't want to process while command information is coming in


	if ( logger != null ) {
	    logger.trace("bulkio.OutPort pushSRI  EXIT (port=" + name +")" );
	}
        return;
    }

    public void updateConnectionFilter(List<connection_descriptor_struct> _filterTable) {
        this.filterTable = _filterTable;
    }

    private void _pushPacket(
        float[] data,
        PrecisionUTCTime time,
        boolean endOfStream,
        String streamID)
    {
        float[] odata = data;
        SriMapStruct sriStruct = this.currentSRIs.get(streamID);
        if (this.active) {
	    boolean portListed = false;
            for (Entry<String, dataFloatOperations> p : this.outConnections.entrySet()) {

		for (connection_descriptor_struct ftPtr : bulkio.utils.emptyIfNull(this.filterTable) ) {

		    if (ftPtr.port_name.getValue().equals(this.name)) {
			portListed = true;		    
		    }
		    if ( (ftPtr.port_name.getValue().equals(this.name)) && 
			 (ftPtr.connection_id.getValue().equals(p.getKey())) && 
			 (ftPtr.stream_id.getValue().equals(streamID)) ) {
			try {
                            //If SRI for given streamID has not been pushed to this connection, push it
                            if (!sriStruct.connections.contains(p.getKey())){
                                p.getValue().pushSRI(sriStruct.sri);
                                sriStruct.connections.add(p.getKey());
                            }
			    p.getValue().pushPacket( odata, time, endOfStream, streamID);
			    this.stats.get(p.getKey()).update( odata.length, (float)0.0, endOfStream, streamID, false);
			} catch(Exception e) {
			    if ( logger != null ) {
				logger.error("Call to pushPacket failed on port " + name + " connection " + p.getKey() );
			    }
			}
		    }
		}
	    }

	    if (!portListed ){
		for (Entry<String, dataFloatOperations> p : this.outConnections.entrySet()) {
		    try {
                        //If SRI for given streamID has not been pushed to this connection, push it
                        if (!sriStruct.connections.contains(p.getKey())){
                            p.getValue().pushSRI(sriStruct.sri);
                            sriStruct.connections.add(p.getKey());
                        }
			p.getValue().pushPacket( odata, time, endOfStream, streamID);
			this.stats.get(p.getKey()).update( odata.length, (float)0.0, endOfStream, streamID, false);
		    } catch(Exception e) {
			if ( logger != null ) {
			    logger.error("Call to pushPacket failed on port " + name + " connection " + p.getKey() );
			}
		    }
		}
	    }
	}

        if ( endOfStream ) {
            if ( this.currentSRIs.containsKey(streamID) ) {
                this.currentSRIs.remove(streamID);
            }
        }
        return;
    }

    private void pushOversizedPacket(
            float[] data,
            PrecisionUTCTime time,
            boolean endOfStream,
            String streamID)
    {
        // If there is no need to break data into smaller packets, skip
        // straight to the pushPacket call and return.
        if (data.length <= MAX_SAMPLES_PER_PUSH) {
            _pushPacket(data, time, endOfStream, streamID);
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
        for (int offset = 0; offset < data.length;) {
            // Don't send more samples than are remaining
            int pushSize = java.lang.Math.min(data.length-offset, MAX_SAMPLES_PER_PUSH);

            // Copy the range for this sub-packet and advance the offset
            float[] subPacket = Arrays.copyOfRange(data, offset, offset+pushSize);
            offset += pushSize;

            // Send end-of-stream as false for all sub-packets except for the
            // last one (when there are no samples remaining after this push),
            // which gets the input EOS.
            boolean packetEOS = false;
            if (offset == data.length) {
                packetEOS = endOfStream;
            }

            if ( logger != null ) {
                logger.trace("bulkio.OutPort pushOversizedPacket() calling pushPacket with pushSize " + pushSize + " and packetTime twsec: " + packetTime.twsec + " tfsec: " + packetTime.tfsec);
            }
            this._pushPacket(subPacket, packetTime, packetEOS, streamID);
            packetTime = bulkio.time.utils.addSampleOffset(packetTime, pushSize, xdelta);
        }
    }

    /**
     * @generated
     */
    public void pushPacket(float[] data, PrecisionUTCTime time, boolean endOfStream, String streamID)
    {
        if ( logger != null ) {
            logger.trace("bulkio.OutPort pushPacket  ENTER (port=" + name +")" );
        }

        synchronized(this.updatingPortsLock) {    // don't want to process while command information is coming in

            if (!this.currentSRIs.containsKey(streamID)) {
                StreamSRI header = bulkio.sri.utils.create();
                header.streamID = streamID;
                this.pushSRI(header);
            }

            pushOversizedPacket(data, time, endOfStream, streamID);
        }    // don't want to process while command information is coming in

        if ( logger != null ) {
            logger.trace("bulkio.OutPort pushPacket  EXIT (port=" + name +")" );
        }
        return;

    }


    /**
     * @generated
     */
    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort
    {

        if ( logger != null ) {
            logger.trace("bulkio.OutPort connectPort ENTER (port=" + name +")" );
        }

        synchronized (this.updatingPortsLock) {
            final dataFloatOperations port;
            try {
                port = BULKIO.jni.dataFloatHelper.narrow(connection);
            } catch (final Exception ex) {
                if ( logger != null ) {
                    logger.error("bulkio.OutPort CONNECT PORT: " + name + " PORT NARROW FAILED");
                }
                throw new CF.PortPackage.InvalidPort((short)1, "Invalid port for connection '" + connectionId + "'");
            }
            this.outConnections.put(connectionId, port);
            this.active = true;
            this.stats.put(connectionId, new linkStatistics( this.name, new FloatSize() ) );

            if ( logger != null ) {
                logger.debug("bulkio.OutPort CONNECT PORT: " + name + " CONNECTION '" + connectionId + "'");
            }
        }

        if ( logger != null ) {
            logger.trace("bulkio.OutPort connectPort EXIT (port=" + name +")" );
        }

        if ( callback != null ) {
            callback.connect(connectionId);
        }
    }

    /**
     * @generated
     */
    public void disconnectPort(String connectionId) {
        if ( logger != null ) {
            logger.trace("bulkio.OutPort disconnectPort ENTER (port=" + name +")" );
        }
        synchronized (this.updatingPortsLock) {
            boolean portListed = false;
            for (connection_descriptor_struct ftPtr : bulkio.utils.emptyIfNull(this.filterTable)) {
                if (ftPtr.port_name.getValue().equals(this.name)) {
                    portListed = true;
                    break;
                }
            }
            dataFloatOperations port = this.outConnections.remove(connectionId);
            if (port != null)
            {
                float[] odata = new float[0];
                BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.notSet();
                for (Map.Entry<String, SriMapStruct > entry: this.currentSRIs.entrySet()) {
                    String streamID = entry.getKey();
                    if (entry.getValue().connections.contains(connectionId)) {
                        if (portListed) {
                            for (connection_descriptor_struct ftPtr : bulkio.utils.emptyIfNull(this.filterTable) ) {
                                if ( (ftPtr.port_name.getValue().equals(this.name)) &&
	                        	 (ftPtr.connection_id.getValue().equals(connectionId)) &&
					 (ftPtr.stream_id.getValue().equals(streamID))) {
                                    try {
                                        port.pushPacket(odata,tstamp,true,streamID);
                                    } catch(Exception e) {
                                        if ( logger != null ) {
                                            logger.error("Call to pushPacket failed on port " + name + " connection " + connectionId );
                                        }
                                    }
                                }
                            }
                        } else {
                            try {
                                port.pushPacket(odata,tstamp,true,streamID);
                            } catch(Exception e) {
                                if ( logger != null ) {
                                    logger.error("Call to pushPacket failed on port " + name + " connection " + connectionId );
                                }
                            }
                        }
                    }
                }
            }
            this.stats.remove(connectionId);
            this.active = (this.outConnections.size() != 0);

            // Remove connectionId from any sets in the currentSRIs.connections values
            for(Map.Entry<String, SriMapStruct > entry :  this.currentSRIs.entrySet()) {
                entry.getValue().connections.remove(connectionId);
            }

            if ( logger != null ) {
                logger.trace("bulkio.OutPort DISCONNECT PORT:" + name + " CONNECTION '" + connectionId + "'");
                for(Map.Entry<String, SriMapStruct > entry: this.currentSRIs.entrySet()) {
                    logger.trace("bulkio.OutPort updated currentSRIs key=" + entry.getKey() + ", value.sri=" + entry.getValue().sri + ", value.connections=" + entry.getValue().connections);
                }
            }
        }

        if ( callback != null ) {
            callback.disconnect(connectionId);
        }

        if ( logger != null ) {
            logger.trace("bulkio.OutPort disconnectPort EXIT (port=" + name +")" );
        }
    }

    /**
     * @generated
     */
    public UsesConnection[] connections() {
        final List<UsesConnection> connList = new ArrayList<UsesConnection>();
        synchronized (this.updatingPortsLock) {
            for (Entry<String, dataFloatOperations> ent : this.outConnections.entrySet()) {
                connList.add(new UsesConnection(ent.getKey(), (org.omg.CORBA.Object) ent.getValue()));
            }
        }
        return connList.toArray(new UsesConnection[connList.size()]);
    }

}
