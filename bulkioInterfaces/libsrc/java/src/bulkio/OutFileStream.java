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

import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import bulkio.OutStreamBase;
import bulkio.OutFilePort;

import java.util.Arrays;

public class OutFileStream extends OutStreamBase {

    protected OutFilePort _port;

    public OutFileStream(BULKIO.StreamSRI sri, OutFilePort port) {
        super(sri);
        this._port = port;
    };

    public int modcount() {
        return 0;
    };

    protected void _writeMultiple(String data, BULKIO.PrecisionUTCTime time, boolean eos) {
        if (this.checkSRI()) {
            this._port.pushSRI(this._sri);
        }
        this._port.pushPacket(data, time, eos, this._sri.streamID);
    };

    /**
     * @brief  Flushes the internal buffer.
     * @pre  Stream is valid.
     *
     * Any data in the internal buffer is sent to the port to be pushed.
     */
    public void flush() {
    };

    /*
        * @brief  Write real sample data to the stream.
        * @param data  %shared_buffer containing real data.
        * @param time  Time stamp of first sample.

        * Sends the real data in @a data as single packet with the time stamp
        * @a time via the associated output port.
        *
        * If there are any pending SRI changes, the new SRI is pushed first.
        */
    public void write(String data, BULKIO.PrecisionUTCTime time) {
        this._writeMultiple(data, time, false);
    };

    public void close() {
        String data = new String();
        BULKIO.PrecisionUTCTime time = bulkio.time.utils.now();
        this._writeMultiple(data, time, true);
    };
};
