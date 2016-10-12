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
#include <iostream>
#include "ossie/Logging_impl.h"
#include "ossie/logging/loghelpers.h"
#include "ossie/debug.h"

#ifdef LOCAL_DEBUG_ON
#define STDOUT_DEBUG(x)    std::cout << x << std::endl
#else
#define STDOUT_DEBUG(x)    
#endif


struct null_deleter
{
  void operator()(void const *) const
  {
  }
};


Logging_impl::Logging_impl() :
  _logName(""),
  _logLevel(CF::LogLevels::INFO),
  _logCfgContents(),
  _logCfgURL(""),
  _loggingCtx()
{

  // get default set of macros to fill in
  _loggingMacros = ossie::logging::GetDefaultMacros();

  ossie::logging::ResolveHostInfo( _loggingMacros );

  _logCfgContents = ossie::logging::GetDefaultConfig();

  // setup logger to be root by default and assign logging level INFO`
  getLogger( _logName, true );
};



void  Logging_impl::setLoggingMacros ( ossie::logging::MacroTable &tbl, bool applyCtx ) {

  // copy over contents
  _loggingMacros = tbl;

  if ( applyCtx  ){
    if ( _loggingCtx ) {
      _loggingCtx->apply( _loggingMacros );
    }
  }
}

void Logging_impl::setResourceContext( ossie::logging::ResourceCtxPtr ctx ) {
  if ( ctx ) {
    ctx->apply( _loggingMacros );
    _loggingCtx = ctx;
  }
}


void Logging_impl::setLoggingContext( ossie::logging::ResourceCtxPtr ctx ) {

  STDOUT_DEBUG( "Logging_impl setLoggingContext START:" );
  if ( ctx ) {
    STDOUT_DEBUG( "Logging_impl setLoggingContext Apply Macro Context:" );
    ctx->apply( _loggingMacros );
    _loggingCtx = ctx;
  }
  else if ( _loggingCtx ) {
    _loggingCtx->apply( _loggingMacros );
  }

  STDOUT_DEBUG( "Logging_impl setLoggingContext setLogConfigURL:");
  setLogConfigURL( _logCfgURL.c_str() );

  STDOUT_DEBUG( "Logging_impl setLoggingContext setLogLevel:");
  setLogLevel( _logName.c_str(), _logLevel );

  STDOUT_DEBUG("Logging_impl setLoggingContext END" );

}


void Logging_impl::setLoggingContext( const std::string &logcfg_url, int logLevel, ossie::logging::ResourceCtxPtr ctx ) {

  if ( logcfg_url == "" ) {
    ossie::logging::ConfigureDefault();
  }
  else {
    STDOUT_DEBUG( "Logging_impl setLoggingContext START:");
    if ( ctx ) {
      STDOUT_DEBUG( "Logging_impl setLoggingContext Apply Macro Context:" );
      ctx->apply( _loggingMacros );
      _loggingCtx = ctx;
    }

    STDOUT_DEBUG( "Logging_impl setLoggingContext setLogConfigURL:" );
    setLogConfigURL( logcfg_url.c_str() );
  }

  if ( logLevel > -1  ) {
    STDOUT_DEBUG("Logging_impl setLoggingContext setLogLevel:" );
    setLogLevel( "", ossie::logging::ConvertDebugToCFLevel(logLevel) );
  }
  else {
    LOGGER root = rh_logger::Logger::getRootLogger();
    if ( root ) {
      _logLevel = ossie::logging::ConvertRHLevelToCFLevel( root->getLevel() );
    }
  }

  STDOUT_DEBUG("Logging_impl setLoggingContext END" );
}



void Logging_impl::saveLoggingContext( const std::string &logcfg_url, int logLevel, ossie::logging::ResourceCtxPtr ctx ) {

  STDOUT_DEBUG("Logging_impl saveLoggingContext START:");
  if ( ctx ) {
    STDOUT_DEBUG( "Logging_impl saveLoggingContext Apply Macro Context:");
    ctx->apply( _loggingMacros );
    _loggingCtx = ctx;
  }

  try {
      // save off logging config url
      _logCfgURL = logcfg_url;

    // test we have a logging URI
    if ( logcfg_url == "" ) {
      STDOUT_DEBUG( "Logging_impl saveLoggingContext Default Configuration.");
      _logCfgContents=ossie::logging::GetDefaultConfig();
    }
    else{
      // grab contents of URL and save
      _logCfgContents = "";
      std::string config_contents = ossie::logging::GetConfigFileContents(logcfg_url);
      if ( config_contents.size() > 0  ){
	_logCfgContents= ossie::logging::ExpandMacros(config_contents, _loggingMacros);
      }
    }
  }
  catch( std::exception &e ) {
  }

  if ( logLevel > -1  ) {
    STDOUT_DEBUG( "Logging_impl setLoggingContext save _logLevel:" << logLevel );
    _logLevel = ossie::logging::ConvertDebugToCFLevel(logLevel);
  }
  else {
    LOGGER root = rh_logger::Logger::getRootLogger();
    if ( root ) {
      _logLevel = ossie::logging::ConvertRHLevelToCFLevel( root->getLevel() );
    }
  }

  // resource loggers are private to the component/device implementation class, we need to grab the resource logger's logger by
  // name and assign it to our logging interface, so external configuration can affect the correct logger and not just the logging interface class
  std::string lname = rh_logger::Logger::getResourceLoggerName();
  if ( lname != "" ) {
      std::string _lname = _logger->getName();
      if ( lname != _lname )  {
          LOGGER _l = getLogger( lname, true );
          _logName = lname;
      }
  }

  STDOUT_DEBUG("Logging_impl setLoggingContext END" );
}



LOGGER Logging_impl::getLogger () {
  return _logger;
}

LOGGER Logging_impl::getLogger (const std::string &logger_name, bool assignToResource) {
  LOGGER retval;
  if ( logger_name == "" ) {
    retval = rh_logger::Logger::getRootLogger();
  }
  else {
    retval= rh_logger::Logger::getLogger(logger_name);
  }
  
  if ( assignToResource ) {
    _logName = logger_name;
    _logger = retval;
  }
  
  return retval;
}


char *Logging_impl::getLogConfig () {
  return CORBA::string_dup( _logCfgContents.c_str() );
}

void Logging_impl::setLogConfig( const char *config_contents ) {

  if ( logConfigCallback) {
    std::string lcfg =   ossie::logging::ExpandMacros( config_contents, _loggingMacros );
    (*logConfigCallback)( lcfg.c_str() );
    _logCfgContents = lcfg;
  }
  else {
    ossie::logging::Configure( config_contents, _loggingMacros, _logCfgContents );
    // check if my level has changed for the logger
    log_level();    
  }
  
}

void Logging_impl::setLogConfigURL( const char *in_url ) {

  //
  // Get File contents....
  //
  std::string url("");
  try{
    if ( in_url ) url=in_url;

    _logCfgURL = url;
    std::string config_contents = ossie::logging::GetConfigFileContents(url);

    if ( config_contents.size() > 0  ){
      setLogConfig( config_contents.c_str() );
    }
    else {
      RH_WARN(_logger, "URL contents could not be resolved, url: " << url );
    }

  }
  catch( std::exception &e ){
      RH_WARN(_logger, "Exception caught during logging configuration using URL, url: " << url );
  }

}


void Logging_impl::setLogLevel( const char *logger_id, const CF::LogLevel newLevel ) 
  throw (CF::UnknownIdentifier)
{
  _logLevel = newLevel;
  if ( logLevelCallback ) {
    (*logLevelCallback)(logger_id, newLevel);
  }
  else {
    std::string logid("");
    if ( logger_id )  logid=logger_id;
    ossie::logging::SetLogLevel( logid, newLevel );
    if ( _logger && logid == _logger->getName() ) {
        _logLevel = newLevel;
    }
  }
}

CF::LogLevel Logging_impl::log_level() {
    if ( _logger ) {
        CF::LogLevel level = ossie::logging::ConvertRHLevelToCFLevel( _logger->getLevel() );
        if ( level != _logLevel ) {
            _logLevel = level;
        }
    }
    return _logLevel;
}


void Logging_impl::log_level( const CF::LogLevel newLevel ) {

  if ( logLevelCallback ) {
    _logLevel = newLevel;
    (*logLevelCallback)( "", newLevel);
  }
  else {
    _logLevel = newLevel;
    rh_logger::LevelPtr level = ossie::logging::ConvertCFLevelToRHLevel( newLevel );
    // apply new level to resource logger
    if ( _logger ) {
      _logger->setLevel( level );
    }
  }   
}


CF::LogEventSequence *Logging_impl::retrieve_records( CORBA::ULong &howMany,
					  CORBA::ULong startingRecord ) {
  howMany=0;
  CF::LogEventSequence_var seq;
  return  seq._retn();
};

CF::LogEventSequence *Logging_impl::retrieve_records_by_date( CORBA::ULong &howMany,
							      CORBA::ULongLong to_timeStamp ) {
  howMany=0;
  CF::LogEventSequence_var seq;
  return  seq._retn();
};

CF::LogEventSequence *Logging_impl::retrieve_records_from_date( CORBA::ULong &howMany,
								CORBA::ULongLong from_timeStamp ) {
  howMany=0;
  CF::LogEventSequence_var seq;
  return  seq._retn();
};



void Logging_impl::setLogConfigCallback(LogConfigListener *func) {
  logConfigCallback =  boost::shared_ptr< LogConfigListener >(func, null_deleter());
}

void Logging_impl::setLogConfigCallback(LogConfigCallbackFn func) {
  logConfigCallback =  boost::make_shared< StaticLogConfigListener >(func );
}

void Logging_impl::setLogLevelCallback(LogLevelListener *func) {
  logLevelCallback =  boost::shared_ptr< LogLevelListener >(func, null_deleter());
}

void Logging_impl::setLogLevelCallback(LogLevelCallbackFn func) {
  logLevelCallback =  boost::make_shared< StaticLogLevelListener >(func);
}



