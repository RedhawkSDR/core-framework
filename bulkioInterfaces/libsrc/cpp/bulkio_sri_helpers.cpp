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
#include <ossie/prop_helpers.h>

#include "bulkio_base.h"
#include "bulkio_p.h"

namespace  bulkio {

namespace  sri {

bool DefaultComparator( const BULKIO::StreamSRI &SRI_1, const BULKIO::StreamSRI &SRI_2){
    if (SRI_1.hversion != SRI_2.hversion)
        return false;
    if (SRI_1.xstart != SRI_2.xstart)
        return false;
    if (SRI_1.xdelta != SRI_2.xdelta)
        return false;
    if (SRI_1.xunits != SRI_2.xunits)
        return false;
    if (SRI_1.subsize != SRI_2.subsize)
        return false;
    if (SRI_1.ystart != SRI_2.ystart)
        return false;
    if (SRI_1.ydelta != SRI_2.ydelta)
        return false;
    if (SRI_1.yunits != SRI_2.yunits)
        return false;
    if (SRI_1.mode != SRI_2.mode)
        return false;
    if (SRI_1.blocking != SRI_2.blocking)
        return false;
    if (strcmp(SRI_1.streamID, SRI_2.streamID) != 0)
        return false;
    if (!compareKeywords(SRI_1.keywords, SRI_2.keywords))
        return false;
    return true;
}

bool compareKeywords(const _CORBA_Unbounded_Sequence<CF::DataType>& lhs,
                     const _CORBA_Unbounded_Sequence<CF::DataType>& rhs)
{
    if (lhs.length() != rhs.length()) {
        return false;
    }

    std::string action = "eq";
    for (unsigned int index=0; index<lhs.length(); index++) {
        if (strcmp(lhs[index].id, rhs[index].id)) {
            return false;
        }
        if (!ossie::compare_anys(lhs[index].value, rhs[index].value, action)) {
            return false;
        }
    }

    return true;
}

  int compareFields(const BULKIO::StreamSRI& lhs, const BULKIO::StreamSRI& rhs)
  {
    int result = NONE;
    if (lhs.hversion != rhs.hversion) result |= HVERSION;
    if (lhs.xstart != rhs.xstart) result |= XSTART;
    if (lhs.xdelta != rhs.xdelta) result |= XDELTA;
    if (lhs.xunits != rhs.xunits) result |= XUNITS;
    if (lhs.subsize != rhs.subsize) result |= SUBSIZE;
    if (lhs.ystart != rhs.ystart) result |= YSTART;
    if (lhs.ydelta != rhs.ydelta) result |= YDELTA;
    if (lhs.yunits != rhs.yunits) result |= YUNITS;
    if (lhs.mode != rhs.mode) result |= MODE;
    if (lhs.blocking != rhs.blocking) result |= BLOCKING;
    if (strcmp(lhs.streamID, rhs.streamID)) result |= STREAMID;
    if (!compareKeywords(lhs.keywords, rhs.keywords)) result |= KEYWORDS;

    return result;
  }

  BULKIO::StreamSRI create( std::string sid, const double srate , const Int16 xunits, const bool blocking ) {

    BULKIO::StreamSRI sri;
    sri.hversion = 1;
    sri.xstart = 0.0;
    if ( srate <= 0.0 )
        sri.xdelta =  1.0;
    else
        sri.xdelta = 1/srate;
    sri.xunits = xunits;
    sri.subsize = 0;
    sri.ystart = 0.0;
    sri.ydelta = 0.0;
    sri.yunits = BULKIO::UNITS_NONE;
    sri.mode = 0;
    sri.blocking=blocking;
    sri.streamID = sid.c_str();
    return sri;

}


} // namespace sri


} // namespace bulkio

