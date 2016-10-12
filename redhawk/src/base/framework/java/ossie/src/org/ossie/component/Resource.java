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
import org.ossie.PropertyContainer;
import org.ossie.events.PropertyEventSupplier;
import org.ossie.properties.IProperty;
import org.ossie.properties.SimpleProperty;
import org.ossie.properties.AnyUtils;

import CF.AggregateDevice;
import CF.AggregateDeviceHelper;
import CF.DataType;
import CF.Device;
import CF.DeviceHelper;
import CF.DeviceManager;
import CF.DeviceManagerHelper;
import CF.InvalidObjectReference;
import CF.PortPOA;
import CF.PropertiesHolder;
import CF.ResourceHelper;
import CF.ResourceOperations;
import CF.ResourcePOA;
import CF.ResourcePOATie;
import CF.UnknownProperties;
import CF.LifeCyclePackage.InitializeError;
import CF.LifeCyclePackage.ReleaseError;
import CF.PortSupplierPackage.UnknownPort;
import CF.PropertySetPackage.InvalidConfiguration;
import CF.PropertySetPackage.PartialConfiguration;
import CF.ResourcePackage.StartError;
import CF.ResourcePackage.StopError;
import CF.TestableObjectPackage.UnknownTest;

public abstract class Resource implements ResourceOperations, Runnable { // SUPPRESS CHECKSTYLE Name
    public final static Logger logger = Logger.getLogger(Resource.class.getName());
    
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
        final ArrayList<DataType> validProperties = new ArrayList<DataType>();
        final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();
        
        try {
            c_validate(configProperties, validProperties, invalidProperties);
            if (configProperties.length == 0) {
                return;
            }

            for (final DataType validProp : validProperties) {
                for (final IProperty prop : this.propSet.values()) {
                    // make sure that the property ids match
                    if (prop.getId().equals(validProp.id)) {
                        
                        // see if the value has changed
                        if (AnyUtils.compareAnys(prop.toAny(),(validProp.value), "ne")) {
                            
                            // update the value on the property
                            prop.fromAny(validProp.value);
                            
                            // check to see if this property should issue property change events
                            if (prop.isEventable()) {
                                
                                // make sure that the port exists
                                if (this.propertyChangePort != null) {
                                    this.propertyChangePort.sendPropertyEvent(validProp.id);
                                }
                            }
                        }
                        logger.trace("Configured property: " + prop);
                    }
                }
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }
        
        if ((validProperties.size() == 0) && (invalidProperties.size() != 0)) {
            throw new InvalidConfiguration("Error configuring component", invalidProperties.toArray(new DataType[0]));
        } else if ((validProperties.size() != 0) && (invalidProperties.size() != 0)) {
            throw new PartialConfiguration();
        }
    }

    /**
     * {@inheritDoc}
     */
    public void query(final PropertiesHolder configProperties) throws UnknownProperties {
        logger.trace("query()");
        // for queries of zero length, return all id/value pairs in propertySet
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

        // for queries of length > 0, return all requested pairs in propertySet
        final ArrayList<DataType> validProperties = new ArrayList<DataType>();
        final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();

        q_validate(configProperties.value, validProperties, invalidProperties);

        //returns values for valid queries in the same order as requested
        for (final DataType valid : validProperties) {
            for (int j = 0; j < configProperties.value.length; j++) {
                if (configProperties.value[j].id.equals(valid.id)) {
                    configProperties.value[j].value = this.propSet.get(valid.id).toAny();
                    break;
                }
            }
        }

        if (invalidProperties.size() != 0) {
            throw new UnknownProperties(invalidProperties.toArray(new DataType[invalidProperties.size()]));
        }
    }

    /**
     * This method checks if the given configProperties are valid for this
     * component for the query operation.
     * 
     * @param configProperties the properties to validate
     * @param validProperties storage for the valid properties
     * @param invalidProperties storage for the invalid properties
     */
    private void q_validate(final DataType[] configProperties, final ArrayList<DataType> validProperties, final ArrayList<DataType> invalidProperties) {
        for (final DataType p : configProperties) {
            boolean success = false;
            for (final IProperty prop : this.propSet.values()) {
                if (prop.getId().equals(p.id)) {
                    if (prop.isQueryable()){
                        success = true;
                    }
                    break;
                }
            }

            if (success) {
                validProperties.add(p);
            } else {
                invalidProperties.add(p);
            }
        }
    }
    
    /**
     * This method checks if the given configProperties are valid for this
     * component for the configure operation.
     * 
     * @param configProperties the properties to validate
     * @param validProperties storage for the valid properties
     * @param invalidProperties storage for the invalid properties
     */
    private void c_validate(final DataType[] configProperties, final ArrayList<DataType> validProperties, final ArrayList<DataType> invalidProperties) {
        for (final DataType p : configProperties) {
            boolean success = false;
            for (final IProperty prop : this.propSet.values()) {
                if (prop.getId().equals(p.id)) {
                    if (prop.isConfigurable()){
                        success = true;
                    }
                    break;
                }
            }

            if (success) {
                validProperties.add(p);
            } else {
                invalidProperties.add(p);
            }
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
    public static void start_component(final Class clazz, final String[] args, final boolean builtInORB, final int fragSize, final int bufSize) 
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
    public static void start_component(final Class clazz,  final String[] args, final Properties props) 
    throws InstantiationException, IllegalAccessException, InvalidObjectReference, NotFound, CannotProceed, org.omg.CosNaming.NamingContextPackage.InvalidName, ServantNotActive, WrongPolicy 
    {
        if (! Resource.class.isAssignableFrom(clazz)) {
            throw new IllegalArgumentException("start_component() can only start classes of type org.ossie.component.Resource");
        }
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

        final Resource resource_i = (Resource)clazz.newInstance();
        final CF.Resource resource = resource_i.setup(identifier, nameBinding, orb, rootpoa);
        resource_i.initializeProperties(execparams);

        if ((nameContext != null) && (nameBinding != null)) {
            nameContext.rebind(nameContext.to_name(nameBinding), resource);
        } else {
            // Print out the IOR so that someone can debug against the component
            System.out.println("The IOR for your component is:\n" + orb.object_to_string(resource));
        }

        String loggingConfigURI = null;
        if (execparams.containsKey("LOGGING_CONFIG_URI")) {
            loggingConfigURI = execparams.get("LOGGING_CONFIG_URI");
            if (loggingConfigURI.indexOf("file://") != -1){
                int startIndex = loggingConfigURI.indexOf("file://") + 7;
                PropertyConfigurator.configure(loggingConfigURI.substring(startIndex));
            }else if (loggingConfigURI.indexOf("sca:") != -1){
                int startIndex = loggingConfigURI.indexOf("sca:") + 4;
                String localFile = resource_i.getLogConfig(loggingConfigURI.substring(startIndex));
                File testLocalFile = new File(localFile);
                if (localFile.length() > 0 && testLocalFile.exists()){
                    PropertyConfigurator.configure(localFile);
                }
            }
        }else{
            // If no logging config file, then set up logging using DEBUG_LEVEL exec param
            int debugLevel = 3; // Default level is INFO
            if (execparams.containsKey("DEBUG_LEVEL")) {
                debugLevel = Integer.parseInt(execparams.get("DEBUG_LEVEL"));
            }
            LogManager.getLoggerRepository().resetConfiguration();
            Logger root = Logger.getRootLogger();
            Layout layout = new PatternLayout("%p:%c - %m%n");
            Appender appender = new ConsoleAppender(layout);
            root.addAppender(appender);
            if (debugLevel == 0) {
                root.setLevel(Level.FATAL);
            } else if (debugLevel == 1) {
                root.setLevel(Level.ERROR);
            } else if (debugLevel == 2) {
                root.setLevel(Level.WARN);
            } else if (debugLevel == 3) {
                root.setLevel(Level.INFO);
            } else if (debugLevel == 4) {
                root.setLevel(Level.DEBUG);
            } else if (debugLevel >= 5) {
                root.setLevel(Level.ALL);
            }
        }

        // Create a thread that watches for the resource to be deactivated
        Thread shutdownWatcher = new Thread(new Runnable() {

            public void run() {
                while (!resource_i.isDisposed()) {
                    try {
                        Thread.sleep(500);
                    } catch (InterruptedException e) {
                        // PASS
                    }
                }
                logger.trace("Shutting down orb");
                orb.shutdown(true);
            }
        });

        shutdownWatcher.start();

        orb.run();
        logger.trace("Waiting for shutdown watcher to join");
        try {
            shutdownWatcher.join(1000);
        } catch (InterruptedException e) {
            // PASS
        }
        logger.debug("Goodbye!");
    }

    /**
     * This function returns the filename of the local logging configuration file given an SCA FileSystem logging file.
     * 
     * @param uri string containing SCA File System filename for logging configuration file
     * @return the string reprenting the local logging configuration filename
     */
    protected String getLogConfig(String uri) {
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
}
