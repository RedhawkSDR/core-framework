/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package burstio;

import java.util.ArrayList;
import java.util.Collection;
import java.util.List;
import java.util.Map;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.ScheduledThreadPoolExecutor;
import java.util.concurrent.TimeUnit;
import java.lang.reflect.Array;
import org.omg.CORBA.MARSHAL;

import org.apache.log4j.Logger;

import org.ossie.properties.IProperty;
import org.ossie.properties.StructDef;

import burstio.stats.SenderStatistics;
import burstio.traits.BurstTraits;

import org.ossie.component.PortBase;

abstract class OutPort<E,B,A> extends BULKIO.UsesPortStatisticsProviderPOA implements PortBase {
    public static final int DEFAULT_MAX_BURSTS = 100;
    public static final int DEFAULT_LATENCY_THRESHOLD = 10000; // 10000 us = 10ms

    // Basic port information
    static class Connection<E>
    {
        public Connection(final E port, final String name, final int bitsPerElement)
        {
            this.port = port;
            this.alive = true;
            this.stats = new SenderStatistics(name, bitsPerElement);
        }

        public final E port;
        public boolean alive;
        public SenderStatistics stats;
    }

    protected final String name_;
    protected final Map<String,Connection<E>> connections_ = new HashMap<String,Connection<E>>();
    protected final List<ConnectionListener> connectionListeners_ = new ArrayList<ConnectionListener>();

    protected Logger logger_;

    private BurstTraits<B,A> traits_;

    private class Queue implements OutputPolicy {
        protected Queue (final String streamID, final int maxBursts, final int thresholdBytes, final int thresholdLatency)
        {
            this.streamID_ = streamID;
            this.maxBursts_ = maxBursts;
            this.thresholdBytes_ = thresholdBytes;
            this.setLatencyThreshold(thresholdLatency);
        }

        public synchronized int getMaxBursts ()
        {
            return this.maxBursts_;
        }

        public synchronized void setMaxBursts (int bursts)
        {
            this.maxBursts_ = bursts;
            if (this.queue_.size() >= this.maxBursts_) {
                OutPort.this.logger_.debug("New max bursts " + this.maxBursts_ + " triggering push");
                this.executeThreadedFlush();
            }
        }

        public synchronized int getLatencyThreshold ()
        {
            return (int)TimeUnit.MICROSECONDS.convert(this.thresholdLatency_, TimeUnit.NANOSECONDS);
        }

        public synchronized void setLatencyThreshold (int usec)
        {
            this.thresholdLatency_ = TimeUnit.NANOSECONDS.convert(usec, TimeUnit.MICROSECONDS);
            if (!this.queue_.isEmpty()) {
                OutPort.this.scheduleCheck(this.startTime_ + this.thresholdLatency_);
            }
        }

        public synchronized int getByteThreshold ()
        {
            return this.thresholdBytes_;
        }

        public synchronized void setByteThreshold (int bytes)
        {
            this.thresholdBytes_ = bytes;
            if (this.queuedBytes_ >= this.thresholdBytes_) {
                OutPort.this.logger_.debug("New byte threshold " + this.thresholdBytes_ + " triggering push");
                this.executeThreadedFlush();
            }
        }

        public synchronized void flush ()
        {
            flushQueue();
        }

        protected synchronized void queueBurst (B burst)
        {
            // If this is the first burst, make the time for latency guarantees
            if (this.queue_.isEmpty()) {
                this.startTime_ = System.nanoTime();
                // Wake up the monitor thread so it can set its timeout
                OutPort.this.logger_.trace("Waking monitor thread on first queued burst");
                OutPort.this.scheduleCheck(this.startTime_ + this.thresholdLatency_);
            }

            this.queue_.add(burst);
            this.queuedBytes_ += OutPort.this.traits_.burstLength(burst) * OutPort.this.bytesPerElement_;
            OutPort.this.logger_.trace("Queue size: " + this.queue_.size() + " bursts / " + this.queuedBytes_ + " bytes");

            if (this.shouldFlush()) {
                OutPort.this.logger_.debug("Queued burst exceeded threshold, flushing queue");
                this.flushQueue();
            }
        }

        protected boolean shouldFlush ()
        {
            if (this.queue_.size() >= this.maxBursts_) {
                return true;
            } else if (this.queuedBytes_ >= this.thresholdBytes_) {
                return true;
            } else if (this.elapsed() >= this.thresholdLatency_) {
                return true;
            }
            return false;
        }

        protected void flushQueue()
        {
            if (!this.queue_.isEmpty()) {
                float queue_depth = this.queue_.size() / (float)this.maxBursts_;
                OutPort.this.sendBursts(this.queue_, this.startTime_, queue_depth, this.streamID_);
                this.queue_.clear();
                this.queuedBytes_ = 0;
                this.startTime_ = -1;
            }
        }

        protected synchronized void checkFlush()
        {
            if (this.shouldFlush()) {
                this.flushQueue();
            }
        }

        private void executeThreadedFlush ()
        {
            OutPort.this.monitor_.execute(new Runnable() {
                    public void run () {
                        Queue.this.flush();
                    }
                });
        }

        private long elapsed ()
        {
            if (this.startTime_ < 0) {
                return -1;
            } else {
                return System.nanoTime() - this.startTime_;
            }
        }

        private String streamID_;

        private int maxBursts_;
        private long thresholdLatency_;
        private int thresholdBytes_;

        private ArrayList<B> queue_ = new ArrayList<B>();
        private long startTime_ = -1;
        private int queuedBytes_ = 0;
    }

    private Queue defaultQueue_ = new Queue("(default)", DEFAULT_MAX_BURSTS, (int)(0.9 * 2*1024*1024), DEFAULT_LATENCY_THRESHOLD);
    private Map<String,Queue> streamQueues_ = new HashMap<String,Queue>();

    private int bytesPerElement_;

    private ScheduledThreadPoolExecutor monitor_ = new ScheduledThreadPoolExecutor(1);
    private boolean running_ = false;

    private Map<String,Set<String>> routes_ = new HashMap<String,Set<String>>();
    private RoutingMode routingMode_ = RoutingMode.ROUTE_ALL_INTERLEAVED;

    public OutPort(final String name, BurstTraits<B,A> traits)
    {
        this.logger_ = Logger.getLogger(this.getClass().getName());
        this.name_ = name;
        this.traits_ = traits;
        this.bytesPerElement_ = this.traits_.byteSize();
    }

    public void addConnectionListener (final ConnectionListener listener)
    {
        synchronized (this.connectionListeners_) {
            this.connectionListeners_.add(listener);
        }
    }

    public void removeConnectionListener (final ConnectionListener listener)
    {
        synchronized (this.connectionListeners_) {
            this.connectionListeners_.remove(listener);
        }
    }

    public void connectPort(final org.omg.CORBA.Object connection, final String connectionId) throws CF.PortPackage.InvalidPort, CF.PortPackage.OccupiedPort {
        // Give a specific exception message for nil
        if (connection == null) {
            throw new CF.PortPackage.InvalidPort((short)1, "Nil object reference");
        }

        // Attempt to narrow the reference to the correct type (which must be
        // implemented by a subclass); note this does not require the lock
        E port = null;
        try {
            port = narrow(connection);
        } catch (final org.omg.CORBA.BAD_PARAM ex) {
            // In this context, a CORBA.BAD_PARAM exception indicates that the
            // object is of the wrong type
            throw new CF.PortPackage.InvalidPort((short)1, "Object is not a " + repoId());
        } catch (final org.omg.CORBA.SystemException ex) {
            // If the object is not obviously the desired type, narrow will
            // invoke _is_a, which may throw a CORBA exception if a remote
            // object is unreachable (e.g., dead)
            throw new CF.PortPackage.InvalidPort((short)1, "Object unreachable");
        }

        synchronized (this.connections_) {
            this.connections_.put(connectionId, new Connection<E>(port, this.name_, this.bytesPerElement_*8));
        }

        synchronized (this.connectionListeners_) {
            for (ConnectionListener listener : this.connectionListeners_) {
                listener.portConnected(connectionId);
            }
        }
    }

    public void disconnectPort(final String connectionId) throws CF.PortPackage.InvalidPort {
        synchronized (this.connections_) {
            // Check that remove returns a value to ensure the connection ID
            // was valid (the connection table should never contain nulls)
            if (null == this.connections_.remove(connectionId)) {
                throw new CF.PortPackage.InvalidPort((short)2, "No connection " + connectionId);
            }
        }

        synchronized (this.connectionListeners_) {
            for (ConnectionListener listener : this.connectionListeners_) {
                listener.portDisconnected(connectionId);
            }
        }
    }

    protected abstract E narrow(org.omg.CORBA.Object connection);
    protected abstract String repoId();

    public ExtendedCF.UsesConnection[] connections() {
        synchronized (this.connections_) {
            final ExtendedCF.UsesConnection[] results = new ExtendedCF.UsesConnection[this.connections_.size()];
            int index = 0;
            for (Map.Entry<String,Connection<E>> entry : this.connections_.entrySet()) {
                Connection<E> connection = entry.getValue();
                org.omg.CORBA.Object my_obj = (org.omg.CORBA.Object)connection.port;
                if (my_obj instanceof omnijni.ObjectImpl) {
                    String ior = omnijni.ORB.object_to_string(my_obj);
                    my_obj = this._orb().string_to_object(ior);
                }
                results[index++] = new ExtendedCF.UsesConnection(entry.getKey(), my_obj);
            }
            return results;
        }
    }

    public void updateConnectionFilter(final Collection<? extends StructDef> filterTable)
    {
        Map<String,Set<String>> new_routes = new HashMap<String,Set<String>>();
        for (StructDef filter : filterTable) {
            String port_name = null;
            String stream_id = null;
            String connection_id = null;
            for (IProperty property : filter.getElements()) {
                org.omg.CORBA.Any value = property.toAny();
                if (property.getName().equals("port_name")) {
                    port_name = value.extract_string();
                } else if (property.getName().equals("stream_id")) {
                    stream_id = value.extract_string();
                } else if (property.getName().equals("connection_id")) {
                    connection_id = value.extract_string();
                }
            }
            if ((port_name == null) || (stream_id == null) || (connection_id == null)) {
                throw new IllegalArgumentException("Invalid struct for connection filter");
            }
            if (this.name_.equals(port_name.toString())) {
                if (!new_routes.containsKey(stream_id)) {
                    new_routes.put(stream_id, new HashSet<String>());
                }
                new_routes.get(stream_id).add(connection_id);
            }
        }

        synchronized (this.connections_) {
            this.routes_ = new_routes;
        }
    }

    public void addConnectionFilter(final String streamID, final String connectionID)
    {

        synchronized (this.connections_) {
            if (!this.routes_.containsKey(streamID)) {
                this.routes_.put(streamID, new HashSet<String>());
            }
            this.routes_.get(streamID).add(connectionID);
        }
    }

    public void removeConnectionFilter(final String streamID, final String connectionID)
    {
        synchronized (this.connections_) {
            if (this.routes_.containsKey(streamID)) {
                this.routes_.get(streamID).remove(connectionID);
            }
        }
    }

    public BULKIO.UsesPortStatistics[] statistics()
    {
        synchronized (this.connections_) {
            List<BULKIO.UsesPortStatistics> results = new ArrayList<BULKIO.UsesPortStatistics>();
            for (Map.Entry<String,Connection<E>> entry : this.connections_.entrySet()) {
                final String connectionId = entry.getKey();
                final Connection<E> connection = entry.getValue();
                final BULKIO.PortStatistics stats = connection.stats.retrieve();

                // Report all streams being routed to this connection
                final List<String> streams = new ArrayList<String>();
                for (String stream_id : this.streamQueues_.keySet()) {
                    if (isStreamRoutedToConnection(stream_id, connectionId)) {
                        streams.add(stream_id);
                    }
                }
                stats.streamIDs = streams.toArray(new String[streams.size()]);

                results.add(new BULKIO.UsesPortStatistics(connectionId, stats));
            }
            return results.toArray(new BULKIO.UsesPortStatistics[results.size()]);
        }
    }

    public String getName () {
        return this.name_;
    }

    public int getMaxBursts ()
    {
        return this.getDefaultPolicy().getMaxBursts();
    }

    public void setMaxBursts (int bursts)
    {
        this.getDefaultPolicy().setMaxBursts(bursts);
    }

    public int getLatencyThreshold ()
    {
        return this.getDefaultPolicy().getLatencyThreshold();
    }

    public void setLatencyThreshold (int usec)
    {
        this.getDefaultPolicy().setLatencyThreshold(usec);
    }

    public int getByteThreshold ()
    {
        return this.getDefaultPolicy().getByteThreshold();
    }

    public void setByteThreshold (int bytes)
    {
        this.getDefaultPolicy().setByteThreshold(bytes);
    }

    public void setRoutingMode (final RoutingMode mode)
    {
        this.routingMode_ = mode;
    }

    public OutputPolicy getDefaultPolicy ()
    {
        return this.defaultQueue_;
    }

    public OutputPolicy getStreamPolicy (final String streamID)
    {
        return this.getQueueForStream(streamID);
    }

    public void setLogger (Logger logger)
    {
        this.logger_ = logger;
    }

    public synchronized void start ()
    {
        if (this.running_) {
            return;
        }

        this.running_ = true;
    }

    public void stop ()
    {
        synchronized (this) {
            if (!this.running_) {
                return;
            }

            this.running_ = false;
        }
        this.flush();
    }

    public BULKIO.PortUsageType state ()
    {
        synchronized (this.connections_) {
            if (this.connections_.isEmpty()) {
                return BULKIO.PortUsageType.IDLE;
            } else {
                return BULKIO.PortUsageType.ACTIVE;
            }
        }
    }

    public void flush ()
    {
        synchronized (this.streamQueues_) {
            if (isInterleaved()) {
                this.defaultQueue_.flush();
            } else {
                for (Queue queue : this.streamQueues_.values()) {
                    queue.flush();
                }
            }
        }
    }

    public void pushBurst(A data, BURSTIO.BurstSRI sri)
    {
        this.pushBurst(data, sri, burstio.Utils.now(), false);
    }

    public void pushBurst(A data, BURSTIO.BurstSRI sri, BULKIO.PrecisionUTCTime timestamp)
    {
        this.pushBurst(data, sri, timestamp, false);
    }

    public void pushBurst(A data, BURSTIO.BurstSRI sri, boolean eos)
    {
        this.pushBurst(data, sri, burstio.Utils.now(), eos);
    }

    public void pushBurst(A data, BURSTIO.BurstSRI sri, BULKIO.PrecisionUTCTime timestamp, boolean eos)
    {
        B burst = this.traits_.createBurst(data, sri, timestamp, eos);
        synchronized (this.streamQueues_) {
            Queue queue = this.getQueueForStream(sri.streamID);
            queue.queueBurst(burst);
            if (eos) {
                if (!isInterleaved()) {
                    this.logger_.debug("Flushing " + sri.streamID + " on EOS");
                    queue.flush();
                }
                this.streamQueues_.remove(sri.streamID);
            }
        }
    }

    public void pushBursts(B[] bursts)
    {
        this.sendBursts(bursts, System.nanoTime(), 0.0f, null);
    }

    public void pushBursts (Collection<B> bursts)
    {
        this.pushBursts(this.traits_.toArray(bursts));
    }

	public String getRepid ()
	{
		return "IDL:CORBA/Object:1.0";
	}

	public String getDirection ()
	{
		return "Uses";
	}

    protected void sendBursts(B[] bursts, long startTime, float queueDepth, final String streamID)
    {
        int total_elements = 0;
        for (B burst : bursts) {
            total_elements += this.traits_.burstLength(burst);
        }

        synchronized (this.connections_) {
            for (Map.Entry<String,Connection<E>> entry : this.connections_.entrySet()) {
                final String connectionId = entry.getKey();
                if (!isStreamRoutedToConnection(streamID, connectionId)) {
                    continue;
                }

                final Connection<E> connection = entry.getValue();

                long delay = System.nanoTime() - startTime;
                try {
                    this.pushBursts(connection.port, bursts);
                    connection.alive = true;
                    connection.stats.record(bursts.length, total_elements, queueDepth, delay * 1e-9);
                } catch (org.omg.CORBA.SystemException ex) {
                    if (bursts.length == 1) {
                        if (connection.alive) {
                            this.logger_.error("pushBursts to " + connectionId + " failed the burst size is too long");
                            connection.alive = false;
                        }
                    } else {
                        this.partitionBursts(bursts, startTime, queueDepth, connection);
                    }
                } catch (final Exception ex) {
                    if (connection.alive) {
                        this.logger_.error("pushBursts to " + connectionId + " failed: " + ex);
                        connection.alive = false;
                    }
                }
            }
        }
    }

    protected void partitionBursts (B[] bursts, long startTime, float queueDepth, final Connection<E> connection)
    {
        B[] first_burst = this.createBursts(bursts.length/2);
        B[] second_burst = this.createBursts(bursts.length - first_burst.length);
        long delay = System.nanoTime() - startTime;
        for (int i=0; i<bursts.length; i++) {
            if (i<first_burst.length) {
                first_burst[i] = bursts[i];
            } else {
                second_burst[i-first_burst.length] = bursts[i];
            }
        }
        try {
            int total_elements = 0;
            for (B burst : first_burst) {
                total_elements += this.traits_.burstLength(burst);
            }
            this.pushBursts(connection.port, first_burst);
            connection.alive = true;
            connection.stats.record(first_burst.length, total_elements, queueDepth, delay * 1e-9);
        } catch (org.omg.CORBA.SystemException ex) {
            this.partitionBursts(first_burst, startTime, queueDepth, connection);
        }
        try {
            int total_elements = 0;
            for (B burst : second_burst) {
                total_elements += this.traits_.burstLength(burst);
            }
            this.pushBursts(connection.port, second_burst);
            connection.alive = true;
            connection.stats.record(second_burst.length, total_elements, queueDepth, delay * 1e-9);
        } catch (org.omg.CORBA.SystemException ex) {
            this.partitionBursts(second_burst, startTime, queueDepth, connection);
        }
    }
    
    protected void sendBursts (Collection<B> bursts, long startTime, float queueDepth, final String streamID)
    {
        this.sendBursts(this.traits_.toArray(bursts), startTime, queueDepth, streamID);
    }
    
    protected abstract void pushBursts(E port, B[] burts);
    
    protected abstract B[] createBursts(int size);

    protected boolean isStreamRoutedToConnection (final String streamID, final String connectionID)
    {
        if (RoutingMode.ROUTE_CONNECTION_STREAMS != this.routingMode_) {
            return true;
        }

        Set<String> stream_routes = this.routes_.get(streamID);
        if (stream_routes == null) {
            return false;
        }
        return stream_routes.contains(connectionID);
    }

    private void scheduleCheck (long when)
    {
        long delay = when - System.nanoTime();
        this.monitor_.schedule(new Runnable() {
                public void run() {
                    OutPort.this.checkQueues();
                }
            }, delay, TimeUnit.NANOSECONDS);
    }

    private void checkQueues ()
    {
        synchronized (this.streamQueues_) {
            if (isInterleaved()) {
                this.defaultQueue_.checkFlush();
            } else {
                for (Queue queue : this.streamQueues_.values()) {
                    queue.checkFlush();
                }
            }
        }
    }

    private Queue getQueueForStream (final String streamID)
    {
        Queue queue = this.streamQueues_.get(streamID);
        if (queue == null) {
            if (isInterleaved()) {
                queue = this.defaultQueue_;
            } else {
                this.logger_.trace("Creating new queue for stream " + streamID);
                // Propagate the default queue's policy settings
                final int max_bursts = this.defaultQueue_.getMaxBursts();
                final int byte_threshold = this.defaultQueue_.getByteThreshold();
                final int latency_threshold = this.defaultQueue_.getLatencyThreshold();
                queue = new Queue(streamID, max_bursts, byte_threshold, latency_threshold);
            }
            this.streamQueues_.put(streamID, queue);
        }
        return queue;
    }

    private boolean isInterleaved ()
    {
        return (RoutingMode.ROUTE_ALL_INTERLEAVED == routingMode_);
    }
}
