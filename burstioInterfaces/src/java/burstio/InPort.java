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

public interface InPort<E>
{

    /**
     * Return the name for this port
     * <p>
     * Return the port name used when the port was created
     *
     */
    public String getName();

    /**
     * Starts accepting burst data on this port.
     * <p>
     * If the port is not started, received bursts will be dropped.
     *
     * @see stop
     */
    public void start ();

    /**
     * Stops accepting burst data on this port.
     * <p>
     * Any calls to <code>getBurst</code> or <code>getBursts</code> currently
     * waiting for a burst will return immediately, and received bursts will
     * be dropped. Any blocked senders will be unblocked.
     *
     * @see start
     */
    public void stop ();

    /**
     * Gets the maximum number of bursts that can be queued before subsequent
     * <code>pushBursts</code> calls will block.
     * <p>
     * There is no guarantee that the queue will not exceed the threshold at
     * any given time, as an upstream output port may send arbitrarily large
     * numbers of bursts (within the limits of the max GIOP message size).
     *
     * @return the current queue threshold value
     * @see setQueueThreshold
     */
    public int getQueueThreshold ();

    /**
     * Sets the maximum number of bursts that can be queued before subsequent
     * <code>pushBursts</code> calls will block.
     * <p>
     * There is no guarantee that the queue will not exceed the threshold at
     * any given time, as an upstream output port may send arbitrarily large
     * numbers of bursts (within the limits of the max GIOP message size).
     *
     * @param count new queue threshold; must be positive
     * @see getQueueThreshold
     */
    public void setQueueThreshold (int count);

    /**
     * Checks whether a <code>pushBursts</code> call has blocked since the last
     * time this method was called.
     * <p>
     * Calling this method clears the blocked flag, so subsequent calls will
     * return false unless another call blocks.
     *
     * @return true if a queue block has occurred
     */
    public boolean blockOccurred ();

    /**
     * Gets the current queue depth, in bursts.
     * <p>
     * Note that the queue depth may exceed the configured burst threshold.
     *
     * @return current queue depth in bursts
     */
    public int getQueueDepth ();

    /**
     * Drops all queued bursts.
     * <p>
     * Any senders that are currently blocked will be unblocked, and may start
     * queueing bursts again.
     */
    public void flush ();

    public BULKIO.PortUsageType state();
    public BULKIO.PortStatistics statistics();

    public void pushBursts(E[] bursts);

    /**
     * Gets the next queued burst.
     *
     * @param timeout time to wait in seconds
     * @return        the next burst in the queue
     */
    //public E getBurst (float timeout);
    
    /**
     * Gets all queued bursts.
     *
     * @param timeout time to wait in seconds
     * @return        an array containing all queued bursts
     */
    public E[] getBursts (float timeout);
}
