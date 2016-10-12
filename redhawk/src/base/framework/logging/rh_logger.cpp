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
#include <sys/time.h>
#include <algorithm>
#include <sstream>

// logging macros used by redhawk resources
#include <ossie/debug.h>

#ifdef HAVE_LOG4CXX
#include <log4cxx/logger.h>
#include <log4cxx/level.h>
#include <log4cxx/logstring.h>
#include <log4cxx/patternlayout.h>
#include <log4cxx/helpers/messagebuffer.h>
#include <fstream>
#endif 

// internal logging classes for std::out and log4cxx
#include "./rh_logger_p.h"

//
// deprecate this method... moving to ossie:logging  and rh_logger 
//
#ifdef  NO_RH_LOGGER

#include <ossie/logging/loghelpers.h>
void ossie::configureLogging(const char* logcfgUri, int defaultLevel)
{
  ossie::logging::Configure(logcfgUri, defaultLevel );
}

unsigned int LoggingConfigurator::ossieDebugLevel = 0;

#endif


#ifdef LOCAL_DEBUG_ON
#define STDOUT_DEBUG(x)    std::cout << x << std::endl
#else
#define STDOUT_DEBUG(x)    
#endif


bool caseInsCompare ( const char a, const char b ) {
  return toupper(a) == toupper(b);
}

bool strCaseInsCompare( const std::string &s1, const std::string &s2 ) {
  return ( s1.size() == s2.size() &&
	   std::equal( s1.begin(), s1.end(), s2.end(), caseInsCompare ) );
}


namespace rh_logger {


  struct null_deleter
  {
    void operator()(void const *) const
    {
    }
  };


  /*
   */
  namespace spi
  {

    const char* const LocationInfo::NA = "?";
    const char* const LocationInfo::NA_METHOD = "?::?";

    /*
     *   When location information is not available the constant
     * <code>NA</code> is returned. Current value of this string constant is <b>?</b>.
     */

    const LocationInfo& LocationInfo::getLocationUnavailable() {
      static const LocationInfo unavailable;
      return unavailable;
    }

    LocationInfo::LocationInfo( const char * const fileName1,
				const char * const methodName1,
				int lineNumber1 )
      :  lineNumber( lineNumber1 ),
	 fileName( fileName1 ),
	 methodName( methodName1 ) {
    }

    LocationInfo::LocationInfo()
      : lineNumber( -1 ),
	fileName(LocationInfo::NA),
	methodName(LocationInfo::NA_METHOD) {
    }

    LocationInfo::LocationInfo( const LocationInfo & src )
      :  lineNumber( src.lineNumber ),
	 fileName( src.fileName ),
	 methodName( src.methodName ) {
    }

    LocationInfo & LocationInfo::operator = ( const LocationInfo & src )
    {
      fileName = src.fileName;
      methodName = src.methodName;
      lineNumber = src.lineNumber;
      return * this;
    }

    void LocationInfo::clear() {
      fileName = NA;
      methodName = NA_METHOD;
      lineNumber = -1;
    }

    const char * LocationInfo::getFileName() const
    {
      return fileName;
    }

    int LocationInfo::getLineNumber() const
    {
      return lineNumber;
    }

    const std::string LocationInfo::getMethodName() const
    {
      std::string tmp(methodName);
      size_t colonPos = tmp.find("::");
      if (colonPos != std::string::npos) {
	tmp.erase(0, colonPos + 2);
      } else {
	size_t spacePos = tmp.find(' ');
	if (spacePos != std::string::npos) {
	  tmp.erase(0, spacePos + 1);
	}
      }
      size_t parenPos = tmp.find('(');
      if (parenPos != std::string::npos) {
	tmp.erase(parenPos);
      }
      return tmp;
    }


    const std::string LocationInfo::getClassName() const {
      std::string tmp(methodName);
      size_t colonPos = tmp.find("::");
      if (colonPos != std::string::npos) {
	tmp.erase(colonPos);
	size_t spacePos = tmp.find_last_of(' ');
	if (spacePos != std::string::npos) {
	  tmp.erase(0, spacePos + 1);
	}
	return tmp;
      }
      tmp.erase(0, tmp.length() );
      return tmp;
    }
  }



  Appender::Appender( const std::string &inname ):
    name(inname)
  {}

  // used by getEffectiveLevel calls so we have an object to return...
  static LevelPtr  _localRet;

  Level::Level( int val, const std::string &name ): 
    level(val),
      name(name)
      {};

  Level::Level( int val, const char  *name ): 
    level(val),
      name(name)
      {};

  //
  // static class objects for changing log level, needed to use shared_ptr
  // because the static object constructors would not be initialized when
  // the library would load
  // 
  //
  LevelPtr Level::_Off;
  LevelPtr Level::_Fatal;
  LevelPtr Level::_Error;
  LevelPtr Level::_Warn;
  LevelPtr Level::_Info;
  LevelPtr Level::_Debug;
  LevelPtr Level::_Trace;
  LevelPtr Level::_All;

  LevelPtr Level::getOff() {
    if ( !_Off ) {
      _Off = LevelPtr( new Level(OFF_INT, "OFF" ) );
    }
    return _Off;
  }

  LevelPtr Level::getFatal() { 
    if ( !_Fatal ) {
      _Fatal = LevelPtr( new Level(FATAL_INT, "FATAL" ) );
    }
    return _Fatal;
  }

  LevelPtr Level::getError() { 
    if ( !_Error ) {
      _Error = LevelPtr( new Level(ERROR_INT, "ERROR" ) );
    }
    return _Error;
  }

  LevelPtr Level::getWarn() { 
    if ( !_Warn ) {
      _Warn = LevelPtr( new Level(WARN_INT, "WARN" ) );
    }
    return _Warn;
  }

  LevelPtr Level::getInfo() { 
    if ( !_Info ) {
      _Info = LevelPtr( new Level(INFO_INT, "INFO" ) );
    }
    return _Info;
  }

  LevelPtr Level::getDebug() { 
    if ( !_Debug ) {
      _Debug = LevelPtr( new Level(DEBUG_INT, "DEBUG" ) );
    }
    return _Debug;
  }

  LevelPtr Level::getTrace() { 
    if ( !_Trace ) {
      _Trace = LevelPtr( new Level(TRACE_INT, "TRACE" ) );
    }
    return _Trace;
  }

  LevelPtr Level::getAll() { 
    if ( !_All ) {
      _All = LevelPtr( new Level(ALL_INT, "ALL" ) );
    }
    return _All;
  }

  LevelPtr Level::toLevel(int val )
  {
    LevelPtr defaultLevel = getInfo();
    switch(val)
      {
      case ALL_INT: return getAll();
      case DEBUG_INT: return getDebug();
      case TRACE_INT: return getTrace();
      case INFO_INT: return getInfo();
      case WARN_INT: return getWarn();
      case ERROR_INT: return getError();
      case FATAL_INT: return getFatal();
      case OFF_INT: return getOff();
      default: return defaultLevel;
      }
  }

  LevelPtr Level::toLevel( const std::string &val )
  {
    LevelPtr defaultLevel = getInfo();
    if ( val == "OFF" ) return getOff();
    if ( val == "FATAL" ) return getFatal();
    if ( val == "ERROR" ) return getError();
    if ( val == "WARN" ) return getWarn();
    if ( val == "DEBUG" ) return getDebug();
    if ( val == "TRACE" ) return getTrace();
    if ( val == "ALL" ) return getAll();
    return defaultLevel;
  };

  bool Level::equals(const LevelPtr& level1) const
  {
    return (this->level == level1->level);
  }

  bool Level::isGreaterOrEqual(const LevelPtr& level1) const
  {
    return this->level >= level1->level;
  }


  //
  //
  //  Logger abstraction 
  //
  //


  Logger::Logger( const char *name ) :
    name(name),
    level(),
    log_records(30)
  {
  }

  Logger::Logger( const std::string &name ) :
    name(name),
    level(),
    log_records(30)
  {
  }

  Logger::~Logger() {
    STDOUT_DEBUG( "RH_LOGGER DTOR name <" << name << "> ");
  }
    
  //
  // static root logger object
  //
  LoggerPtr Logger::_rootLogger;

  //
  // save off the resource logger name when the resource gets it's initial logger
  //
  std::string _rsc_logger_name;


  //
  //  Return the root logger object as a shared pointer
  //
  LoggerPtr Logger::getRootLogger() {
    STDOUT_DEBUG( "RH_LOGGER getRootLogger  BEGIN ");
    if (!_rootLogger ){
#ifdef HAVE_LOG4CXX
      _rootLogger = L4Logger::getRootLogger();
#else
      _rootLogger = StdOutLogger::getRootLogger();
#endif
    }
    STDOUT_DEBUG( "RH_LOGGER getRootLogger  END ");
    return _rootLogger;
  }


  LoggerPtr Logger::getResourceLogger( const std::string &name ) {
      _rsc_logger_name = name;
      return getLogger( name );
  }

  const std::string &Logger::getResourceLoggerName() {
      return _rsc_logger_name;
  }

  LoggerPtr Logger::getLogger( const std::string &name ) {
    STDOUT_DEBUG( "RH_LOGGER getLogger  BEGIN ");
    LoggerPtr ret;
    if ( name != "" ) {
#ifdef HAVE_LOG4CXX
      ret = L4Logger::getLogger( name );
#else
      ret = StdOutLogger::getLogger( name );
#endif
      if ( ret->getLevel() ) {
	STDOUT_DEBUG( "RH_LOGGER getLogger name/level :" << ret->getName() <<  "/" << ret->getLevel()->toString() );
      }
      else{
	STDOUT_DEBUG( "RH_LOGGER getLogger name/level :" << ret->getName() <<  "/UNSET");
      }
    }
    else  {
      ret = getRootLogger();
    }
    STDOUT_DEBUG( "RH_LOGGER getLogger  END ");
    return ret;
  }


  LoggerPtr Logger::getLogger( const char *name ) {
    std::string n(name);
    return getLogger(n);
  }

  void Logger::setLevel ( const LevelPtr &newLevel ) {
    STDOUT_DEBUG( " RH LOGGER  setLevel - logger: " << name );    
    if ( newLevel ) {
      STDOUT_DEBUG( " RH LOGGER  setLevel - level: " << newLevel->toString() );
    }
    else {
      STDOUT_DEBUG( " RH LOGGER  setLevel - level: UNSET" );
    }
    level=newLevel;
  }

  LevelPtr Logger::getLevel () const {
    STDOUT_DEBUG( " RH LOGGER  getLevel - logger: " << name );
    if ( !level ) return getRootLogger()->getLevel();
    return level;
  }

  std::string Logger::getName() const {
    STDOUT_DEBUG( " RH LOGGER  getName - logger: " << name );
    return name;
  }

  void Logger::getName( std::string &ret ) const {
    STDOUT_DEBUG( " RH LOGGER  getName - logger: " << name );
    ret = name;
  }

  void Logger::fatal( const std::string &msg )  {
    LevelPtr lvl = getLevel();
    if ( Level::FATAL_INT >= lvl->toInt() ) {
      handleLogEvent( Level::getFatal(), msg );
    }
  }

  void Logger::error( const std::string &msg )  {
    LevelPtr lvl = getLevel();
    if ( Level::ERROR_INT >= lvl->toInt() ) {
      handleLogEvent( Level::getError(), msg );
    }
  }

  void Logger::warn( const std::string &msg )  {
    LevelPtr lvl = getLevel();
    if ( Level::WARN_INT >= lvl->toInt() ) {
      handleLogEvent( Level::getWarn(),msg );
    }
  }

  void Logger::info( const std::string &msg )  {
    LevelPtr lvl = getLevel();
    if ( Level::INFO_INT >= lvl->toInt() ) {
      handleLogEvent( Level::getInfo(), msg );
    }
  }

  void Logger::debug( const std::string &msg )  {
    LevelPtr lvl = getLevel();
    if ( Level::DEBUG_INT >= lvl->toInt() ) {
      handleLogEvent( Level::getDebug(), msg );
    }
  }

  void Logger::trace( const std::string &msg )  {
    LevelPtr lvl = getLevel();
    if ( Level::TRACE_INT >= lvl->toInt() ) {
      handleLogEvent( Level::getTrace(), msg );
    }
  }

  bool Logger::isFatalEnabled() const
  {
    STDOUT_DEBUG( "RH_LOGGER isFatalEnabled ");
    return getEffectiveLevel()->toInt() <= Level::FATAL_INT;
  }

  bool Logger::isErrorEnabled() const
  {
    STDOUT_DEBUG( "RH_LOGGER isErrorEnabled ");
    return getEffectiveLevel()->toInt() <= Level::ERROR_INT;
  }

  bool Logger::isWarnEnabled() const
  {
    STDOUT_DEBUG( "RH_LOGGER isWarnEnabled ");
    return getEffectiveLevel()->toInt() <= Level::WARN_INT;
  }

  bool Logger::isInfoEnabled() const
  {
    STDOUT_DEBUG( "RH_LOGGER isInfoEnabled ");
    return getEffectiveLevel()->toInt() <= Level::INFO_INT;
  }

  bool Logger::isDebugEnabled() const
  {
    STDOUT_DEBUG( "RH_LOGGER isDebugEnabled ");
    return getEffectiveLevel()->toInt() <= Level::DEBUG_INT;
  }

  bool Logger::isTraceEnabled() const
  {
    STDOUT_DEBUG( "RH_LOGGER isTraceEnabled ");
    return getEffectiveLevel()->toInt() <= Level::TRACE_INT;
  }

  const LevelPtr& Logger::getEffectiveLevel() const
  {
    STDOUT_DEBUG( "RH_LOGGER getEffectiveLevel ");
    return level;
  }

  /*
  void Logger::handleLogEvent( const LevelPtr &level, const std::string &msg )  {
    STDOUT_DEBUG( " RH LOGGER  handleLogEvent " << msg );
    if ( getLevel() ) {
      STDOUT_DEBUG( " RH_LOGGER  name:" << getName() << " level:" << getLevel()->toString() );
    }
    else {
      STDOUT_DEBUG( " RH_LOGGER  name:" << getName() << " level:UNSET" );
    }
    appendLogRecord( level, msg );
  }
  */


  AppenderPtr Logger::getAppender( const std::string &name ) {
    AppenderPtr ret;
    return ret;
  }

  void  Logger::addAppender( const AppenderPtr &newAppender ) {
  }


  Logger::LogRecords Logger::getLogRecords() {
    return  log_records;
  }


  //  append log record to circular buffer
  void Logger::appendLogRecord( const LevelPtr &level, const std::string &msg)  {
    struct timeval tmp_time;
    struct timezone tmp_tz;
    uint64_t ts;
    gettimeofday(&tmp_time, &tmp_tz);
    ts = tmp_time.tv_sec;
    boost::mutex::scoped_lock lock(log_mutex);
    log_records.push_back( LogRecord( name, level, ts, msg) );
    return;
  }

  void Logger::appendLogRecord( const LogRecord &rec)  {
    boost::mutex::scoped_lock lock(log_mutex);
    log_records.push_back(rec);
    return; 
  }

  void Logger::setLogRecordLimit( size_t newSize ) {
      boost::mutex::scoped_lock lock(log_mutex);
      log_records.set_capacity(newSize);
      return;
  }
  
  size_t  Logger::getLogRecordLimit() {
      return log_records.size();
  }  


  //
  //
  //  StdOut Logger
  //
  //

  StdOutLogger::StdOutLoggerPtr  StdOutLogger::_rootLogger;


  LoggerPtr StdOutLogger::getRootLogger() {
    STDOUT_DEBUG( " StdOutLogger  getRootLogger BEGIN ");
    if ( !_rootLogger ) {
      _rootLogger = StdOutLoggerPtr( new StdOutLogger("") );
      _rootLogger->setLevel( Level::getInfo() );
    }
    STDOUT_DEBUG( " StdOutLogger  getRootLogger END");
    return _rootLogger;
  }


  LoggerPtr StdOutLogger::getLogger( const std::string &name ) {

    STDOUT_DEBUG(  " StdOutLogger::getLogger:  name: " << name  );
    LoggerPtr ret;
    if ( name != "" ) {
      ret  = LoggerPtr(new StdOutLogger( name ));
      ret->setLevel( rh_logger::Logger::getRootLogger()->getLevel() );
      if ( ret->getLevel() ) {
	STDOUT_DEBUG(  " StdOutLogger::getLogger: name /level " << ret->getName()  <<  "/" << ret->getLevel()->toString() );
      }
      else {
	STDOUT_DEBUG(  " StdOutLogger::getLogger: name /level " << ret->getName()  <<  "/UNSET" );
      }
    }
    else {
      ret = rh_logger::Logger::getRootLogger();
    }

    STDOUT_DEBUG(  " StdOutLogger::getLogger:  END name: " << name  );
    return ret;
  }

  LoggerPtr StdOutLogger::getLogger( const char *name ) {
    std::string n(name);
    return StdOutLogger::getLogger( n );
  }

  
  StdOutLogger::StdOutLogger( const std::string &name ) : 
    Logger(name),
    _os(std::cout)
  {
  }

  StdOutLogger::StdOutLogger( const char *name ) :
    Logger(name),
    _os(std::cout)
  {
  }

  void StdOutLogger::setLevel ( const LevelPtr &newLevel ) {
    if ( newLevel ) {
        STDOUT_DEBUG( "--->> StdOutLogger::setLevel level:" <<  newLevel->getName() );
    }
    Logger::setLevel(newLevel);
    // affect root logger since everyone shares the same level
    rh_logger::Logger::setLevel(newLevel);
  }


  const LevelPtr& StdOutLogger::getEffectiveLevel() const
  {
    //
    // prior method used ossieDebugLevel for comparision of debug statements
    // so that is our root loggers state
    //
    STDOUT_DEBUG( "--->> StdOutLogger::getEffectiveLevel level:" <<  _rootLogger->level->getName() );
    return _rootLogger->level;
  }

  void StdOutLogger::handleLogEvent( const LevelPtr &level, const std::string &msg )  {
    STDOUT_DEBUG( "--->> StdOutLogger::handleLogEvent  name/level:" <<  name << "/" << level->getName() << " msg:" << msg );
    std::ostringstream _msg;						\
    _msg << level->getName() << ":" << getName() << " - " << msg << std::endl; \
    _os << _msg.str();
    appendLogRecord( level, msg );
  }

  void StdOutLogger::handleLogEvent( const LevelPtr &level, const std::string &msg, const spi::LocationInfo &loc )  {
    STDOUT_DEBUG( "--->> StdOutLogger::handleLogEvent  name/level:" <<  name << "/" << level->getName() << " msg:" << msg );
    std::ostringstream _msg;					       
#if ENABLE_TRACE    
    _msg << level->getName() << ":" << getName() << " - " << msg << " [" << loc.getFileName() << ":" << loc.getLineNumber() << "]" << std::endl;     _msg << level->getName() << ":" << getName() << " - " << msg << " [" << loc.getFileName() << ":" << loc.getLineNumber() << "]" << std::endl; 
#else
    _msg << level->getName() << ":" << getName() << " - " << msg << std::endl; 
#endif
    _os << _msg.str();
    appendLogRecord( level, msg );
  }

  



#ifdef HAVE_LOG4CXX


  log4cxx::LevelPtr ConvertRHLevelToLog4 ( rh_logger::LevelPtr rh_level ) {
    if (rh_level == rh_logger::Level::getOff() )   return log4cxx::Level::getOff();
    if (rh_level == rh_logger::Level::getFatal() ) return log4cxx::Level::getFatal();
    if (rh_level == rh_logger::Level::getError() ) return log4cxx::Level::getError();
    if (rh_level == rh_logger::Level::getWarn() )  return log4cxx::Level::getWarn();
    if (rh_level == rh_logger::Level::getInfo() )  return log4cxx::Level::getInfo();
    if (rh_level == rh_logger::Level::getDebug() ) return log4cxx::Level::getDebug();
    if (rh_level == rh_logger::Level::getTrace() ) return log4cxx::Level::getTrace();
    if (rh_level == rh_logger::Level::getAll() )   return log4cxx::Level::getAll();
      return log4cxx::Level::getInfo();
    };

  rh_logger::LevelPtr ConvertLog4ToRHLevel ( log4cxx::LevelPtr l4_level ) {
    if (l4_level == log4cxx::Level::getOff() )   return rh_logger::Level::getOff();
    if (l4_level == log4cxx::Level::getFatal() ) return rh_logger::Level::getFatal();
    if (l4_level == log4cxx::Level::getError() ) return rh_logger::Level::getError();
    if (l4_level == log4cxx::Level::getWarn() )  return rh_logger::Level::getWarn();
    if (l4_level == log4cxx::Level::getInfo() )  return rh_logger::Level::getInfo();
    if (l4_level == log4cxx::Level::getDebug() ) return rh_logger::Level::getDebug();
    if (l4_level == log4cxx::Level::getTrace() ) return rh_logger::Level::getTrace();
    if (l4_level == log4cxx::Level::getAll() )   return rh_logger::Level::getAll();
      return rh_logger::Level::getInfo();
    };

  L4Logger::L4LoggerPtr L4Logger::_rootLogger;

  //
  //  Return the root logger object as a shared pointer
  //
  LoggerPtr L4Logger::getRootLogger() {
    STDOUT_DEBUG(  " L4Logger  getRootLogger:  BEGIN ");
    if ( !_rootLogger ) {
      _rootLogger = L4LoggerPtr( new L4Logger("") );
      _rootLogger->l4logger = log4cxx::Logger::getRootLogger();
     LevelPtr l= _rootLogger->getLevel();
    }
    STDOUT_DEBUG(  " L4Logger  getRootLogger:  END ");
    return _rootLogger;
  }

  LoggerPtr L4Logger::getLogger( const std::string &name ) {
    STDOUT_DEBUG(  " L4Logger::getLogger:  BEGIN name: " << name  );

    LoggerPtr ret;
    if ( name != "" ) {
      ret = LoggerPtr( new L4Logger( name ) );
      if ( ret->getLevel() )  {
	STDOUT_DEBUG(  " L4Logger::getLogger: name /level " << ret->getName()  <<  "/" << ret->getLevel()->toString() );
      }
      else {
	STDOUT_DEBUG(  " L4Logger::getLogger: name /level " << ret->getName()  <<  "/UNSET" );
      }
    }
    else {
      ret = getRootLogger();
    }

    STDOUT_DEBUG(  " L4Logger::getLogger:  END name: " << name  );
    return ret;
  }

  LoggerPtr L4Logger::getLogger( const char *name ) {
    std::string n(name);
    return L4Logger::getLogger( n );
  }

  L4Logger::L4Logger( const std::string &name ) : 
    Logger(name),
    l4logger()
  {
    l4logger = log4cxx::Logger::getLogger(name);
  }

  L4Logger::L4Logger( const char *name ) :
    Logger(name),
    l4logger()
  {
    l4logger = log4cxx::Logger::getLogger(name);
  }

  void L4Logger::setLevel ( const rh_logger::LevelPtr &newLevel ) {
    STDOUT_DEBUG(  " L4Logger::setLevel:  logger: " << name  );
    if ( newLevel ) {
      STDOUT_DEBUG(  " L4Logger::setLevel:  level: " << newLevel->toString()  );
    }
    else {
      STDOUT_DEBUG(  " L4Logger::setLevel:  level: UNSET");
    }
    level=newLevel;
    if ( l4logger ) {
      l4logger->setLevel(ConvertRHLevelToLog4(newLevel));
    }
  }


  rh_logger::LevelPtr L4Logger::getLevel ( ) const {
    STDOUT_DEBUG(  " L4Logger::getLevel:  BEGIN logger: " << name  );
    if ( l4logger ) {
      log4cxx::LevelPtr l4l = l4logger->getLevel();
      if ( l4l )  {
	STDOUT_DEBUG(  " L4Logger::getLevel:  l4level: " << l4l->toString()  ); 
        return ConvertLog4ToRHLevel( l4l );
      }
    }

    STDOUT_DEBUG(  " L4Logger::getLevel:  level ptr: " << level.get()  );
    STDOUT_DEBUG(  " L4Logger::getLevel:  END logger: " << name  );
    return rh_logger::Logger::getLevel();
  }


  bool L4Logger::isFatalEnabled() const
  {
    STDOUT_DEBUG( "--->> L4Logger::isFataEnabled" );
    return l4logger->isFatalEnabled();
  }

  bool L4Logger::isErrorEnabled() const
  {
    STDOUT_DEBUG( "--->> L4Logger::isErrorEnabled" );
    return l4logger->isErrorEnabled();
  }

  bool L4Logger::isWarnEnabled() const
  {
    STDOUT_DEBUG( "--->> L4Logger::isWarnEnabled" );
    return l4logger->isWarnEnabled();
  }

  bool L4Logger::isInfoEnabled() const
  {
    STDOUT_DEBUG( "--->> L4Logger::isInfoEnabled" );
    return l4logger->isInfoEnabled();
  }

  bool L4Logger::isDebugEnabled() const
  {
    STDOUT_DEBUG( "--->> L4Logger::isDebugEnabled" );
    return l4logger->isDebugEnabled();
  }

  bool L4Logger::isTraceEnabled() const
  {
    STDOUT_DEBUG( "--->> L4Logger::isTraceEnabled" );
    return l4logger->isTraceEnabled();
  }

  void L4Logger::handleLogEvent( const LevelPtr &level, const std::string &msg )  {
    STDOUT_DEBUG( "--->> L4Logger::handleLogEvent  name/level:" <<  name << "/" << level->getName() << " msg:" << msg );
    //
    // translate rh level to log4level.... 
    //   
    appendLogRecord( level, msg );

    //
    // push log message to log4cxx logger...need to call basic log methods (info, debug, etc)
    // since the underlying 
    ::log4cxx::helpers::MessageBuffer oss_;
    l4logger->forcedLog( ConvertRHLevelToLog4(level), oss_.str(oss_ << msg) );
  }

  void L4Logger::handleLogEvent( const LevelPtr &level, const std::string &msg, const spi::LocationInfo &loc )  {
    STDOUT_DEBUG( "--->> L4Logger::handleLogEvent  name/level:" <<  name << "/" << level->getName() << " msg:" << msg );
    //
    // translate rh level to log4level.... 
    //   
    appendLogRecord( level, msg );

    log4cxx::spi::LocationInfo l4loc( loc.getFileName(), loc.getMethodName().c_str(), loc.getLineNumber() );
    //
    // push log message to log4cxx logger...need to call basic log methods (info, debug, etc)
    // since the underlying 
    ::log4cxx::helpers::MessageBuffer oss_;
    l4logger->forcedLog( ConvertRHLevelToLog4(level), oss_.str(oss_ << msg), l4loc );
  }

  const LevelPtr& L4Logger::getEffectiveLevel() const
  {
    STDOUT_DEBUG( "--->> L4Logger::getEffectiveLevel logger:" <<  name );
    if ( l4logger ) {
      _localRet = ConvertLog4ToRHLevel( l4logger->getEffectiveLevel() );
      return _localRet;
    }
    else {
      return level;
    }
  }


#endif


};

