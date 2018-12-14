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

package org.ossie.redhawk.time;

import java.lang.System;
import java.util.Calendar;
import java.util.TimeZone;

import CF.UTCTime;

public class utils {

    public static UTCTime create( double wholesecs, double fractionalsecs) {

        double wsec = wholesecs;
        double fsec = fractionalsecs;
        if ( wsec < 0.0 || fsec < 0.0 ) {
            long tmp_time = System.currentTimeMillis();
            wsec = tmp_time /1000;
            fsec = (tmp_time % 1000)/1000.0;
        }
        UTCTime tstamp = new UTCTime();
        tstamp.tcstatus = 1;
        tstamp.twsec = wsec;
        tstamp.tfsec = fsec;
        return tstamp;
    }


    public static UTCTime now() {
        return create(-1.0,-1.0);
    }

    public static UTCTime notSet() {
        UTCTime tstamp = create(0.0,0.0);
        tstamp.tcstatus = 0;
        return tstamp;
    }

    /**
     * Normalizes a UTCTime, such that the whole portion contains an integral number of seconds,
     * and the fractional portion is in the range [0.0, 1.0).
     */
    public static void normalize(UTCTime time) {
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
     * Returns a new copy of a UTCTime.
     */
    public static UTCTime copy(UTCTime time) {
        return new UTCTime(time.tcstatus, time.twsec, time.tfsec);
    }

    public static int compare(UTCTime time1, UTCTime time2) {
        if (time1.twsec == time2.twsec) {
            return Double.compare(time1.tfsec, time2.tfsec);
        }
        return Double.compare(time1.twsec, time2.twsec);
    }

    /**
     * Returns the result of adding an offset to a UTCTime.
     */
    public static UTCTime add(UTCTime time, double seconds) {
        return utils.increment(utils.copy(time), seconds);
    }

    /**
     * Adds an offset to a UTCTime.
     */
    public static UTCTime increment(UTCTime time, double seconds) {
        // Separate the fractional and whole portions of the offset
        double fractional = seconds % 1.0;
        double whole = seconds - fractional;
        time.tfsec += fractional;
        time.twsec += (seconds - fractional);
        utils.normalize(time);
        return time;
    }

    /**
     * Returns the result of substracting an offset from a UTCTime.
     */
    public static UTCTime subtract(UTCTime time, double seconds) {
        return utils.add(time, -seconds);
    }

    /**
     * Subtracts an offset from a UTCTime.
     */
    public static UTCTime decrement(UTCTime time, double seconds) {
        return utils.increment(time, -seconds);
    }

    /**
     * Returns the difference, in seconds, between two UTCTime values (i.e., lhs - rhs).
     */
    public static double difference(UTCTime lhs, UTCTime rhs) {
        return (lhs.twsec - rhs.twsec) + (lhs.tfsec - rhs.tfsec);
    }

    /**
     * String format to produce YYYY:MM:DD::HH:MM:SS.SSSSSS output for UTCTime.
     */
    private static final String TIME_FORMAT = "%1$tY:%1$tm:%1$td::%1$tH:%1$tM:%1$tS.%2$06d";

    /**
     * Formats a UTCTime as a human-readable string following the format:
     *   YYYY:MM:DD::HH:MM:SS.SSSSSS
     */
    public static String toString(UTCTime time) {
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

    /**
     * Converts a human-readable string following of the format:
     *   YYYY:MM:DD::HH:MM:SS.SSSSSS or 'now'
     * to UTCTime
     */
    public static UTCTime convert(String time) {
        if (time.equals("now")) {
            return now();
        }
        String[] token = time.split(":");
        if (token.length != 7)
            return new CF.UTCTime();
        int year = Integer.parseInt(token[0]);
        int month = Integer.parseInt(token[1])-1;
        int day = Integer.parseInt(token[2]);
        int hours = Integer.parseInt(token[4]);
        int minutes = Integer.parseInt(token[5]);
        double full_seconds = Double.parseDouble(token[6]);
        int seconds = (int)full_seconds;
        Calendar _calendar = Calendar.getInstance(TimeZone.getTimeZone("UTC"));
        
        _calendar.set(year, month, day, hours, minutes, seconds);
        double wsec = _calendar.getTimeInMillis()/1000;
        double fsec = full_seconds - seconds;
        return new CF.UTCTime((short)1, wsec, fsec);
    }
}
