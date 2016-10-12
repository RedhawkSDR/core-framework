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

package bulkio.time;

import java.lang.System;
import java.util.Calendar;
import java.util.TimeZone;

import BULKIO.PrecisionUTCTime;

public class utils {

    public static PrecisionUTCTime create( double wholesecs, double fractionalsecs ) {
        return create( wholesecs, fractionalsecs, BULKIO.TCM_CPU.value );
    }

    public static PrecisionUTCTime create( double wholesecs, double fractionalsecs, short tsrc ) {

        double wsec = wholesecs;
        double fsec = fractionalsecs;
        if ( wsec < 0.0 || fsec < 0.0 ) {
            long tmp_time = System.currentTimeMillis();
            wsec = tmp_time /1000;
            fsec = (tmp_time % 1000)/1000.0;
        }
        PrecisionUTCTime tstamp = new PrecisionUTCTime();
        tstamp.tcmode = tsrc;
        tstamp.tcstatus = BULKIO.TCS_VALID.value;
        tstamp.toff = 0.0;
        tstamp.twsec = wsec;
        tstamp.tfsec = fsec;
        return tstamp;
    }


    public static PrecisionUTCTime now() {
        return create(-1.0,-1.0,BULKIO.TCM_CPU.value);
    }

    public static PrecisionUTCTime notSet() {
        PrecisionUTCTime tstamp = create(0.0,0.0,BULKIO.TCM_OFF.value);
        tstamp.tcstatus = BULKIO.TCS_INVALID.value;
        return tstamp;
    }

    public static PrecisionUTCTime addSampleOffset(final PrecisionUTCTime T, int numSamples, double xdelta) {
        PrecisionUTCTime tstamp = utils.copy(T);
        tstamp.twsec += (int)(numSamples*xdelta);
        tstamp.tfsec += numSamples*xdelta-(int)(numSamples*xdelta);
        if (tstamp.tfsec >= 1.0){
            tstamp.twsec += 1;
            tstamp.tfsec -= 1.0;
        }
        return tstamp;
    }

    /**
     * Normalizes a PrecisionUTCTime, such that the whole portion contains an integral number of seconds,
     * and the fractional portion is in the range [0.0, 1.0).
     */
    public static void normalize(PrecisionUTCTime time) {
        // Get fractional adjustment from whole seconds
        double fadj = time.twsec % 1.0;
        time.twsec -= fadj;

        // Adjust fractional seconds and get whole seconds adjustment
        time.tfsec += fadj;
        double wadj = Math.floor(time.tfsec);
        time.twsec += wadj;
        time.tfsec -= wadj;
    }

    /**
     * Returns a new copy of a PrecisionUTCTime.
     */
    public static PrecisionUTCTime copy(PrecisionUTCTime time) {
        return new PrecisionUTCTime(time.tcmode, time.tcstatus, time.toff, time.twsec, time.tfsec);
    }

    public static int compare(PrecisionUTCTime time1, PrecisionUTCTime time2) {
        if (time1.twsec == time2.twsec) {
            return Double.compare(time1.tfsec, time2.tfsec);
        }
        return Double.compare(time1.twsec, time2.twsec);
    }

    /**
     * Returns the result of adding an offset to a PrecisionUTCTime.
     */
    public static PrecisionUTCTime add(PrecisionUTCTime time, double seconds) {
        return utils.increment(utils.copy(time), seconds);
    }

    /**
     * Adds an offset to a PrecisionUTCTime.
     */
    public static PrecisionUTCTime increment(PrecisionUTCTime time, double seconds) {
        // Separate the fractional and whole portions of the offset
        double fractional = seconds % 1.0;
        double whole = seconds - fractional;
        time.tfsec += fractional;
        time.twsec += (seconds - fractional);
        utils.normalize(time);
        return time;
    }

    /**
     * Returns the result of substracting an offset from a PrecisionUTCTime.
     */
    public static PrecisionUTCTime subtract(PrecisionUTCTime time, double seconds) {
        return utils.add(time, -seconds);
    }

    /**
     * Subtracts an offset from a PrecisionUTCTime.
     */
    public static PrecisionUTCTime decrement(PrecisionUTCTime time, double seconds) {
        return utils.increment(time, -seconds);
    }

    /**
     * Returns the difference, in seconds, between two PrecisionUTCTime values (i.e., lhs - rhs).
     */
    public static double difference(PrecisionUTCTime lhs, PrecisionUTCTime rhs) {
        return (lhs.twsec - rhs.twsec) + (lhs.tfsec - rhs.tfsec);
    }

    /**
     * String format to produce YYYY:MM:DD::HH:MM:SS.SSSSSS output for PrecisionUTCTime.
     */
    private static final String TIME_FORMAT = "%1$tY:%1$tm:%1$td::%1$tH:%1$tM:%1$tS.%2$06d";

    /**
     * Formats a PrecisionUTCTime as a human-readable string following the format:
     *   YYYY:MM:DD::HH:MM:SS.SSSSSS
     */
    public static String toString(PrecisionUTCTime time) {
        // Use Calendar to hold the integral seconds, but since it is limited
        // to millisecond precision, exclude the fractional seconds. It must be
        // created with the UTC time zone, otherwise the formatter will return
        // local time.
        Calendar calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
        calendar.setTimeInMillis((long)(time.twsec * 1000.0));

        // Append the fractional seconds down to microsecond precision.
        int usec = (int) Math.round(time.tfsec * 1000000.0);

        return String.format(utils.TIME_FORMAT, calendar, usec);
    }
}
