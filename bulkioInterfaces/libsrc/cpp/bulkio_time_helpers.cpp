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
#include <sys/time.h>
#include <cmath>

#include <BULKIO/bulkioDataTypes.h>

#include "bulkio_base.h"
#include "bulkio_time_operators.h"

namespace  bulkio {


  namespace  time {


    namespace utils {

      BULKIO::PrecisionUTCTime create(double wsec, double fsec, CORBA::Short tsrc)
      {
        if ((wsec < 0.0) || (fsec < 0.0)) {
          struct timespec tod;
          clock_gettime(CLOCK_REALTIME, &tod);
          wsec = tod.tv_sec;
          fsec = tod.tv_nsec * 1e-9;
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

      void normalize(BULKIO::PrecisionUTCTime& time)
      {
        // Get fractional adjustment from whole seconds
        double fadj = std::modf(time.twsec, &time.twsec);

        // Adjust fractional seconds and get whole seconds adjustment
        double wadj = 0;
        time.tfsec = std::modf(time.tfsec + fadj, &wadj);

        // If fractional seconds are negative, borrow a second from the whole
        // seconds to make it positive, normalizing to [0,1)
        if (time.tfsec < 0.0) {
          time.tfsec += 1.0;
          wadj -= 1.0;
        }
        time.twsec += wadj;
      }
    }

    bool DefaultComparator( const BULKIO::PrecisionUTCTime &T1, const BULKIO::PrecisionUTCTime &T2  ){
      return (T1 == T2);
    }

  }  // end of timestamp namespace



} // end of bulkio namespace

