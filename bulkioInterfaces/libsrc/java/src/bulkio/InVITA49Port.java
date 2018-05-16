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
import java.util.Iterator;
import java.util.Map;

import org.apache.log4j.Logger;

import org.ossie.component.PortBase;
import org.ossie.component.RHLogger;

import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import BULKIO.PortStatistics;
import BULKIO.PortUsageType;
import BULKIO.VITA49StreamDefinition;
import BULKIO.dataVITA49Package.AttachError;
import BULKIO.dataVITA49Package.DetachError;
import BULKIO.dataVITA49Package.InputUsageState;
import BULKIO.dataVITA49Package.StreamInputError;

/**
 * @generated
 */
public class InVITA49Port extends BULKIO.jni.dataVITA49POA implements PortBase {

    public interface Callback  {

	public String attach(BULKIO.VITA49StreamDefinition stream, String userid)
	    throws BULKIO.dataVITA49Package.AttachError, BULKIO.dataVITA49Package.StreamInputError;

        public void detach(String attachId) 
	    throws BULKIO.dataVITA49Package.DetachError, BULKIO.dataVITA49Package.StreamInputError;
    };

    /**
     * @generated
     */
    protected String name;

    /**
     * @generated
     */
    protected linkStatistics stats;

    /**
     * @generated
     */
    protected Object sriUpdateLock;

    /**
     * @generated
     */
    protected Object statUpdateLock;

    /**
     * @generated
     */
    protected Map<String, VITA49StreamDefinition> attachedStreamMap;

    /**
     * @generated
     */
    protected Map<String, String> attachedUsers;

    /**
     * @generated
     */
    protected Map<String, streamTimePair> currentHs;

    /**
     * @generated
     */
    protected boolean sriChanged;

    /**
     *
     */
    protected Logger   logger = null;

    public RHLogger _portLog = null;

    // callback when VITA49 Stream Requests happen
    protected Callback                 attach_detach_callback;

    protected bulkio.sri.Comparator    sri_cmp;

    protected bulkio.time.Comparator   time_cmp;

    protected bulkio.SriListener       sriCallback;

    public InVITA49Port( String portName ) {
	this( portName, 
	      null,
	      null, 
	      new bulkio.sri.DefaultComparator(), 
	      new bulkio.time.DefaultComparator() );
    }
    
   public InVITA49Port( String portName, Callback actionCB ) {
	this( portName, 
	      null,
	      actionCB,
	      new bulkio.sri.DefaultComparator(), 
	      new bulkio.time.DefaultComparator() );
    }

    /**
     * @generated
     */
    public InVITA49Port( String                   portName, 
		       Logger                   logger,
		       Callback                 actionCB, 
		       bulkio.sri.Comparator    sriCmp,
		       bulkio.time.Comparator   timeCmp )
    {
	this.name = portName;
        this.stats = new linkStatistics(this.name, 1);
	this.sriUpdateLock = new Object();
	this.statUpdateLock = new Object();
	this.attachedStreamMap = new HashMap<String, VITA49StreamDefinition>();
	this.attachedUsers = new HashMap<String, String>();
	this.currentHs = new HashMap<String, streamTimePair>();
	this.sriChanged = false;
	
	attach_detach_callback = actionCB;
	sri_cmp = sriCmp;
	time_cmp = timeCmp;
	sriCallback = null;
	this.logger = logger;
	if ( this.logger == null ) {
            this.logger = Logger.getLogger("redhawk.bulkio.inport."+portName);
        }
        this.logger.debug( "bulkio::InVITA49Port  CTOR port:" + name  );

    }


    public void setLogger( Logger newlogger ){
        synchronized (this.sriUpdateLock) {
	    logger = newlogger;
	}
    }

    public void setLogger(RHLogger logger)
    {
        synchronized (this.sriUpdateLock) {
            this._portLog = logger;
        }
    }

    /**
     * 
     */
    public void setSriListener( bulkio.SriListener sriCallback ) {
        synchronized(this.sriUpdateLock) {
	    this.sriCallback = sriCallback;
	}
    }

    /**
     * 
     */
    public void setAttachDetachCallback( Callback newCB ) {
        synchronized(this.sriUpdateLock) {
	    this.attach_detach_callback = newCB;
	}
    }



    /**
     * @generated
     */
    public void enableStats(boolean enable) {
        this.stats.setEnabled(enable);
    }

    /**
     * @generated
     */
    public void setBitSize(double bitSize) {
        synchronized (statUpdateLock) {
            this.stats.setBitSize(bitSize);
        }
    };

    /**
     * @generated
     */
    public void updateStats(int elementsReceived, float queueSize, boolean EOS, String streamID, boolean flush) {
        synchronized (statUpdateLock) {
            this.stats.update(elementsReceived, queueSize, EOS, streamID, flush);
        }
    };

    /**
     * @generated
     */
    public PortStatistics statistics() {
        synchronized (statUpdateLock) {
            return this.stats.retrieve();
        }
    }

    /**
     * @generated
     */
    public PortUsageType state() {
        if (this.currentHs.size() == 0) {
            return PortUsageType.IDLE;
        }
        return PortUsageType.ACTIVE;
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
    public class streamTimePair {
        /** @generated */
        StreamSRI sri;
        /** @generated */
        PrecisionUTCTime time;

        /**
         * @generated
         */
        public streamTimePair(final StreamSRI sri, final PrecisionUTCTime time) {
            this.sri = sri;
            this.time = time;
        }
    }

    /**
     * @generated
     */
    public StreamSRI[] activeSRIs() {
        StreamSRI[] sris = new StreamSRI[0];
        synchronized (this.sriUpdateLock) {
            sris = new StreamSRI[this.currentHs.size()];
            int idx = 0;

            for (streamTimePair vals : this.currentHs.values()) {
                sris[idx++] = vals.sri;
            }
        }
        return sris;
   }


    /**
     * @generated
     */
    public void pushSRI(StreamSRI H, PrecisionUTCTime T) {

	if ( _portLog != null ) {
	    _portLog.trace("bulkio.InPort pushSRI  ENTER (port=" + name +")" );
	}

        synchronized (this.sriUpdateLock) {
            streamTimePair tmpH = this.currentHs.get(H.streamID);
            if (tmpH != null) {
		boolean s_same = false;
		if ( this.sri_cmp != null )  {
		    s_same = this.sri_cmp.compare(tmpH.sri, H);
		}
		boolean t_same = false;
		if ( this.time_cmp != null )  {
		    t_same =  this.time_cmp.compare(tmpH.time, T);
		}
                this.sriChanged = ( s_same == false ) || ( t_same == false ) ;
		if ( this.sriChanged )  {
		    this.currentHs.put(H.streamID, new streamTimePair(H, T));		
		    if ( sriCallback != null ) { sriCallback.changedSRI(H); }
		}
            } else {
                this.currentHs.put(H.streamID, new streamTimePair(H, T));
                this.sriChanged = true;
                if ( sriCallback != null ) { sriCallback.newSRI(H); }
            }
        }

	if ( _portLog != null ) {
	    _portLog.trace("bulkio.InPort pushSRI  EXIT (port=" + name +")" );
	}

    }



    /**
     * @generated
     */
    public String attach(VITA49StreamDefinition stream, String userid) throws AttachError, StreamInputError {

	if ( _portLog != null ) {
	    _portLog.trace("bulkio.InPort attach  ENTER (port=" + name +")" );
	    _portLog.debug("VITA49 PORT: ATTACH REQUEST STREAM/USER:" + stream.id +"/" + userid );
	}

	String attachId = null;
	if ( attach_detach_callback != null ) {
	    if ( _portLog != null ) {
		_portLog.debug("VITA49 PORT: CALLING ATTACH CALLBACK, STREAM/USER:" + stream.id +"/" + userid );
	    }
	    try {
		attachId = attach_detach_callback.attach(stream, userid);
	    }
	    catch(Exception e) {
		if ( _portLog != null ) {
		    _portLog.error("VITA49 PORT: CALLING ATTACH EXCEPTION, STREAM/USER:" + stream.id +"/" + userid );
		}
		throw new AttachError("Callback Failed");		
	    }
	}
	if ( attachId == null ) {
	    attachId = java.util.UUID.randomUUID().toString();
	}
        this.attachedStreamMap.put(attachId, stream);
        this.attachedUsers.put(attachId, userid);


	if ( _portLog != null ) {
	    _portLog.debug("VITA49 PORT: ATTACH COMPLETED, ID:" + attachId + " STREAM/USER:" + stream.id +"/" + userid );
	    _portLog.trace("bulkio.InPort attach  EXIT (port=" + name +")" );
	}

        return attachId;
    }

    /**
     * @generated
     */
    public void detach(String attachId) throws DetachError, StreamInputError {

	if ( _portLog != null ) {
	    _portLog.trace("bulkio.InPort detach  ENTER (port=" + name +")" );
	    _portLog.debug("VITA49 PORT: DETACH REQUEST ID:" + attachId  );
	}

	if ( attach_detach_callback != null ) {
	    try {
		if ( _portLog != null ) {
		    _portLog.debug("VITA49 PORT: CALLING DETACH CALLBACK ID:" + attachId  );
		}
		attach_detach_callback.detach(attachId);
	    }
	    catch( Exception e ) {
		if ( _portLog != null ) {
		    _portLog.error("VITA49 PORT: DETACH CALLBACK EXCEPTION, ID:" + attachId  );
		}
		throw new DetachError();
	    }
	}
        this.attachedStreamMap.remove(attachId);
        this.attachedUsers.remove(attachId);

	if ( _portLog != null ) {
	    _portLog.debug("VITA49 PORT: DETACH SUCCESS, ID:" + attachId  );
	    _portLog.trace("bulkio.InPort detach  EXIT (port=" + name +")" );
	}
    }

   /**
     * @generated
     */
    public StreamSRI[] attachedSRIs() {
        return this.activeSRIs();
    }

    /**
     * @generated
     */
    public VITA49StreamDefinition getStreamDefinition(String attachId) throws StreamInputError {
        return this.attachedStreamMap.get(attachId);
    }

 
    /**
     * @generated
     */
    public VITA49StreamDefinition[] attachedStreams() {
        return this.attachedStreamMap.values().toArray(new VITA49StreamDefinition[0]);
    }

    /**
     * @generated
     */
    public String getUser(String attachId) throws StreamInputError {
        return this.attachedUsers.get(attachId);
    }

    /**
     * @generated
     */
    public InputUsageState usageState() {
        if (this.attachedStreamMap.size() == 0) {
            return InputUsageState.IDLE;
        } else if (this.attachedStreamMap.size() == 1) {
            return InputUsageState.BUSY;
        } else {
            return InputUsageState.ACTIVE;
        }
    }

    /**
     * @generated
     */
    public String[] attachmentIds() {
        return this.attachedStreamMap.keySet().toArray(new String[0]);
    }



    /**
     * @generated
     */
    public boolean hasSriChanged() {
        return this.sriChanged;
    }

	public String getRepid()
	{
		return BULKIO.dataVITA49Helper.id();
	}

	public String getDirection()
	{
		return CF.PortSet.DIRECTION_PROVIDES;
	}

}
