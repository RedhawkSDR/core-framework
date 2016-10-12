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
import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;
import java.util.Properties;

import org.apache.log4j.Appender;
import org.apache.log4j.ConsoleAppender;
import org.apache.log4j.Layout;
import org.apache.log4j.Level;
import org.apache.log4j.LogManager;
import org.apache.log4j.Logger;
import org.apache.log4j.PatternLayout;
import org.apache.log4j.PropertyConfigurator;
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

import CF.AggregateDevice;
import CF.AggregateDeviceHelper;
import CF.DataType;
import CF.DeviceHelper;
import CF.DeviceManager;
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
import COS.CosEventComm.Disconnected;
import COS.CosEventComm.PushSupplierHolder;
import CosEventChannelAdmin.AlreadyConnected;
import CosEventChannelAdmin.EventChannel;
import CosEventChannelAdmin.ProxyPushConsumer;
import CosEventChannelAdmin.SupplierAdmin;
import StandardEvent.StateChangeCategoryType;
import StandardEvent.StateChangeEventType;
import StandardEvent.StateChangeType;

public abstract class Device extends Resource implements DeviceOperations {

    protected DeviceManager devMgr;
    protected AggregateDevice compositeDevice;
    protected CF.Device device;

    protected String softwareProfile;
    protected String label;
    protected AdminType adminState = AdminType.UNLOCKED;
    protected UsageType usageState = UsageType.IDLE;
    protected OperationalType operationState  = OperationalType.ENABLED;
    protected HashMap <String, AllocCapacity> callbacks;
    private DataType propSetOrig[];
    private boolean firstTime = true;
    private ProxyPushConsumer proxy_consumer;
    private PushSupplierHolder supplier;

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
        setup(devMgr, null, compId, label, softwareProfile, orb, poa);
    }

    public Device(final DeviceManager devMgr, final AggregateDevice compositeDevice, final String compId, final String label, final String softwareProfile, final ORB orb, final POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy {
        this();
        setup(devMgr, compositeDevice, compId, label, softwareProfile, orb, poa);
    }

    public String softwareProfile() {
        return softwareProfile;
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
    }

    public void connectEventChannel(EventChannel idm_channel){
        // TODO check for null idm_channel
        SupplierAdmin supplier_admin = idm_channel.for_suppliers();
        proxy_consumer = supplier_admin.obtain_push_consumer();

        supplier = new PushSupplierHolder();
        try {
            proxy_consumer.connect_push_supplier(supplier.value);
        } catch (AlreadyConnected e) {
            logger.warn("Failed to connect to IDM channel.");
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
            final ORB orb, 
            final POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy {
        super.setup(compId, label, orb, poa);
        this.label = label;
        this.softwareProfile = softwareProfile;

        DevicePOATie tie = new DevicePOATie(this, poa);
        tie._this(orb);

        device = DeviceHelper.narrow(poa.servant_to_reference((Servant)tie));

        this.devMgr = devMgr;
        if (devMgr != null) {
            devMgr.registerDevice(device);
        }

        this.compositeDevice = compositeDevice;
        if (compositeDevice != null) {
            compositeDevice.addDevice(device);
        }

        // TODO Handle IDM_CHANNEL_IOR


        return device;
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
    public static void start_device(final Class clazz, final String[] args, final boolean builtInORB, final int fragSize, final int bufSize) 
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
    public static void start_device(final Class clazz,  final String[] args, final Properties props) 
    throws InstantiationException, IllegalAccessException, InvalidObjectReference, ServantNotActive, WrongPolicy 
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

        DeviceManager devMgr = null;
        if (execparams.containsKey("DEVICE_MGR_IOR")) {
            devMgr = DeviceManagerHelper.narrow(orb.string_to_object(execparams.get("DEVICE_MGR_IOR")));
        }

        AggregateDevice compositeDevice = null;
        if (execparams.containsKey("COMPOSITE_DEVICE_IOR")) {
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

        final Device device_i = (Device)clazz.newInstance();
        device_i.initializeProperties(execparams);
        final CF.Device device = device_i.setup(devMgr, compositeDevice, identifier, label, profile, orb, rootpoa);

        // Get DomainManager incoming event channel and connect the device to it, where applicable
        if (execparams.containsKey("IDM_CHANNEL_IOR")){
            try {
                Object idm_channel_obj = orb.string_to_object(execparams.get("IDM_CHANNEL_IOR"));
                EventChannel idm_channel = CosEventChannelAdmin.EventChannelHelper.narrow(idm_channel_obj);
                device_i.connectEventChannel(idm_channel);
            } catch (Exception e){
                logger.warn("Error connecting to IDM channel.");
            }
        }

        // Sets up the logging
        String loggingConfigURI = null;
        if (execparams.containsKey("LOGGING_CONFIG_URI")) {
            loggingConfigURI = execparams.get("LOGGING_CONFIG_URI");
            if (loggingConfigURI.indexOf("file://") != -1){
                int startIndex = loggingConfigURI.indexOf("file://") + 7;
                PropertyConfigurator.configure(loggingConfigURI.substring(startIndex));
            }else if (loggingConfigURI.indexOf("sca:") != -1){
                int startIndex = loggingConfigURI.indexOf("sca:") + 4;
                String localFile = device_i.getLogConfig(loggingConfigURI.substring(startIndex));
                File testLocalFile = new File(localFile);
                if (localFile.length() > 0 && testLocalFile.exists()){
                    PropertyConfigurator.configure(localFile);
                }
            }
        } else {
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

        // Create a thread that watches for the device to be deactivated
        Thread shutdownWatcher = new Thread(new Runnable() {

            public void run() {
                while (device_i.adminState() != AdminType.LOCKED) {
                    try {
                        Thread.currentThread().sleep(500);
                    } catch (InterruptedException e) {
                        // PASS
                    }
                }
                orb.shutdown(true);
            }
        });

        shutdownWatcher.start();

        orb.run();

        try {
            shutdownWatcher.join();
        } catch (InterruptedException e) {
            // PASS
        }
    }


    /**
     * Attempts to allocate a list of capacities on a device
     * 
     * @param capacities
     * @throws InvalidCapacity
     * @throws InvalidState
     */
    public boolean allocateCapacity(DataType[] capacities)
    throws InvalidCapacity, InvalidState {
        logger.debug("allocateCapacity : " + capacities.toString());

        // If first time store a snapshot of orig propeties
        if (firstTime){
            this.propSetOrig = getProps(this.propSet);
            firstTime = false;
        }

        boolean extraCap = false;    // Flag to check remaining extra capacity to allocate
        boolean foundProperty;

        // Checks for empty
        if (capacities.length == 0){
            logger.trace("no capacities to configure.");
            return true;
        }

        // Verify that the device is in a valid state
        if (adminState == AdminType.LOCKED || operationState == OperationalType.DISABLED){
            logger.warn("Cannot allocate capacity: System is either LOCKED, SHUTTING DOWN, or DISABLED.");
            throw new InvalidState("Cannot allocate capacity. System is either LOCKED, SHUTTING DOWN or DISABLED.");
        }

        if (usageState != UsageType.BUSY){    
            // Gets a current copy of the device capacities
            DataType currentCapacities[] = getProps(this.propSet);

            // Loops through all requested capacities
            for (DataType cap : capacities){
                foundProperty = false;

                // Checks to see if the device has a call back function registered
                if (callbacks.containsKey(cap.id) && callbacks.get(cap.id).allocate(cap)){
                    // If it does, use it
                    foundProperty = true;
                    logger.trace("Capacity allocated by user-defined function.");
                } else {
                    // Otherwise try to use default function
                    for (DataType currentCap : currentCapacities){
                        logger.trace("Comparing IDs: " + currentCap.id + ", " + cap.id);
                        if (cap.id.equals(currentCap.id)){
                            // Verify that both values have the same type
                            if (!cap.value.getClass().equals(currentCap.value.getClass())){
                                logger.error("Cannot allocate capacity: Incorrect data type.");
                                throw new InvalidCapacity("Cannot allocate capacity. Incorrect Data Type.", capacities);
                            } else {
                                // Checks to make sure the prop is allocatable
                                if (!propSet.get(cap.id).isAllocatable()){
                                    logger.warn("Cannot allocate capacity, not allocatable: " + cap.id);
                                    //throw new InvalidCapacity("Cannot allocate capacity: not allocatable " + cap.id, capacities);
                                    return false;
                                }

                                // Check for sufficient capacity and allocate it
                                if (!allocate(currentCap.value, cap.value)){
                                    logger.error("Cannot allocate capacity: Insufficient capacity.");
                                    return false;
                                }
                                logger.trace("Device Capacity ID: " + currentCap.id + ", New Capacity: " +  AnyUtils.convertAny(currentCap.value));
                            }

                            // Report the requested property was found
                            foundProperty = true;
                            break;
                        }
                    }
                }

                if (!foundProperty){
                    logger.error("Cannot allocate capacity: Invalid property ID: " + cap.id);
                    throw new InvalidCapacity("Cannot allocate capacity. Invalid property ID", capacities);
                }
            }

            // Check for remaining capacity
            for (DataType currentCap : currentCapacities){
                if (compareAnyToZero(currentCap.value) == AnyComparisonType.POSITIVE && propSet.get(currentCap.id).isAllocatable()){
                    extraCap = true;

                    // No need to keep going. if after allocation there is any capacity available, the device is ACTIVE.
                    break;
                }
            }

            // Store new capacities, here is when the allocation takes place
            try {
                configure (currentCapacities);
            } catch (InvalidConfiguration e) {
                logger.warn("Error configuring device.");
            } catch (PartialConfiguration e) {
                logger.warn("Partial configuration in device.");
            }

            // Update usage state 
            if (!extraCap){
                setUsageState(UsageType.BUSY);
            } else {
                setUsageState(UsageType.ACTIVE);  /* Assumes it allocated something. Not considering zero allocations */
            }

        } else {
            logger.warn("Cannot allocate capacity: System is BUSY");

            return false;
        }

        return true;
    }


    /**
     * Attempts to allocate a list of capacities on a device
     * 
     * @param capacities
     * @throws InvalidCapacity
     * @throws InvalidState
     */
    public void deallocateCapacity(DataType[] capacities)
    throws InvalidCapacity, InvalidState {
        boolean totalCap = true;            /* Flag to check remaining extra capacity to allocate */
        boolean foundProperty = false;      /* Flag to indicate if the requested property was found */
        AnyComparisonType compResult;

        /* Verify that the device is in a valid state */
        if (adminState == AdminType.LOCKED || operationState == OperationalType.DISABLED){
            logger.warn("Cannot deallocate capacity. System is either LOCKED or DISABLED.");
            throw new InvalidState("Cannot deallocate capacity. System is either LOCKED or DISABLED.");
        }

        /* Now verify that there is capacity currently being used */
        if (usageState != UsageType.IDLE){
            // Gets a current copy of the device capacities
            DataType currentCapacities[] = getProps(this.propSet);

            for (DataType cap : capacities){
                foundProperty = false;

                // Checks to see if the device has a call back function registered
                if (callbacks.containsKey(cap.id) && callbacks.get(cap.id).deallocate(cap)){
                    // If it does, use it
                    foundProperty = true;
                    logger.trace("Capacity allocated by user-defined function.");
                } else {
                    // Otherwise try to use default function
                    for (DataType currentCap : currentCapacities){
                        logger.trace("Comparing IDs: " + currentCap.id + ", " + cap.id);
                        // Checks for the same ID
                        if (cap.id.equals(currentCap.id)){
                            // Verify that both values have the same type
                            if (!cap.value.getClass().equals(currentCap.value.getClass())){
                                logger.warn("Cannot deallocate capacity. Incorrect Data Type.");
                                throw new InvalidCapacity("Cannot deallocate capacity. Incorrect Data Type.", capacities);
                            } else {
                                // Checks to make sure the prop is allocatable
                                if (!propSet.get(cap.id).isAllocatable()){
                                    logger.warn("Cannot allocate capacity: not allocatable.");
                                    throw new InvalidCapacity("Cannot allocate capacity: not allocatable " + cap.id, capacities);
                                } else {
                                    deallocate (currentCap.value, cap.value);
                                }
                            }

                            foundProperty = true;     /* Report that the requested property was found */
                            break;
                        }
                    }
                }
            }

            if (!foundProperty){
                logger.warn("Cannot deallocate capacity. Invalid property ID");
                throw new InvalidCapacity("Cannot deallocate capacity. Invalid property ID", capacities);
            }

            // Check for exceeding dealLocations and back-to-total capacity
            for (DataType currentCap : currentCapacities){
                for (DataType origCap : this.propSetOrig){
                    if (currentCap.id.equals(origCap.id)){
                        compResult = compareAnys(currentCap.value, origCap.value);

                        if (compResult == AnyComparisonType.FIRST_BIGGER){
                            logger.warn("Cannot deallocate capacity. New capacity would exceed original bound.");
                            throw new InvalidCapacity("Cannot deallocate capacity. New capacity would exceed original bound.", capacities);
                        } else if (compResult == AnyComparisonType.SECOND_BIGGER){
                            totalCap = false;
                            break;
                        }
                    }
                }
            }       

            /* Write new capacities */
            try {
                configure (currentCapacities);
            } catch (InvalidConfiguration e) {
                logger.warn("Error configuring device.");
            } catch (PartialConfiguration e) {
                logger.warn("Partial configuration in device.");
            }

            /* Update usage state */
            if (!totalCap) {
                setUsageState(UsageType.ACTIVE);
            } else {
                setUsageState(UsageType.IDLE);  /* Assumes it allocated something. Not considering zero allocations */
            }

        } else {
            logger.warn("Cannot deallocate capacity. System is IDLE.");
            throw  new InvalidCapacity ("Cannot deallocate capacity. System is IDLE.", capacities);
        }

    }


    /**
     * Helper function to determine if device is able to allocate the 
     * specific CORBA Any object 
     * 
     * @param deviceCapacity
     * @param resourceRequest
     * @return
     */
    private boolean allocate(Any deviceCapacity, Any resourceRequest){
        TypeCode tc1 = deviceCapacity.type();

        switch (tc1.kind().value()){
        case TCKind._tk_ulong: {
            Number devCapac, rscReq;    
            devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
            rscReq = (Number) AnyUtils.convertAny(resourceRequest);

            if (rscReq.intValue() <= devCapac.intValue()){
                AnyUtils.insertInto(deviceCapacity, devCapac.intValue() - rscReq.intValue(), tc1.kind());
                return true;
            } else {
                return false;
            }
        }

        case TCKind._tk_long: {
            Number devCapac, rscReq;
            devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
            rscReq = (Number) AnyUtils.convertAny(resourceRequest);

            if (rscReq.intValue() <= devCapac.intValue()){
                AnyUtils.insertInto(deviceCapacity, devCapac.intValue() - rscReq.intValue(), tc1.kind());
                return true;
            } else {
                return false;
            }

        }

        case TCKind._tk_short: {
            Number devCapac, rscReq;
            devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
            rscReq = (Number) AnyUtils.convertAny(resourceRequest);

            if (rscReq.shortValue() <= devCapac.shortValue()){
                AnyUtils.insertInto(deviceCapacity, devCapac.shortValue() - rscReq.shortValue(), tc1.kind());
                return true;
            } else {
                return false;
            }
        }

        default:
            return false;
        }
    }

    /**
     * Helper function to determine if device is able to deallocate the
     * specific CORBA Any object
     * 
     * @param deviceCapacity
     * @param resourceRequest
     */
    private void deallocate(Any deviceCapacity, Any resourceRequest) {
        TypeCode tc1 = deviceCapacity.type();

        switch(tc1.kind().value()){
        case TCKind._tk_ulong: {
            Number devCapac, rscReq;
            devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
            rscReq = (Number) AnyUtils.convertAny(resourceRequest);
            AnyUtils.insertInto(deviceCapacity, devCapac.intValue() + rscReq.intValue(), tc1.kind());
            break;
        }

        case TCKind._tk_long: {
            Number devCapac, rscReq;
            devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
            rscReq = (Number) AnyUtils.convertAny(resourceRequest);
            AnyUtils.insertInto(deviceCapacity, devCapac.intValue() + rscReq.intValue(), tc1.kind());
            break;
        }

        case TCKind._tk_short: {
            Number devCapac, rscReq;
            devCapac = (Number) AnyUtils.convertAny(deviceCapacity);
            rscReq = (Number) AnyUtils.convertAny(resourceRequest);
            AnyUtils.insertInto(deviceCapacity, devCapac.shortValue() + rscReq.shortValue(), tc1.kind());
            break;
        }

        default:
            break;
        }
    }


    /** 
     * Helper function to get a DataType array containing property 
     * information from the propSet Hashtable
     * 
     * @param propsIn
     * @return
     */
    private DataType[] getProps(Hashtable <String, IProperty> propsIn){
        DataType props[] = new DataType[propsIn.size()];
        int i = 0;
        for (final IProperty prop : propsIn.values()) {
            props[i++] = new DataType(prop.getId(), prop.toAny());
        }
        return props;
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
    private void setUsageState(UsageType newUsageState){
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
                if (proxy_consumer != null){
                    proxy_consumer.push(AnyUtils.toAny(event, TCKind.tk_objref));
                }
            } catch (Disconnected e) {
                logger.warn("Error sending event.");
            }

            usageState = newUsageState;
        }
    }


    /**
     * Sets a new admin state and sends an event if it has changed
     * 
     * @param newAdminState
     */
    private void setAdminState(AdminType newAdminState){
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
                if (proxy_consumer != null){
                    proxy_consumer.push(AnyUtils.toAny(event, TCKind.tk_objref));
                }
            } catch (Disconnected e) {
                logger.warn("Error sending event.");
            } 

            adminState = newAdminState;
        }
    }


    /**
     * Sets a new operation state and sends an event if it has changed
     * @param newOperationState
     */
    private void setOperationState(OperationalType newOperationState){
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
                if (proxy_consumer != null){
                    proxy_consumer.push(AnyUtils.toAny(event, TCKind.tk_objref));
                }
            } catch (Disconnected e) {
                logger.warn("Error sending event.");
            }

            operationState = newOperationState;
        }
    }

}

