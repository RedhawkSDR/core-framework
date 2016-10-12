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

/*******************************************************************************************


*******************************************************************************************/
#include <bulkio_p.h>

namespace  bulkio {


  namespace  time {


    namespace utils {

      BULKIO::PrecisionUTCTime create( const double wholeSecs, const double fractionalSecs, const bulkio::Int16 tsrc ) {

	double wsec = wholeSecs;
	double fsec = fractionalSecs;
	if ( wsec < 0.0 || fsec < 0.0 ) {
	  struct timeval tmp_time;
	  struct timezone tmp_tz;
	  gettimeofday(&tmp_time, &tmp_tz);
	  wsec = tmp_time.tv_sec;
	  fsec = tmp_time.tv_usec / 1e6;
	}
	BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
	tstamp.tcmode = tsrc;
	tstamp.tcstatus = BULKIO::TCS_VALID;
	tstamp.toff = 0.0;
	tstamp.twsec = wsec;
	tstamp.tfsec = fsec;
	return tstamp;
      }

      BULKIO::PrecisionUTCTime now() {
	return create();
      }

      BULKIO::PrecisionUTCTime notSet() {
        BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime();
        tstamp.tcmode = BULKIO::TCM_OFF;
        tstamp.tcstatus = BULKIO::TCS_INVALID;
        tstamp.toff = 0.0;
        tstamp.twsec = 0.0;
        tstamp.tfsec = 0.0;
        return tstamp;
      }

      BULKIO::PrecisionUTCTime addSampleOffset( const BULKIO::PrecisionUTCTime &T, const size_t numSamples, const double xdelta  ){
	BULKIO::PrecisionUTCTime tstamp = BULKIO::PrecisionUTCTime(T);
        tstamp.twsec += (long)(numSamples*xdelta);
        tstamp.tfsec += numSamples*xdelta-(long)(numSamples*xdelta);
        if (tstamp.tfsec >= 1.0){
            tstamp.twsec += 1;
            tstamp.tfsec -= 1.0;
        }
        return tstamp;
      }
    };

    bool DefaultComparator( const BULKIO::PrecisionUTCTime &T1, const BULKIO::PrecisionUTCTime &T2  ){
      if (T1.tcmode != T2.tcmode)
	return false;
      if (T1.tcstatus != T2.tcstatus)
	return false;
      if (T1.tfsec != T2.tfsec)
	return false;
      if (T1.toff != T2.toff)
	return false;
      if (T1.twsec != T2.twsec)
	return false;
      return true;
    }

  }  // end of timestamp namespace



} // end of bulkio namespace

