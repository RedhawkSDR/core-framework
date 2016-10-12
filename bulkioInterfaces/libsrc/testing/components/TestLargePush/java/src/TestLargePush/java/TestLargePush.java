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
package TestLargePush.java;

import java.util.Properties;

public class TestLargePush extends TestLargePush_base {

    public TestLargePush() {
        super();
    }

    protected int serviceFunction() {
        String stream_id = "testStream";
        BULKIO.StreamSRI sri = new BULKIO.StreamSRI();
        sri.mode = 0;
        sri.xdelta = 0.001;
        sri.ydelta = 0.0;
        sri.subsize = 0;
        sri.xunits = 1; // TIME_S
        sri.streamID = (stream_id != null) ? stream_id : "";

        BULKIO.PrecisionUTCTime tstamp = bulkio.time.utils.now();

        Boolean  EOS = true;

        float[] outDataFloat = new float[this.numSamples.getValue().intValue()];
        this.port_dataFloat.pushSRI(sri);
        this.port_dataFloat.pushPacket(outDataFloat, tstamp, EOS, sri.streamID);

        double[] outDataDouble = new double[this.numSamples.getValue().intValue()];
        this.port_dataDouble.pushSRI(sri);
        this.port_dataDouble.pushPacket(outDataDouble, tstamp, EOS, sri.streamID);

        short[] outDataShort = new short[this.numSamples.getValue().intValue()];
        this.port_dataShort.pushSRI(sri);
        this.port_dataShort.pushPacket(outDataShort, tstamp, EOS, sri.streamID);

        short[] outDataUshort = new short[this.numSamples.getValue().intValue()];
        this.port_dataUshort.pushSRI(sri);
        this.port_dataUshort.pushPacket(outDataUshort, tstamp, EOS, sri.streamID);

        int[] outDataLong = new int[this.numSamples.getValue().intValue()];
        this.port_dataLong.pushSRI(sri);
        this.port_dataLong.pushPacket(outDataLong, tstamp, EOS, sri.streamID);

        int[] outDataULong = new int[this.numSamples.getValue().intValue()];
        this.port_dataUlong.pushSRI(sri);
        this.port_dataUlong.pushPacket(outDataULong, tstamp, EOS, sri.streamID);

        long[] outDataLongLong = new long[this.numSamples.getValue().intValue()];
        this.port_dataLongLong.pushSRI(sri);
        this.port_dataLongLong.pushPacket(outDataLongLong, tstamp, EOS, sri.streamID);

        long[] outDataUlongLong = new long[this.numSamples.getValue().intValue()];
        this.port_dataUlongLong.pushSRI(sri);
        this.port_dataUlongLong.pushPacket(outDataUlongLong, tstamp, EOS, sri.streamID);

        byte[] outDataOctet = new byte[this.numSamples.getValue().intValue()];
        this.port_dataOctet.pushSRI(sri);
        this.port_dataOctet.pushPacket(outDataOctet, tstamp, EOS, sri.streamID);

        return FINISH;
    }

    public static void configureOrb(final Properties orbProps) {
    }
}
