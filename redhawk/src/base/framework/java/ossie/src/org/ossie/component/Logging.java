
package org.ossie.component;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.UUID;

import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.Layout;
import org.apache.log4j.Level;

import org.ossie.logging.logging;

import CF.UnknownProperties;
import CF.UnknownIdentifier;
import CF.InvalidIdentifier;

/**
  Loggging

  Class that implements all the features for the Logging IDL interface

 */

abstract public class Logging {

    /**
     *  LoggingListener 
     *  This interface allows developers to modify the normal behavior when a request to 
     *  change the logging level or configuration context.
     *
     *  Developers can provide an interface to the Resource through the 
     *  setNewLoggingListener method.
     */
    public interface ConfigurationChangeListener {

	/**
	 * logLevelChanged 
	 *
	 * This method is called when a logging level request is made for this resource. The name of the
	 * the logger to affect and the new level are provided.  Logging level values are defined by the 
         * CF::LogLevels constants.
	 *
	 * @param logId name of the logger to change the level,  logId=="" is the root logger 
	 * @param newLevel the new logging level to apply
	 *
	 */
	public void logLevelChanged( String logId, int newLevel );

	/**
	 * logConfigChanged
	 *
	 * This method is called when the logging configuration change is requested.  The configuration
	 * data follows the log4j configuration format (java props or xml).
	 *
	 * @param config_data  the log4j formatted configuration data, either java properties or xml.
	 */
	public void logConfigChanged( String config_data );
    }


    /** internal log4j logger object */
    protected Logger               _logger;

    /** log identifier, by default uses root logger or "" **/
    protected String               logName;

    /** current log level assigned to the resource **/
    protected int                  logLevel;
    
    /** current string used configure the log **/
    protected String               logConfig;
    
    /** callback listener **/
    protected ConfigurationChangeListener    logListener;

    /** logging macro definitions maintained by resource */
    protected logging.MacroTable   loggingMacros;

    /** logging context assigned  to resource */
    protected logging.ResourceCtx  loggingCtx=null;

    /** holds the url for the logging configuration */
    protected String               loggingURL;

    /**
       Constructor that sets the base logging context for a resource. The logger that is passed in is 
       established by Domain base classes: Resource, Device, and Service, to maintain backwards
       compatablity.  

       The logger name for Java logging resources will evalute to the actual class name of the resource.
       
     */
    public Logging ( Logger establishedLogger, String logName  ) {

        if  ( establishedLogger != null ) {
            // retain handle to underlying classes logger so we can configure 
            // as needed
            _logger = establishedLogger;
        }
        else {
            _logger = Logger.getLogger(logName);
        }

	this.logName=logName;
	this.logLevel=CF.LogLevels.INFO;
	this.logConfig ="";
	this.logListener=null;
	this.loggingCtx = null;
	this.loggingURL = null;
	this.loggingMacros=logging.GetDefaultMacros();
	logging.ResolveHostInfo( this.loggingMacros );
    }

    ///////////////////////////////////////////////////////////////////////
    //
    //        Logging Configuration Section
    //
    ///////////////////////////////////////////////////////////////////////

    /**
     * setNewLoggingListener
     *
     * Assign callback listeners for log level changes and log configuration
     * changes
     *
     * @param Resource.LoggingListener  callback interface definition
     *
     */
    public void setNewLoggingListener( ConfigurationChangeListener newListener ) {
	this.logListener = newListener;
    }

    /**
     *  getLogger
     *  
     *  @returns Logger return the logger assigned to this resource
     */
    public Logger     getLogger() {
	return _logger;
    }

    /**
     *  getLogger
     * 
     *  return the logger assigned to this resource
     *  @param String name of logger to retrieve
     *  @param boolan true, will reassign new logger to the resource, false do not assign
     *  @returns Logger return the named logger 
     */
    public Logger     getLogger( String logid,  boolean assignToResource ) {

	Logger log =null;
	if ( logid != null )  {
	    log = Logger.getLogger( logid );
	}
	if ( assignToResource && log != null ) {
	    _logger = log;
	}
	return log;
    }


    /**
     *  setLogger
     * 
     *  Override the default resource logger and name..
     *
     * @param logging.Logger  logger to use 
     * @param String          name of the logger 
     */
    public void setLogger( Logger logger, String logName ) {
       if  ( logger != null ) {
            // retain handle to underlying classes logger so we can configure 
            // as needed
           _logger = logger;
        }
        else {
            _logger = Logger.getLogger(logName);
        }

	this.logName=logName;
    }


    /**
     *  setLoggingMacros
     * 
     *  Use the logging resource context class to set any logging macro definitions.
     *
     * @param logging.Resource  a content class from the logging.ResourceCtx tree
     */
    public void setLoggingMacros( logging.MacroTable newTbl, boolean applyCtx ) {
	if ( newTbl != null  ) {
	    this.loggingMacros = newTbl;
	    this.loggingCtx.apply( this.loggingMacros );
	}
    }

    /**
     *  setResourceContext
     * 
     *  Use the logging resource context class to set any logging macro definitions.
     *
     * @param logging.Resource  a content class from the logging.ResourceCtx tree
     */
    public void setResourceContext( logging.ResourceCtx ctx ) {
	if ( ctx !=  null ) {
	    ctx.apply( this.loggingMacros );
	    this.loggingCtx = ctx;
	}
    }

    /**
     *  setLoggingContext
     * 
     *  Set the logging configuration and logging level for this resource
     *
     * @param logging.Resource  a content class from the logging.ResourceCtx tree
     */
    public void setLoggingContext( logging.ResourceCtx ctx ) {

	// apply any context data
	if ( ctx !=  null ) {
	    ctx.apply( this.loggingMacros );
	    this.loggingCtx = ctx;
	}
	else if ( this.loggingCtx != null ) {
	    this.loggingCtx.apply( this.loggingMacros );
	}

	// call setLogConfigURL to load configuration and set log4j
	if ( this.loggingURL != null  ) {
	    setLogConfigURL( this.loggingURL );
	}

	try {
	    // set log level for this logger 
	    setLogLevel( this.logName,  this.logLevel );
	}
	catch( Exception e ){
	}
    }

    /**
     *  saveLoggingContext
     * 
     *  Set the logging configuration and logging level for this resource.  This original 
     *  configuration context for log4j is applied to the library before a Domain resource
     *  is actually constructed.  
     *
     *  The logging configuration information is retained by this class for the underlying
     *  Domain resource. Additionaly, the EventChannelManager accessible via the Domain
     *  can now be used by the RH_LogEventAppender.
     *  
     *
     * @param String  URL of the logging configuration file to load
     * @param int  oldstyle_loglevel used from command line startup of a resource
     * @param logging.Resource  a content class from the logging.ResourceCtx tree
     */
    public void saveLoggingContext( String logcfg_url, 
                                    int oldstyle_loglevel, 
                                    logging.ResourceCtx ctx,
                                    org.ossie.events.Manager ECM ) {

	// apply any context data
	if ( ctx !=  null ) {
	    ctx.apply( this.loggingMacros );
	    this.loggingCtx = ctx;
	}

	// save off configuration that we are given
	try{
	    this.loggingURL = logcfg_url;	    
	    if ( logcfg_url == null || logcfg_url == "" ) {
		this.logConfig= logging.GetDefaultConfig();
	    }
	    else {
		String cfg_data="";
		cfg_data = logging.GetConfigFileContents(logcfg_url);
	    
		if ( cfg_data.length() > 0  ){
		    // process file with macro expansion....
		    this.logConfig = logging.ExpandMacros(cfg_data, loggingMacros );
		}
		else {
		    _logger.warn( "URL contents could not be resolved, url: " + logcfg_url );
		}
	    }
	}
	catch( Exception e ){
	    _logger.warn( "Exception caught during logging configuration using URL, url: "+ logcfg_url );
	}

	if  ( oldstyle_loglevel > -1  ) {
            logLevel = logging.ConvertLogLevel(oldstyle_loglevel);
            try {
                if ( _logger != null ) {
                    _logger.setLevel( logging.ConvertLogLevelToLog4(oldstyle_loglevel) );
                }
                else {
                    setLogLevel( logName, logging.ConvertLogLevel(oldstyle_loglevel) );
                }
            }
            catch( Exception e ){
            }
	}
	else {
	    // grab root logger's level
	    logLevel = logging.ConvertLog4ToCFLevel( Logger.getRootLogger().getLevel() );
	}
        
        // attach this resource to any special appenders that require access to domain resources
        // set the resource context for the logging library to use
        // for event channel appenders... needs to occur after
        // Domain awareness is established
        logging.SetEventChannelManager( ECM );
        
    }

    public void  setEventChannelManager( org.ossie.events.Manager ECM ) {
        logging.SetEventChannelManager( ECM );
    }



    /**
     *  setLoggingContext
     * 
     *  Set the logging configuration and logging level for this resource.
     *
     * @param String  URL of the logging configuration file to load
     * @param int  oldstyle_loglevel used from command line startup of a resource
     * @param logging.Resource  a content class from the logging.ResourceCtx tree
     */
    public void setLoggingContext( String logcfg_url, int oldstyle_loglevel, logging.ResourceCtx ctx ) {

	// test we have a logging URI
	if ( logcfg_url == null || logcfg_url == "" ) {
	    logging.ConfigureDefault();
	}
	else {
	    // apply any context data
	    if ( ctx !=  null ) {
		ctx.apply( this.loggingMacros );
		this.loggingCtx = ctx;
	    }

	    // call setLogConfigURL to load configuration and set log4j
	    if ( logcfg_url != null ) {
		setLogConfigURL( logcfg_url );
	    }
	}

	try {
	    if  ( oldstyle_loglevel > -1  ) {
		// set log level for this logger 
		setLogLevel( logName, logging.ConvertLogLevel(oldstyle_loglevel) );
	    }
	    else {
		// grab root logger's level
		logLevel = logging.ConvertLog4ToCFLevel( Logger.getRootLogger().getLevel() );
	    }
	}
	catch( Exception e ){
	}
    }

    //////////////////////////////////////////////////////////////////////////////
    //
    // LogConfiguration IDL Support
    //
    //////////////////////////////////////////////////////////////////////////////

    /**
     *  log_level
     * 
     *  Return the current logging level as defined by the Logging Interface  IDL
     *
     * @return int value of a CF::LogLevels enumeration
     */
    public int log_level() {
        if ( _logger != null ) {
            Level logger_level = _logger.getLevel();
            Level cur_loglevel= logging.ConvertToLog4Level(logLevel);
            if ( logger_level != null && logger_level != cur_loglevel ) {
                logLevel = logging.ConvertLog4ToCFLevel(logger_level);
            }
        }
        return logLevel;

    }

    /**
     *  log_level
     * 
     *  Set the logging level for the logger assigned to the resource. If a callback listener 
     *  is assigned to the resource then invoke the listener to handle the assignment
     *
     * @param int value of a CF::LogLevels enumeration
     */
    public void log_level( int newLogLevel ) {
	if ( this.logListener != null  ) {
	    logLevel = newLogLevel;
	    this.logListener.logLevelChanged( logName, newLogLevel );
	}
	else {
	    logLevel = newLogLevel;
	    Level tlevel= logging.ConvertToLog4Level(newLogLevel);
	    if ( _logger != null ) {
		_logger.setLevel(tlevel);
	    }
	    else {
		Logger.getRootLogger().setLevel(tlevel);
	    }
	}
	
    }


    /**
     *  setLogLevel
     * 
     *  Set the logging level for a named logger associated with this resource. If a callback listener 
     *  is assigned to the resource then invoke the listener to handle the assignment
     *
     * @param int value of a CF::LogLevels enumeration
     */
    public void setLogLevel( String logger_id, int newLogLevel ) throws UnknownIdentifier {

	if ( this.logListener != null ) {
	    if ( logger_id == logName ){
		this.logLevel = newLogLevel;
	    }

	    this.logListener.logLevelChanged( logger_id, newLogLevel );
	}
	else {
	    Level tlevel=Level.INFO;
	    tlevel = logging.ConvertToLog4Level(newLogLevel);	       
	    
	    if ( logger_id != null ){
		Logger logger = Logger.getLogger( logger_id );
		if ( logger != null ) {
		    logger.setLevel( tlevel );
                    if ( logger_id == logName ) {
                        logLevel=newLogLevel;
                    }
		}
	    }
	    else {
		Logger.getRootLogger().setLevel(tlevel);
	    }

	}
    }

    /**
     *  getLogConfig
     * 
     *  return the logging configration information used to configure the log4j library
     *
     * @returns String contents of the configuration file 
     */
    public  String getLogConfig() {
	return logConfig;
    }


    /**
     *  setLogConfig
     * 
     * Process the config_contents param as the contents for the log4j configuration
     * information. 
     * 
     * First, run the configuration information against the current macro defintion
     * list and then assigned that data to the log4j library.  If this operation
     * completed sucessfully, the new configuration is saved.
     *
     *
     * @param String contents of the configuration file 
     */
    public void setLogConfig( String config_contents ) {
	if ( this.logListener != null ) {
            String lcfg = logging.ExpandMacros( config_contents, loggingMacros );
	    this.logListener.logConfigChanged( lcfg );
	    this.logConfig = lcfg;
	}
	else {
	    try {
		String newcfg="";
		newcfg = logging.Configure( config_contents, loggingMacros );
		this.logConfig = newcfg;
	    }
	    catch( Exception e ) {
		_logger.warn("setLogConfig failed, reason:" + e.getMessage() );
	    }
	}
    }

    /**
     *  setLogConfig
     * 
     *  Use the config_url value to read in the contents of the file as the new log4j
     *  configuration context. If the file is sucessfully loaded then setLogConfig
     *  is called to finish the processing.
     *
     * @param String URL of file to load
     */
    public void setLogConfigURL( String config_url ) {

	//
	// Get File contents....
	//
	try{
	    String config_contents="";
	    
	    config_contents = logging.GetConfigFileContents(config_url);
	    
	    if ( config_contents.length() > 0  ){
		this.loggingURL = config_url;
		//  apply contents of file to configuration
		this.setLogConfig( config_contents );
	    }
	    else {
		_logger.warn( "URL contents could not be resolved, url: " + config_url );
	    }

	}
	catch( Exception e ){
	    _logger.warn( "Exception caught during logging configuration using URL, url: "+ config_url );
	}
    }


    //////////////////////////////////////////////////////////////////////////////
    //
    // LogEventConsumer IDL Support
    //
    //////////////////////////////////////////////////////////////////////////////

    public CF.LogEvent[] retrieve_records( org.omg.CORBA.IntHolder howMany, int startingPoint ) {
        howMany=new org.omg.CORBA.IntHolder(0);
        CF.LogEvent[] seq = new CF.LogEvent[0]; 
	return seq;
    }

    public CF.LogEvent[] retrieve_records_by_date( org.omg.CORBA.IntHolder howMany, long to_timeStamp ) {
        howMany=new org.omg.CORBA.IntHolder(0);
        CF.LogEvent[] seq = new CF.LogEvent[0]; 
	return seq;
    }

    public CF.LogEvent[] retrieve_records_from_date( org.omg.CORBA.IntHolder howMany, long from_timeStamp ) {
        howMany=new org.omg.CORBA.IntHolder(0);
        CF.LogEvent[] seq = new CF.LogEvent[0]; 
	return seq;
    }

}
