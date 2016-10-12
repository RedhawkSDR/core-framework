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
package JavaTestDevice.java;

import java.util.HashMap;
import java.util.Properties;

import org.apache.log4j.Logger;

import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import CF.DevicePackage.AdminType;
import CF.DevicePackage.OperationalType;
import CF.DevicePackage.UsageType;
import CF.InvalidObjectReference;

import org.ossie.component.*;
import org.ossie.properties.*;

/**
 * This is the device code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general device housekeeping
 *
 * Source: JavaTestDevice.spd.xml
 *
 * @generated
 */
public abstract class JavaTestDevice_base extends Device implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(JavaTestDevice_base.class.getName());

    /**
     * Return values for service function.
     */
    public final static int FINISH = -1;
    public final static int NOOP   = 0;
    public final static int NORMAL = 1;

    /**
     * The property DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d
     * This specifies the device kind
     *
     * @generated
     */
    public final StringProperty device_kind =
        new StringProperty(
            "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d", //id
            "device_kind", //name
            "TestDevice", //default value
            Mode.READONLY, //mode
            Action.EQ, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    /**
     * The property DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb
     *  This specifies the specific device
     *
     * @generated
     */
    public final StringProperty device_model =
        new StringProperty(
            "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb", //id
            "device_model", //name
            "JavaTestDevice", //default value
            Mode.READONLY, //mode
            Action.EQ, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    /**
     * The property disk_space
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongLongProperty disk_space =
        new LongLongProperty(
            "disk_space", //id
            "disk_space", //name
            100000000000L, //default value
            Mode.READONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    /**
     * The property load_average
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final FloatProperty load_average =
        new FloatProperty(
            "load_average", //id
            "load_average", //name
            0.0F, //default value
            Mode.READONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );

    /**
     * The property memory_capacity
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongLongProperty memory_capacity =
        new LongLongProperty(
            "memory_capacity", //id
            "memory_capacity", //name
            2147483648L, //default value
            Mode.READONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property shared_memory
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongProperty shared_memory =
        new LongProperty(
            "shared_memory", //id
            "shared_memory", //name
            33554432, //default value
            Mode.READONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The structure for property memory_allocation
     * 
     * @generated
     */
    public static class memory_allocation_struct extends StructDef {
        /**
         * The property capacity
         * If the meaning of this property isn't clear, a description should be added.
         *
         * @generated
         */
        public final LongProperty capacity =
            new LongProperty(
                "capacity", //id
                "capacity", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property contiguous
         * If the meaning of this property isn't clear, a description should be added.
         *
         * @generated
         */
        public final BooleanProperty contiguous =
            new BooleanProperty(
                "contiguous", //id
                "contiguous", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        /**
         * The property memory_type
         * If the meaning of this property isn't clear, a description should be added.
         *
         * @generated
         */
        public final LongProperty memory_type =
            new LongProperty(
                "memory_type", //id
                "memory_type", //name
                0, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public memory_allocation_struct(Integer capacity, Boolean contiguous, Integer memory_type) {
            this();
            this.capacity.setValue(capacity);
            this.contiguous.setValue(contiguous);
            this.memory_type.setValue(memory_type);
        }
    
        /**
         * @generated
         */
        public memory_allocation_struct() {
            addElement(this.capacity);
            addElement(this.contiguous);
            addElement(this.memory_type);
        }
    };
    
    /**
     * The property memory_allocation
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StructProperty<memory_allocation_struct> memory_allocation =
        new StructProperty<memory_allocation_struct>(
            "memory_allocation", //id
            "memory_allocation", //name
            memory_allocation_struct.class, //type
            new memory_allocation_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );

    // Provides/inputs
    // Uses/outputs
    /**
     * @generated
     */
    public JavaTestDevice_base()
    {
        super();
        this.usageState = UsageType.IDLE;
        this.operationState = OperationalType.ENABLED;
        this.adminState = AdminType.UNLOCKED;
        this.callbacks = new HashMap<String, AllocCapacity>();
        addProperty(device_kind);
        addProperty(device_model);
        addProperty(disk_space);
        addProperty(load_average);
        addProperty(memory_capacity);
        addProperty(shared_memory);
        addProperty(memory_allocation);

        // Provides/input

        // Uses/output
    }

    public void run() 
    {
        while(this.started())
        {
            int state = this.serviceFunction();
            if (state == NOOP) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    break;
                }
            } else if (state == FINISH) {
                return;
            }
        }
    }

    protected abstract int serviceFunction();

    /**
     * The main function of your device.  If no args are provided, then the
     * CORBA object is not bound to an SCA Domain or NamingService and can
     * be run as a standard Java application.
     * 
     * @param args
     * @generated
     */
    public static void main(String[] args) 
    {
        final Properties orbProps = new Properties();
        JavaTestDevice.configureOrb(orbProps);

        try {
            Device.start_device(JavaTestDevice.class, args, orbProps);
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
        } catch (ServantNotActive e) {
            e.printStackTrace();
        } catch (WrongPolicy e) {
            e.printStackTrace();
        } catch (InstantiationException e) {
            e.printStackTrace();
        } catch (IllegalAccessException e) {
            e.printStackTrace();
        }
    }
}
