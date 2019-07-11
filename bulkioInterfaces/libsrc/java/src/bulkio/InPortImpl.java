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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Iterator;
import java.util.Map;
import java.util.ArrayDeque;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Logger;

import CF.DataType;
import org.ossie.component.RHLogger;

import BULKIO.PortStatistics;
import BULKIO.PortUsageType;
import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;

/**
 * 
 */
class InPortImpl<A> {

    /**
     * 
     */
    protected String name;

    /**
     * 
     */
    protected linkStatistics stats;

    /**
     * 
     */
    protected Object sriUpdateLock;

    /**
     * 
     */
    protected Object statUpdateLock;

    /**
     * 
     */
    protected Map<String, sriState> currentHs;

    /**
     * 
     */
    protected Object dataBufferLock;

    /**
     * 
     */
    protected int maxQueueDepth;

    /**
     * 
     */
    protected Semaphore queueSem;

    /**
     * 
     */
    protected Semaphore dataSem;

    /**
     * 
     */
    protected boolean blocking;

    /**
     *
     */
    protected Logger   logger = null;

    protected bulkio.sri.Comparator    sri_cmp;

    protected bulkio.SriListener   sriCallback;


    /**
     * This queue stores all packets received from pushPacket.
     * 
     */
    private ArrayDeque<DataTransfer<A>> workQueue;

    private DataHelper<A> helper;

    /**
     * 
     */
    public InPortImpl(String portName, Logger logger, bulkio.sri.Comparator compareSRI, bulkio.SriListener sriCallback, DataHelper<A> helper) {
        this.name = portName;
        this.logger = logger;
        this.stats = new linkStatistics(this.name, 1);
        // Update bit size from the helper, because element size does not take
        // sub-byte elements (i.e., dataBit) into account.
        this.stats.setBitSize(helper.bitSize());
        this.sriUpdateLock = new Object();
        this.statUpdateLock = new Object();
        this.currentHs = new HashMap<String, sriState>();
        this.dataBufferLock = new Object();
        this.maxQueueDepth = 100;
        this.queueSem = new Semaphore(this.maxQueueDepth);
        this.dataSem = new Semaphore(0);
        this.blocking = false;
        this.helper = helper;

        this.workQueue = new ArrayDeque<DataTransfer<A>>();

        sri_cmp = compareSRI;
        sriCallback = sriCallback;

	if ( this.logger == null ) {
            this.logger = Logger.getLogger("redhawk.bulkio.inport."+portName);
        }
        this.logger.debug( "bulkio::InPort CTOR port: " + portName ); 
    }

    public Logger getLogger() {
        synchronized (this.sriUpdateLock) {
            return logger;
        }
    }

    public void setLogger(Logger newlogger) {
        synchronized (this.sriUpdateLock) {
            logger = newlogger;
        }
    }

    public void setLogger(RHLogger logger) {
        if (logger != null) {
            setLogger(logger.getL4Logger());
        } else {
            setLogger((Logger) null);
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
    public String getName() {
        return this.name;
    }

    /**
     * 
     */
    public void enableStats(boolean enable) {
        this.stats.setEnabled(enable);
    }

    /**
     * 
     */
    public PortStatistics statistics() {
        synchronized (statUpdateLock) {
            return this.stats.retrieve();
        }
    }

    /**
     * 
     */
    public PortUsageType state() {
        int queueSize = 0;
        synchronized (dataBufferLock) {
            queueSize = workQueue.size();
            if (queueSize == maxQueueDepth) {
                return PortUsageType.BUSY;
            } else if (queueSize == 0) {
                return PortUsageType.IDLE;
            }
            return PortUsageType.ACTIVE;
        }
    }

    /**
     * 
     */
    public StreamSRI[] activeSRIs() {
        synchronized (this.sriUpdateLock) {
            ArrayList<StreamSRI> sris = new ArrayList<StreamSRI>();
            Iterator<sriState> iter = this.currentHs.values().iterator();
            while(iter.hasNext()) {
                sris.add(iter.next().getSRI());
            }
            return sris.toArray(new StreamSRI[sris.size()]);
        }
    }

    /**
     * 
     */
    public int getCurrentQueueDepth() {
        synchronized (this.dataBufferLock) {
            return workQueue.size();
        }
    }

    /**
     * 
     */
    public int getMaxQueueDepth() {
        synchronized (this.dataBufferLock) {
            return this.maxQueueDepth;
        }
    }

    /**
     * 
     */
    public void setMaxQueueDepth(int newDepth) {
        synchronized (this.dataBufferLock) {
            this.maxQueueDepth = newDepth;
            queueSem = new Semaphore(newDepth);
        }
    }

    /**
     * 
     */
    public void pushSRI(StreamSRI header) {

        if (( logger != null ) && (logger.isTraceEnabled())) {
            logger.trace("bulkio.InPort pushSRI  ENTER (port=" + name +")" );
        }

        synchronized (sriUpdateLock) {
            if (!currentHs.containsKey(header.streamID)) {
                if (( logger != null ) && (logger.isDebugEnabled())) {
                    logger.debug("pushSRI PORT:" + name + " NEW SRI:" +
                                 header.streamID );
                }
                if ( sriCallback != null ) { sriCallback.newSRI(header); }
                currentHs.put(header.streamID, new sriState(header, true));
                if (header.blocking) {
                    //If switching to blocking we have to set the semaphore
                    synchronized (dataBufferLock) {
                        if (!blocking) {
                                try {
                                    queueSem.acquire(workQueue.size());
                                } catch (InterruptedException e) {
                                    e.printStackTrace();
                                }
                        }
                        blocking = true;
                    }
                }
            } else {
                StreamSRI oldSri = currentHs.get(header.streamID).getSRI();
                boolean cval = false;
                if ( sri_cmp != null ) {
                    cval = sri_cmp.compare( header, oldSri );
                }
                if ( cval == false ) {
                    if ( sriCallback != null ) { sriCallback.changedSRI(header); }
                    this.currentHs.put(header.streamID, new sriState(header, true));
                    if (header.blocking) {
                        //If switching to blocking we have to set the semaphore
                        synchronized (dataBufferLock) {
                            if (!blocking) {
                                    try {
                                        queueSem.acquire(workQueue.size());
                                    } catch (InterruptedException e) {
                                        e.printStackTrace();
                                    }
                            }
                            blocking = true;
                        }
                    }
                }
            }
        }
        if (( logger != null ) && (logger.isTraceEnabled())) {
            logger.trace("bulkio.InPort pushSRI  EXIT (port=" + name +")" );
        }
    }

    /**
     * 
     */
    public void pushPacket(A data, PrecisionUTCTime time, boolean eos, String streamID)
    {
        if (( logger != null ) && (logger.isTraceEnabled())) {
            logger.trace("bulkio.InPort pushPacket ENTER (port=" + name +")" );
        }

        // Discard empty packets if EOS is not set, as there is no useful data
        // or metadata to be had--since T applies to the 1st sample (which does
        // not exist), all we have is a stream ID
        if (helper.isEmpty(data) && !eos) {
            return;
        }

        synchronized (this.dataBufferLock) {
            if (this.maxQueueDepth == 0) {
                if (( logger != null ) && (logger.isTraceEnabled())) {
                    logger.trace("bulkio.InPort pushPacket EXIT (port=" + name +")" );
                }
                return;
            }
        }

        boolean portBlocking = false;
        StreamSRI tmpH = null;
        boolean sriChanged = false;
        synchronized (this.sriUpdateLock) {
            if (this.currentHs.containsKey(streamID)) {
                tmpH = this.currentHs.get(streamID).getSRI();
                sriChanged = this.currentHs.get(streamID).isChanged();
                if ( eos == false ) {
                    this.currentHs.get(streamID).setChanged(false);
                }
                portBlocking = blocking;
            } else {
                if (( logger != null ) && (logger.isEnabledFor(org.apache.log4j.Level.WARN))) {
                    logger.warn("bulkio.InPort pushPacket received data from stream '" + streamID + "' with no SRI");
                }
                tmpH = new StreamSRI(1, 0.0, 1.0, (short)1, 0, 0.0, 0.0, (short)0, (short)0, streamID, false, new DataType[0]);
                if (sriCallback != null) {
                    sriCallback.newSRI(tmpH);
                }
                sriChanged = true;
                currentHs.put(streamID, new sriState(tmpH, false));
            }
        }

        // determine whether to block and wait for an empty space in the queue
        int elements = helper.arraySize(data);

        if (portBlocking) {
            DataTransfer<A> p = new DataTransfer<A>(data, time, eos, streamID, tmpH, sriChanged, false);

            try {
                queueSem.acquire();
            } catch (InterruptedException e) {
                e.printStackTrace();
            }

            synchronized (this.dataBufferLock) {
                this.stats.update(elements, this.workQueue.size()/(float)this.maxQueueDepth, eos, streamID, false);
                this.workQueue.add(p);
                this.dataSem.release();
            }

            if (eos) {
                synchronized (this.sriUpdateLock) {
                    this.currentHs.remove(streamID);
                }
            }
        } else {
            synchronized (this.dataBufferLock) {
                boolean flushToReport = false;
                if ((this.maxQueueDepth >= 0) && (this.workQueue.size() >= this.maxQueueDepth)) {
                    if (( logger != null ) && (logger.isDebugEnabled())) {
                        logger.debug( "bulkio::InPort pushPacket PURGE INPUT QUEUE (SIZE"  + this.workQueue.size() + ")" );
                    }
                    flushToReport = true;

                    // Need to hold the SRI mutex while flushing the queue
                    // because it may update SRI change state
                    synchronized (this.sriUpdateLock) {
                        this._flushQueue();

                        // Update the SRI change flag for this stream, which
                        // may have been modified during the queue flush
                        sriState currH = this.currentHs.get(streamID);
                        sriChanged = currH.isChanged();
                        currH.setChanged(false);
                    }
                }
                this.stats.update(elements, (this.workQueue.size()+1)/(float)this.maxQueueDepth, eos, streamID, flushToReport);
                if (( logger != null ) && (logger.isTraceEnabled())) {
                    logger.trace( "bulkio::InPort pushPacket NEW Packet (QUEUE=" + workQueue.size() + ")");
                }
                DataTransfer<A> p = new DataTransfer<A>(data, time, eos, streamID, tmpH, sriChanged, false);
                this.workQueue.add(p);

                if (eos) {
                    synchronized (this.sriUpdateLock) {
                        this.currentHs.remove(streamID);
                    }
                }
                // If a flush occurred, always set the flag on the first
                // packet; this may not be the packet that was just inserted if
                // there were any EOS packets on the queue
                if (flushToReport) {
                    DataTransfer<A> first = this.workQueue.removeFirst();
                    first = new DataTransfer<A>(first.dataBuffer, first.T, first.EOS, first.streamID, first.SRI, first.sriChanged, true);
                    this.workQueue.addFirst(first);
                }
                this.dataSem.release();
            }
        }

        if (( logger != null ) && (logger.isTraceEnabled())) {
            logger.trace("bulkio.InPort pushPacket EXIT (port=" + name +")" );
        }
        return;

    }

    private void _flushQueue()
    {
        Set<String> sri_changed = new HashSet<String>();
        ArrayDeque<DataTransfer<A>> saved_packets = new ArrayDeque<DataTransfer<A>>();
        for (DataTransfer<A> packet : this.workQueue) {
            if (packet.EOS) {
                // Remove the SRI change flag for this stream, as further SRI
                // changes apply to a different stream; set the SRI change flag
                // for the EOS packet if there was one for this stream earlier
                // in the queue
                boolean sri_flag = packet.sriChanged;
                if (sri_changed.remove(packet.streamID)) {
                    sri_flag = true;
                }

                // Discard data and preserve the EOS packet
                DataTransfer<A> modified_packet = new DataTransfer<A>(this.helper.emptyArray(), packet.T, true, packet.streamID, packet.SRI, sri_flag, false);
                saved_packets.addLast(modified_packet);
            } else if (packet.sriChanged) {
                sri_changed.add(packet.streamID);
            }
        }
        this.workQueue = saved_packets;

        // Save any SRI change flags that were collected and not applied to an
        // EOS packet
        for (String stream_id : sri_changed) {
            // It should be safe to assume that an entry exists for the stream
            // ID, but just in case, check the result of get
            sriState currH = this.currentHs.get(stream_id);
            if (currH != null) {
                currH.setChanged(true);
            }
        }
    }

    /**
     * 
     */
    public DataTransfer<A> getPacket(long wait)
    {

        if (( logger != null ) && (logger.isTraceEnabled())) {
            logger.trace("bulkio.InPort getPacket ENTER (port=" + name +")" );
        }

        try {
            if (wait < 0) {
                if (( logger != null ) && (logger.isTraceEnabled())) {
                    logger.trace("bulkio.InPort getPacket PORT:" + name +" Block until data arrives" );
                }
                this.dataSem.acquire();
            } else {
                if (( logger != null ) && (logger.isTraceEnabled())) {
                    logger.trace("bulkio.InPort getPacket PORT:" + name +" TIMED WAIT:" + wait );
                }
                this.dataSem.tryAcquire(wait, TimeUnit.MILLISECONDS);
            }
        } catch (InterruptedException ex) {
            if (( logger != null ) && (logger.isTraceEnabled())) {
                logger.trace("bulkio.InPort getPacket EXIT (port=" + name +")" );
            }
            return null;
        }

        DataTransfer<A> p = null;
        synchronized (this.dataBufferLock) {
            p = this.workQueue.poll();
        }

        if (p != null) {
            if (p.getEndOfStream()) {
                synchronized (this.sriUpdateLock) {
                    boolean stillBlocking = false;
                    if (this.currentHs.containsKey(p.getStreamID())) {
                        Iterator<sriState> iter = currentHs.values().iterator();
                        while (iter.hasNext()) {
                            if (iter.next().getSRI().blocking) {
                                stillBlocking = true;
                                break;
                            }
                        }
                    }
                    if (!stillBlocking) {
                        blocking = false;
                    }
                }
            }

            if (blocking) {
                queueSem.release();
            }
        }

        if (( logger != null ) && (logger.isTraceEnabled())) {
            logger.trace("bulkio.InPort getPacket EXIT (port=" + name +")" );
        }

        return p;
    }
}
