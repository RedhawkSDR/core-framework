package timeprop_java.java;


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
 * Source: timeprop_java.spd.xml
 *
 * @generated
 */

public abstract class timeprop_java_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(timeprop_java_base.class.getName());

    /**
     * The property prop
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty prop =
        new StringProperty(
            "prop", //id
            null, //name
            "value", //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * @generated
     */
    public timeprop_java_base()
    {
        super();

        setLogger( logger, timeprop_java_base.class.getName() );


        // Properties
        addProperty(prop);

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
        timeprop_java.configureOrb(orbProps);

        try {
            Component.start_component(timeprop_java.class, args, orbProps);
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
