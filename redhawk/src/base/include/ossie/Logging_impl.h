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
#ifndef LOGGING_IMPL_H
#define LOGGING_IMPL_H
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>
#include "ossie/CF/LogInterfaces.h"
#include "ossie/logging/rh_logger.h"
#include "ossie/logging/loghelpers.h"
#include "ossie/EventChannelSupport.h"
#include "ossie/Autocomplete.h"

class Logging_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
    : public virtual POA_CF::Logging
#endif
{
 public:

  Logging_impl ();
    
  virtual ~Logging_impl() {};

  // override this method to return the current logging configuration information
  char *        getLogConfig ();

  // override this method to accept logging configuration information as a string
  void         setLogConfig( const char *config_contents );

  // override this method to accept a string object that contains a URL. 
  //  the contents of the URL will be used as the logging configuration information
  void         setLogConfigURL ( const char *config_url );

  // returns the current logging level state
  CF::LogLevel log_level();

  // sets the current logging level  for the resource, if LogLevelCallback is set
  void         log_level( const CF::LogLevel newLevel );

  // override this method to accept logging configuration information as a string
  void         setLogLevel( const char *logger_id, const CF::LogLevel newLevel ) throw (CF::UnknownIdentifier);

  // returns the current logger assigned to the resource, by default it is the root logger
  LOGGER       getLogger();

  // Get a new logger object and if assign to resource if param is true
  LOGGER       getLogger( const std::string &logger_name, const bool assignToResource=false);


  /*
   * LogEventConsumer
   */

  CF::LogEventSequence *retrieve_records( CORBA::ULong &howMany,
					  CORBA::ULong startingRecord );
  CF::LogEventSequence *retrieve_records_by_date( CORBA::ULong &howMany,
						  CORBA::ULongLong to_timeStamp );
  CF::LogEventSequence *retrieve_records_from_date( CORBA::ULong &howMany,
						    CORBA::ULongLong from_timeStamp );

  /*
   *
   *
   */
  void   setLoggingMacros ( ossie::logging::MacroTable &tbl, bool applyCtx );

  /*
   *  setResourceContext
   *   
   *   
   */
  void    setResourceContext(  ossie::logging::ResourceCtxPtr ctx );

  /*
   *  setLoggingContext
   * Set the logging context for each type of Resource.
   * 
   */
  void    setLoggingContext( ossie::logging::ResourceCtxPtr ctx );

  /*
   *  setLoggingContext
   * Set the logging context for each type of Resource.
   * 
   */
  void    setLoggingContext( const std::string &url, int loglevel, ossie::logging::ResourceCtxPtr ctx );

  /*
   *  saveLoggingContext
   * Save the logging context for each type of Resource, do not apply setting to 
   * underlying logging library
   * 
   */
  void    saveLoggingContext( const std::string &url, int loglevel, ossie::logging::ResourceCtxPtr ctx );


  //
  // RESOLVE: refactor to use boost::function and boost::bind 
  //

  typedef void (*LogConfigCallbackFn)(const char *config_data);

  typedef void (*LogLevelCallbackFn)(const char *logid, const CF::LogLevel&);


  //
  // 
  //
  class LogLevelListener {

  public:
    virtual void operator() ( const char *logid, const CF::LogLevel &level ) = 0;
    virtual ~LogLevelListener() {};

  };

  /*
   * Allow for member functions to receive configuration notifications
   */
  template <class T>
    class MemberLogLevelListener : public LogLevelListener
    {
    public:
      typedef boost::shared_ptr< MemberLogLevelListener< T > > SPtr;
      
      typedef void (T::*MemberFn)( const char *logid, const CF::LogLevel &level );

      static SPtr Create( T &target, MemberFn func ){
	return SPtr( new MemberLogLevelListener(target, func ) );
      };

      virtual void operator() ( const char *logid, const CF::LogLevel &level )
      {
	(target_.*func_)(logid, level);
      }

      // Only allow PropertySet_impl to instantiate this class.
      MemberLogLevelListener ( T& target,  MemberFn func) :
      target_(target),
	func_(func)
	{
	}
    private:
      T& target_;
      MemberFn func_;
    };

  /*
   * Wrap Callback functions as LogLevelListener objects
   */
  class StaticLogLevelListener : public LogLevelListener
  {
  public:
    virtual void operator() ( const char *logid, const CF::LogLevel &level )
    {
      (*func_)(logid, level);
    }

    StaticLogLevelListener ( LogLevelCallbackFn func) :
    func_(func)
    {
    }

  private:

    LogLevelCallbackFn func_;
  };



  //
  // 
  //
  class LogConfigListener {

  public:
    virtual void operator() ( const char *config_data ) = 0;
    virtual ~LogConfigListener() {};

  };

  /*
   * Allow for member functions to receive configuration notifications
   */
  template <class T>
    class MemberLogConfigListener : public LogConfigListener
    {
    public:
      typedef boost::shared_ptr< MemberLogConfigListener< T > > SPtr;
      
      typedef void (T::*MemberFn)( const char *config_data );

      static SPtr Create( T &target, MemberFn func ){
	return SPtr( new MemberLogConfigListener(target, func ) );
      };

      virtual void operator() ( const char *config_data )
      {
	(target_.*func_)(config_data);
      }

      // Only allow PropertySet_impl to instantiate this class.
      MemberLogConfigListener ( T& target,  MemberFn func) :
      target_(target),
	func_(func)
	{
	}
    private:
      T& target_;
      MemberFn func_;
    };

  /*
   * Wrap Callback functions as LogConfigListener objects
   */
  class StaticLogConfigListener : public LogConfigListener
  {
  public:
    virtual void operator() ( const char *config_data )
    {
      (*func_)(config_data);
    }

    StaticLogConfigListener ( LogConfigCallbackFn func) :
    func_(func)
    {
    }

  private:

    LogConfigCallbackFn func_;
  };


 protected:

  typedef boost::shared_ptr< LogConfigListener  >   LogConfigListenerPtr;
  typedef boost::shared_ptr< LogLevelListener  >    LogLevelListenerPtr;

  template< typename T > inline
    void setLogConfigListener(T &target, void (T::*func)( const char *config_data )  ) {
    logConfigCallback =  boost::make_shared< MemberLogConfigListener< T > >( boost::ref(target), func );
  };

  template< typename T > inline
    void setLogConfigListener(T *target, void (T::*func)( const char *config_data )  ) {
    logConfigCallback =  boost::make_shared< MemberLogConfigListener< T > >( boost::ref(*target), func );
  };

  template< typename T > inline
    void setLogLevelListener(T &target, void (T::*func)( const char *logid, const CF::LogLevel &level )  ) {
    logLevelCallback =  boost::make_shared< MemberLogLevelListener< T > >( boost::ref(target), func );
  };

  template< typename T > inline
    void setLogLevelListener(T *target, void (T::*func)( const char *logid, const CF::LogLevel &level )  ) {
    logLevelCallback =  boost::make_shared< MemberLogLevelListener< T > >( boost::ref(*target), func );
  };


  virtual void setLogConfigCallback(LogConfigListener *func);
  virtual void setLogConfigCallback(LogConfigCallbackFn func);
  virtual void setLogLevelCallback(LogLevelListener *func);
  virtual void setLogLevelCallback(LogLevelCallbackFn  func);

  // logger identifier
  std::string                    _logName;

  // current logging level set for this component via execparams, or  LogConfiguration API
  CF::LogLevel                   _logLevel;

  // current logging object
  LOGGER                         _logger;

  // logging macro defintion table;
  ossie::logging::MacroTable     _loggingMacros;
  
 private:

  // logging configuration data, 
  std::string                    _logCfgContents;

  std::string                    _logCfgURL;

  ossie::logging::ResourceCtxPtr _loggingCtx;

  // Event channel to listen for configuration and log level changes
  boost::shared_ptr< ossie::events::PushEventConsumer > logConfigChannel;

  // callback to notify when logging configuration change is requested
  LogConfigListenerPtr              logConfigCallback;

  // callback to notify when a logging level change is requested
  LogLevelListenerPtr              logLevelCallback;

};
#endif                                       
