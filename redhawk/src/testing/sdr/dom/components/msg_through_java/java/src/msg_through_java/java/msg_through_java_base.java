package msg_through_java.java;


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

import org.ossie.events.MessageConsumerPort;
import org.ossie.events.MessageSupplierPort;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: msg_through_java.spd.xml
 *
 * @generated
 */

public abstract class msg_through_java_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(msg_through_java_base.class.getName());

    /**
     * The property foo
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property foo
     * 
     * @generated
     */
    public static class foo_struct extends StructDef {
        public final StringProperty a =
            new StringProperty(
                "a", //id
                null, //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final StringProperty b =
            new StringProperty(
                "b", //id
                null, //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final StringProperty c =
            new StringProperty(
                "c", //id
                null, //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public foo_struct(String a, String b, String c) {
            this();
            this.a.setValue(a);
            this.b.setValue(b);
            this.c.setValue(c);
        }
    
        /**
         * @generated
         */
        public void set_a(String a) {
            this.a.setValue(a);
        }
        public String get_a() {
            return this.a.getValue();
        }
        public void set_b(String b) {
            this.b.setValue(b);
        }
        public String get_b() {
            return this.b.getValue();
        }
        public void set_c(String c) {
            this.c.setValue(c);
        }
        public String get_c() {
            return this.c.getValue();
        }
    
        /**
         * @generated
         */
        public foo_struct() {
            addElement(this.a);
            addElement(this.b);
            addElement(this.c);
        }
    
        public String getId() {
            return "foo";
        }
    };
    
    public final StructProperty<foo_struct> foo =
        new StructProperty<foo_struct>(
            "foo", //id
            null, //name
            foo_struct.class, //type
            new foo_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.MESSAGE} //kind
            );
    
    // Provides/inputs
    /**
     * If the meaning of this port isn't clear, a description should be added.
     *
     * @generated
     */
    public MessageConsumerPort port_input;

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
    public msg_through_java_base()
    {
        super();

        setLogger( logger, msg_through_java_base.class.getName() );


        // Properties
        addProperty(foo);


        // Provides/inputs
        this.port_input = new MessageConsumerPort("input", this.logger);
        this.addPort("input", this.port_input);

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
        msg_through_java.configureOrb(orbProps);

        try {
            Component.start_component(msg_through_java.class, args, orbProps);
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
