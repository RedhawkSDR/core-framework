#ifndef _LOGTESTDEBUG_H
#define _LOGTESTDEBUG_H

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
