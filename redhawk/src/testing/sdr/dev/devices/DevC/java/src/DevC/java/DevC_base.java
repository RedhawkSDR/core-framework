package DevC.java;

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
 * Source: DevC.spd.xml
 *
 * @generated
 */

public abstract class DevC_base extends ThreadedDevice {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(DevC_base.class.getName());

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
            new Kind[] {Kind.ALLOCATION}
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
            new Kind[] {Kind.ALLOCATION}
            );
    
    /**
     * The property myulong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ULongProperty myulong =
        new ULongProperty(
            "myulong", //id
            "myulong", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.ALLOCATION, Kind.PROPERTY}
            );
    
    /**
     * @generated
     */
    public DevC_base()
    {
        super();

        setLogger( logger, DevC_base.class.getName() );


        // Properties
        addProperty(device_kind);

        addProperty(device_model);

        addProperty(myulong);

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
        DevC.configureOrb(orbProps);

        try {
            ThreadedDevice.start_device(DevC.class, args, orbProps);
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
