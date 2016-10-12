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

#include <cmath>
#include <ctime>
#include <iomanip>

#include "bulkio_base.h"
#include "bulkio_time_operators.h"

namespace BULKIO {

  BULKIO::PrecisionUTCTime operator+(const BULKIO::PrecisionUTCTime& lhs, double seconds)
  {
    BULKIO::PrecisionUTCTime result = lhs;
    result += seconds;
    return result;
  }

  BULKIO::PrecisionUTCTime& operator+=(BULKIO::PrecisionUTCTime& lhs, double seconds)
  {
    // Split fractional and whole seconds to preserve precision
    lhs.tfsec += std::modf(seconds, &seconds);
    lhs.twsec += seconds;
    bulkio::time::utils::normalize(lhs);
    return lhs;
  }

  BULKIO::PrecisionUTCTime operator-(const BULKIO::PrecisionUTCTime& lhs, double seconds)
  {
    BULKIO::PrecisionUTCTime result = lhs;
    result -= seconds;
    return result;
  }

  double operator-(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs)
  {
    return (lhs.twsec - rhs.twsec) + (lhs.tfsec - rhs.tfsec);
  }

  BULKIO::PrecisionUTCTime& operator-=(BULKIO::PrecisionUTCTime& lhs, double seconds)
  {
    // Split fractional and whole seconds to preserve precision
    lhs.tfsec -= std::modf(seconds, &seconds);
    lhs.twsec -= seconds;
    bulkio::time::utils::normalize(lhs);
    return lhs;
  }

  bool operator==(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs)
  {
    if (lhs.tcmode != rhs.tcmode) {
      return false;
    } else if (lhs.tcstatus != rhs.tcstatus) {
      return false;
    } else if (lhs.toff != rhs.toff) {
      return false;
    } else if (lhs.twsec != rhs.twsec) {
      return false;
    } else if (lhs.tfsec != rhs.tfsec) {
      return false;
    }
    return true;
  }

  bool operator!=(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs)
  {
    if (lhs.tcmode != rhs.tcmode) {
      return true;
    } else if (lhs.tcstatus != rhs.tcstatus) {
      return true;
    } else if (lhs.toff != rhs.toff) {
      return true;
    } else if (lhs.twsec != rhs.twsec) {
      return true;
    } else if (lhs.tfsec != rhs.tfsec) {
      return true;
    }
    return false;
  }

  bool operator<(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs)
  {
    if (lhs.twsec == rhs.twsec) {
      return lhs.tfsec < rhs.tfsec;
    } else {
      return lhs.twsec < rhs.twsec;
    }
  }

  bool operator<=(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs)
  {
    if (lhs.twsec == rhs.twsec) {
      return lhs.tfsec <= rhs.tfsec;
    } else {
      return lhs.twsec <= rhs.twsec;
    }
  }

  bool operator>(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs)
  {
    if (lhs.twsec == rhs.twsec) {
      return lhs.tfsec > rhs.tfsec;
    } else {
      return lhs.twsec > rhs.twsec;
    }
  }

  bool operator>=(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs)
  {
    if (lhs.twsec == rhs.twsec) {
      return lhs.tfsec >= rhs.tfsec;
    } else {
      return lhs.twsec >= rhs.twsec;
    }
  }

  std::ostream& operator<<(std::ostream& stream, const BULKIO::PrecisionUTCTime& utc)
  {
    struct tm time;
    time_t seconds = utc.twsec;
    gmtime_r(&seconds, &time);
    stream << (1900+time.tm_year) << ':';
    stream << std::setw(2) << std::setfill('0') << (time.tm_mon+1) << ':';
    stream << std::setw(2) << time.tm_mday << "::";
    stream << std::setw(2) << time.tm_hour << ":";
    stream << std::setw(2) << time.tm_min << ":";
    stream << std::setw(2) << time.tm_sec;
    int usec = round(utc.tfsec * 1000000.0);
    stream << "." << std::setw(6) << usec;
    return stream;
  }

}
