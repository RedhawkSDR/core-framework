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

import org.ossie.component.RHLogger;

import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import BULKIO.PortStatistics;
import BULKIO.PortUsageType;

/**
 * 
 */
public class InXMLPort extends BULKIO.jni.dataXMLPOA implements InDataPort<BULKIO.dataXMLOperations,String> {

    /**
     * A class to hold packet data.
     * 
     */
    public class Packet extends DataTransfer < String > {

	public Packet( String data, PrecisionUTCTime timeStamp, boolean endOfStream, String streamID, StreamSRI H, boolean sriChanged, boolean inputQueueFlushed ) {
	    super(data,timeStamp,endOfStream,streamID,H,sriChanged,inputQueueFlushed); 
	};
    };

    private InPortImpl<String> impl;

    /**
     * 
     */
    public InXMLPort( String portName ) {
	this( portName, null, new bulkio.sri.DefaultComparator(), null );
    }

    public InXMLPort( String portName, 
		       bulkio.sri.Comparator compareSRI ) {
	this( portName, null, compareSRI, null );
    }

    public InXMLPort( String portName, 
			bulkio.sri.Comparator compareSRI, 
			bulkio.SriListener sriCallback
		       ) {
	this( portName, null, compareSRI, sriCallback );
    }

    public InXMLPort( String portName, Logger logger ) {
	this( portName, logger, new bulkio.sri.DefaultComparator(), null );
    }

    public InXMLPort( String portName, 
		       Logger logger,
		       bulkio.sri.Comparator compareSRI, 
		       bulkio.SriListener sriCallback ){
        impl = new InPortImpl<String>(portName, logger, compareSRI, sriCallback, new XMLDataHelper());
    }

    public Logger getLogger() {
        return impl.getLogger();
    }

    public void setLogger(Logger logger){
        impl.setLogger(logger);
    }

    public void setLogger(RHLogger logger)
    {
        impl.setLogger(logger);
    }

    /**
     * 
     */
    public void setSriListener(bulkio.SriListener sriCallback) {
        impl.setSriListener(sriCallback);
    }

    /**
     * 
     */
    public String getName() {
        return impl.getName();
    }

    /**
     * 
     */
    public void enableStats(boolean enable) {
        impl.enableStats(enable);
    }

    /**
     * 
     */
    public PortStatistics statistics() {
        return impl.statistics();
    }

    /**
     * 
     */
    public PortUsageType state() {
        return impl.state();
    }

    /**
     * 
     */
    public StreamSRI[] activeSRIs() {
        return impl.activeSRIs();
    }

    /**
     * 
     */
    public int getCurrentQueueDepth() {
        return impl.getCurrentQueueDepth();
    }

    /**
     * 
     */
    public int getMaxQueueDepth() {
        return impl.getMaxQueueDepth();
    }

    /**
     * 
     */
    public void setMaxQueueDepth(int newDepth) {
        impl.setMaxQueueDepth(newDepth);
    }

    /**
     * 
     */
    public void pushSRI(StreamSRI header) {
        impl.pushSRI(header);
    }

    /**
     * 
     */
    public void pushPacket(String data, boolean eos, String streamID) 
    {
        impl.pushPacket(data, null, eos, streamID);
    }
     
    /**
     * 
     */
    public Packet getPacket(long wait) 
    {
        DataTransfer<String> p = impl.getPacket(wait);
        if (p == null) {
            return null;
        } else {
            return new Packet(p.getData(), p.getTime(), p.getEndOfStream(), p.getStreamID(), p.getSRI(), p.sriChanged(), p.inputQueueFlushed());
        }
    }

    public String getRepid()
    {
        return BULKIO.dataXMLHelper.id();
    }

    public String getDirection()
    {
        return CF.PortSet.DIRECTION_PROVIDES;
    }
}
