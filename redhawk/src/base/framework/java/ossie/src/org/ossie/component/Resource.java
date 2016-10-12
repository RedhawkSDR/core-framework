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
import java.util.UUID;
import java.util.Set;


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
import org.omg.CORBA.Any;
import org.omg.CORBA.UserException;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NameComponent;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;
import org.omg.CosNaming.NamingContext;
import org.omg.CosNaming.NamingContextHelper;
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
import org.ossie.properties.PropertyListener;
import org.ossie.properties.AnyUtils;
import org.ossie.properties.StructProperty;
import org.ossie.properties.StructDef;
import org.ossie.logging.logging;
import org.ossie.redhawk.DomainManagerContainer;
import org.ossie.corba.utils.*;

import CF.AggregateDevice;
import CF.AggregateDeviceHelper;
import CF.DataType;
import CF.Device;
import CF.DeviceHelper;
import CF.ApplicationRegistrar;
import CF.ApplicationRegistrarHelper;
import CF.DeviceManager;
import CF.DeviceManagerHelper;
import CF.InvalidObjectReference;
import CF.LogLevels;
import CF.PortPOA;
import CF.PropertiesHolder;
import CF.PropertiesHelper;
import CF.ResourceHelper;
import CF.ResourceOperations;
import CF.ResourcePOA;
import CF.ResourcePOATie;
import CF.UnknownProperties;
import CF.UnknownIdentifier;
import CF.InvalidIdentifier;
import CF.LifeCyclePackage.InitializeError;
import CF.LifeCyclePackage.ReleaseError;
import CF.PortSetPackage.PortInfoType;
import CF.PortSupplierPackage.UnknownPort;
import CF.PropertyEmitterPackage.AlreadyInitialized;
import CF.PropertySetPackage.InvalidConfiguration;
import CF.PropertySetPackage.PartialConfiguration;
import CF.ResourcePackage.StartError;
import CF.ResourcePackage.StopError;
import CF.TestableObjectPackage.UnknownTest;

public abstract class Resource extends Logging implements ResourceOperations, Runnable { // SUPPRESS CHECKSTYLE Name

    /**
     *  LoggingListener 
     *  This interface allows developers to modify the normal behavior when a request to 
     *  change the logging level or configuration context.
     *
     *  Developers can provide an interface to the Resource through the 
     *  setNewLoggingListener method.
     */
    public interface LoggingListener extends Logging.ConfigurationChangeListener {
    };


    class PropertyChangeProcessor implements ThreadedComponent, Runnable { 

        private Resource rsc = null;
        private ProcessThread _processThread=null;

        public PropertyChangeProcessor( Resource inRsc ) {
            rsc = inRsc;
            this._processThread = new ProcessThread(this);
        }

        public void run () {
            this._processThread.run();
        }

        public void start () {
            this._processThread.start();
        }
        public void stop () {
            this._processThread.stop();
        }

        public int process () {
            if ( rsc != null ) {
                return rsc._propertyChangeServiceFunction();
            }
            else {
                return NOOP;
            }
        }
        
        public float getThreadDelay (){
            return this._processThread.getDelay();
        }

        public void setThreadDelay (float delay) {
            this._processThread.setDelay(delay);
        }
    }


    public  static Logger logger = Logger.getLogger(Resource.class.getName());

    public  static logging.ResourceCtx loggerCtx = null;
    
    protected CF.Resource resource;
    
    protected DomainManagerContainer _domMgr = null;

    protected org.ossie.events.Manager _ecm = null;
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
    /** Map of port descriptions for this resource */
    protected HashMap<String, String> portDescriptions;
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
    /** flag for whether initializeProperties has been called */
    private boolean _propertiesInitialized = false;

    protected Hashtable< String, PropertyChangeRec >   _propChangeRegistry;
    protected PropertyChangeProcessor                  _propChangeProcessor;
    protected Thread                                   _propChangeThread;

    /**
     * Constructor intended to be used by start_component.
     */
    public Resource() {

        // configure Logging resource context
        super( logger, Resource.class.getName() );

        this.compId = "";
        this.compName = "";
        this._started = false;
        this.propertyChangePort = null;
        this.ports = new Hashtable<String, org.omg.CORBA.Object>();
        this.portServants = new Hashtable<String, Servant>();
        this.portObjects = new Hashtable<String, Object>();
        this.nativePorts = new Hashtable<String, omnijni.Servant>();
        this.propSet = new Hashtable<String, IProperty>();
		this.portDescriptions = new HashMap<String, String>();

        this._propChangeRegistry = new Hashtable< String, PropertyChangeRec >();
        this._propChangeProcessor = new PropertyChangeProcessor(this);
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

    public void addPort(String name, String description, Object object) {
		this.portDescriptions.put(name, description);
        addPort(name, object);
    }

    protected void addPort(String name, String description, omnijni.Servant servant) {
		this.portDescriptions.put(name, description);
        addPort(name, servant);
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
     * REDHAWK constructor
     */
    public void constructor()
    {
    }

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

            try {
                this.constructor();
            } catch (final Throwable exc) {
                final String message = exc.getMessage(); 
                logger.error("initialize(): " + message);
                throw new InitializeError(new String[]{message});
            }
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
            this.stopPropertyChangeMonitor();
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
                // Might have to do something different here?
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

    public void initializeProperties(final DataType[] ctorProperties) throws AlreadyInitialized, InvalidConfiguration, PartialConfiguration {
        logger.trace("initializeProperties() - star ");

        // Disallow multiple calls
        if (this._propertiesInitialized) {
            throw new AlreadyInitialized();
        }
        this._propertiesInitialized = true;

        // Ensure there's something to do
        if (ctorProperties.length == 0) {
            return;
        }

        final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();
        for (final DataType dt : ctorProperties) {
            // Look up the property and ensure it is configurable
            final IProperty prop = this.propSet.get(dt.id);
            if (prop == null) {
                invalidProperties.add(dt);
                continue;
            }

            try {
                // See if the value has changed
                if (AnyUtils.compareAnys(prop.toAny(), dt.value, "ne")) {
                    // Update the value on the property, which may trigger a
                    // callback.
                    prop.configureNoCallbacks(dt.value);
                }
                logger.trace("Construct property: " + prop);
            } catch (Throwable t) {
                logger.error("Unable to construct property " + dt.id + ": " + t.getMessage());
                invalidProperties.add(dt);
            }
        }
        
        if (invalidProperties.size() == ctorProperties.length) {
            throw new InvalidConfiguration("Error constructing component", invalidProperties.toArray(new DataType[0]));
        } else if (invalidProperties.size() > 0) {
            throw new PartialConfiguration(invalidProperties.toArray(new DataType[0]));
        }

        logger.trace("initializeProperties() - end");
    }

    public PortInfoType[] getPortSet () {
        final ArrayList<PortInfoType> ports = new ArrayList<PortInfoType>();

        for (String name : this.nativePorts.keySet()) {
            PortInfoType info = new PortInfoType();
            try {
                info.obj_ptr = getPort(name);
            } catch (UnknownPort ex) {
                continue;
            }
            info.name = name;
            omnijni.Servant port = this.nativePorts.get(name);
            if (port instanceof PortBase) {
                PortBase cast = (PortBase)port;
                info.repid = cast.getRepid();
                info.direction = cast.getDirection();
            } else {
                info.repid = "IDL:CORBA/Object:1.0";
                info.direction = "direction";
            }
            if (this.portDescriptions.containsKey(name)) {
                info.description = this.portDescriptions.get(name);
            } else {
                info.description = "";
            }
            ports.add(info);
        }
        for (String name : this.portServants.keySet()) {
            PortInfoType info = new PortInfoType();
            try {
                info.obj_ptr = getPort(name);
            } catch (UnknownPort ex) {
                continue;
            }
            info.name = name;
            Servant port = this.portServants.get(name);
            if (port instanceof PortBase) {
                PortBase cast = (PortBase)port;
                info.repid = cast.getRepid();
                info.direction = cast.getDirection();
            } else {
                info.repid = "IDL:CORBA/Object:1.0";
                info.direction = "direction";
            }
            if (this.portDescriptions.containsKey(name)) {
                info.description = this.portDescriptions.get(name);
            } else {
                info.description = "";
            }
            ports.add(info);
        }

        return ports.toArray(new PortInfoType[0]);
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
                Any value_before = prop.toAny();
                // Update the value on the property, which may trigger a
                // callback.
                prop.configure(dt.value);
                if (AnyUtils.compareAnys(value_before, prop.toAny(), "eq")) {
                    logger.debug("Value has not changed on configure for property " + dt.id + ". Not triggering callback");
                } else {
                    // The property value changed.
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
                    if (prop instanceof StructProperty) {
                        Any structAny = ORB.init().create_any();
                        final ArrayList<DataType> structProps = new ArrayList<DataType>();
                        StructDef def = (StructDef)((StructProperty)prop).getValue();
                        for (final IProperty p : def.getElements()) {
			    if (p.isSet()) {
			        structProps.add(new DataType(p.getId(), p.toAny()));
			    }
                        }
			PropertiesHelper.insert(structAny, structProps.toArray(new DataType[structProps.size()]));
			props.add(new DataType(prop.getId(), structAny));
                    } else {
                        props.add(new DataType(prop.getId(), prop.toAny()));
                    }
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
                if (prop instanceof StructProperty) {
		    Any structAny = ORB.init().create_any();
                    final ArrayList<DataType> structProps = new ArrayList<DataType>();
                    StructDef def = (StructDef)((StructProperty)prop).getValue();
                    for (final IProperty p : def.getElements()) {
			if (p.isSet()) {
			    structProps.add(new DataType(p.getId(), p.toAny()));
			}
                    }
		    PropertiesHelper.insert(structAny, structProps.toArray(new DataType[structProps.size()]));
		    validProperties.add(new DataType(prop.getId(), structAny));
                } else {
                    validProperties.add(new DataType(prop.getId(), prop.toAny()));
                }
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
     * {@inheritDoc}
     */
    public String registerPropertyListener(final org.omg.CORBA.Object listener, String[] prop_ids, float interval)
        throws CF.UnknownProperties, CF.InvalidObjectReference
    {
        logger.trace("registerPropertyListener - start ");
        ArrayList<String> pids = new ArrayList<String>();
        final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();
	String reg_id;
        synchronized(this) {
            // For queries of zero length, return all id/value pairs in propertySet
            if (prop_ids.length == 0) {
                logger.trace("registering all properties...");
                for (final IProperty prop : this.propSet.values()) {
                    if (prop.isQueryable()) {
                        logger.debug("..... property:" + prop.getId());
                        pids.add(prop.getId());
                    }
                }
            }
            else {
                // For queries of length > 0, return all requested pairs in propertySet
                logger.trace("registering fixed property: N:" + prop_ids.length);
                // Return values for valid queries in the same order as requested
                for (final String id : prop_ids) {
                    // Look up the property and ensure it is queryable
                    final IProperty prop = this.propSet.get(id);
                    if ((prop != null) && prop.isQueryable()) {
                        logger.debug("..... property:" + id);
                        pids.add(prop.getId());
                    } else {
                        DataType dt = new DataType(id, null);
                        invalidProperties.add(dt);
                    }
                }
            }

            if (invalidProperties.size() > 0) {
                throw new UnknownProperties(invalidProperties.toArray(new DataType[invalidProperties.size()]));
            }

            logger.trace("PropertyChangeListener: register N properties: " + pids.size());
            PropertyChangeRec prec = new PropertyChangeRec( listener, 
                                                            compId,
                                                            interval,
                                                            pids,
                                                            this.propSet );
            // check if our listener is valid
            if ( prec.pcl == null ) {
                logger.error("PropertyChangeListener: caller provided invalid listener interface ");
                prec = null;
                throw new CF.InvalidObjectReference();
            }

            // Add the registry record to our map
            logger.debug("registerPropertyListener REGISTERING id-s/regid: " + pids.size() + "/" + prec.regId );
            this._propChangeRegistry.put( prec.regId, prec );
	    reg_id = prec.regId;
        
            // start monitoring thread if not started
            if ( this._propChangeThread == null )  {
                logger.debug("registerPropertyListener - First registration ... starting monitoring thread ");
                this._propChangeProcessor.start();
                this._propChangeThread = new Thread( this._propChangeProcessor );
                this._propChangeThread.start();
            }
        }
        logger.trace("registerPropertyListener - end");
        return reg_id;
    }


    public void unregisterPropertyListener(final String reg_id)
        throws CF.InvalidIdentifier
    {
        logger.trace("unregisterPropertyListener - start ");
        synchronized(this) {
            PropertyChangeRec prec = this._propChangeRegistry.get(reg_id);
            if ( prec != null ) {
                logger.trace("unregisterPropertyListener - Remove registration " +reg_id );
                for ( String id : prec.props.keySet() ) {
                    final IProperty prop = this.propSet.get(id);
                    if ( prop != null ) {
                        @SuppressWarnings("unchecked")
                        final PropertyListener<Object> pcl = prec.props.get(id);
                        if ( pcl != null )  {
                            prop.removeObjectListener( pcl );
                        }
                    }
                }

                this._propChangeRegistry.remove(reg_id);
                logger.debug("unregisterPropertyListener - UNREGISTER  REG-ID:" + reg_id );
                if ( this._propChangeRegistry.size() == 0 ) {
                    logger.debug("unregisterPropertyListener - No more registrants... stopping thread ");
                    this.stopPropertyChangeMonitor();
                    this._propChangeThread=null;
                }
            }
            else {
                throw new  CF.InvalidIdentifier();       
           }
        }
        return;
    }

    public void stopPropertyChangeMonitor() {

        if ( _propChangeProcessor != null )  {
            _propChangeProcessor.stop();
            try {
                if ( _propChangeThread != null ) {
                    _propChangeThread.join();
                }
            }
            catch( InterruptedException ex ) {
            }
         }
    }

    /**
     * Perform reporting portion of PropertyChangeListeners that are registered with this resource.
     * After the requested interval has expired report and changes since the last reporting cycle.
     * Notifications are sent out via EventChannel or implementors of PropertyChangeListern interface
     * is not told to release.
     * 
     * @return NOOP informs calling thread controller to delay before next cycle
     */
    protected int  _propertyChangeServiceFunction() {

        logger.trace("_propertyChangeServiceFunction ... start ");
        long  delay=0;
        synchronized(this) {
            // for each registration record
            for ( PropertyChangeRec prec : _propChangeRegistry.values() ) {

                // get current time stamp.... and duration since last check
                long now = System.currentTimeMillis();
                long dur = prec.expiration - now;

                logger.debug( "Resource::_propertyChangeServiceFunction ... reg_id/interval :" + prec.regId + "/" + prec.reportInterval + "  expiration=" + dur );
                // determine if time has expired
		if ( dur <= 0 ) {

		    // For queries of length > 0, return all requested pairs in propertySet
		    final ArrayList<DataType> rpt_props = new ArrayList<DataType>();
                    // check all registered properties for changes
		    for (Map.Entry<String, PCL_Callback> iter : prec.props.entrySet() ) {

			String pid = iter.getKey();
		        PCL_Callback pcb = iter.getValue();
                        logger.trace("   Check Property/Set " + pid + "/" + pcb.isSet() );
                        // check if property changed
			if ( pcb.isSet() == true ) {
			    final IProperty prop = this.propSet.get(pid);
			    if (prop != null) {
				rpt_props.add(new DataType(pid, prop.toAny()));
			    } 
			}

                        // reset change indicator for next reporting cycle
                        pcb.reset();
		    }

                    // publish changes to listener
                    if ( rpt_props.size() > 0 && prec.pcl != null )  {
                        logger.debug("   Notify PropertyChangeListener ...size/reg :" + rpt_props.size() + "/" + prec.regId );
                        DataType [] rprops = rpt_props.toArray( new DataType[ rpt_props.size() ] );
                        if ( prec.pcl.notify( prec, rprops ) != 0 ) {
                            logger.error("Publishing changes to PropertyChangeListener FAILED, reg_id:" +  prec.regId );
                            // probably should mark for removal... if last one then stop the thread...
                        }
                    }
                        
                    // reset time indicator 
                    prec.expiration =  System.currentTimeMillis() + prec.reportInterval;
                    dur = prec.reportInterval;
                }

		
                // find smallest increment of time to wait
		if ( delay == 0 ) { delay=dur; }
		logger.trace( "  Test for delay/duration (millisecs) ... :"  + delay + "/" + dur );
		if ( dur > 0 )  { delay = Math.min( delay, dur ); }
		logger.trace( "   Minimum  delay (millisecs) ... :" + delay ); 

	    }  // end synchronized

            if ( delay > 0 ) {
		logger.debug( "....Set monitoring thread delay (millisecs) ... :"  + delay );
		_propChangeProcessor.setThreadDelay( delay/1000.0f );
	    }
        }

        logger.trace("_propertyChangeServiceFunction ... end ");
        return ThreadedComponent.NOOP;
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
    public org.omg.CORBA.ORB getOrb() {
        return this.orb;
    }

    /**
     * This returns the RootPOA manager for the ORB in use
     * 
     * @return the POA manager
     */
    public POA getPoa() {
        return this.poa;
    }
    
    /**
     * This function is used by derived classes to set a pointer to the 
     * Domain Manager and Application
     * 
     * @param ApplicationRegistrarIOR IOR to either the Application Registrar or Naming Context
     */
    public void setAdditionalParameters(final String ApplicationRegistrarIOR, String nic) {
        final org.omg.CORBA.Object obj = this.orb.string_to_object(ApplicationRegistrarIOR);
        ApplicationRegistrar appReg = null;
        try {
            appReg = ApplicationRegistrarHelper.narrow(obj);
        } catch (Exception e) {}
        if (appReg != null) {
            if (appReg.domMgr()!=null) {
                this._domMgr = new DomainManagerContainer(appReg.domMgr());
                this.logger.info("setAdditionalParameters domain: " + this._domMgr.getRef().name() );
            }

            if ( this._domMgr != null ) {
                try {
                    this._ecm = org.ossie.events.Manager.GetManager(this);
                }catch( org.ossie.events.Manager.OperationFailed e){
                    logger.warn("Unable to resolve EventChannelManager");
                }
            }
        }
    }
    
    public DomainManagerContainer getDomainManager() {
        return this._domMgr;
    }


    public org.ossie.events.Manager getEventChannelManager() {
        return this._ecm;
    }
    

   public void saveLoggingContext( String logcfg_url, 
                                    int oldstyle_loglevel, 
                                   logging.ResourceCtx ctx ) {
       super.saveLoggingContext( logcfg_url, oldstyle_loglevel, ctx, this.getEventChannelManager () );
   }


    public void setNewLoggingListener( LoggingListener newListener ) {
        super.setNewLoggingListener( newListener );
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
        // initialize library's ORB reference 
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, props );

        final POA rootpoa  = org.ossie.corba.utils.RootPOA();

        Map<String, String> execparams = parseArgs(args);

        // Configure log4j from the execparams (or use default settings).
        // TOBERM Resource.configureLogging(execparams, orb);

        NamingContext nameContext = null;
        NamingContextExt nameContextExt = null;
        ApplicationRegistrar applicationRegistrar = null;
        if (execparams.containsKey("NAMING_CONTEXT_IOR")) {
            try {
                final org.omg.CORBA.Object tmp_obj = orb.string_to_object(execparams.get("NAMING_CONTEXT_IOR"));
                try {
                    applicationRegistrar = ApplicationRegistrarHelper.narrow(tmp_obj);
                } catch (Exception e) {}
                nameContext = NamingContextHelper.narrow(tmp_obj);
                try {
                    nameContextExt = NamingContextExtHelper.narrow(tmp_obj);
                } catch (Exception e) { // the application registrar inherits from naming context, not naming context ext
                }
            } catch (Exception e) {
                System.out.println(e);
            }
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
        String nic = "";
        if (execparams.containsKey("NIC")) {
            nic = execparams.get("NIC");
        }
        resource_i.setAdditionalParameters(execparams.get("NAMING_CONTEXT_IOR"), nic);
        resource_i.initializeProperties(execparams);

        // save logging context used by the resource... 
	resource_i.saveLoggingContext( logcfg_uri, debugLevel, ctx );

        if ((nameContext != null) && (nameBinding != null)) {
            if (applicationRegistrar != null) {
                try {
                    applicationRegistrar.registerComponent(nameBinding, resource);
                } catch (Exception e) {
                    System.out.println("Unable to register "+nameBinding);
                    System.out.println(e);
                }
            } else {
                String[] names = nameBinding.split("/");
                ArrayList<org.omg.CosNaming.NameComponent> name_to_bind = new ArrayList<org.omg.CosNaming.NameComponent>();
                for (String name : names) {
                    org.omg.CosNaming.NameComponent Cos_name = new org.omg.CosNaming.NameComponent(name, "");
                    name_to_bind.add(Cos_name);
                }
                org.omg.CosNaming.NameComponent[] name_binding_array = new org.omg.CosNaming.NameComponent[0];
                nameContext.rebind(name_to_bind.toArray(name_binding_array), resource);
            }
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
