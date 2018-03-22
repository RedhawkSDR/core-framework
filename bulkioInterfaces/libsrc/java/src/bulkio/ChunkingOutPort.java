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

import BULKIO.PrecisionUTCTime;

/**
 * Adds chunking of pushes to fit within the maximum CORBA transfer size for
 * BulkIO data types that support it.
 */
abstract class ChunkingOutPort<E extends BULKIO.updateSRIOperations,A> extends OutDataPort<E,A> {
    /**
     * CORBA transfer limit in bytes
     */
    // Multiply by some number < 1 to leave some margin for the CORBA header
    protected static final int MAX_PAYLOAD_SIZE = (int)(Const.MAX_TRANSFER_BYTES * 0.9);

    /**
     * CORBA transfer limit in samples
     */
    protected int maxSamplesPerPush;

    protected ChunkingOutPort(String portName, Logger logger, ConnectionEventListener connectionListener, DataHelper<A> helper) {
        super(portName, logger, connectionListener, helper);
        this.maxSamplesPerPush = (8 * MAX_PAYLOAD_SIZE) / helper.bitSize();
    }

    protected void pushPacketData(A data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        pushOversizedPacket(data, time, endOfStream, streamID);
    }

    private void pushOversizedPacket(A data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        final int length = helper.arraySize(data);

        // If there is no need to break data into smaller packets, skip
        // straight to the pushPacket call and return.
        if (length <= maxSamplesPerPush) {
            this.pushSinglePacket(data, time, endOfStream, streamID);
            return;
        }

        BULKIO.StreamSRI sri = this.currentSRIs.get(streamID).sri;
        double xdelta = sri.xdelta;
        int item_size = 1;
        if (sri.mode != 0) {
            item_size = 2;
        }
        int frame_size = item_size;
        if (sri.subsize > 0) {
            frame_size *= sri.subsize;
        }
        // Quantize the push size (in terms of scalars) to the nearest frame,
        // which takes both the complex mode and subsize into account
        final int max_push_size = (maxSamplesPerPush/frame_size) * frame_size;

        // Initialize time of first subpacket
        PrecisionUTCTime packetTime = time;
        for (int offset = 0; offset < length;) {
            // Don't send more samples than are remaining
            final int pushSize = java.lang.Math.min(length-offset, max_push_size);

            // Copy the range for this sub-packet and advance the offset
            A subPacket = helper.slice(data, offset, offset+pushSize);
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
            int data_xfer_len = pushSize / item_size;
            packetTime = bulkio.time.utils.addSampleOffset(packetTime, data_xfer_len, xdelta);
        }
    }
}
