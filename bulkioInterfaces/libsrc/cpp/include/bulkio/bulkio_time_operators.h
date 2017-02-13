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

#ifndef __bulkio_time_operators_h
#define __bulkio_time_operators_h

#include <ostream>

#include <BULKIO/bulkioDataTypes.h>

namespace BULKIO {

  BULKIO::PrecisionUTCTime operator+(const BULKIO::PrecisionUTCTime& lhs, double seconds);

  BULKIO::PrecisionUTCTime& operator+=(BULKIO::PrecisionUTCTime& lhs, double seconds);

  double operator-(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs);
  BULKIO::PrecisionUTCTime operator-(const BULKIO::PrecisionUTCTime& lhs, double seconds);

  BULKIO::PrecisionUTCTime& operator-=(BULKIO::PrecisionUTCTime& lhs, double seconds);

  bool operator==(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs);
  bool operator!=(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs);
  bool operator<(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs);
  bool operator<=(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs);
  bool operator>(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs);
  bool operator>=(const BULKIO::PrecisionUTCTime& lhs, const BULKIO::PrecisionUTCTime& rhs);

  std::ostream& operator<<(std::ostream&, const BULKIO::PrecisionUTCTime&);
}

#endif
