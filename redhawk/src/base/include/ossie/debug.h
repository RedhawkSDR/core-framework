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


#ifndef DEBUG_H
#define DEBUG_H

#include<iostream>
#include<fstream>
#include<string>
#include<cstring>

//
//  Disable Redhawk Logger class
//
#ifdef  NO_RH_LOGGER

//
// include prior generation macros if Redhawk Logging is disabled
//

#include <ossie/debug_old.h>

#else    //  NO_RH_LOGGER NOT SET   (default)

//
//  Begin Macros to use RedHawk Logging Abstraction
//
#include <sstream>
#include "ossie/logging/rh_logger.h"


#define ENABLE_LOGGING \
    private: \
    static rh_logger::LoggerPtr __logger;

#define PREPARE_LOGGING(classname) \
    rh_logger::LoggerPtr classname::__logger(rh_logger::Logger::getLogger(#classname));


#define PREPARE_ALT_LOGGING(classname, loggername) \
    rh_logger::LoggerPtr classname::__logger(rh_logger::Logger::getLogger(#loggername));

#define CREATE_LOGGER(classname) \
    class classname \
    { \
        public: \
	static rh_logger::LoggerPtr __logger; \
    }; \
    rh_logger::LoggerPtr classname::__logger(rh_logger::Logger::getLogger(#classname));

#define _RH_LOG( level, logger, msg)	\
  if ( logger && logger->is##level##Enabled() ) {			\
    std::ostringstream _msg;						\
    _msg <<  msg;				          		\
    logger->handleLogEvent( rh_logger::Level::get##level(), _msg.str(), rh_logger::spi::LocationInfo(__FILE__,__PRETTY_FUNCTION__,__LINE__) ); \
  }


#define LOG_TRACE(classname, expression)  _RH_LOG( Trace,  classname::__logger, expression)
#define LOG_DEBUG(classname, expression)  _RH_LOG( Debug,  classname::__logger, expression)
#define LOG_INFO(classname, expression)   _RH_LOG( Info,   classname::__logger, expression)
#define LOG_WARN(classname, expression)   _RH_LOG( Warn,   classname::__logger, expression)
#define LOG_ERROR(classname, expression)  _RH_LOG( Error,   classname::__logger, expression)
#define LOG_FATAL(classname, expression)  _RH_LOG( Fatal,   classname::__logger, expression)

#define RH_TRACE( logger, expression )  _RH_LOG( Trace,  logger, expression)
#define RH_DEBUG( logger, expression )  _RH_LOG( Debug,  logger, expression)
#define RH_INFO( logger, expression )   _RH_LOG( Info,   logger, expression)
#define RH_WARN( logger, expression )   _RH_LOG( Warn,   logger, expression)
#define RH_ERROR( logger, expression )  _RH_LOG( Error,  logger, expression)
#define RH_FATAL( logger, expression )  _RH_LOG( Fatal,  logger, expression)


#ifdef HAVE_LOG4CXX
#if  defined(LOG4CXX_TRACE) || defined(LOG4CXX_DEBUG)  || defined(LOG4CXX_INFO)  || defined(LOG4CXX_WARN)  || defined(LOG4CXX_ERROR)  || defined(LOG4CXX_FATAL) 
#warning  "For non-ENABLE_LOGGING logger objects, use rh_logger::Logger::getLogger method and RH_XXXX log macros"
#endif 
#else
#define LOG4CXX_TRACE( logger, expression )  _RH_LOG( Trace,  logger, expression)
#define LOG4CXX_DEBUG( logger, expression )  _RH_LOG( Debug,  logger, expression)
#define LOG4CXX_INFO( logger, expression )   _RH_LOG( Info,   logger, expression)
#define LOG4CXX_WARN( logger, expression )   _RH_LOG( Warn,   logger, expression)
#define LOG4CXX_ERROR( logger, expression )  _RH_LOG( Error,  logger, expression)
#define LOG4CXX_FATAL( logger, expression )  _RH_LOG( Fatal,  logger, expression)
#endif


#endif    //  NO_RH_LOGGER NOT SET   (default)

/*
* Provide a standardized mechanism for  TRACING into and out of functions so
* they all look similar.  Unlike LOG_TRACE, these are only included
* in the code if DEBUG is defined
*/
#ifdef ENABLE_TRACE
#define TRACE_ENTER(classname) \
    LOG_TRACE(classname, "Entering " << #classname << "." << __PRETTY_FUNCTION__ << " [" << __FILE__ << ":" << __LINE__ << "]")

#define TRACE_EXIT(classname) \
    LOG_TRACE(classname, "Exiting " << #classname << "." << __PRETTY_FUNCTION__ << " [" << __FILE__ << ":" << __LINE__ << "]")

#else
#define TRACE_ENTER(classname)
#define TRACE_EXIT(classname)

#endif

/*
* Provide standardized exception handling for common calls.
*/
#define CATCH_LOG_EXCEPTION(classname, expression, levelname) \
    catch( std::exception& ex ) { \
        LOG_##levelname(classname, expression << "; std::exception info: " << ex.what()) \
    } \
    catch( CORBA::Exception& ex ) { \
        LOG_##levelname(classname, expression << "; CORBA::Exception name: " << ex._name()) \
    } \
    catch( ... ) { \
        LOG_##levelname(classname, expression << "; unknown exception") \
    }

#define CATCH_LOG_TRACE(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, TRACE)
#define CATCH_LOG_DEBUG(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, DEBUG)
#define CATCH_LOG_INFO(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, INFO)
#define CATCH_LOG_WARN(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, WARN)
#define CATCH_LOG_ERROR(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, ERROR)
#define CATCH_LOG_FATAL(classname, expression) CATCH_LOG_EXCEPTION(classname, expression, FATAL)

/*
* Provide standardized exception handling for catching and throwing a new exception.
*/
#define CATCH_THROW_LOG_EXCEPTION(classname, expression, levelname, newexception) \
    catch( std::exception& ex ) { \
        LOG_##levelname(classname, expression << "; std::exception info: " << ex.what()) \
        throw(newexception); \
    } \
    catch( CORBA::Exception& ex ) { \
        LOG_##levelname(classname, expression << "; CORBA::Exception name: " << ex._name()) \
        throw(newexception); \
    } \
    catch( ... ) { \
        LOG_##levelname(classname, expression << "; unknown exception") \
        throw(newexception); \
    }

#define CATCH_THROW_LOG_TRACE(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, TRACE, newexception)
#define CATCH_THROW_LOG_DEBUG(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, DEBUG, newexception)
#define CATCH_THROW_LOG_INFO(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, INFO, newexception)
#define CATCH_THROW_LOG_WARN(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, WARN, newexception)
#define CATCH_THROW_LOG_ERROR(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, ERROR, newexception)
#define CATCH_THROW_LOG_FATAL(classname, expression, newexception) CATCH_THROW_LOG_EXCEPTION(classname, expression, FATAL, newexception)

/*
* Provide standardized exception handling for catching and rethrowing.
*/
#define CATCH_RETHROW_LOG_EXCEPTION(classname, expression, levelname) \
    catch( std::exception& ex ) { \
        LOG_##levelname(classname, expression << "; std::exception info: " << ex.what()) \
        throw; \
    } \
    catch( CORBA::Exception& ex ) { \
        LOG_##levelname(classname, expression << "; CORBA::Exception name: " << ex._name()) \
        throw; \
    } \
    catch( ... ) { \
        LOG_##levelname(classname, expression << "; unknown exception") \
        throw; \
    }

#define CATCH_RETHROW_LOG_TRACE(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, TRACE)
#define CATCH_RETHROW_LOG_DEBUG(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, DEBUG)
#define CATCH_RETHROW_LOG_INFO(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, INFO)
#define CATCH_RETHROW_LOG_WARN(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, WARN)
#define CATCH_RETHROW_LOG_ERROR(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, ERROR)
#define CATCH_RETHROW_LOG_FATAL(classname, expression) CATCH_RETHROW_LOG_EXCEPTION(classname, expression, FATAL)


/*
 * Provide a backwards compatible macro.
 * THIS MACRO SHOULD BE AVOIDED FOR ALL NEW DEVELOPMENT
 */
#define DEBUG(level, classname, expression) LOG_DEBUG(classname, expression)


#endif
