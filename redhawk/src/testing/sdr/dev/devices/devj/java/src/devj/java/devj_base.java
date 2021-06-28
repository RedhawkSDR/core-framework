package devj.java;

import java.lang.reflect.InvocationTargetException;
import java.util.Properties;
import org.ossie.component.RHLogger;

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
 * Source: devj.spd.xml
 *
 * @generated
 */

public abstract class devj_base extends ThreadedDevice 
{
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(devj_base.class.getName());

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
            new Kind[] {Kind.ALLOCATION,Kind.PROPERTY}
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
            new Kind[] {Kind.ALLOCATION,Kind.PROPERTY}
            );
    
    /**
     * The property LOGGING_CONFIG_URI
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty LOGGING_CONFIG_URI =
        new StringProperty(
            "LOGGING_CONFIG_URI", //id
            null, //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property busy_state
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final BooleanProperty busy_state =
        new BooleanProperty(
            "busy_state", //id
            "busy_state", //name
            false, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property a_number
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ShortProperty a_number =
        new ShortProperty(
            "a_number", //id
            "a_number", //name
            (short)100, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.ALLOCATION}
            );
    
    /**
     * @generated
     */
    public devj_base()
    {
        super();

        setLogger( logger, devj_base.class.getName() );


        // Properties
        addProperty(device_kind);

        addProperty(device_model);

        addProperty(LOGGING_CONFIG_URI);

        addProperty(busy_state);

        addProperty(a_number);

    }


    protected void setupPortLoggers() {
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
        devj.configureOrb(orbProps);

        try {
            ThreadedDevice.start_device(devj.class, args, orbProps);
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
        } catch (NoSuchMethodException e) {
            e.printStackTrace();
        } catch (InvocationTargetException e) {
            e.printStackTrace();
        }
    }
}
