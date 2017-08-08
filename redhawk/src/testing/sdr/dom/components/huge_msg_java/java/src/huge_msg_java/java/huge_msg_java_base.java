package huge_msg_java.java;


import java.util.Properties;

import org.apache.log4j.Logger;

import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import CF.InvalidObjectReference;

import org.ossie.component.*;
import org.ossie.properties.*;

import org.ossie.events.MessageSupplierPort;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: huge_msg_java.spd.xml
 *
 * @generated
 */

public abstract class huge_msg_java_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(huge_msg_java_base.class.getName());

    /**
     * The property my_msg
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property my_msg
     * 
     * @generated
     */
    public static class my_msg_struct extends StructDef {
        public final StringProperty string_payload =
            new StringProperty(
                "string_payload", //id
                null, //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public my_msg_struct(String string_payload) {
            this();
            this.string_payload.setValue(string_payload);
        }
    
        /**
         * @generated
         */
        public void set_string_payload(String string_payload) {
            this.string_payload.setValue(string_payload);
        }
        public String get_string_payload() {
            return this.string_payload.getValue();
        }
    
        /**
         * @generated
         */
        public my_msg_struct() {
            addElement(this.string_payload);
        }
    
        public String getId() {
            return "my_msg";
        }
    };
    
    public final StructProperty<my_msg_struct> my_msg =
        new StructProperty<my_msg_struct>(
            "my_msg", //id
            null, //name
            my_msg_struct.class, //type
            new my_msg_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.MESSAGE} //kind
            );
    
    // Uses/outputs
    /**
     * If the meaning of this port isn't clear, a description should be added.
     *
     * @generated
     */
    public MessageSupplierPort port_output;

    /**
     * @generated
     */
    public huge_msg_java_base()
    {
        super();

        setLogger( logger, huge_msg_java_base.class.getName() );


        // Properties
        addProperty(my_msg);


        // Uses/outputs
        this.port_output = new MessageSupplierPort("output");
        this.addPort("output", this.port_output);
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
     * The main function of your component.  If no args are provided, then the
     * CORBA object is not bound to an SCA Domain or NamingService and can
     * be run as a standard Java application.
     * 
     * @param args
     * @generated
     */
    public static void main(String[] args) 
    {
        final Properties orbProps = new Properties();
        huge_msg_java.configureOrb(orbProps);

        try {
            Component.start_component(huge_msg_java.class, args, orbProps);
        } catch (InvalidObjectReference e) {
            e.printStackTrace();
        } catch (NotFound e) {
            e.printStackTrace();
        } catch (CannotProceed e) {
            e.printStackTrace();
        } catch (InvalidName e) {
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
