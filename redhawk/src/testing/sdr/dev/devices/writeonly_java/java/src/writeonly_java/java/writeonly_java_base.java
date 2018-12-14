package writeonly_java.java;


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
 * Source: writeonly_java.spd.xml
 *
 * @generated
 */

public abstract class writeonly_java_base extends ThreadedDevice {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(writeonly_java_base.class.getName());

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
     * The property foo
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty foo =
        new StringProperty(
            "foo", //id
            null, //name
            "something", //default value
            Mode.WRITEONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.ALLOCATION}
            );
    
    /**
     * The property foo_seq
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringSequenceProperty foo_seq =
        new StringSequenceProperty(
            "foo_seq", //id
            null, //name
            StringSequenceProperty.asList("abc"), //default value
            Mode.WRITEONLY, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.ALLOCATION}
            );
    
    /**
     * The property foo_struct
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property foo_struct
     * 
     * @generated
     */
    public static class foo_struct_struct extends StructDef {
        public final StringProperty abc =
            new StringProperty(
                "abc", //id
                null, //name
                "def", //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public foo_struct_struct(String abc) {
            this();
            this.abc.setValue(abc);
        }
    
        /**
         * @generated
         */
        public void set_abc(String abc) {
            this.abc.setValue(abc);
        }
        public String get_abc() {
            return this.abc.getValue();
        }
    
        /**
         * @generated
         */
        public foo_struct_struct() {
            addElement(this.abc);
        }
    
        public String getId() {
            return "foo_struct";
        }
    };
    
    public final StructProperty<foo_struct_struct> foo_struct =
        new StructProperty<foo_struct_struct>(
            "foo_struct", //id
            null, //name
            foo_struct_struct.class, //type
            new foo_struct_struct(), //default value
            Mode.WRITEONLY, //mode
            new Kind[] {Kind.ALLOCATION} //kind
            );
    
    /**
     * The property foo_struct_seq
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property ghi
     * 
     * @generated
     */
    public static class ghi_struct extends StructDef {
        public final StringProperty jkl =
            new StringProperty(
                "jkl", //id
                null, //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public ghi_struct(String jkl) {
            this();
            this.jkl.setValue(jkl);
        }
    
        /**
         * @generated
         */
        public void set_jkl(String jkl) {
            this.jkl.setValue(jkl);
        }
        public String get_jkl() {
            return this.jkl.getValue();
        }
    
        /**
         * @generated
         */
        public ghi_struct() {
            addElement(this.jkl);
        }
    
        public String getId() {
            return "ghi";
        }
    };
    
    public final StructSequenceProperty<ghi_struct> foo_struct_seq =
        new StructSequenceProperty<ghi_struct> (
            "foo_struct_seq", //id
            null, //name
            ghi_struct.class, //type
            StructSequenceProperty.asList(
                new ghi_struct("mno")
                ), //defaultValue
            Mode.WRITEONLY, //mode
            new Kind[] { Kind.ALLOCATION } //kind
        );
    
    /**
     * @generated
     */
    public writeonly_java_base()
    {
        super();

        setLogger( logger, writeonly_java_base.class.getName() );


        // Properties
        addProperty(device_kind);

        addProperty(device_model);

        addProperty(foo);

        addProperty(foo_seq);

        addProperty(foo_struct);

        addProperty(foo_struct_seq);

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
        writeonly_java.configureOrb(orbProps);

        try {
            ThreadedDevice.start_device(writeonly_java.class, args, orbProps);
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
