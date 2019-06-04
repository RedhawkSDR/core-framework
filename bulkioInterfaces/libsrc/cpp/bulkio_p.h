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

#ifndef __bulkio_p_h__
#define __bulkio_p_h__

#include <queue>
#include <list>
#include <boost/make_shared.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <ossie/prop_helpers.h>
#include <ossie/BULKIO/bio_runtimeStats.h>

#include "bulkio_base.h"
#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#ifdef LOGGING
#undef LOG_INFO
#undef LOG_ERROR
#undef LOG_WARN
#undef LOG_FATAL
#undef LOG_DEBUG
#undef LOG_TRACE
#undef TRACE_ENTER
#undef TRACE_EXIT

#define LOG_INFO(logger, expr )   if ( logger ) RH_INFO(logger, expr );
#define LOG_ERROR(logger, expr )  if ( logger ) RH_ERROR(logger, expr );
#define LOG_WARN(logger, expr )  if ( logger )  RH_WARN(logger, expr );
#define LOG_FATAL(logger, expr )  if ( logger ) RH_FATAL(logger, expr );
#define LOG_DEBUG(logger, expr )  if ( logger ) RH_DEBUG(logger, expr );
#define LOG_TRACE(logger, expr )  if ( logger ) RH_TRACE(logger, expr );

#ifdef TRACE_ENABLE
#define TRACE_ENTER(logger, method)						\
    LOG_TRACE(logger, "ENTER bulkio::" << method << " [" << __FILE__ << ":" << __LINE__ << "]")
#define TRACE_EXIT(logger, method)						\
    LOG_TRACE(logger, "EXIT bulkio::" << method << " [" << __FILE__ << ":" << __LINE__ << "]")
#else
#define TRACE_ENTER(logger, method )
#define TRACE_EXIT(logger, method )
#endif

#else
#define LOG_INFO(logger, expr )  
#define LOG_ERROR(logger, expr ) 
#define LOG_WARN(logger, expr )  
#define LOG_FATAL(logger, expr ) 
#define LOG_DEBUG(logger, expr ) 
#define LOG_TRACE(logger, expr ) 
#define TRACE_ENTER(logger, method )
#define TRACE_EXIT(logger, method )

#endif


namespace bulkio    {

  //
  // used for boost shared pointer instantion when user
  // supplied callback is provided
  //
  struct null_deleter
  {
    void operator()(void const *) const
    {
    }
  };

  namespace sri {

    // Compare SRI keyword lists.
    bool compareKeywords(const _CORBA_Unbounded_Sequence<CF::DataType>& lhs,
                         const _CORBA_Unbounded_Sequence<CF::DataType>& rhs);
  }  // namespace sri

}  // namespace bulkio

#define FOREACH_NUMERIC_PORT_TYPE(x) \
    x(BULKIO::dataChar);             \
    x(BULKIO::dataOctet);            \
    x(BULKIO::dataShort);            \
    x(BULKIO::dataUshort);           \
    x(BULKIO::dataLong);             \
    x(BULKIO::dataUlong);            \
    x(BULKIO::dataLongLong);         \
    x(BULKIO::dataUlongLong);        \
    x(BULKIO::dataFloat);            \
    x(BULKIO::dataDouble);

#define FOREACH_PORT_TYPE(x)     \
    FOREACH_NUMERIC_PORT_TYPE(x) \
    x(BULKIO::dataBit);          \
    x(BULKIO::dataFile);         \
    x(BULKIO::dataXML);

#endif  // __bulkio_p_h__

