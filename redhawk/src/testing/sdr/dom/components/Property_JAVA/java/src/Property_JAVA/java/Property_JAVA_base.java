package Property_JAVA.java;

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


/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: Property_JAVA.spd.xml
 *
 * @generated
 */

public abstract class Property_JAVA_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(Property_JAVA_base.class.getName());

    /**
     * The property p1
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty p1 =
        new StringProperty(
            "p1", //id
            null, //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property p2
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final DoubleProperty p2 =
        new DoubleProperty(
            "p2", //id
            null, //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property p3
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongProperty p3 =
        new LongProperty(
            "p3", //id
            null, //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property p4
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property p4
     * 
     * @generated
     */
    public static class p4_struct extends StructDef {
        public final StringProperty p4sub1 =
            new StringProperty(
                "p4sub1", //id
                null, //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.PROPERTY}
                );
        public final FloatProperty p4sub2 =
            new FloatProperty(
                "p4sub2", //id
                null, //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.PROPERTY}
                );
    
        /**
         * @generated
         */
        public p4_struct(String p4sub1, Float p4sub2) {
            this();
            this.p4sub1.setValue(p4sub1);
            this.p4sub2.setValue(p4sub2);
        }
    
        /**
         * @generated
         */
        public p4_struct() {
            addElement(this.p4sub1);
            addElement(this.p4sub2);
        }
    
        public String getId() {
            return "p4";
        }
    };
    
    public final StructProperty<p4_struct> p4 =
        new StructProperty<p4_struct>(
            "p4", //id
            null, //name
            p4_struct.class, //type
            new p4_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.PROPERTY} //kind
            );
    
    /**
     * @generated
     */
    public Property_JAVA_base()
    {
        super();

        // Properties
        addProperty(p1);

        addProperty(p2);

        addProperty(p3);

        addProperty(p4);

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
        Property_JAVA.configureOrb(orbProps);

        try {
            Component.start_component(Property_JAVA.class, args, orbProps);
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
