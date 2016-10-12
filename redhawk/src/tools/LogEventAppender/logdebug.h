/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef _LL_DEBUG_H
#define _LL_DEBUG_H

//#define DEBUG_ON
//#define STDOUT_ON

#ifdef DEBUG_ON

// set -DLIB_ON when compiling library for debug mode...do no use log4 logging....
#ifdef STDOUT_ON
#include <iostream>
extern std::ostream   & _logger_;
#define LTRACE( exp )   _logger_ << "TRACE " << exp << std::endl
#define LDEBUG( exp )   _logger_ << "DEBUG " << exp << std::endl
#define LINFO( exp )    _logger_ << "INFO  " << exp << std::endl
#define LWARN( exp )    _logger_ << "WARN  " << exp << std::endl
#define LERROR( exp )   _logger_ << "ERROR " << exp << std::endl
#define LFATAL( exp )   _logger_ << "FATAL " << exp << std::endl

#define LNTRACE( lname, exp )   _logger_ << "TRACE " << lname << " " << exp << std::endl
#define LNDEBUG( lname, exp )   _logger_ << "DEBUG " << lname << " " << exp << std::endl
#define LNINFO( lname, exp )    _logger_ << "INFO  " << lname << " " << exp << std::endl
#define LNWARN( lname, exp )    _logger_ << "WARN  " << lname << " " << exp << std::endl
#define LNERROR( lname, exp )   _logger_ << "ERROR " << lname << " " << exp << std::endl
#define LNFATAL( lname, exp )   _logger_ << "FATAL " << lname << " " << exp << std::endl

#define LOGGER_CFG( name )               \
  std::ostream  &_logger_  = std::cout;	

#define LOGGER_END( name )              

#else

// set when compiling library for test mode... OK to use log4 logging....
#include <log4cxx/logger.h>
#include <log4cxx/logmanager.h>
extern  log4cxx::LoggerPtr         _logger_;
#define LTRACE( expression ) LOG4CXX_TRACE( _logger_, expression )
#define LDEBUG( expression ) LOG4CXX_DEBUG( _logger_ , expression )
#define LINFO( expression )  LOG4CXX_INFO( _logger_, expression )
#define LWARN( expression )  LOG4CXX_WARN( _logger_, expression )
#define LERROR( expression ) LOG4CXX_ERROR( _logger_, expression )
#define LFATAL( expression ) LOG4CXX_FATAL( _logger_, expression )

#define LNTRACE( lname, expression ) LOG4CXX_TRACE( log4cxx::Logger::getLogger(lname), expression )
#define LNDEBUG( lname, expression ) LOG4CXX_DEBUG( log4cxx::Logger::getLogger(lname), expression )
#define LNINFO( lname, expression )  LOG4CXX_INFO( log4cxx::Logger::getLogger(lname), expression )
#define LNWARN( lname, expression )  LOG4CXX_WARN( log4cxx::Logger::getLogger(lname), expression )
#define LNERROR( lname, expression ) LOG4CXX_ERROR( log4cxx::Logger::getLogger(lname), expression )
#define LNFATAL( lname, expression ) LOG4CXX_FATAL( log4cxx::Logger::getLogger(lname), expression )

#define LOGGER_CFG( name )               \
  log4cxx::LoggerPtr _logger_  = log4cxx::Logger::getLogger(name);

#define LOGGER_END( name )               \
    log4cxx::LogManager::shutdown();

#endif

#else

typedef void                    LOGGER;
// turn off debug code
#define LTRACE( exp ) 
#define LDEBUG( exp ) 
#define LINFO( exp )  
#define LWARN( exp )  
#define LERROR( exp ) 
#define LFATAL( exp ) 
#define LNTRACE( lname, exp ) 
#define LNDEBUG( lname, exp ) 
#define LNINFO( lname, exp )  
#define LNWARN( lname, exp )  
#define LNERROR( lname, exp ) 
#define LNFATAL( lname, exp ) 
#define LOGGER_CFG( name )              
#define LOGGER_END( name )              

#endif

#endif
