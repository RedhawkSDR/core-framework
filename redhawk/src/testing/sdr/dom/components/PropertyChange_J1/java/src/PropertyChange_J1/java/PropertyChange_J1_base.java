package PropertyChange_J1.java;

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
 * Source: PropertyChange_J1.spd.xml
 *
 * @generated
 */

public abstract class PropertyChange_J1_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(PropertyChange_J1_base.class.getName());

    /**
     * The property prop1
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final FloatProperty prop1 =
        new FloatProperty(
            "prop1", //id
            "prop1", //name
            1.0F, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property prop2
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongProperty prop2 =
        new LongProperty(
            "prop2", //id
            "prop2", //name
            1, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * @generated
     */
    public PropertyChange_J1_base()
    {
        super();

        // Properties
        addProperty(prop1);

        addProperty(prop2);

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
        PropertyChange_J1.configureOrb(orbProps);

        try {
            Component.start_component(PropertyChange_J1.class, args, orbProps);
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
