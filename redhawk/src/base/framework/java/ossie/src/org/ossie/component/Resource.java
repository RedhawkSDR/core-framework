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

/**
 *
 * Identification: $Revision$
 */
package org.ossie.component;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;
import java.io.FileOutputStream;
import java.io.File;

import org.apache.log4j.BasicConfigurator;
import org.apache.log4j.Logger;
import org.apache.log4j.LogManager;
import org.apache.log4j.PropertyConfigurator;
import org.apache.log4j.LogManager;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.Layout;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Appender;
import org.apache.log4j.Level;
import org.omg.CORBA.ORB;
import org.omg.CORBA.UserException;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import org.omg.PortableServer.Servant;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;
import org.omg.PortableServer.POAPackage.ObjectNotActive;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.ossie.events.PropertyEventSupplier;
import org.ossie.properties.IProperty;
import org.ossie.properties.AnyUtils;
import org.ossie.logging.logging;

import CF.AggregateDevice;
import CF.AggregateDeviceHelper;
import CF.DataType;
import CF.Device;
import CF.DeviceHelper;
import CF.DeviceManager;
import CF.DeviceManagerHelper;
import CF.InvalidObjectReference;
import CF.LogLevels;
import CF.PortPOA;
import CF.PropertiesHolder;
import CF.ResourceHelper;
import CF.ResourceOperations;
import CF.ResourcePOA;
import CF.ResourcePOATie;
import CF.UnknownProperties;
import CF.UnknownIdentifier;
import CF.LifeCyclePackage.InitializeError;
import CF.LifeCyclePackage.ReleaseError;
import CF.PortSupplierPackage.UnknownPort;
import CF.PropertySetPackage.InvalidConfiguration;
import CF.PropertySetPackage.PartialConfiguration;
import CF.ResourcePackage.StartError;
import CF.ResourcePackage.StopError;
import CF.TestableObjectPackage.UnknownTest;

public abstract class Resource implements ResourceOperations, Runnable { // SUPPRESS CHECKSTYLE Name

    /**
     *  LoggingListener 
     *  This interface allows developers to modify the normal behavior when a request to 
     *  change the logging level or configuration context.
     *
     *  Developers can provide an interface to the Resource through the 
     *  setNewLoggingListener method.
     */
    public interface LoggingListener {

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


    public final static Logger logger = Logger.getLogger(Resource.class.getName());

    public  static logging.ResourceCtx loggerCtx = null;
    
    protected CF.Resource resource;

    /**
     * The CORBA ORB to use for servicing requests
     */
    protected org.omg.CORBA.ORB orb = null;
    /** The POA object that this object should use for any CORBA objects it creates*/
    protected POA poa = null;
    /** The component ID */
    protected String compId;
    /** The component name */
    protected String compName;
    /** Map of input ports for this resource */
    protected Hashtable<String, Object> portObjects;
    /** Map of input ports for this resource */
    protected Hashtable<String, Servant> portServants;
    /** Map of input ports for this resource */
    protected Hashtable<String, org.omg.CORBA.Object> ports;
    /** Map of native ports for this resource */
    protected Hashtable<String, omnijni.Servant> nativePorts;
    /** Map of properties for this resource */
    protected Hashtable<String, IProperty> propSet;
    /** flag if we're started */
    protected volatile boolean _started = false;
    /** flag if we're released */
    protected boolean disposed = false;
    /** flag if we're already initialized */
    protected boolean initialized = false;
    /** the processing thread, if any */
    protected Thread processingThread;
    /** port to be used to output property changes */
    protected PropertyEventSupplier propertyChangePort;
    protected String softwareProfile;

    /** log identifier, by default uses root logger or "" **/
    protected String               logName;

    /** current log level assigned to the resource **/
    protected int                  logLevel;
    
    /** current string used configure the log **/
    protected String               logConfig;
    
    /** callback listener **/
    protected LoggingListener      logListener;

    /** logging macro definitions maintained by resource */
    protected logging.MacroTable   loggingMacros;

    /** logging context assigned  to resource */
    protected logging.ResourceCtx  loggingCtx=null;

    /** holds the url for the logging configuration */
    protected String               loggingURL;

    /**
     * Constructor intended to be used by start_component.
     */
    public Resource() {
        this.compId = "";
        this.compName = "";
        this._started = false;
        this.propertyChangePort = null;
        this.ports = new Hashtable<String, org.omg.CORBA.Object>();
        this.portServants = new Hashtable<String, Servant>();
        this.portObjects = new Hashtable<String, Object>();
        this.nativePorts = new Hashtable<String, omnijni.Servant>();
        this.propSet = new Hashtable<String, IProperty>();

	// support for logging idl
	this.logName=Resource.class.getName();
	this.logLevel=CF.LogLevels.INFO;
	this.logConfig ="";
	this.logListener=null;
	this.loggingCtx = null;
	this.loggingURL = null;
	this.loggingMacros=logging.GetDefaultMacros();
	logging.ResolveHostInfo( this.loggingMacros );
	
    }
    
    public void addProperty(IProperty prop) {
        this.propSet.put(prop.getId(), prop); 
    }
    
    public void addPort(String name, Object object) {
        this.portObjects.put(name, object);
        this.portServants.put(name, (Servant)object);
    }

    protected void addPort(String name, omnijni.Servant servant) {
        this.nativePorts.put(name, servant);
    }
    
    /**
     * Default Constructor that automatically sets parameters for the Sun ORB
     * and the JacORB ORB.
     * 
     * @param compId Name of this resource
     * @param orb the ORB to use
     * @throws WrongPolicy 
     * @throws ServantNotActive 
     */
    public Resource(final String compId, final String compName, final ORB orb, final POA poa) throws ServantNotActive, WrongPolicy {
        this();
        this.setup(compId, compName, orb, poa);
    }

    /**
     * {@inheritDoc}
     */
    public String identifier() {
        return this.compId;
    }

    public String softwareProfile() {
        if (softwareProfile == null) {
            return "";
        } else {
            return softwareProfile;
        }
    }

    /**
     * {@inheritDoc}
     */
    public boolean started() {
        return this._started;
    }

    public String getName() {
        return this.compName;
    }
    /* METHODS EXPECTED TO BE IMPLEMENTED BY THE USER */

    /**
     * {@inheritDoc}
     */
    public void initialize() throws InitializeError {
        logger.trace("initialize()");
        if (!initialized) {
            this.ports.clear();
            for (Map.Entry<String, Servant> me : this.portServants.entrySet()) {
                org.omg.CORBA.Object obj = activateObject(me.getValue());
                this.ports.put(me.getKey(), obj);
            }
            initialized = true;
        }
    }

    /**
     * {@inheritDoc}
     */
    public void start() throws StartError {
        // While we are starting or stopping don't let anything else occur
        logger.trace("start()");
        synchronized (this) {
            this._started = true;
            if (processingThread == null) {
                processingThread = new Thread(this);
                processingThread.setDaemon(true);
                processingThread.start();
            }
        }
    }

    /**
     * {@inheritDoc}
     */
    public void stop() throws StopError {
        logger.trace("stop()");
        synchronized (this) {
            if (processingThread != null) {
                this._started = false;
                try {
                    processingThread.interrupt();
                    processingThread.join(1000);
                    if (processingThread.isAlive()) {
                        logger.error("Error stopping processing thread");
                        throw new StopError(CF.ErrorNumberType.CF_NOTSET, "Error stopping processing thread");
                    } else {
                        processingThread = null;
                    }
                } catch (InterruptedException e) {
                    logger.error("Error stopping processing thread", e);
                    throw new StopError(CF.ErrorNumberType.CF_NOTSET, "Error stopping processing thread due to: " + e.toString());
                }

            }
        }
    }



    /**
     * {@inheritDoc}
     */
    public void runTest(final int testid, final PropertiesHolder testValues) throws UnknownTest, UnknownProperties {
        logger.trace("runTest()");
    }

    /* BASE CLASS METHODS */

    public void releaseObject() throws ReleaseError {
        logger.trace("releaseObject()");
        try {
            this.stop();
        } catch (StopError e1) {
            logger.error("Failed to stop during release", e1);
        }
        
        // These loops deactivate the port objects so that they can be destroyed without incident
        synchronized (this) {
            try {
                // Deactivate all native ports; this will delete the omniORB servant.
                for (final omnijni.Servant s : this.nativePorts.values()) {
                    s._deactivate();
                }
                this.nativePorts.clear();

                for (final org.omg.CORBA.Object p : this.ports.values()) {
                    this.poa.deactivate_object(this.poa.reference_to_id(p));
                }

                // multi-stage destruction for the ports is necessary to account for the initial memory
                // allocation and the creation of the different maps
                // TODO Might have to do something different here
                this.ports.clear();

                this.poa.deactivate_object(this.poa.reference_to_id(resource));
            } catch (final Exception e) {
                throw new ReleaseError(new String[] {e.toString()});
            }
            disposed = true;
            notifyAll();
        }
    }

    public Object getPortObject(final String name) throws UnknownPort {
        // the mapping of ports assumes that port names are unique to the component
        //  the Ports_var maps are kept different (they could be made into one)
        //  because it's less confusing this way

        Object p = this.portObjects.get(name);
        if (p != null) {
            return p;
        }
        
        throw new UnknownPort("Unknown port: " + name);
    }

    public org.omg.CORBA.Object getPort(final String name) throws UnknownPort {
        // the mapping of ports assumes that port names are unique to the component
        // the Ports_var maps are kept different (they could be made into one)
        // because it's less confusing this way

        logger.trace("getPort(" + name + ")");
        if (this.nativePorts.containsKey(name)) {
            return this.nativePorts.get(name)._this_object(getOrb());
        }

        org.omg.CORBA.Object p = this.ports.get(name);
        if (p != null) {
            return p;
        }
        
        throw new UnknownPort("Unknown port: " + name);
    }

    /**
     * {@inheritDoc}
     */
    public void configure(final DataType[] configProperties) throws InvalidConfiguration, PartialConfiguration {
        logger.trace("configure()");

        // Ensure there's something to do
        if (configProperties.length == 0) {
            return;
        }

        final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();
        for (final DataType dt : configProperties) {
            // Look up the property and ensure it is configurable
            final IProperty prop = this.propSet.get(dt.id);
            if ((prop == null) || !prop.isConfigurable()) {
                invalidProperties.add(dt);
                continue;
            }

            try {
                // See if the value has changed
                if (AnyUtils.compareAnys(prop.toAny(), dt.value, "ne")) {
                    // Update the value on the property, which may trigger a
                    // callback.
                    prop.configure(dt.value);

                    // Check to see if this property should issue property change
                    // events and a port is registered.
                    if (prop.isEventable() && (this.propertyChangePort != null)) {
                        this.propertyChangePort.sendPropertyEvent(prop.getId());
                    }
                }
                logger.trace("Configured property: " + prop);
            } catch (Throwable t) {
                logger.error("Unable to configure property " + dt.id + ": " + t.getMessage());
                invalidProperties.add(dt);
            }
        }
        
        if (invalidProperties.size() == configProperties.length) {
            throw new InvalidConfiguration("Error configuring component", invalidProperties.toArray(new DataType[0]));
        } else if (invalidProperties.size() > 0) {
            throw new PartialConfiguration(invalidProperties.toArray(new DataType[0]));
        }
    }

    /**
     * {@inheritDoc}
     */
    public void query(final PropertiesHolder configProperties) throws UnknownProperties {
        logger.trace("query()");
        // For queries of zero length, return all id/value pairs in propertySet
        if (configProperties.value.length == 0) {
            final ArrayList<DataType> props = new ArrayList<DataType>(this.propSet.size());
            for (final IProperty prop : this.propSet.values()) {
                logger.trace("Querying property: " + prop);
                if (prop.isQueryable()) {
                    props.add(new DataType(prop.getId(), prop.toAny()));
                }
            }

            configProperties.value = props.toArray(new DataType[props.size()]);
            return;
        }

        // For queries of length > 0, return all requested pairs in propertySet
        final ArrayList<DataType> validProperties = new ArrayList<DataType>();
        final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();

        // Return values for valid queries in the same order as requested
        for (final DataType dt : configProperties.value) {
            // Look up the property and ensure it is queryable
            final IProperty prop = this.propSet.get(dt.id);
            if ((prop != null) && prop.isQueryable()) {
                validProperties.add(new DataType(prop.getId(), prop.toAny()));
            } else {
                invalidProperties.add(dt);
            }
        }

        // Store query results back in holder
        configProperties.value = validProperties.toArray(new DataType[validProperties.size()]);

        if (invalidProperties.size() > 0) {
            throw new UnknownProperties(invalidProperties.toArray(new DataType[invalidProperties.size()]));
        }
    }

    /**
     * This returns true if the component constructor has run and the component
     * is not told to release.
     * 
     * @deprecated use started instead
     * @return true if the component is started
     */
    public boolean isRunning() {
        return this._started;
    }

    
    /**
     * Register whichever port is to be used to issue property changes
     */
    public void registerPropertyChangePort(final PropertyEventSupplier _propertyChangePort) {
        this.propertyChangePort = _propertyChangePort;
    }
    
    protected boolean isDisposed() {
        return this.disposed;
    }

    /**
     * This returns the CORBA ORB used by the component
     * 
     * @return the ORB in use
     */
    protected org.omg.CORBA.ORB getOrb() {
        return this.orb;
    }

    /**
     * This returns the RootPOA manager for the ORB in use
     * 
     * @return the POA manager
     */
    protected POA getPoa() {
        return this.poa;
    }
    
    public void setAdditionalParameters(String _softwareProfile) {
        this.softwareProfile = _softwareProfile;
    }
    
    /**
     * This function explicitly activates the given Servant.
     * 
     * @param s the Servant to activate
     * @return the activated CORBA Object
     */
    protected org.omg.CORBA.Object activateObject(final Servant s) {
        try {
            final byte[] oid = this.poa.activate_object(s);
            return this.poa.id_to_reference(oid);
        } catch (final ServantAlreadyActive e) {
            // PASS
        } catch (final WrongPolicy e) {
            // PASS
        } catch (final ObjectNotActive e) {
            // PASS
        }
        return null;
    }

    /**
     * Initializes properties via the execparams.
     * 
     * @param execparams
     */
    protected void initializeProperties(Map<String, String> execparams) {
        for (Map.Entry<String, String> execparam : execparams.entrySet()) {
            IProperty prop = propSet.get(execparam.getKey());
            if (prop != null) {
                prop.fromString(execparam.getValue());
            }
        }
    }

    /**
     * Protected initialize intended only to be used by start_component.
     * 
     * @deprecated use {@link setup(String, String, String, ORB, POA)}
     * @param compId
     * @param orb
     * @throws WrongPolicy 
     * @throws ServantNotActive 
     */
    protected CF.Resource setup(final String compId, final String compName, final ORB orb, final POA poa) throws ServantNotActive, WrongPolicy {
        this.compId = compId;
        this.compName = compName;
        this.orb = orb;
        this.poa = poa;

        ResourcePOATie tie = new ResourcePOATie(this, poa);
        tie._this(orb);
        resource = ResourceHelper.narrow(poa.servant_to_reference((Servant)tie));
        return resource;
    }

    /**
     * Protected initialize intended only to be used by start_component.
     * 
     * @param compId
     * @param compName
     * @param softwareProfile
     * @param orb
     * @param poa
     * @throws WrongPolicy 
     * @throws ServantNotActive 
     */
    protected CF.Resource setup(final String compId, final String compName, final String softwareProfile, final ORB orb, final POA poa) throws ServantNotActive, WrongPolicy {
        CF.Resource result = this.setup(compId, compName, orb, poa);
        this.softwareProfile = softwareProfile;
        return result;
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
    public void setNewLoggingListener( Resource.LoggingListener newListener ) {
	this.logListener = newListener;
    }

    /**
     *  getLogger
     *  
     *  @returns Logger return the logger assigned to this resource
     */
    public Logger     getLogger() {
	return logger;
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
	if ( assignToResource && logger != null ) {
	    //RESOLVE logger = log;
	}
	return log;
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
     *  setLoggingContext
     * 
     *  Set the logging configuration and logging level for this resource.
     *
     * @param String  URL of the logging configuration file to load
     * @param int  oldstyle_loglevel used from command line startup of a resource
     * @param logging.Resource  a content class from the logging.ResourceCtx tree
     */
    public void saveLoggingContext( String logcfg_url, int oldstyle_loglevel, logging.ResourceCtx ctx ) {

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
		    logger.warn( "URL contents could not be resolved, url: " + logcfg_url );
		}
	    }
	}
	catch( Exception e ){
	    logger.warn( "Exception caught during logging configuration using URL, url: "+ logcfg_url );
	}

	if  ( oldstyle_loglevel > -1  ) {
	    logLevel = logging.ConvertLogLevel(oldstyle_loglevel);
	}
	else {
	    // grab root logger's level
	    logLevel = logging.ConvertLog4ToCFLevel( Logger.getRootLogger().getLevel() );
	}
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
	    if ( logger != null ) {
		logger.setLevel(tlevel);
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
	    logLevel = newLogLevel;
	    Level tlevel=Level.INFO;
	    tlevel = logging.ConvertToLog4Level(newLogLevel);	       
	    
	    if ( logger_id != null ){
		Logger logger = Logger.getLogger( logger_id );
		if ( logger != null ) {
		    logger.setLevel( tlevel );
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
	    this.logConfig = config_contents;
	    this.logListener.logConfigChanged( config_contents );
	}
	else {
	    try {
		String newcfg="";
		newcfg = logging.Configure( config_contents, loggingMacros );
		this.logConfig = newcfg;
	    }
	    catch( Exception e ) {
		logger.warn("setLogConfig failed, reason:" + e.getMessage() );
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
		logger.warn( "URL contents could not be resolved, url: " + config_url );
	    }

	}
	catch( Exception e ){
	    logger.warn( "Exception caught during logging configuration using URL, url: "+ config_url );
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

    /**
     * Parse the set of SCA execparam arguments into a Map
     * 
     * @param args as passed in from the command line
     * @return a map of arg/value pairs
     */
    protected static Map<String, String> parseArgs(String[] args) {
        logger.trace("Starting with execparams: " + Arrays.toString(args));
        Map<String, String> result = new Hashtable<String, String>();
        for (int i=0; i < (args.length - 1); i = i +2) {
            result.put(args[i], args[i+1]);
        }
        logger.trace("Starting with execparams: " + result);
        return result;
    }

    /**
     * Start-up function to be used from a main() function.
     * 
     * @param clazz
     * @param args
     * @param builtInORB
     * @param fragSize
     * @param bufSize
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvalidObjectReference 
     * @throws org.omg.CosNaming.NamingContextPackage.InvalidName 
     * @throws CannotProceed 
     * @throws NotFound 
     * @throws WrongPolicy 
     * @throws ServantNotActive 
     */
    public static void start_component(final Class<? extends Resource> clazz, final String[] args, final boolean builtInORB, final int fragSize, final int bufSize) 
	throws InstantiationException, IllegalAccessException, InvalidObjectReference, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, ServantNotActive, WrongPolicy 
    {
        final Properties props = new Properties();
        if (!builtInORB) {
            props.put("org.omg.CORBA.ORBClass", "org.jacorb.orb.ORB");
            props.put("org.omg.CORBA.ORBSingletonClass", "org.jacorb.orb.ORBSingleton");
            props.put("jacorb.fragment_size", Integer.toString(fragSize));
            props.put("jacorb.outbuf_size", Integer.toString(bufSize));
            props.put("jacorb.maxManagedBufSize", "23");
        } else {
            props.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
            props.put("com.sun.CORBA.giop.ORBBufferSize", Integer.toString(bufSize));
        }
        start_component(clazz, args, props);
    }

    /**
     * Start-up function to be used from a main() function.
     * 
     * @param clazz
     * @param args
     * @param builtInORB
     * @param fragSize
     * @param bufSize
     * @throws InstantiationException
     * @throws IllegalAccessException
     * @throws InvalidObjectReference 
     * @throws org.omg.CosNaming.NamingContextPackage.InvalidName 
     * @throws CannotProceed 
     * @throws NotFound 
     * @throws WrongPolicy 
     * @throws ServantNotActive 
     */
    public static void start_component(final Class<? extends Resource> clazz,  final String[] args, final Properties props) 
	throws InstantiationException, IllegalAccessException, InvalidObjectReference, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, ServantNotActive, WrongPolicy 
    {
        final org.omg.CORBA.ORB orb = ORB.init((String[]) null, props);

        // get reference to RootPOA & activate the POAManager
        POA rootpoa = null;
        try {
            final org.omg.CORBA.Object poa = orb.resolve_initial_references("RootPOA");
            rootpoa = POAHelper.narrow(poa);
            rootpoa.the_POAManager().activate();
        } catch (final AdapterInactive e) {
            // PASS
        } catch (final InvalidName e) {
            // PASS
        }

        Map<String, String> execparams = parseArgs(args);

        // Configure log4j from the execparams (or use default settings).
        // TOBERM Resource.configureLogging(execparams, orb);

        NamingContextExt nameContext = null;
        if (execparams.containsKey("NAMING_CONTEXT_IOR")) {
            nameContext = NamingContextExtHelper.narrow(orb.string_to_object(execparams.get("NAMING_CONTEXT_IOR")));
        }

        String identifier = null;
        if (execparams.containsKey("COMPONENT_IDENTIFIER")) {
            identifier = execparams.get("COMPONENT_IDENTIFIER");
        }

        String nameBinding = null;
        if (execparams.containsKey("NAME_BINDING")) {
            nameBinding = execparams.get("NAME_BINDING");
        }

        String dom_path = "";
        if (execparams.containsKey("DOM_PATH")) {
            dom_path = execparams.get("DOM_PATH");
        }

        String logcfg_uri = "";
        if (execparams.containsKey("LOGGING_CONFIG_URI")) {
            logcfg_uri = execparams.get("LOGGING_CONFIG_URI");
        }

	int debugLevel = -1; // use logging config properties for level
	if (execparams.containsKey("DEBUG_LEVEL")) {
	    debugLevel = Integer.parseInt(execparams.get("DEBUG_LEVEL"));
	}

	if ( debugLevel > 3 ) {
	    System.out.println("Resource Args: " );
	    System.out.println("                NAME_BINDING:"+ nameBinding );
	    System.out.println("                COMPONENT_IDENTIFIER:"+ identifier );
	    System.out.println("                NAMING_CONTEXT_IOR:"+ nameContext );
	    System.out.println("                DOM_PATH:"+ dom_path );
	    System.out.println("                LOG_CONFIG_URI:"+ logcfg_uri );
	    System.out.println("                DEBUG_LEVEL:"+ debugLevel );
	}

        String profile = "";
        if (execparams.containsKey("PROFILE_NAME")) {
            profile = execparams.get("PROFILE_NAME");
        }

        if ((nameContext == null) || (nameBinding == null)) {
            if ((!Arrays.toString(args).contains("-i")) && (!Arrays.toString(args).contains("--interactive"))) {
                System.out.println("usage: "+clazz+" [options] [execparams]\n");
                System.out.println("The set of execparams is defined in the .prf for the component");
                System.out.println("They are provided as arguments pairs ID VALUE, for example:");
                System.out.println("     "+clazz+" INT_PARAM 5 STR_PARAM ABCDED\n");
                System.out.println("Options:");
                System.out.println("     -i,--interactive           Run the component in interactive test mode\n");
                System.exit(-1);
            }
        }

	logging.ComponentCtx ctx = new	logging.ComponentCtx( nameBinding, identifier, dom_path );
	logging.Configure( logcfg_uri, debugLevel, ctx );

        final Resource resource_i = clazz.newInstance();
        final CF.Resource resource = resource_i.setup(identifier, nameBinding, profile, orb, rootpoa);
        resource_i.initializeProperties(execparams);

	resource_i.saveLoggingContext( logcfg_uri, debugLevel, ctx );

        if ((nameContext != null) && (nameBinding != null)) {
            nameContext.rebind(nameContext.to_name(nameBinding), resource);
        } else {
            // Print out the IOR so that someone can debug against the component
            System.out.println("The IOR for your component is:\n" + orb.object_to_string(resource));
        }

        // Create a thread that watches for the resource to be deactivated
        Thread shutdownWatcher = new Thread(new Runnable() {
                public void run() {
                    resource_i.waitDisposed();
                    shutdownORB(orb);
                }
            });

        shutdownWatcher.start();

        orb.run();

        logger.trace("Waiting for shutdown watcher to join");
        try {
            shutdownWatcher.join();
        } catch (InterruptedException e) {
            // PASS
        }

        // Destroy the ORB, otherwise the JVM shutdown will take an unusually
        // long time (~300ms).
        orb.destroy();

        // Shut down native ORB, if it's running
        omnijni.ORB.shutdown();

        logger.debug("Goodbye!");
    }

    protected void waitDisposed() {
        synchronized(this) {
            while (!this.isDisposed()) {
                try {
                    this.wait();
                } catch (InterruptedException e) {
                    // PASS
                }
            }
        }
    }

    /**
     * Configure log4j for use in a component. The order of preference for settings is:
     *
     *   1. LOGGING_CONFIG_URI execparam
     *   2. DEBUG_LEVEL execparam
     *   3. Default debug level (INFO)
     *
     * @deprecated use {@link logging#Configure(String, int)}
     * @param execparams Map of component execparam values
     * @param orb CORBA ORB instance for contacting SCA FileSystem (if LOGGING_CONFIG_URI is an SCA URI)
     */
    protected static void configureLogging(final Map<String, String> execparams, final org.omg.CORBA.ORB orb) {
	// Sets up the logging
	String loggingConfigURI = null;
	if (execparams.containsKey("LOGGING_CONFIG_URI")) {
	    loggingConfigURI = execparams.get("LOGGING_CONFIG_URI");
	    if (loggingConfigURI.indexOf("file://") != -1){
		int startIndex = loggingConfigURI.indexOf("file://") + 7;
		PropertyConfigurator.configure(loggingConfigURI.substring(startIndex));
	    }else if (loggingConfigURI.indexOf("sca:") != -1){
		int startIndex = loggingConfigURI.indexOf("sca:") + 4;
		String localFile = Resource.getLogConfig(loggingConfigURI.substring(startIndex), orb);
		File testLocalFile = new File(localFile);
		if (localFile.length() > 0 && testLocalFile.exists()){
		    PropertyConfigurator.configure(localFile);
		}
	    }
	} else {
	    int debugLevel = 3; // Default level is INFO
	    if (execparams.containsKey("DEBUG_LEVEL")) {
		debugLevel = Integer.parseInt(execparams.get("DEBUG_LEVEL"));
	    }

	    logging.ConfigureDefault();

	    logging.SetLevel( null, debugLevel );
	}
    }

    /**
     * This function returns the filename of the local logging configuration file given an SCA FileSystem logging file.
     * 
     * @param uri string containing SCA File System filename for logging configuration file
     * @param orb CORBA ORB instance for contacting SCA FileSystem
     * @return the string reprenting the local logging configuration filename
     */
    protected static String getLogConfig(String uri, org.omg.CORBA.ORB orb) {
	String localPath = "";

	int fsPos = uri.indexOf("?fs=");
	if (fsPos == -1) {
	    return localPath;
	}

	String IOR = uri.substring(fsPos + 4);
	org.omg.CORBA.Object obj = orb.string_to_object(IOR);
	if (obj == null) {
	    return localPath;
	}

	CF.FileSystem fileSystem = CF.FileSystemHelper.narrow(obj);
	if (fileSystem == null) {
	    return localPath;
	}

	String remotePath = uri.substring(0, fsPos);
	CF.OctetSequenceHolder data = new CF.OctetSequenceHolder ();
	try {
	    CF.File remoteFile = fileSystem.open(remotePath, true);
	    int size = remoteFile.sizeOf();
	    remoteFile.read(data, size);

	    String tempPath = remotePath;
	    int slashPos = remotePath.lastIndexOf('/');
	    if (slashPos != -1) {
		tempPath = tempPath.substring(slashPos + 1);
	    }
            
	    FileOutputStream localFile = new FileOutputStream(tempPath);
	    localFile.write(data.value);
	    localPath = tempPath;
	    localFile.close();
	    return localPath;
	} catch (Exception e){
	    return localPath;
	}
    }


    /**
     * This function returns the filename of the local logging configuration file given an SCA FileSystem logging file.
     * 
     * @param uri string containing SCA File System filename for logging configuration file
     * @return the string reprenting the local logging configuration filename
     */
    protected String getLogConfig(String uri) {
	return Resource.getLogConfig(uri, this.orb);
    }

    protected static void shutdownORB(final org.omg.CORBA.ORB orb)
    {
        // Disable the default ORB logger to avoid dumping warnings
        // on shutdown that are unavoidable and otherwise harmless
        disableCORBALogging();

        logger.trace("Shutting down orb");
        orb.shutdown(true);
    }

    private static void disableCORBALogging()
    {
        java.util.logging.Logger.getLogger("javax.enterprise.resource.corba").setLevel(java.util.logging.Level.OFF);
    }
}
