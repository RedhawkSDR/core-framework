package emptyString.java;

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


public abstract class EmptyString_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(EmptyString_base.class.getName());

    /**
     * The property estr
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty estr =
        new StringProperty(
            "estr", //id
            "estr", //name
            "", //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );

    /**
     * @generated
     */
    public EmptyString_base()
    {
        super();

        setLogger( logger, EmptyString_base.class.getName() );


        // Properties
        addProperty(estr);

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
        EmptyString.configureOrb(orbProps);

        try {
            Component.start_component(EmptyString.class, args, orbProps);
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
