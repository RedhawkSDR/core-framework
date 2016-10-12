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
#ifndef  RH_LOGGER_H
#define  RH_LOGGER_H

#include <values.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <boost/thread/mutex.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/circular_buffer.hpp>
#include <stdint.h>

//
// Redhawk Abstraction for logging, follows log4cxx logger/level pattern
//
// There are 2 main objects that are used 
//  Level    - mimics log4cxx directly
//  Logger   - mimics a subset of log4cxx, no support for Appenders and limited forcedLog methods
//
// This implemention allows the framework to encapsulate the underlying available logging 
// methods: std::cout or log4cxx, without having to recompile the components. This also
// simplifies the LOG_XXX macros that are defined in ossie/debug.h.
//
// The selection of the logging implementation is controlled with the following 
// compile time options, --disable-log4cxx and --enable-log4cxx,  when the framework is built.
//
// Using these compile time options, rh_logger.cpp will be customized to build the appropriate
// logger objects, and correctly manage the static logger factory methods: getLogger and getRootLogger.
//
//

namespace rh_logger {

  
  class Level;
  class Logger;
  class Appender;

  // return values for Appenders
  typedef boost::shared_ptr< Appender >   AppenderPtr;

  // return values for Level class
  typedef boost::shared_ptr< Level >   LevelPtr;

  // return values from Logger class
  typedef boost::shared_ptr< Logger >  LoggerPtr;

  typedef boost::mutex                 Mutex;

  /**
   */
  namespace spi
  {
      /**
       * This class represents the location of a logging statement.
       *
       */
      class LocationInfo
      {
      public:

        static const char * const NA;
        static const char * const NA_METHOD;

        static const LocationInfo& getLocationUnavailable();


        LocationInfo( const char * const fileName,
                      const char * const functionName,
                      int lineNumber);

        LocationInfo();

        LocationInfo( const LocationInfo & src );

        LocationInfo & operator = ( const LocationInfo & src );

        void clear();

        const std::string getClassName() const;

        const char * getFileName() const;

        int getLineNumber() const;

        const std::string getMethodName() const;

        private:

        int lineNumber;
        const char * fileName;
        const char * methodName;
      };
  };

  /**
   */
  class Appender {

  public:
    
    Appender( const std::string &name );

    Appender( const Appender &src ) {
      name=src.name;
    }

    virtual ~Appender(){};

  private:
    
    std::string name;

  };

  
  /**
   * Level
   *  
   * Mimics log4cxx logging levels and enumerations
   *
   */
  class Level 
  {

  public:

    enum {OFF_INT = INT_MAX, FATAL_INT = 50000, ERROR_INT = 40000, WARN_INT = 30000,
	  INFO_INT = 20000, DEBUG_INT = 10000, TRACE_INT = 5000, ALL_INT = INT_MIN};

    Level( const Level & src) {
      level = src.level;
       name = src.name;
    }

    virtual ~Level() {}

    static LevelPtr getOff();
    static LevelPtr getFatal();
    static LevelPtr getError();
    static LevelPtr getWarn();
    static LevelPtr getInfo();
    static LevelPtr getDebug();
    static LevelPtr getTrace();
    static LevelPtr getAll();

    static LevelPtr toLevel( int val );
    static LevelPtr toLevel( const std::string &arg );

    inline int          toInt() const { return level; };
    inline int          getLevel() const { return level; };
    inline std::string  getName()  const { return name; };
    inline std::string  toString() const { return name; };

    virtual bool equals(const LevelPtr& level) const;
    virtual bool isGreaterOrEqual(const LevelPtr& level) const;

    inline bool operator==(const Level& level1) const
    { return (this->level == level1.level); }
    
    inline bool operator!=(const Level& level1) const
    { return (this->level != level1.level); }
      
  private:

    static LevelPtr _Off;
    static LevelPtr _Fatal;
    static LevelPtr _Error;
    static LevelPtr _Warn;
    static LevelPtr _Info;
    static LevelPtr _Debug;
    static LevelPtr _Trace;
    static LevelPtr _All;

    Level() {};

    Level( int val, const std::string &name );

    Level( int val, const char *name );

    int level;
    std::string  name;
      
  };

  /**
   * Logger
   *
   * Encapsulates the underlying logging implementation. 
   * Allows for easy removal of the logging implementation by switching out a single
   * library, rather than recompiling the entire framework and all components.
  */
  class Logger {

  public:

    //
    // Logging record events
    //
    struct LogRecord {
      uint64_t      timeStamp;
      std::string   name;
      LevelPtr      level;
      std::string   msg;
      
      LogRecord() 
      {};

    LogRecord(  const std::string &inname, LevelPtr inlevel, uint64_t ts, const std::string &m ) : 
      timeStamp(ts), name(inname), level(inlevel), msg(m)
      {};

    LogRecord(  const char *inname, LevelPtr inlevel, uint64_t ts, const char *m ) : 
      timeStamp(ts), name(inname), level(inlevel), msg(m)
      {};

    };

    // DTOR
    virtual ~Logger();

    //
    // Queue to hold logging history to assist with CosLwLog interface
    //
    typedef boost::circular_buffer< LogRecord >  LogRecords;

    //
    // Return the root logger object
    //
    static LoggerPtr getRootLogger();

    //
    // Return a named logger. This just creates a new logger object and defaults
    // 
    //
    static LoggerPtr getLogger( const std::string &name );
    static LoggerPtr getLogger( const char *name );

    //
    // Set the logging level for this logger
    //
    virtual void setLevel ( const LevelPtr &newLevel );

    //
    // Return the logging level for this logger
    //
    virtual LevelPtr getLevel () const;

    //
    // Return the name of the logger
    //
    virtual std::string getName() const;
    virtual void  getName( std::string &oname ) const;

    //
    // Log a message to the logging output stream
    //
    virtual void   fatal( const std::string &msg );
    virtual void   error( const std::string &msg );
    virtual void   warn( const std::string &msg );
    virtual void   info( const std::string &msg );
    virtual void   debug( const std::string &msg );
    virtual void   trace( const std::string &msg );


    virtual bool isFatalEnabled() const;
    virtual bool isErrorEnabled() const;
    virtual bool isWarnEnabled() const;
    virtual bool isInfoEnabled() const;
    virtual bool isDebugEnabled() const;
    virtual bool isTraceEnabled() const;
      
    virtual const LevelPtr&  getEffectiveLevel() const;

    virtual void handleLogEvent( const LevelPtr &lvl, const std::string &msg ) =0;

    virtual void handleLogEvent( const LevelPtr &lvl, const std::string &msg, const spi::LocationInfo &location )=0;

    virtual AppenderPtr getAppender( const std::string &name );

    virtual void  addAppender( const AppenderPtr &newAppender );

    //
    // return a copy of the current logging records
    // 
    virtual LogRecords getLogRecords();

    //
    //  append log event records to the history queue
    //
    virtual void appendLogRecord( const LogRecord &rec);
    virtual void appendLogRecord( const LevelPtr &level, const std::string &msg);

    //
    //  Set the logging event history limit
    //
    virtual void  setLogRecordLimit( size_t newSize );

    //
    //  Get the logging event history limit
    //
    virtual size_t  getLogRecordLimit();
    
  protected:

    Logger( const char *name );
    Logger( const std::string &name );

    std::string    name;
    LevelPtr       level;
    LogRecords     log_records;
    Mutex          log_mutex;   
    
  
  private:

    //
    // root most logger object
    //
    static LoggerPtr  _rootLogger;
    
  };


};  // end of rh_logger namespace

//
// common LOGGER definition for framework base classes
//
typedef rh_logger::LoggerPtr               LOGGER;

//
// Alias log4cxx namespace when library is disabled, (ossie::logging, and others)
//
#ifndef HAVE_LOG4CXX
namespace  log4cxx = rh_logger;
#endif



#endif   // RH_LOGGER_H
