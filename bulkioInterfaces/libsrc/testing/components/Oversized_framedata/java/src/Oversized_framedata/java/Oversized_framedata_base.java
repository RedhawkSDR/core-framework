package Oversized_framedata.java;

import java.util.Properties;

import org.apache.log4j.Logger;

import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;

import CF.InvalidObjectReference;

import org.ossie.component.*;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: Oversized_framedata.spd.xml
 *
 * @generated
 */
public abstract class Oversized_framedata_base extends ThreadedResource {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(Oversized_framedata_base.class.getName());

    // Uses/outputs
    /**
     * @generated
     */
    public bulkio.OutShortPort port_dataShort_out;

    /**
     * @generated
     */
    public Oversized_framedata_base()
    {
        super();

        // Uses/outputs
        this.port_dataShort_out = new bulkio.OutShortPort("dataShort_out");
        this.addPort("dataShort_out", this.port_dataShort_out);
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
        Oversized_framedata.configureOrb(orbProps);

        try {
            ThreadedResource.start_component(Oversized_framedata.class, args, orbProps);
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
