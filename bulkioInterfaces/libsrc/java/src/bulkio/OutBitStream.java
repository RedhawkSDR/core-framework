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

import org.ossie.buffer.bitbuffer;

import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import bulkio.OutStreamBase;
import bulkio.OutBitPort;

import java.util.Arrays;

public class OutBitStream extends OutStreamBase {

    protected OutBitPort _port;

    public OutBitStream(BULKIO.StreamSRI sri, OutBitPort port) {
        super(sri);
        this._port = port;
        _bufferSize = 0;
        _bufferOffset = 0;
        _bufferWritePtr = 0;
        _bufferTime = null;
        _buffer = new bitbuffer();
    };

    public int modcount() {
        return 0;
    };

    protected bitbuffer _buffer;
    protected int _bufferSize;
    protected int _bufferOffset;
    protected int _bufferWritePtr;
    protected BULKIO.PrecisionUTCTime _bufferTime;

    /**
     * @brief  Gets the internal buffer size.
     * @returns  Number of real samples to buffer per push.
     * @pre  Stream is valid.
     *
     * The buffer size is in terms of real samples, ignoring the complex
     * mode of the stream. Complex samples count as two real samples for
     * the purposes of buffering.
     *
     * A buffer size of 0 indicates that buffering is disabled.
     */
    public int bufferSize() {
        return _bufferSize;
    };

    public void setBufferSize(int samples) {
        if (samples == _bufferSize) {
            return;
        }
        _bufferSize = samples;
        if (_bufferSize <= _bufferOffset) {
            this.flush();
        } else if (_bufferSize > _buffer.length) {
            bitbuffer new_buffer = new bitbuffer(_bufferSize);
            for (int i=0; i<_buffer.length; i++) {
                if (_buffer.get(i)) {
                    new_buffer.set(i);
                }
            }
            _buffer = new_buffer;
        }
    };

    protected void _writeMultiple(bitbuffer data, bulkio.SampleTimestamp[] times, boolean eos) {
        if (this.checkSRI()) {
            this._port.pushSRI(this._sri);
        }
        int bit_offset = 0;
        int orig_data_idx = 0;
        for (int times_idx = 0; times_idx < times.length; times_idx++) {
            if (times.length == 1) {
                BULKIO.BitSequence out_data = new BULKIO.BitSequence();
                out_data.bits = data.length;
                out_data.data = data.toByteArray();
                this._port.pushPacket(out_data, times[0].time, eos, this._sri.streamID);
            } else {
                int end_offset = data.length;
                if (times_idx != times.length - 1) {
                    end_offset = times[times_idx+1].offset;
                }
                if (times[times_idx].offset >= data.length) {
                    continue;
                }
                if (end_offset > data.length) {
                    end_offset = data.length;
                }
                BULKIO.BitSequence out_data = new BULKIO.BitSequence();
                bitbuffer _data = new bitbuffer(end_offset-times[times_idx].offset);
                for (int i=0; i<end_offset-times[times_idx].offset; i++) {
                    if (data.get(i+times[times_idx].offset)) {
                        _data.set(i);
                    }
                }
                out_data.bits = _data.length;
                out_data.data = _data.toByteArray();
                this._port.pushPacket(out_data, times[times_idx].time, eos, this._sri.streamID);
            }
        }
    };

    /**
     * @brief  Flushes the internal buffer.
     * @pre  Stream is valid.
     *
     * Any data in the internal buffer is sent to the port to be pushed.
     */
    public void flush() {
        this._flush(false);
    };

    protected void _flush(boolean eos) {
        bulkio.SampleTimestamp[] times = new SampleTimestamp[1];
        if (this._bufferTime != null) {
            times[0] = new bulkio.SampleTimestamp(this._bufferTime, 0, false);
            if (this._bufferSize != 0) {
                if ((this._bufferWritePtr != 0) || (eos)) {
                    bitbuffer tmp_buffer = new bitbuffer(this._bufferWritePtr);
                    for (int ii=0;ii<this._bufferWritePtr;ii++) {
                        if (this._buffer.get(ii)) {
                            tmp_buffer.set(ii);
                        }
                    }
                    this._writeMultiple(tmp_buffer, times, eos);
                }
            } else {
                if ((this._bufferOffset == 0) && (eos)) {
                    bitbuffer tmp_buffer = new bitbuffer(0);
                    this._writeMultiple(tmp_buffer, times, eos);
                } else if (this._bufferOffset != 0) {
                    bitbuffer tmp_buffer = new bitbuffer(this._bufferOffset);
                    for (int ii=0;ii<this._bufferOffset;ii++) {
                        if (this._buffer.get(ii)) {
                            tmp_buffer.set(ii);
                        }
                    }
                    this._writeMultiple(tmp_buffer, times, eos);
                }
            }
        }
        this._bufferOffset = 0;
        this._bufferWritePtr = 0;
        this._buffer = new bitbuffer(this._bufferSize);
        this._bufferTime = null;
    };

    protected void _doBuffer(bitbuffer data, BULKIO.PrecisionUTCTime time) {
        // If this is the first data being queued, use its timestamp for the start
        // time of the buffered data
        if (this._bufferOffset == 0) {
            this._bufferTime = time;
        }

        // Only buffer up to the currently configured buffer size
        int count = Math.min(data.length, this._bufferSize - this._bufferOffset);
        for (int data_idx=0; data_idx<count; data_idx++) {
            if (data.get(data_idx)) {
                _buffer.set(data_idx+this._bufferOffset);
            }
        }

        // Advance buffer offset, flushing if the buffer is full
        this._bufferOffset += count;
        this._bufferWritePtr = this._bufferOffset;
        if (this._bufferOffset >= this._bufferSize) {
            this._bufferWritePtr = this._bufferSize;
            _flush(false);
        }

        // Handle remaining data
        if (count < data.length) {
            double synthetic_offset = _sri.xdelta * count;
            BULKIO.PrecisionUTCTime next = bulkio.time.utils.add(time, synthetic_offset);
            bitbuffer recursive_data = new bitbuffer(data.length-count);
            for (int i=0; i<data.length-count; i++) {
                if (data.get(i+count)) {
                    recursive_data.set(i);
                }
            }
        this._doBuffer(recursive_data, next);
        }
    }

    /*
        * @brief  Write real sample data to the stream.
        * @param data  %shared_buffer containing real data.
        * @param time  Time stamp of first sample.

        * Sends the real data in @a data as single packet with the time stamp
        * @a time via the associated output port.
        *
        * If there are any pending SRI changes, the new SRI is pushed first.
        */
    public void write(bitbuffer data, BULKIO.PrecisionUTCTime time) {
        if ((this._bufferSize == 0) || ((this._bufferWritePtr == 0) && (data.length >= this._bufferSize))) {
            bulkio.SampleTimestamp[] times = new bulkio.SampleTimestamp[1];
            times[0] = new bulkio.SampleTimestamp(time, 0, false);
            this._writeMultiple(data, times, false);
            return;
        }
        this._doBuffer(data, time);
    };

    /**
     * @brief  Write real sample data to the stream.
     * @param data   %shared_buffer containing real data.
     * @param times  List of time stamps, with offsets.
     * @pre  Stream is valid.
     * @pre  @p times is sorted in order of offset.
     * @throw std::logic_error  If @p times is empty.
     *
     * Writes the real data in @a data to the stream, where each element of
     * @a times gives the offset and time stamp of an individual packet.
     * The offset of the first time stamp is ignored and assumed to be 0,
     * while subsequent offsets determine the length of the prior packet.
     * All offsets should be less than @a data.size().
     *
     * For example, given @a data with size 25 and three time stamps with
     * offsets 0, 10, and 20, @a data is broken into three packets of size
     * 10, 10, and 5 samples.
     *
     * If there are any pending SRI changes, the new SRI is pushed first.
     */
    public void write(bitbuffer data, bulkio.SampleTimestamp[] times) {
        if ((this._bufferSize == 0) || ((this._bufferWritePtr == 0) && (data.length >= this._bufferSize))) {
        //if (this._bufferSize == 0) {
                this._writeMultiple(data, times, false);
            return;
        }
        for (int times_idx = 0; times_idx < times.length; times_idx++) {
            if (times.length == 1) {
                this._doBuffer(data, times[0].time);
            } else {
                int end_offset = data.length;
                if (times_idx != times.length) {
                    end_offset = times[times_idx+1].offset;
                }
                bitbuffer _data = new bitbuffer(end_offset-times[times_idx].offset);
                for (int i=0; i<end_offset-times[times_idx].offset; i++) {
                    if (data.get(i+times[times_idx].offset)) {
                        _data.set(i);
                    }
                }
                this._doBuffer(_data, times[times_idx].time);
            }
        }
    };

    public void close() {
        bitbuffer data = new bitbuffer(0);
        BULKIO.PrecisionUTCTime time = bulkio.time.utils.now();
        bulkio.SampleTimestamp[] times = new bulkio.SampleTimestamp[1];
        times[0] = new bulkio.SampleTimestamp(time, 0, false);
        this._writeMultiple(data, times, true);
    };
};
