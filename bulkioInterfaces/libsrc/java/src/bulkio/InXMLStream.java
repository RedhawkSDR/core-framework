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
/*
 * WARNING: This file is generated from InPort.java.template.
 *          Do not modify directly.
 */
package bulkio;

import org.apache.log4j.Logger;

import org.ossie.component.RHLogger;

import BULKIO.PortStatistics;
import BULKIO.PortUsageType;
import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import bulkio.InStreamBase;
import bulkio.InXMLPort;
import bulkio.XMLDataBlock;
import bulkio.sri.utils;

import java.util.ArrayDeque;
import java.util.Arrays;
import java.util.Iterator;

public class InXMLStream extends InStreamBase {

    protected InXMLPort _port;
    protected int _eosState;
    protected int _samplesQueued;
    protected int _sampleOffset;
    protected int _queuedPacketIndex;
    protected boolean _enabled;
    protected boolean _newstream;
    protected StreamSRI _sri;
    protected ArrayDeque<InXMLPort.Packet> _queue;
    
    public InXMLStream(BULKIO.StreamSRI sri, InXMLPort port) {
        /*
        Create an InXMLStream.

        Warning:
            Input streams are created by the input port. This constructor
            should not be called directly.

        See Also:
            InPort.getCurrentStream
            InPort.getStream
        */
        this._port = port;
        this._eosState = InStreamBase.EOS_NONE;
        this._enabled = true;
        this._newstream = true;
        this._samplesQueued = 0;
        this._sampleOffset = 0;
        this._queuedPacketIndex = 0;
        this._sri = sri;
        this._queue = new ArrayDeque<InXMLPort.Packet>();
        //self.__blockType = blockType
    }

    public BULKIO.StreamSRI sri() {
        return this._sri;
    }

    public XMLDataBlock read() {
        /*
        Blocking read of the next packet for this stream.

        The read may fail if:
            * End-of-stream has been reached
            * The input port is stopped
 
        Returns:
            DataBlock if successful.
            None if the read failed.
        */
        return this._readPacket(true);
    }

    public XMLDataBlock tryread() {
        /*
        Non-blocking version of read().

        The read returns immediately whether data is available or not.

        Returns:
            DataBlock if successful.
            None if no data is available or the read failed.
        */
        return this._readPacket(false);
    }

    public boolean enabled() {
        /*
        bool: Indicates whether this stream can receive data.

        If a stream is enabled, packets received for its stream ID are queued
        in the input port, and the stream may be used for reading. Conversely,
        packets for a disabled stream are discarded, and no reading may be
        performed.
        */
        return this._enabled;
    }

    public void enable() {
        /*
        Enable this stream for reading data.

        The input port will resume queuing packets for this stream.
        */
        // Changing the enabled flag requires holding the port's streamsMutex
        // (that controls access to the stream map) to ensure that the change
        // is atomic with respect to handling end-of-stream packets. Otherwise,
        // there is a race condition between the port's IO thread and the
        // thread that enables the stream--it could be re-enabled and start
        // reading in between the port checking whether to discard the packet
        // and closing the stream. Because it is assumed that the same thread
        // that calls enable is the one doing the reading, it is not necessary
        // to apply mutual exclusion across the entire public stream API, just
        // enable/disable.

        synchronized (this._port.streamsMutex) {
            this._enabled = true;
        }
    }

    public boolean ready()
    {
        return samplesAvailable() > 0;
    }

    public int samplesAvailable()
    {
        this._refreshQueue();
        return this._samplesQueued;
    }
    
    public void disable() {
        /*
        Disable this stream for reading data.
        
        The input port will discard any packets that are currently queued for
        this stream, and all future packets for this stream will be discarded
        upon receipt until an end-of-stream is received.

        Disabling unwanted streams may improve performance and queueing
        behavior by reducing the number of queued packets on a port.
        */

        // Unless end-of-stream has been received by the port (meaning any further
        // packets with this stream ID are for a different instance), purge any
        // packets for this stream from the port's queue
        this._disable();
    }

    public boolean eos() {
        /*
        Checks whether this stream has ended.

        A stream is considered at the end when it has read and consumed all
        data up to the end-of-stream marker. Once end-of-stream has been
        reached, all read operations will fail immediately, as no more data
        will ever be received for this stream.

        The recommended practice is to check @a eos any time a read operation
        fails or returns fewer samples than requested. When the end-of-stream
        is acknowledged, either by checking @a eos or when successive reads
        fail due to an end-of-stream, the stream is removed from the input
        port. If the input port has another stream with the same streamID
        pending, it will become active.

        Returns:
            True if this stream has reached the end.
            False if the end of stream has not been reached.
        */
        return this._checkEos();
    }

    protected void _addTimestamp(XMLDataBlock data, double inputOffset, int outputOffset, BULKIO.PrecisionUTCTime time)
    {
        // Determine the timestamp of this chunk of data; if this is the
        // first chunk, the packet offset (number of samples already read)
        // must be accounted for, so adjust the timestamp based on the SRI.
        // Otherwise, the adjustment is a noop.
        double time_offset = inputOffset * data.xdelta();
        // Check the SRI directly for the complex mode because bit data blocks
        // intentionally do not have a complex() method.
        if (data.sri().mode != 0) {
            // Complex data; each sample is two values
            time_offset /= 2.0;
            outputOffset /= 2;
        }

        // If there is a time offset, apply the adjustment and mark the timestamp
        // so that the caller knows it was calculated rather than received
        boolean synthetic = false;
        if (time_offset > 0.0) {
            time = bulkio.time.utils.add(time, time_offset);
            synthetic = true;
        }

        data.addTimestamp(time, outputOffset, synthetic);
    }

    protected void _setBlockFlags(XMLDataBlock block, InXMLPort.Packet packet)
    {
        // Allocate empty data block and propagate the SRI change and input
        // queue flush flags
        int flags=0;
        if (packet.sriChanged) {
            flags = bulkio.sri.utils.compareFields(this._sri, packet.SRI);
            block.setSriChangeFlags(flags);
        }
        if (_newstream) {
            _newstream=false;
            flags = flags | bulkio.sri.utils.STREAMID | bulkio.sri.utils.XDELTA | bulkio.sri.utils.YDELTA | bulkio.sri.utils.KEYWORDS | bulkio.sri.utils.MODE;
            block.setSriChangeFlags(flags);
        }
        if (packet.inputQueueFlushed) {
            block.setInputQueueFlushed(true);
        }
    }

    protected void _disable() {
        this._queue.clear();
        this._sampleOffset = 0;
        this._samplesQueued = 0;
        this._enabled = false;
        if (this._eosState == EOS_NONE) {
            this._port._discardPacketsForStream(this._sri.streamID);
        }
    }

    protected boolean _checkEos() {
        // Internal method to check for end-of-stream. Subclasses should extend
        //  or override this method.

        this._reportIfEosReached();
        //  At this point, if end-of-stream has been reached, the state is
        //  reported (it gets set above), so the checking for the latter is
        //  sufficient
        boolean retval = (this._eosState == EOS_REPORTED);
        return retval;
    }

    protected boolean _hasBufferedData() {
        this._refreshQueue();
        if (!_queue.isEmpty()) {
                // Return true if either there are queued or pending packets
            return true;
        }
        // For the base class, there is no data to report; however, to nudge
        // the check end-of-stream, return true if it has been reached but not
        // reported
        boolean retval = (this._eosState == EOS_REACHED);
        return retval;
    }

    protected boolean _queuePacket(InXMLPort.Packet packet) {
        if ((packet.EOS) && (packet.dataBuffer.length() == 0)) {
            // Handle end-of-stream packet with no data (assuming that
            // timestamps, SRI changes, and queue flushes are irrelevant at this
            // point)
            if (this._queue.isEmpty()){ 
                // No queued packets, read pointer has reached end-of-stream
                this._eosState = EOS_REACHED;
            } else {
                // Assign the end-of-stream flag to the last packet in the queue
                // so that it is handled on read
                this._queue.peekLast().EOS = true;
            }
            // Let the caller know that no more sample data is forthcoming
            return false;
        } else {
            // Add the packet to the queue
            this._samplesQueued += 1;
            this._queue.add(packet);
            return true;
        }
    }

    protected void _close() {
        // NB: This method is always called by the port with streamsMutex held
        // Consider end-of-stream reported, since the stream has already been
        // removed from the port; otherwise, there's nothing to do
        this._eosState = EOS_REPORTED;
    }

    //this is where the eos needs to be processed wrt read eos vs retrieve data
    protected XMLDataBlock _readPacket(boolean blocking) {
        InXMLPort.Packet packet = this._fetchNextPacket(blocking);
        if (this._eosState == InStreamBase.EOS_RECEIVED) {
            this._eosState = InStreamBase.EOS_REACHED;
        }
        if (packet == null) {
            this._reportIfEosReached();
            return null;
        }
        if (packet.EOS && (packet.dataBuffer.length() == 0)) {
            this._reportIfEosReached();
            return null;
        }

        // Turn packet into a data block
        int sri_flags = this._getSriChangeFlags(packet);
        XMLDataBlock block = new XMLDataBlock(packet.SRI, packet.dataBuffer, sri_flags, packet.inputQueueFlushed);
        // Only add a timestamp if one was given (XML does not include one in
        //  the CORBA interface, but passes a None to adapt to the common port
        //  implementation)
        if (packet.T != null) {
            block.addTimestamp(packet.T, 0, false);
        }

        // Update local SRI from packet
        this._sri = packet.SRI;
        return block;
    }

    protected boolean _canBridge(InXMLPort.Packet packet) {
        boolean retval = (packet.sriChanged || packet.inputQueueFlushed);
        return (!retval);
    }

    protected InXMLPort.Packet _fetchPacket(boolean blocking) {
        return this._fetchNextPacket(blocking);
    }

    protected void _refreshQueue() {
        InXMLPort.Packet packet = null;
        if ((this._eosState != InStreamBase.EOS_REACHED) && (this._eosState != InStreamBase.EOS_RECEIVED)) {
            do {
                packet = this._port.fetchPacket(this._sri.streamID);
                if (packet != null) {
                    // TODO: Data conversion (no-op for all types except dataChar/dataByte)
                    //packet.dataBuffer = this._port._reformat(packet.dataBuffer);
                    this._queuePacket(packet);
                    if (packet.EOS) {
                        this._eosState = InStreamBase.EOS_RECEIVED;
                        break;
                    }
                }
            } while (packet != null);
        }
        if (this._queue.size() <= 1) {
            if (this._eosState == InStreamBase.EOS_RECEIVED) {
                this._eosState = InStreamBase.EOS_REACHED;
            }
        }
    }

    protected InXMLPort.Packet _fetchNextPacket(boolean blocking) {
            // Don't fetch a packet from the port if stream is disabled
        if (!this._enabled) {
            return null;
        }

        // Any future packets with this stream ID belong to another InputStream
        if ((this._eosState != InStreamBase.EOS_NONE) && (this._eosState != InStreamBase.EOS_RECEIVED) && (this._eosState != InStreamBase.EOS_REACHED)) {
            return null;
        }

        long timeout = 0;
        if (blocking) {
            timeout = bulkio.Const.BLOCKING;
        } else {
            timeout = bulkio.Const.NON_BLOCKING;
        }

        this._refreshQueue();

        return this._queue.poll();
    }

    protected void _reportIfEosReached() {
        if (this._eosState == InStreamBase.EOS_REACHED) {
            /* This is the first time end-of-stream has been checked since it
                was reached; remove the stream from the port now, since the
                caller knows that the stream ended */
            // TODO
            this._port._removeStream(this._sri.streamID);
            this._eosState = InStreamBase.EOS_REPORTED;
        }
    }

    protected int _getSriChangeFlags(InXMLPort.Packet packet) {
        if (this._newstream) {
            this._newstream = false;
            return -1;
        } else if (packet.sriChanged) {
            return bulkio.sri.utils.compareFields(this._sri, packet.SRI);
        } else {
            return bulkio.sri.utils.NONE;
        }
    }

    protected BULKIO.StreamSRI _nextSRI(boolean blocking) {
        this._refreshQueue();
        if (this._queue.isEmpty()) {
            if (_fetchPacket(blocking) == null) {
                return null;
            }
        }

        return this._queue.peekFirst().SRI;
    }

    protected void _consumePacket() {
        // Acknowledge any end-of-stream flag and delete the packet (the queue will
        // automatically delete it when it's removed)
        if (_queue.peekFirst().EOS) {
            this._eosState = InStreamBase.EOS_REACHED;
        }
        _queue.poll();
    }
};
