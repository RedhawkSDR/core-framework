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

package org.ossie.component;

import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Hashtable;
import java.util.List;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.Logger;
import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.Object;
import org.omg.CORBA.TCKind;
import org.omg.CORBA.TypeCode;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import org.omg.PortableServer.Servant;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.ossie.properties.AnyUtils;
import org.ossie.properties.IProperty;
import org.ossie.logging.logging;
import org.ossie.redhawk.DomainManagerContainer;
import org.ossie.redhawk.DeviceManagerContainer;
import org.ossie.corba.utils.*;

import CF.AggregateDevice;
import CF.AggregateDeviceHelper;
import CF.DataType;
import CF.DeviceHelper;
import CF.DeviceManager;
import CF.DomainManager;
import CF.DeviceManagerHelper;
import CF.DeviceOperations;
import CF.DevicePOATie;
import CF.InvalidObjectReference;
import CF.DevicePackage.AdminType;
import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.InvalidState;
import CF.DevicePackage.OperationalType;
import CF.DevicePackage.UsageType;
import CF.LifeCyclePackage.ReleaseError;
import CF.PropertySetPackage.InvalidConfiguration;
import CF.PropertySetPackage.PartialConfiguration;
import org.omg.CosEventComm.Disconnected;
import org.omg.CosEventChannelAdmin.AlreadyConnected;
import org.omg.CosEventChannelAdmin.EventChannel;
import org.omg.CosNaming.*;
import org.omg.CosNaming.NamingContextPackage.*;
import org.omg.CosNaming.NamingContextExt;
import org.omg.CosNaming.NamingContextExtHelper;
import org.ossie.events.*;
import org.ossie.events.Manager.*;
import CF.EventChannelManagerPackage.*;
import StandardEvent.StateChangeCategoryType;
import StandardEvent.StateChangeEventType;
import StandardEvent.StateChangeType;

public abstract class Device extends Resource implements DeviceOperations {

    protected DeviceManager devMgr;
    protected DeviceManagerContainer _devMgr = null;
    protected AggregateDevice compositeDevice;
    protected CF.Device device;

    protected String label;
    protected AdminType adminState = AdminType.UNLOCKED;
    protected UsageType usageState = UsageType.IDLE;
    protected OperationalType operationState  = OperationalType.ENABLED;
    private boolean firstTime = true;
    private Publisher idm_publisher=null;

    /**
     * Deprecated old-style allocation callbacks. Use the Allocator interface and
     * setAllocator() on the desired property to implement custom allocation
     * behavior.
     *
     * NB: To be removed when backwards compatibility with pre-1.9 devices is no 
     *     longer required.
     *
     * @deprecated
     */
    protected HashMap <String, AllocCapacity> callbacks = new HashMap<String,AllocCapacity>();

    /**
     * Track legacy allocation properties so that backwards-compatible allocation
     * behavior can be maintained as much as possible. If it is null, no legacy
     * allocation properties have been registered and no emulation of legacy
     * behavior is required.
     *
     * NB: To be removed when backwards compatibility with pre-1.9 devices is no
     *     longer required.
     *
     * @deprecated
     */
    private List<IProperty> legacyAllocProps = null;
    private boolean warnedLegacyAllocProps = false;

    public final static Logger logger = Logger.getLogger(Device.class.getName());

    enum AnyComparisonType {
        FIRST_BIGGER,
        SECOND_BIGGER,
        BOTH_EQUAL,
        POSITIVE,
        NEGATIVE,
        ZERO,
        UNKNOWN
    };

    /**
     * Constructor intendend to be used by start_device
     */
    protected Device() {
        super();
    }

    public Device(final DeviceManager devMgr, final String compId, final String label, final String softwareProfile, final ORB orb, final POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy {
        this();
        setup(devMgr, null, compId, label, softwareProfile, "", orb, poa);
    }

    public Device(final DeviceManager devMgr, final AggregateDevice compositeDevice, final String compId, final String label, final String softwareProfile, final ORB orb, final POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy {
        this();
        setup(devMgr, compositeDevice, compId, label, softwareProfile, "", orb, poa);
    }

    public String label() {
        return label;
    }

    public UsageType usageState() {
        return usageState;
    }

    public AdminType adminState() {
        return adminState;
    }

    public void adminState(AdminType newAdminState) {
        adminState = newAdminState;
    }

    public OperationalType operationalState() {
        return operationState;
    }

    public AggregateDevice compositeDevice() {
        return compositeDevice;
    }

    public void releaseObject() throws ReleaseError {
        if (adminState == AdminType.UNLOCKED) {
            setAdminState(AdminType.SHUTTING_DOWN);
        }

        try {
            // Release all registered childDevices

            if (compositeDevice != null) {
                compositeDevice.removeDevice(device);
            }

            // Unregister device
            if (devMgr != null) {
                devMgr.unregisterDevice(device);
            }
            
        } catch (Exception e) {
            e.printStackTrace();
            throw new ReleaseError(new String[] {e.toString()});
        }
        super.releaseObject();
        setAdminState(AdminType.LOCKED);

        if ( idm_publisher != null ) {
            idm_publisher.terminate();
            idm_publisher = null;
        }

        org.ossie.events.Manager.Terminate();

    }


    public void connectIDMChannel(final String idm_channel_ior){

        if ( (idm_channel_ior != null && !idm_channel_ior.equals("")) || idm_channel_ior.length() > 0 ) {
            EventChannel idm_channel=null;
            // Get DomainManager incoming event channel and connect the device to it, where applicable
            try {
                logger.debug("connectIDMChannel: idm_channel_ior:" + idm_channel_ior);
                Object idm_channel_obj = orb.string_to_object(idm_channel_ior);
                idm_channel = org.omg.CosEventChannelAdmin.EventChannelHelper.narrow(idm_channel_obj);
                idm_publisher = new org.ossie.events.Publisher(idm_channel);
            } 
            catch (Exception e){
                logger.warn("Error connecting to IDM channel.");
            }

        }
        else {
            try {
                Manager evt_mgr = Manager.GetManager(this);
                
                idm_publisher = evt_mgr.Publisher( Manager.IDM_CHANNEL_SPEC );
            }
            catch( Manager.OperationFailed e) {
                logger.warn("Failed to connect to IDM channel.");
            }
            catch( RegistrationExists e) {
                logger.warn("Failed to connect to IDM channel.");
            }
            catch( RegistrationFailed e) {
                logger.warn("Failed to connect to IDM channel.");
            }
        }
    }

    /**
     * The setup() function exists to make it easy for start_device to invoke the no-arg constructor.
     * 
     * @param devMgr
     * @param compositeDevice
     * @param compId
     * @param label
     * @param softwareProfile
     * @param orb
     * @throws InvalidObjectReference 
     * @throws WrongPolicy 
     * @throws ServantNotActive 
     */
    protected CF.Device setup(final DeviceManager devMgr, 
            final AggregateDevice compositeDevice, 
            final String compId, 
            final String label, 
            final String softwareProfile,
            final String idm_channel_ior,
            final ORB orb, 
            final POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy {
        super.setup(compId, label, softwareProfile, orb, poa);
        this.label = label;

        DevicePOATie tie = new DevicePOATie(this, poa);
        tie._this(orb);

        device = DeviceHelper.narrow(poa.servant_to_reference((Servant)tie));

        this.devMgr = devMgr;
        if (devMgr != null) {
            this._devMgr = new DeviceManagerContainer(devMgr);
            this._domMgr = new DomainManagerContainer(devMgr.domMgr());
            devMgr.registerDevice(device);
        }

        this.compositeDevice = compositeDevice;
        if (compositeDevice != null) {
            compositeDevice.addDevice(device);
        }

        connectIDMChannel( idm_channel_ior );

        // this needs to be established before device saves logging context
        // incase event channels are used
        if ( this._domMgr != null ) {
            try {
                this._ecm = org.ossie.events.Manager.GetManager(this);
            }catch( Manager.OperationFailed e){
                logger.warn("Unable to resolve EventChannelManager");
            }
        }

        return device;
    }
    
    public DeviceManagerContainer getDeviceManager() {
        return this._devMgr;
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
     */
    public static void start_device(final Class<? extends Device> clazz, final String[] args, final boolean builtInORB, final int fragSize, final int bufSize) 
    throws InstantiationException, IllegalAccessException, InvalidObjectReference, ServantNotActive, WrongPolicy 
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
        start_device(clazz, args, props);
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
     * @throws WrongPolicy 
     * @throws ServantNotActive 
     */
    public static void start_device(final Class<? extends Device> clazz,  final String[] args, final Properties props) 
    throws InstantiationException, IllegalAccessException, InvalidObjectReference, ServantNotActive, WrongPolicy 
    {
        // initialize middleware with command line/properties..
        final org.omg.CORBA.ORB orb = org.ossie.corba.utils.Init( args, props );

        final POA rootpoa  = org.ossie.corba.utils.RootPOA();

        Map<String, String> execparams = parseArgs(args);

        DeviceManager devMgr = null;
	String devMgr_ior=null;
        if (execparams.containsKey("DEVICE_MGR_IOR")) {
	    devMgr_ior=execparams.get("DEVICE_MGR_IOR");
            devMgr = DeviceManagerHelper.narrow(orb.string_to_object(execparams.get("DEVICE_MGR_IOR")));
        }

        AggregateDevice compositeDevice = null;
	String composite_ior=null;
        if (execparams.containsKey("COMPOSITE_DEVICE_IOR")) {
	    composite_ior=execparams.get("COMPOSITE_DEVICE_IOR");
            compositeDevice = AggregateDeviceHelper.narrow(orb.string_to_object(execparams.get("COMPOSITE_DEVICE_IOR")));
        }

        String identifier = null;
        if (execparams.containsKey("DEVICE_ID")) {
            identifier = execparams.get("DEVICE_ID");
        }

        String label = null;
        if (execparams.containsKey("DEVICE_LABEL")) {
            label = execparams.get("DEVICE_LABEL");
        }

        String profile = null;
        if (execparams.containsKey("PROFILE_NAME")) {
            profile = execparams.get("PROFILE_NAME");
        }

        String dom_path = "";
        if (execparams.containsKey("DOM_PATH")) {
            dom_path = execparams.get("DOM_PATH");
        }

        String logcfg_uri = "";
        if (execparams.containsKey("LOGGING_CONFIG_URI")) {
            logcfg_uri = execparams.get("LOGGING_CONFIG_URI");
        }

	int debugLevel = -1;   // use logging config URI if specified
	if (execparams.containsKey("DEBUG_LEVEL")) {
	    debugLevel = Integer.parseInt(execparams.get("DEBUG_LEVEL"));
	}

	String idm_channel_ior="";   // idm channel ior 
	if (execparams.containsKey("IDM_CHANNEL_IOR")) {
            idm_channel_ior = execparams.get("IDM_CHANNEL_IOR");
	}

	if ( debugLevel > 3 ) {
	    System.out.println("Device Args: " );
	    System.out.println("                DEVICE_LABEL:"+ label );
	    System.out.println("                DEVICE_ID:"+ identifier );
	    System.out.println("                PROFILE_NAME:"+ profile );
	    System.out.println("                COMPONENT_IDENTIFIER:"+ identifier );
	    System.out.println("                COMPOSITE_IOR:"+ composite_ior );
	    System.out.println("                DEVICE_MGR_IOR:"+ devMgr_ior );
	    System.out.println("                DOM_PATH:"+ dom_path );
	    System.out.println("                LOG_CONFIG_URI:"+ logcfg_uri );
	    System.out.println("                DEBUG_LEVEL:"+ debugLevel );
	}


        // initialize logging library with a device context
        logging.DeviceCtx ctx = new logging.DeviceCtx( label, identifier, dom_path );
	logging.Configure( logcfg_uri, debugLevel, ctx );

        final Device device_i = clazz.newInstance();
        device_i.initializeProperties(execparams);
        final CF.Device device = device_i.setup(devMgr, 
                                                compositeDevice, 
                                                identifier, 
                                                label, 
                                                profile, 
                                                idm_channel_ior,
                                                orb, 
                                                rootpoa);

        // save off logging context
	device_i.saveLoggingContext( logcfg_uri, debugLevel, ctx );

        // Create a thread that watches for the device to be deactivated
        Thread shutdownWatcher = new Thread(new Runnable() {
            public void run() {
                device_i.waitDisposed();
                shutdownORB(orb);
            }
        });

        shutdownWatcher.start();

        orb.run();

        // Destroy the ORB, otherwise the JVM shutdown will take an unusually
        // long time (~300ms).
        orb.destroy();

        try {
            shutdownWatcher.join();
        } catch (InterruptedException e) {
            // PASS
        }

        // Shut down native ORB, if it's running
        omnijni.ORB.shutdown();
    }


    /**
     * Checks that all of the properties given are valid and allocatable.
     *
     * @throws InvalidCapacity
     */
    private void validateAllocProps(final DataType[] capacities) throws InvalidCapacity {
        for (final DataType capacity : capacities) {
            if (AnyUtils.isNull(capacity.value)) {
                throw new InvalidCapacity("Invalid null value", new DataType[]{capacity});
            }
            final IProperty property = this.propSet.get(capacity.id);
            if (property == null) {
                throw new InvalidCapacity("Unknown property", new DataType[]{capacity});
            } else if (!property.isAllocatable()) {
                throw new InvalidCapacity("Property is not allocatable", new DataType[]{capacity});
            }
        }
    }


    /**
     * Attempts to allocate a list of capacities on a device
     * 
     * @param capacities
     * @throws InvalidCapacity
     * @throws InvalidState
     */
    public boolean allocateCapacity(DataType[] capacities) throws InvalidCapacity, InvalidState {
        logger.debug("allocateCapacity : " + capacities.toString());

        // Checks for empty
        if (capacities.length == 0){
            logger.trace("No capacities to allocate.");
            return true;
        }

        // Verify that the device is in a valid state
        if (!isUnlocked() || isDisabled()) {
            String invalidState;
            if (isLocked()) {
                invalidState = "LOCKED";
            } else if (isDisabled()) {
                invalidState = "DISABLED";
            } else {
                invalidState = "SHUTTING_DOWN";
            }
            logger.debug("Cannot allocate capacity: System is " + invalidState);
            throw new InvalidState(invalidState);
        }

        // Check for obviously invalid properties up front.
        validateAllocProps(capacities);

        // Track successful allocations in case they need to be undone.
        List<DataType> allocations = new ArrayList<DataType>();

        // Loops through all requested capacities
        try {
            for (DataType cap : capacities){
                // Checks to see if the device has a call back function registered
                if (callbacks.containsKey(cap.id) && callbacks.get(cap.id).allocate(cap)){
                    // If it does, use it
                    logger.trace("Capacity allocated by user-defined function.");
                    allocations.add(cap);
                } else {
                    // Otherwise defer to the property's allocator.
                    final IProperty property = this.propSet.get(cap.id);
                    try {
                        if (property.allocate(cap.value)) {
                            allocations.add(cap);
                        } else {
                            logger.debug("Cannot allocate capacity. Insufficient capacity for property '" + cap.id + "'");
                            return false;
                        }
                    } catch (final RuntimeException ex) {
                        throw new InvalidCapacity(ex.getMessage(), new DataType[]{cap});
                    }
                }
            }
        } finally {
            // If not all allocations were successful, deallocate any that were.
            // The usage state is implicitly updated by deallocateCapacity.
            if (allocations.size() < capacities.length) {
                deallocateCapacity(allocations.toArray(new DataType[allocations.size()]));
            }
        }

        updateUsageState();
        return true;
    }

    /**
     * Attempts to deallocate a list of capacities on a device
     * 
     * @param capacities
     * @throws InvalidCapacity
     * @throws InvalidState
     */
    public void deallocateCapacity(DataType[] capacities) throws InvalidCapacity, InvalidState {
        /* Verify that the device is in a valid state */
        if (adminState == AdminType.LOCKED || operationState == OperationalType.DISABLED){
            logger.warn("Cannot deallocate capacity. System is either LOCKED or DISABLED.");
            throw new InvalidState("Cannot deallocate capacity. System is either LOCKED or DISABLED.");
        }

        // Check for obviously invalid properties to avoid partial deallocation.
        validateAllocProps(capacities);

        for (DataType cap : capacities){
            // Checks to see if the device has a call back function registered
            if (callbacks.containsKey(cap.id) && callbacks.get(cap.id).deallocate(cap)){
                // If it does, use it
                logger.trace("Capacity allocated by user-defined function.");
            } else {
                // Otherwise defer to the property's deallocator.
                final IProperty property = this.propSet.get(cap.id);
                try {
                    property.deallocate(cap.value);
                } catch (final RuntimeException ex) {
                    throw new InvalidCapacity(ex.getMessage(), new DataType[]{cap});
                }
            }
        }

        updateUsageState();
    }

    /**
     * Determine the current usage state of the device.
     *
     * Override if your device has a more sophisticated way of determining the
     * usage state.
     */
    protected void updateUsageState() {
        // Do nothing, unless the device contains legacy allocation properties.
        if (this.legacyAllocProps != null) {
            updateUsageStateLegacy();
        }
    }

    /**
     * Internal method for updating the device's usage state based on the current
     * values of the legacy allocation properties.
     *
     * NB: To be removed when backwards compatibility with pre-1.9 devices is no
     *     longer required.
     *
     * @deprecated
     */
    @SuppressWarnings("deprecation")
    private void updateUsageStateLegacy() {
        boolean active = false;
        boolean busy = true;
        for (IProperty property : this.legacyAllocProps) {
            org.ossie.properties.SimpleProperty simple = (org.ossie.properties.SimpleProperty)property;
            if (compareAnyToZero(property.toAny()) == AnyComparisonType.POSITIVE) {
                busy = false;
            }
            if (!simple.isFull()) {
                active = true;
            }
        }

        if (busy) {
            setUsageState(UsageType.BUSY);
        } else if (active) {
            setUsageState(UsageType.ACTIVE);
        } else {
            setUsageState(UsageType.IDLE);
        }
    }

    // compareAnyToZero function compares the any type input to zero
    // returns POSITIVE if the first argument is bigger
    // returns NEGATIVE is the second argument is bigger
    // and ZERO if they are equal
    public AnyComparisonType compareAnyToZero(Any first){
        TypeCode tc1 = first.type();

        switch (tc1.kind().value()){
        case TCKind._tk_ulong: {
            long frst;
            frst = (Long) AnyUtils.convertAny(first);
            if (frst > 0){
                return AnyComparisonType.POSITIVE;
            } else if (frst == 0){
                return AnyComparisonType.ZERO;
            } else {
                return AnyComparisonType.NEGATIVE;
            }
        }

        case TCKind._tk_long: {
            int frst;
            frst = (Integer) AnyUtils.convertAny(first);
            if (frst > 0){
                return AnyComparisonType.POSITIVE;
            } else if (frst == 0){
                return AnyComparisonType.ZERO;
            } else {
                return AnyComparisonType.NEGATIVE;
            }
        }

        case TCKind._tk_short: {
            short frst = 0;
            frst = (Short) AnyUtils.convertAny(first);
            if (frst > 0){
                return AnyComparisonType.POSITIVE;
            } else if (frst == 0){
                return AnyComparisonType.ZERO;
            } else {
                return AnyComparisonType.NEGATIVE;
            }
        }

        default:
            return AnyComparisonType.UNKNOWN;
        }
    }

    // compareAnys function compares both Any type inputs
    // returns FIRST_BIGGER if the first argument is bigger
    // returns SECOND_BIGGER is the second argument is bigger
    // and BOTH_EQUAL if they are equal
    public AnyComparisonType compareAnys (Any first, Any second) {
        TypeCode tc1 = first.type ();

        switch (tc1.kind().value()) {
        case TCKind._tk_ulong: {
            long frst, scnd;
            frst = (Long) AnyUtils.convertAny(first);
            scnd = (Long) AnyUtils.convertAny(second);
            if (frst > scnd) {
                return AnyComparisonType.FIRST_BIGGER;
            } else if (frst == scnd) {
                return AnyComparisonType.BOTH_EQUAL;
            } else {
                return AnyComparisonType.SECOND_BIGGER;
            }
        }

        case TCKind._tk_long: {
            int frst, scnd;
            frst = (Integer) AnyUtils.convertAny(first);
            scnd = (Integer) AnyUtils.convertAny(second);
            if (frst > scnd) {
                return AnyComparisonType.FIRST_BIGGER;
            } else if (frst == scnd) {
                return AnyComparisonType.BOTH_EQUAL;
            } else {
                return AnyComparisonType.SECOND_BIGGER;
            }
        }

        case TCKind._tk_short: {
            short frst, scnd;
            frst = (Short) AnyUtils.convertAny(first);
            scnd = (Short) AnyUtils.convertAny(second);

            if (frst > scnd) {
                return AnyComparisonType.FIRST_BIGGER;
            } else if (frst == scnd) {
                return AnyComparisonType.BOTH_EQUAL;
            } else {
                return AnyComparisonType.SECOND_BIGGER;
            }
        }

        default:
            return AnyComparisonType.UNKNOWN;
        }
    }


    /**
     * Sets a new usage state and sends an event if it has changed
     * 
     * @param newUsageState
     */
    protected void setUsageState(UsageType newUsageState){
        /* Checks to see if the usageState has change */
        if (newUsageState.value() != this.usageState().value()){

            /* Keep a copy of the actual usage state */
            StateChangeType current_state = StandardEvent.StateChangeType.BUSY;
            StateChangeType new_state = StandardEvent.StateChangeType.BUSY;

            switch (usageState.value()){
            case UsageType._IDLE:
                current_state = StandardEvent.StateChangeType.IDLE;
                break;
            case UsageType._ACTIVE:
                current_state = StandardEvent.StateChangeType.ACTIVE;
                break;
            case UsageType._BUSY:
                current_state = StandardEvent.StateChangeType.BUSY;
                break;
            }

            switch (newUsageState.value()){
            case UsageType._IDLE:
                new_state = StandardEvent.StateChangeType.IDLE;
                break;
            case UsageType._ACTIVE:
                new_state = StandardEvent.StateChangeType.ACTIVE;
                break;
            case UsageType._BUSY:
                new_state = StandardEvent.StateChangeType.BUSY;
                break;
            }

            StateChangeEventType event = new StateChangeEventType(identifier(), identifier(), 
                    StandardEvent.StateChangeCategoryType.USAGE_STATE_EVENT, current_state, new_state);

            try {
                if ( idm_publisher != null ) {
                    idm_publisher.push(AnyUtils.toAny(event, TCKind.tk_objref) );
                    logger.debug("Sent device StateChangeEvent - USAGE ");
                }
            }
            catch (Exception e) {
                logger.warn("Error sending event.");
            }

            usageState = newUsageState;
        }
    }

    /*
     * NB: This method is overridden to support tracking of legacy (pre-1.9)
     *     allocation properties. When backwards-compatibility is no longer
     *     required, this method should be removed, deferring entirely to
     *     the superclass method.
     */
    @SuppressWarnings("deprecation")
    public void addProperty(IProperty prop) {

        // Perform normal bookkeeping.
        super.addProperty(prop);

        // Ignore non-allocation properties.
        if (!prop.isAllocatable()) {
            return;
        }

        // If true, for a mix of legacy and new-style allocation properties.
        boolean isMixed = false;

        if (prop instanceof org.ossie.properties.SimpleProperty) {
            if (this.legacyAllocProps == null) {
                // First instance of a legacy property; create the list. This
                // allows the allocation/deallocation methods to determine whether
                // to try to maintain legacy behavior.
                this.legacyAllocProps = new ArrayList<IProperty>();
            }
            this.legacyAllocProps.add(prop);

            // If no warning has been issued, check whether the number of legacy
            // allocation properties is the same as the total number of allocation
            // properties.
            if (!this.warnedLegacyAllocProps) {
                int allocPropCount = 0;
                for (IProperty ip : this.propSet.values()) {
                    if (ip.isAllocatable()) {
                        allocPropCount++;
                    }
                }
                isMixed = (this.legacyAllocProps.size() != allocPropCount);
            }
        } else if (this.legacyAllocProps != null) {
            // This is a new-style property, and legacy properties are registered.
            // Issue a warning if needed.
            isMixed = true;
        }

        // Warn about mixed legacy and new-style properties, if we have not already.
        if (isMixed && !this.warnedLegacyAllocProps) {
            this.warnedLegacyAllocProps = true;
            logger.warn("Device uses mix of deprecated and new-style allocation properties. Allocation behavior may be inconsistent.");
        }
    }

    /**
     * Sets a new admin state and sends an event if it has changed
     * 
     * @param newAdminState
     */
    protected void setAdminState(AdminType newAdminState){
        /* Checks to see if the admin state has changed */
        if (newAdminState.value() != this.adminState().value()){
            /* Keep a copy of the actual admin state */
            StateChangeType current_state = StateChangeType.UNLOCKED;
            StateChangeType new_state = StateChangeType.UNLOCKED;

            switch(adminState.value()){
            case AdminType._LOCKED:
                current_state = StateChangeType.LOCKED;
                break;
            case AdminType._UNLOCKED:
                current_state = StateChangeType.UNLOCKED;
                break;
            case AdminType._SHUTTING_DOWN:
                current_state = StateChangeType.SHUTTING_DOWN;
                break;
            }

            switch (newAdminState.value()){
            case AdminType._LOCKED:
                new_state = StateChangeType.LOCKED;
                break;
            case AdminType._UNLOCKED:
                new_state = StateChangeType.UNLOCKED;
                break;
            case AdminType._SHUTTING_DOWN:
                new_state = StateChangeType.SHUTTING_DOWN;
                break;
            }

            StateChangeEventType event = new StateChangeEventType(identifier(), identifier(), 
                    StateChangeCategoryType.ADMINISTRATIVE_STATE_EVENT, current_state, new_state);


            try {
                if ( idm_publisher != null ) {
                    idm_publisher.push(AnyUtils.toAny(event, TCKind.tk_objref) );
                    logger.debug("Sent device StateChangeEvent - ADMIN ");
                }
            }
            catch (Exception e) {
                logger.warn("Error sending event.");
            }

            adminState = newAdminState;
        }
    }


    /**
     * Sets a new operation state and sends an event if it has changed
     * @param newOperationState
     */
    protected void setOperationState(OperationalType newOperationState){
        /* Checks to see if the operational state has changed */
        if (newOperationState.value() != this.operationalState().value()){
            /* Keep a copy of the actual operational state */
            StateChangeType current_state= StateChangeType.ENABLED;
            StateChangeType new_state = StateChangeType.ENABLED;

            switch (operationState.value()){
            case OperationalType._DISABLED:
                current_state = StateChangeType.DISABLED;
                break;
            case OperationalType._ENABLED:
                current_state = StateChangeType.ENABLED;
                break;
            }

            switch (newOperationState.value()){
            case OperationalType._DISABLED:
                new_state = StateChangeType.DISABLED;
                break;
            case OperationalType._ENABLED:
                new_state = StateChangeType.ENABLED;
                break;
            }

            StateChangeEventType event = new StateChangeEventType(identifier(), identifier(),
                    StateChangeCategoryType.OPERATIONAL_STATE_EVENT, current_state, new_state);

            try {
                if ( idm_publisher != null ) {
                    idm_publisher.push(AnyUtils.toAny(event, TCKind.tk_objref) );
                    logger.debug("Sent device StateChangeEvent - OPERATIONAL ");
                }
            }
            catch (Exception e) {
                logger.warn("Error sending event.");
            }

            operationState = newOperationState;
        }
    }

    /**
     * Returns whether this device's administrative state is UNLOCKED
     *
     * @returns true if admin state is UNLOCKED, false otherwise
     */
    public boolean isUnlocked() {
        return adminState.equals(AdminType.UNLOCKED);
    }

    /**
     * Returns whether this device's administrative state is LOCKED
     *
     * @returns true if admin state is LOCKED, false otherwise
     */
    public boolean isLocked() {
        return adminState.equals(AdminType.LOCKED);
    }

    /**
     * Returns whether this device's operational state is DISABLED
     *
     * @returns true if operational state is DISABLED, false otherwise
     */
    public boolean isDisabled() {
        return operationState.equals(OperationalType.DISABLED);
    }
}

