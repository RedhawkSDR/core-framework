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
package java_dev.java;

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
 * Source: java_dev.spd.xml
 *
 * @generated
 */
public abstract class java_dev_base extends ThreadedDevice {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(java_dev_base.class.getName());

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
            null, //default value
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
            null, //default value
            Mode.READONLY, //mode
            Action.EQ, //action
            new Kind[] {Kind.ALLOCATION,Kind.CONFIGURE} //kind
            );
    
    /**
     * The property devmgr_id
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty devmgr_id =
        new StringProperty(
            "devmgr_id", //id
            null, //name
            null, //default value
            Mode.READONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property dom_id
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty dom_id =
        new StringProperty(
            "dom_id", //id
            null, //name
            null, //default value
            Mode.READONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * @generated
     */
    public java_dev_base()
    {
        super();

        // Properties
        addProperty(device_kind);
        addProperty(device_model);
        addProperty(devmgr_id);
        addProperty(dom_id);
    }

    public void start() throws CF.ResourcePackage.StartError
    {
        super.start();
    }

    public void stop() throws CF.ResourcePackage.StopError
    {
        super.stop();
    }

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
        java_dev.configureOrb(orbProps);

        try {
            ThreadedDevice.start_device(java_dev.class, args, orbProps);
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
