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

import java.util.Map;
import java.util.Properties;

import org.omg.CORBA.ORB;
import org.omg.CORBA.ORBPackage.InvalidName;
import org.omg.CosNaming.NamingContextExtHelper;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAHelper;
import org.omg.PortableServer.Servant;
import org.omg.PortableServer.POAManagerPackage.AdapterInactive;
import org.omg.PortableServer.POAPackage.ServantAlreadyActive;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import CF.AggregateDevice;
import CF.AggregateDeviceHelper;
import CF.DataType;
import CF.Device;
import CF.DeviceHelper;
import CF.DeviceManager;
import CF.DeviceManagerHelper;
import CF.DeviceOperations;
import CF.DevicePOA;
import CF.DevicePOATie;
import CF.InvalidObjectReference;
import CF.ResourceHelper;
import CF.DevicePackage.AdminType;
import CF.DevicePackage.InvalidCapacity;
import CF.DevicePackage.InvalidState;
import CF.DevicePackage.OperationalType;
import CF.DevicePackage.UsageType;
import CF.LifeCyclePackage.ReleaseError;

@Deprecated
/**
 * This class has been deprecated, suggest class Device instead
 */
public abstract class Device_impl extends Resource implements DeviceOperations {

    protected DeviceManager devMgr;
    protected AggregateDevice compositeDevice;
    protected Device device;

    protected String softwareProfile;
    protected String label;
    protected AdminType adminState;
    protected UsageType usageState;
    protected OperationalType operationState;

    public Device_impl(final DeviceManager devMgr, final String compId, final String label, final String softwareProfile, final ORB orb, final POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy {
        setup(devMgr, null, compId, label, softwareProfile, orb, poa);
    }

    public Device_impl(final DeviceManager devMgr, final AggregateDevice compositeDevice, final String compId, final String label, final String softwareProfile, final ORB orb, final POA poa) throws InvalidObjectReference, ServantNotActive, WrongPolicy {
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

    public boolean allocateCapacity(DataType[] capacities)
    throws InvalidCapacity, InvalidState {
        // TODO Auto-generated method stub
        return false;
    }

    public void deallocateCapacity(DataType[] capacities)
    throws InvalidCapacity, InvalidState {
        // TODO Auto-generated method stub

    }

    public void releaseObject() throws ReleaseError {
        if (adminState == AdminType.UNLOCKED) {
            adminState = AdminType.SHUTTING_DOWN;
        }

        try {
            // Release all registered childDevices
            
            if (compositeDevice != null) {
                compositeDevice.removeDevice(device);
            }

            // Unregister device
            if (devMgr != null) {
                System.out.println("Unregistering device");
                devMgr.unregisterDevice(device);
            }
        } catch (Exception e) {
            e.printStackTrace();
            throw new ReleaseError(new String[] {e.toString()});
        }
        super.releaseObject();
        System.out.println("Released device");
        adminState = AdminType.LOCKED;
    }

    /**
     * Protected constructor and initializer to be used by start_device
     */
    protected Device_impl() {
        super();
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
    protected Device setup(final DeviceManager devMgr, 
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

        final Device_impl device_i = (Device_impl)clazz.newInstance();
        final Device device = device_i.setup(devMgr, compositeDevice, identifier, label, profile, orb, rootpoa);
        device_i.initializeProperties(execparams);
        
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
}
