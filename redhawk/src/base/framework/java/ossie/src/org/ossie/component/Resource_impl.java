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

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Properties;

import org.omg.CORBA.ORB;
import org.omg.CORBA.UserException;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import org.omg.PortableServer.Servant;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;
import org.omg.PortableServer.POAPackage.ObjectNotActive;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.ossie.PropertyContainer;

import CF.DataType;
import CF.PortPOA;
import CF.PropertiesHolder;
import CF.ResourcePOA;
import CF.UnknownProperties;
import CF.LifeCyclePackage.InitializeError;
import CF.LifeCyclePackage.ReleaseError;
import CF.PortSupplierPackage.UnknownPort;
import CF.PropertySetPackage.InvalidConfiguration;
import CF.PropertySetPackage.PartialConfiguration;
import CF.ResourcePackage.StartError;
import CF.ResourcePackage.StopError;
import CF.TestableObjectPackage.UnknownTest;

/**
 * @deprecated replaced by Resource
 */
@Deprecated
public abstract class Resource_impl extends ResourcePOA implements Runnable { // SUPPRESS CHECKSTYLE Name
    protected static final boolean USEOBJECT = false;
    /**
     * The CORBA ORB to use for servicing requests
     */
    private org.omg.CORBA.ORB orb = null;
    /** The root POA object */
    private POA rootpoa = null;
    /** the NameService helper */
    private NamingContextExt ncRef = null;
    /** The component ID */
    private final String compId;
    /** Map of input ports for this resource */
    protected HashMap<String, org.omg.CORBA.Object> inPortObjects;
    /** Map of output ports for this resource */
    protected HashMap<String, org.omg.CORBA.Object> outPortObjects;
    /** Map of output ports for this resource */
    protected HashMap<String, PortPOA> outPorts;
    /** Map of properties for this resource */
    protected HashMap<String, PropertyContainer> propSet;
    /** flag if we're done processing */
    private boolean componentAlive;
    /** flag if the run thread has started */
    private boolean _started;
    /** The thread the main loop is running in */
    private Thread runThread;
    /** Lock for processing data */
    protected Object processDataLock;

    /**
     * Default Constructor that automatically sets parameters for the Sun ORB
     * and the JacORB ORB.
     * 
     * @param nsIor IOR of the NameService
     * @param compId Name of this resource
     * @param builtInORB true to use the Sun ORB, fals for JacORB
     * @param fragSize ORB Fragment size in bytes
     * @param bufSize ORB Buffer size in bytes
     */
    public Resource_impl(final String nsIor, final String compId, final boolean builtInORB, final int fragSize, final int bufSize) {
        this(compId);
        this.propSet = new HashMap<String, PropertyContainer>();

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
        this.initORB(nsIor, props);
    }

    /**
     * Optional constructor used to pass in ORB configuration properties.
     * 
     * @param nsIor IOR of the NameService
     * @param compId Name of this resource
     * @param orbProps Properties object with custom ORB settings
     */
    public Resource_impl(final String nsIor, final String compId, final Properties orbProps) {
        this(compId);

        this.initORB(nsIor, orbProps);
    }

    /**
     * Private constructor to initialize member variables.
     * 
     * @param compId Name of the component
     */
    private Resource_impl(final String compId) {
        this.compId = compId;
        this._started = false;
        this.processDataLock = new Object();
        this.inPortObjects = new HashMap<String, org.omg.CORBA.Object>();
        this.outPortObjects = new HashMap<String, org.omg.CORBA.Object>();
        this.outPorts = new HashMap<String, PortPOA>();
    }

    /**
     * This method initializes the orb and connect to the NameService.
     * 
     * @param nsIor IOR of the NameService
     * @param props the properties to configure the ORB with
     */
    private void initORB(final String nsIor, final Properties props) {
        this.orb = ORB.init((String[]) null, props);

        // get reference to RootPOA & activate the POAManager
        try {
            final org.omg.CORBA.Object poa = this.orb.resolve_initial_references("RootPOA");
            this.rootpoa = POAHelper.narrow(poa);
            this.rootpoa.the_POAManager().activate();
        } catch (final AdapterInactive e) {
            // PASS
        } catch (final InvalidName e) {
            // PASS
        }

        // get the root naming context
        // Use NamingContextExt which is part of the Interoperable Naming
        // Service (INS) specification.
        this.ncRef = NamingContextExtHelper.narrow(this.orb.string_to_object(nsIor));

        final Thread processingThread = new Thread("Resource_Impl ORB Processing thread") {
            @Override
            public void run() {
                Resource_impl.this.orb.run();
            }
        };
        processingThread.start();
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
    public POA getRootpoa() {
        return this.rootpoa;
    }

    /**
     * This returns the reference to the NameServer
     * 
     * @return the NamingContextExt for the NameServer
     */
    public NamingContextExt getNcRef() {
        return this.ncRef;
    }

    /**
     * This returns true if the component constructor has run and the component
     * is not told to release.
     * 
     * @return true if the component is running
     */
    public boolean isComponentAlive() {
        return this.componentAlive;
    }

    /**
     * This is used to set the component's alive state.
     * 
     * @param componentAlive the new alive state
     */
    protected void setComponentAlive(final boolean componentAlive) {
        this.componentAlive = componentAlive;
    }

    /**
     * This returns true if the components run thread has been started.
     *
     * @deprecated use started instead
     * @return true if the component is running
     */
    public boolean isThreadStarted() {
        return this._started;
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

    /** generated in component */
    public void start() throws StartError {
        System.out.println("Start called!");
        if (!this._started) {
            System.out.println("Starting thread!");
            this._started = true;
            this.runThread = new Thread(this, "Resource_impl Run Thread");
            this.runThread.start();
        }
    }

    /** generated in component */
    public void stop() throws StopError {
    }

    /** generated in component */
    public void initialize() throws InitializeError {
        // This function is called by the framework during construction of the waveform
        //    it is called before configure() is called, so whatever values you set in the xml properties file
        //    won't be available when this is called. I wouldn't have done it in this order, but this
        //    is what the specs call for
    }

    /** Generated in component_support */
    public void releaseObject() throws ReleaseError {
        // Set the main thread condition to terminate
        this.setComponentAlive(false);

        // These loops deactivate the port objects so that they can be destroyed without incident
        for (final org.omg.CORBA.Object p : this.outPortObjects.values()) {
            try {
                this.rootpoa.deactivate_object(this.rootpoa.reference_to_id(p));
            } catch (final UserException e) {
                // PASS
            }
        }
        for (final org.omg.CORBA.Object p : this.inPortObjects.values()) {
            try {
                this.rootpoa.deactivate_object(this.rootpoa.reference_to_id(p));
            } catch (final UserException e) {
                // PASS
            }
        }

        // multi-stage destruction for the ports is necessary to account for the initial memory
        // allocation and the creation of the different maps
        // TODO Might have to do something different here
        this.inPortObjects.clear();

        // output ports have free-running thread
        //  when thread is terminated the port is deleted
        //  so instead of delete, port is just released
        // TODO Might have to do something different here
        this.outPorts.clear();
        this.outPortObjects.clear();

        // Provide main processing thread with signal in case it is currently waiting on incoming data
        this.clearWaits();

        if (this.runThread != null) {
            try {
                this.runThread.join();
            } catch (final InterruptedException e) {
                // PASS
            }
            this._started = false;
        }

    }

    /**
     * This method is used to wake everyone that is waiting on an input queue.
     */
    protected abstract void clearWaits();

    /** Generated in component_support */
    public org.omg.CORBA.Object getPort(final String name) throws UnknownPort {
        // the mapping of ports assumes that port names are unique to the component
        // the Ports_var maps are kept different (they could be made into one)
        // because it's less confusing this way

        // return reference if it's an input port
        org.omg.CORBA.Object p = this.inPortObjects.get(name);
        if (p != null) {
            return p;
        }
        p = this.outPortObjects.get(name);
        if (p != null) {
            return p;
        }

        throw new UnknownPort("Unknown port: " + name);
    }

    /**
     * This returns the output port with the given name
     * 
     * @param name name of the output port to retrieve
     * @return the specified output port, or null if it's not found
     */
    public PortPOA getOutputPort(final String name) {
        return this.outPorts.get(name);
    }

    /**
     * {@inheritDoc}
     */
    public void runTest(final int testid, final PropertiesHolder testValues) throws UnknownTest, UnknownProperties {
    }

    /**
     * {@inheritDoc}
     */
    public synchronized void configure(final DataType[] configProperties) throws InvalidConfiguration, PartialConfiguration {
        try {
            final ArrayList<DataType> validProperties = new ArrayList<DataType>();
            final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();

            validate(configProperties, validProperties, invalidProperties);

            for (final DataType validProp : validProperties) {
                for (final PropertyContainer prop : this.propSet.values()) {
                    if (prop.getId().equals(validProp.id)) {
                        prop.setValue(validProp.value);
                    }
                }
            }

            if ((validProperties.size() == 0) && (invalidProperties.size() != 0)) {
                throw new InvalidConfiguration();
            } else if ((validProperties.size() != 0) && (invalidProperties.size() != 0)) {
                throw new PartialConfiguration();
            }
        } catch (final Throwable t) {
            t.printStackTrace();
        }

        if (!this._started) {
            this._started = true;
            this.runThread = new Thread(this, "Resource_impl Run Thread");
            this.runThread.start();
        }
    }

    /**
     * {@inheritDoc}
     */
    public synchronized void query(final PropertiesHolder configProperties) throws UnknownProperties {
        // for queries of zero length, return all id/value pairs in propertySet
        if (configProperties.value.length == 0) {
            final ArrayList<DataType> props = new ArrayList<DataType>(this.propSet.size());
            for (final PropertyContainer prop : this.propSet.values()) {
                props.add(new DataType(prop.getId(), prop.getBaseProperty().value));
            }

            configProperties.value = props.toArray(new DataType[props.size()]);
            return;
        }

        // for queries of length > 0, return all requested pairs in propertySet
        final ArrayList<DataType> validProperties = new ArrayList<DataType>();
        final ArrayList<DataType> invalidProperties = new ArrayList<DataType>();

        validate(configProperties.value, validProperties, invalidProperties);

        //returns values for valid queries in the same order as requested
        for (final DataType valid : validProperties) {
            for (int j = 0; j < configProperties.value.length; j++) {
                if (configProperties.value[j].id.equals(valid.id)) {
                    configProperties.value[j].value = getProperty(valid.id).value;
                    break;
                }
            }
        }

        if (invalidProperties.size() != 0) {
            throw new UnknownProperties();
        }
    }

    /**
     * This function explicitly activates the given Servant.
     * 
     * @param s the Servant to activate
     * @return the activated CORBA Object
     */
    protected org.omg.CORBA.Object activateObject(final Servant s) {
        try {
            final byte[] oid = this.getRootpoa().activate_object(s);
            return this.getRootpoa().id_to_reference(oid);
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
     * This method checks if the given configProperties are valid for this
     * component.
     * 
     * @param configProperties the properties to validate
     * @param validProperties storage for the valid properties
     * @param invalidProperties storage for the invalid properties
     */
    private void validate(final DataType[] configProperties, final ArrayList<DataType> validProperties, final ArrayList<DataType> invalidProperties) {
        for (final DataType p : configProperties) {
            boolean success = false;
            for (final PropertyContainer prop : this.propSet.values()) {
                if (prop.getId().equals(p.id)) {
                    success = true;
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
     * This returns the property with the given ID.
     * 
     * @param id the ID of the property to get
     * @return the property with the specified ID
     */
    protected DataType getProperty(final String id) {
        for (final PropertyContainer prop : this.propSet.values()) {
            if (prop.getId().equals(id)) {
                return prop.getBaseProperty();
            }
        }
        return new DataType();
    }
}
