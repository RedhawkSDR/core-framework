#include <iostream>
#include <sstream>

#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/logstring.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/rollingfileappender.h>
#include <log4cxx/logmanager.h>
#include <boost/filesystem.hpp>

int main( int argc, char ** argv ) {

  // Set up a simple configuration that logs on the console.
  log4cxx::PropertyConfigurator::configure("log4j.appender" );

  log4cxx::helpers::LogLog::setInternalDebugging(true);

  log4cxx::LoggerPtr logger = log4cxx::Logger::getRootLogger();

  int32_t interval=1000;
  int64_t cnt=10;
  int     pid = getpid();

  if ( argc > 1 ) cnt=strtoll( argv[1], NULL, 0);
  if ( argc > 2 ) interval=strtol( argv[2], NULL, 0);

  
  while ( cnt-- ) {
    LOG4CXX_INFO(logger, "test_log4j_props_appender  MSG 1 - root logger (" << pid << ")" );
    LOG4CXX_INFO(logger, "test_log4j_props_appender  MSG 2 - root logger (" << pid << ")" );
    LOG4CXX_INFO(logger, "test_log4j_props_appender  MSG 3 - root logger (" << pid << ")" );

    std::ostringstream os;
    os << "LOG-MultiProcRollingFileAppender.Sub" << getpid();
    log4cxx::LoggerPtr lp  = log4cxx::Logger::getLogger(os.str());
    LOG4CXX_INFO(lp, "test_log4j_props_appender  MSG 1 - SUB logger (" << pid << ")" );
    LOG4CXX_INFO(lp, "test_log4j_props_appender  MSG 2 - SUB logger (" << pid << ")" );
    LOG4CXX_INFO(lp, "test_log4j_props_appender  MSG 3 - SUB logger (" << pid << ")" );

    LOG4CXX_INFO(logger, "proc_log -END (" << pid << ")" );
    usleep(interval);
  }

  // closes appenders correctly...
  log4cxx::LogManager::shutdown();

  return(0);
  
}

