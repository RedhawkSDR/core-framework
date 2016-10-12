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
import BULKIO.PrecisionUTCTime;

public class  utils {

    public static PrecisionUTCTime  create( double wholesecs, double fractionalsecs ) {
	return create( wholesecs, fractionalsecs, BULKIO.TCM_CPU.value );
    }

    public static PrecisionUTCTime  create( double wholesecs, double fractionalsecs, short tsrc ) {

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
	PrecisionUTCTime tstamp = new PrecisionUTCTime();
        tstamp = T;
	tstamp.twsec += (int)(numSamples*xdelta);
	tstamp.tfsec += numSamples*xdelta-(int)(numSamples*xdelta);
        if (tstamp.tfsec >= 1.0){
	    tstamp.twsec += 1;
	    tstamp.tfsec -= 1.0;
        }
	return tstamp;
    }
}
