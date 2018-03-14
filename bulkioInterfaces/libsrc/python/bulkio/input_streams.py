#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

from bulkio.stream_base import StreamBase
from bulkio.datablock import DataBlock, SampleDataBlock
import bulkio.const

__all__ = ('InputStream', 'BufferedInputStream', 'NumericInputStream')

EOS_NONE = 0
EOS_RECEIVED = 1
EOS_REACHED = 2
EOS_REPORTED = 3

class InputStream(StreamBase):
    def __init__(self, sri, port):
        StreamBase.__init__(self, sri)
        self._port = port
        self._eosState = EOS_NONE
        self.__enabled = True
        self.__newstream = True

    def read(self):
        return self._readPacket(True)

    def tryread(self):
        return self._readPacket(False)

    @property
    def enabled(self):
        return self.__enabled

    def enable(self):
        # Changing the enabled flag requires holding the port's streamsMutex
        # (that controls access to the stream map) to ensure that the change
        # is atomic with respect to handling end-of-stream packets. Otherwise,
        # there is a race condition between the port's IO thread and the
        # thread that enables the stream--it could be re-enabled and start
        # reading in between the port checking whether to discard the packet
        # and closing the stream. Because it is assumed that the same thread
        # that calls enable is the one doing the reading, it is not necessary
        # to apply mutual exclusion across the entire public stream API, just
        # enable/disable.
        with self._port._streamsMutex:
            self.__enabled = True

    def disable(self):
        # See above re: locking
        with self._port._streamsMutex:
            self.__enabled = False

        # Unless end-of-stream has been received by the port (meaning any further
        # packets with this stream ID are for a different instance), purge any
        # packets for this stream from the port's queue
        if self._eosState == EOS_NONE:
            self._port._discardPacketsForStream(self.streamID);

    def eos(self):
        self._reportIfEosReached()
        # At this point, if end-of-stream has been reached, the state is
        # reported (it gets set above), so the checking for the latter is
        # sufficient
        return self._eosState == EOS_REPORTED;

    def _hasBufferedData(self):
        # For the base class, there is no data to report; however, to nudge
        # the check end-of-stream, return true if it has been reached but not
        # reported
        return self._eosState == EOS_REACHED;

    def _close(self):
        # NB: This method is always called by the port with streamsMutex held

        # Consider end-of-stream reported, since the stream has already been
        # removed from the port; otherwise, there's nothing to do
        self._eosState = EOS_REPORTED;

    def _readPacket(self, blocking):
        packet = self._fetchNextPacket(blocking)
        if self._eosState == EOS_RECEIVED:
            self._eosState = EOS_REACHED
        if not packet or (packet.EOS and (len(packet.buffer) == 0)):
            self._reportIfEosReached()
            return None

        # Turn packet into a data block
        block = self._createBlock(packet.SRI, packet.buffer)
        # Only add a timestamp if one was given (XML does not include one in
        # the CORBA interface, but passes a None to adapt to the common port
        # implementation)
        if packet.T is not None:
            block.addTimestamp(packet.T)
        self._setBlockFlags(block, packet)

        # Update local SRI from packet
        self._sri = packet.SRI
        return block

    def _fetchNextPacket(self, blocking):
        # Don't fetch a packet from the port if stream is disabled
        if not self.__enabled:
            return None

        # Any future packets with this stream ID belong to another InputStream
        if self._eosState != EOS_NONE:
            return None

        if blocking:
            timeout = bulkio.const.BLOCKING
        else:
            timeout = bulkio.const.NON_BLOCKING
        packet = self._port._nextPacket(timeout, self.streamID)
        if packet and packet.EOS:
            self._eosState = EOS_RECEIVED

        return packet

    def _reportIfEosReached(self):
        if self._eosState == EOS_REACHED:
            # This is the first time end-of-stream has been checked since it
            # was reached; remove the stream from the port now, since the
            # caller knows that the stream ended
            self._port._removeStream(self.streamID);
            self._eosState = EOS_REPORTED;

    def _setBlockFlags(self, block, packet):
        if self.__newstream:
            self.__newstream = False
            block.sriChangeFlags = -1
        elif packet.sriChanged:
            block.sriChangeFlags = bulkio.sri.compareFields(self._sri, packet.SRI)
        if packet.inputQueueFlushed:
            block.inputQueueFlushed = True

    def _createBlock(self, *args, **kwargs):
        return DataBlock(*args, **kwargs)


class BufferedInputStream(InputStream):
    def __init__(self, sri, port):
        InputStream.__init__(self, sri, port)
        self.__queue = []
        self.__samplesQueued = 0
        self.__sampleOffset = 0
        self.__pending = None

    def eos(self):
        if not self.__queue:
            # Try a non-blocking fetch to see if there's an empty end-of-stream
            # packet waiting; this helps with the case where the last read
            # consumes exactly the remaining data, and the stream will never
            # report a ready state again
            self._fetchPacket(False)

        return InputStream.eos(self)

    def read(self, count=None, consume=None):
        if count is None:
            return InputStream.read(self)
        return self._read(count, consume, True)

    def tryread(self, count=None, consume=None):
        if count is None:
            return InputStream.tryread(self)
        return self._read(count, consume, False)

    def skip(self, count):
        # If the next block of data is complex, double the skip size (which the
        # lower-level I/O handles in terms of scalars) so that the right number
        # of samples is skipped
        sri = self._nextSRI(True)
        if not sri:
            return 0

        item_size = 2 if (sri.mode != 0) else 1
        count *= item_size;

        # Queue up packets from the port until we have enough data to satisfy
        # the requested read amount
        while self.__samplesQueued < count:
            if not self._fetchPacket(True):
                break

        count = min(count, self.__samplesQueued)
        self._consumeData(count)

        # Convert scalars back to samples
        return count / item_size

    def disable(self):
        InputStream.disable(self)

        # NB: The lock is not required to modify the internal stream queue
        # state, because it should only be accessed by the thread that is
        # reading from the stream

        # Clear queued packets and pending packet
        self.__queue = []
        self.__sampleOffset = 0
        self.__samplesQueued = 0
        self.__pending = 0

    def _hasBufferedData(self):
        if self.__queue or self.__pending:
            # Return true if either there are queued or pending packets
            return True
        return InputStream._hasBufferedData(self)

    def _readPacket(self, blocking):
        if self.__samplesQueued == 0:
            self._fetchPacket(blocking)

        if self.__samplesQueued == 0:
            # It's possible that there are no samples queued because of an
            # end-of-stream; if so, report it so that this stream can be
            # dissociated from the port
            self._reportIfEosReached()
            return None

        # Only read up to the end of the first packet in the queue
        samples = len(self.__queue[0].buffer) - self.__sampleOffset;
        return self._readData(samples, samples)

    def _read(self, count, consume, blocking):
        # Consume length not specified, consume entire read
        if consume is None:
            consume = count

        # Try to get the SRI for the upcoming block of data, fetching it from
        # the port's input queue if necessary
        sri = self._nextSRI(blocking);
        if not sri:
            # No SRI retreived implies no data will be retrieved, either due
            # to end-of-stream or because it would block
            self._reportIfEosReached()
            return None

        # If the next block of data is complex, double the read and consume
        # size (which the lower-level I/O handles in terms of scalars) so that
        # the returned block has the right number of samples
        if sri.mode != 0:
            count = count * 2
            consume = consume * 2

        # Queue up packets from the port until we have enough data to satisfy
        # the requested read amount
        while self.__samplesQueued < count:
            if not self._fetchPacket(blocking):
                break

        if self.__samplesQueued == 0:
            # As above, it's possible that there are no samples due to an end-
            # of-stream
            self._reportIfEosReached()
            return None

        # Only read as many samples as are available (e.g., if a new SRI is
        # coming or the stream reached the end)
        samples = min(count, self.__samplesQueued);

        # Handle a partial read, which could mean that there's not enough data
        # at present (non-blocking), or that the read pointer has reached the
        # end of a segment (new SRI, queue flush, end-of-stream)
        if samples < count:
            # Non-blocking: return None if there's not currently a break in the
            # data, under the assumption that a future read might return the
            # full amount
            if (not blocking) and (not self.__pending) and (self._eosState == EOS_NONE):
                return None

            # Otherwise, consume all remaining data (when not explicitly
            # requested as 0)
            if consume != 0:
                consume = samples

        return self._readData(samples, consume)

    def _consumeData(self, count):
        while count > 0:
            data = self.__queue[0].buffer
            data_len = len(data)

            available = data_len - self.__sampleOffset
            nelem = min(available, count)

            self.__sampleOffset += nelem
            self.__samplesQueued -= nelem
            count -= nelem

            if self.__sampleOffset >= data_len:
                # Read pointer has passed the end of the packed data
                self._consumePacket()
                self.__sampleOffset = 0

    def _consumePacket(self):
        # Acknowledge any end-of-stream flag and delete the packet
        front = self.__queue.pop(0)
        if front.EOS:
            self._eosState = EOS_REACHED

        # If the queue is empty, move the pending packet onto the queue
        if not self.__queue and self.__pending:
            self._queuePacket(self.__pending);
            self.__pending = 0

    def _readData(self, count, consume):
        # SRI and flags are taken from the front packet
        front = self.__queue[0]

        last_offset = self.__sampleOffset + count
        if last_offset <= len(front.buffer):
            # The requsted sample count can be satisfied from the first packet
            time_stamps = [self._getTimestamp(front.SRI, self.__sampleOffset, 0, front.T)]
            data = front.buffer[self.__sampleOffset:last_offset]
        else:
            # We have to span multiple packets to get the data
            time_stamps, data = self._mergePacketData(count)

        # Allocate data block and propagate the SRI change and input queue
        # flush flags
        block = self._createBlock(front.SRI, data)
        self._setBlockFlags(block, front);
        if front.sriChanged:
            # Update the stream metadata
            self._sri = front.SRI

        # Add time stamps calculated above
        for (ts, offset, synthetic) in time_stamps:
            block.addTimestamp(ts, offset, synthetic)

        # Clear flags from packet, since they've been reported
        front.sriChanged = False;
        front.inputQueueFlushed = False;

        # Advance the read pointers
        self._consumeData(consume)

        return block

    def _mergePacketData(self, count):
        # Assembles data and calculates time stamps spanning several input
        # packets
        front = self.__queue[0]

        data = type(front.buffer)()
        time_stamps = []
        data_offset = 0

        packet_offset = self.__sampleOffset
        for packet in self.__queue:
            # Add the timestamp for this pass
            time_stamps.append(self._getTimestamp(front.SRI, packet_offset, data_offset, packet.T))

            # The number of samples copied on this pass may be less than
            # the total remaining
            available = len(packet.buffer) - packet_offset;
            nelem = min(available, count);

            # Append chunk to buffer and advance counters
            data += packet.buffer[packet_offset:packet_offset+nelem]
            data_offset += nelem
            count -= nelem

            # Next chunk (if any) will be from the next packet, starting at
            # the beginning
            packet_offset = 0

            # Finished?
            if count == 0:
                break

        return (time_stamps, data)

    def _getTimestamp(self, sri, inputOffset, outputOffset, time):
        # Determine the timestamp of this chunk of data; if this is the first
        # chunk, the packet offset (number of samples already read) must be
        # accounted for, so adjust the timestamp based on the SRI.  Otherwise,
        # the adjustment is a noop.
        time_offset = inputOffset * sri.xdelta
        if sri.mode != 0:
            # Complex data; each sample is two values
            time_offset /= 2.0
            outputOffset /= 2

        # If there is a time offset, apply the adjustment and mark the
        # timestamp so that the caller knows it was calculated rather than
        # received
        if time_offset > 0.0:
            time = time + time_offset
            synthetic = True
        else:
            synthetic = False

        return (time, outputOffset, synthetic)

    def _nextSRI(self, blocking):
        if not self.__queue:
            if not self._fetchPacket(blocking):
                return None

        return self.__queue[0].SRI

    def _fetchPacket(self, blocking):
        if self.__pending:
            # Cannot read another packet until non-bridging packet is
            # acknowledged
            return False

        packet = self._fetchNextPacket(blocking)
        if not packet:
            return False

        if not self.__queue or self._canBridge(packet):
            return self._queuePacket(packet)
        else:
            self.__pending = packet;
            return False

    def _queuePacket(self, packet):
        if packet.EOS and not packet.buffer:
            # Handle end-of-stream packet with no data (assuming that
            # timestamps, SRI changes, and queue flushes are irrelevant at this
            # point)
            if not self.__queue:
                # No queued packets, read pointer has reached end-of-stream
                self._eosState = EOS_REACHED;
            else:
                # Assign the end-of-stream flag to the last packet in the queue
                # so that it is handled on read
                self.__queue[-1].EOS = True
            # Let the caller know that no more sample data is forthcoming
            return False
        else:
            # Add the packet to the queue
            self.__samplesQueued += len(packet.buffer)
            self.__queue.append(packet);
            return True

    def _canBridge(self, packet):
        return not (packet.sriChanged or packet.inputQueueFlushed)


class NumericInputStream(BufferedInputStream):
    def _createBlock(self, *args, **kwargs):
        return SampleDataBlock(*args, **kwargs)

    def _fetchNextPacket(self, blocking):
        packet = BufferedInputStream._fetchNextPacket(self, blocking)
        if packet:
            # Data conversion (no-op for all types except dataChar/dataByte)
            packet.buffer = self._port._reformat(packet.buffer)
        return packet
