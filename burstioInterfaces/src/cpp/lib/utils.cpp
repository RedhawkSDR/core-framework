/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include <time.h>

#include <burstio/utils.h>

namespace burstio {
    namespace utils { 
        BULKIO::PrecisionUTCTime now ()
        {
            struct timespec tod;
            clock_gettime(CLOCK_REALTIME, &tod);
            BULKIO::PrecisionUTCTime tstamp;
            tstamp.tcmode = BULKIO::TCM_CPU;
            tstamp.tcstatus = 1;
            tstamp.toff = 0.0;
            tstamp.twsec = tod.tv_sec;
            tstamp.tfsec = tod.tv_nsec * 1e-9;
            return tstamp;
        }

        double elapsed (const BULKIO::PrecisionUTCTime& begin, const BULKIO::PrecisionUTCTime& end)
        {
            return (end.twsec - begin.twsec) + (end.tfsec - begin.tfsec);
        }


        BURSTIO::BurstSRI createSRI ( const std::string &streamID, double xdelta) {
          BURSTIO::BurstSRI sri;
          sri.hversion = 1;
          sri.streamID = streamID.c_str();
          sri.id = "";
          sri.xdelta = xdelta;
          sri.mode = (short)0;
          sri.flags = (short)0;
          sri.tau = 0.0;
          sri.theta = 0.0f;
          sri.gain = 0.0f;
          sri.uwlength = (short)0;
          sri.bursttype = (short)0;
          sri.burstLength = 0;
          sri.CHAN_RF = 0.0;
          sri.baudestimate = 0.0f;
          sri.carrieroffset = 0.0;
          sri.SNR = 0.0;
          sri.modulation = "";
          sri.baudrate = 0.0;
          sri.fec = "";
          sri.fecrate = "";
          sri.randomizer = "";
          sri.overhead = "";
          sri.expectedStartOfBurstTime = burstio::utils::now();
          sri.keywords.length(0);
          return sri;
        }

      BURSTIO::BurstSRI createSRI ( const std::string &streamID ) {
        return createSRI( streamID, 1.0 );
      }

    }
}
