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

import java.util.Arrays;
import java.util.HashSet;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

import burstio.stats.ReceiverStatistics;
import burstio.traits.BurstTraits;

public class InPortImpl<E> implements InPort<E>
{
    public static final int DEFAULT_QUEUE_THRESHOLD = 100;

    private BurstTraits<E,?> traits_;

    private final String name_;
    private int queueThreshold_ = DEFAULT_QUEUE_THRESHOLD;
    private boolean started_ = false;

    private ReceiverStatistics statistics_;
    private boolean blockOccurred_ = false;
    private Set<String> streamIDs_ = new HashSet<String>();

    private Queue<E> queue_ = new LinkedList<E>();

    protected InPortImpl (final String name, BurstTraits<E,?> traits)
    {
        this.name_ = name;
        this.traits_ = traits;
        this.statistics_ = new ReceiverStatistics(this.name_, this.traits_.byteSize() * 8);
    }

    public String getName ()
    {
        return this.name_;
    }


    public void start ()
    {
        synchronized (queue_) {
            this.started_ = true;
        }
    }

    public void stop ()
    {
        synchronized (queue_) {
            if (this.started_) {
                this.started_ = false;
                this.queue_.notifyAll();
            }
        }
    }

    public int getQueueThreshold ()
    {
        return queueThreshold_;
    }

    public void setQueueThreshold (int count)
    {
        synchronized (queue_) {
            if (count > queueThreshold_) {
                queue_.notifyAll();
            }
            queueThreshold_ = count;
        }
    }

    public BULKIO.PortUsageType state()
    {
        synchronized(this.queue_) {
            if (this.queue_.isEmpty()) {
                return BULKIO.PortUsageType.IDLE;
            } else if (this.queue_.size() < this.queueThreshold_) {
                return BULKIO.PortUsageType.ACTIVE;
            } else {
                return BULKIO.PortUsageType.BUSY;
            }
        }
    }

    public BULKIO.PortStatistics statistics()
    {
        synchronized(this.queue_) {
            BULKIO.PortStatistics stats = this.statistics_.retrieve();
            stats.streamIDs = this.streamIDs_.toArray(new String[this.streamIDs_.size()]);
            return stats;
        }
    }

    public void pushBursts(E[] bursts)
    {
        long start = System.nanoTime();

        synchronized (this.queue_) {

            // Calculate queue depth based on state at invocation; this makes it
            // easy to tell if a consumer is keeping up (average = 0, or at least
            // doesn't grow) or blocking (average >= 100).
            float queue_depth = queue_.size() / (float)queueThreshold_;

            // Only set the block flag once to avoid multiple notifications
            boolean block_reported = false;

            // Wait until the queue is below the blocking threshold
            while (started_ && (queue_.size() >= queueThreshold_)) {
                // Report that this call blocked
                if (!block_reported) {
                    block_reported = true;
                    blockOccurred_ = true;
                }
                try {
                    queue_.wait();
                } catch (final InterruptedException ex) {
                    return;
                }
            }

            // Discard bursts if processing is not started
            if (!started_) {
                return;
            }

            // Add bursts to queue and notify waiters
            queue_.addAll(Arrays.asList(bursts));
            queue_.notifyAll();

            // Count total elements
            int total_elements = 0;
            for (E burst : bursts) {
                total_elements += this.traits_.burstLength(burst);
                final String stream_id = this.traits_.sri(burst).streamID;
                this.streamIDs_.add(stream_id);
            }

            // Record total time spent in pushBursts for latency measurement
            double elapsed = (System.nanoTime() - start) * 1e-9;
            this.statistics_.record(bursts.length, total_elements, queue_depth, elapsed);
        }
    }

    public boolean blockOccurred ()
    {
        synchronized (this.queue_) {
            boolean retval = this.blockOccurred_;
            this.blockOccurred_ = false;
            return retval;
        }
    }

    public int getQueueDepth ()
    {
        synchronized (this.queue_) {
            return this.queue_.size();
        }
    }

    public void flush ()
    {
        synchronized (this.queue_) {
            this.statistics_.flushOccurred(this.queue_.size());
            this.queue_.clear();
            this.queue_.notifyAll();
        }
    }

    public E getBurst (float timeout)
    {
        synchronized (this.queue_) {
            if (!this.waitBurst(timeout)) {
                return null;
            }

            if (queue_.size() <= queueThreshold_) {
                queue_.notifyAll();
            }
            E burst = queue_.remove();
            if (this.traits_.eos(burst)) {
                final String stream_id = this.traits_.sri(burst).streamID;
                this.streamIDs_.remove(stream_id);
            }
            return burst;
        }
    }

    public E[] getBursts (float timeout)
    {
        synchronized (this.queue_) {
            this.waitBurst(timeout);
            E[] bursts = this.traits_.toArray(this.queue_);
            queue_.clear();
            queue_.notifyAll();
            return bursts;
        }
    }

    protected boolean waitBurst (float timeout)
    {
        if (started_ && queue_.isEmpty() && timeout != 0.0 ) {
            try {
                if ( timeout < 0.0 ) timeout = 0.0f;
                queue_.wait(Math.round(timeout));
            } catch (final InterruptedException ex) {
                return false;
            }
        }
        return !queue_.isEmpty();
    }
}
