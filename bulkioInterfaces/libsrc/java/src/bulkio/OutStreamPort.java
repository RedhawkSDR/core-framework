package bulkio;

import org.apache.log4j.Logger;

import BULKIO.PrecisionUTCTime;

public abstract class OutStreamPort<E extends BULKIO.updateSRIOperations,A> extends OutDataPort<E,A> {
    /**
     * CORBA transfer limit in bytes
     */
    // Multiply by some number < 1 to leave some margin for the CORBA header
    protected static final int MAX_PAYLOAD_SIZE = (int)(Const.MAX_TRANSFER_BYTES * 0.9);

    /**
     * CORBA transfer limit in samples
     */
    protected final int maxSamplesPerPush;

    protected OutStreamPort(String portName, Logger logger, ConnectionEventListener connectionListener, int size) {
        super(portName, logger, connectionListener, size);
        // Make sure max samples per push is even so that complex data case is
        // handled properly
        this.maxSamplesPerPush = (MAX_PAYLOAD_SIZE/this.sizeof) & 0xFFFFFFFE;
    }

    protected void doPushPacket(A data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        pushOversizedPacket(data, time, endOfStream, streamID);
    }

    private void pushOversizedPacket(A data, PrecisionUTCTime time, boolean endOfStream, String streamID) {
        final int length = arraySize(data);

        // If there is no need to break data into smaller packets, skip
        // straight to the pushPacket call and return.
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

    protected abstract A copyOfRange(A array, int start, int end);
}
