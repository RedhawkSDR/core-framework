package time_ja_now.java;


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
 * Source: time_ja_now.spd.xml
 *
 * @generated
 */

public abstract class time_ja_now_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(time_ja_now_base.class.getName());

    /**
     * The property rightnow
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final UTCTimeProperty rightnow =
        new UTCTimeProperty(
            "rightnow", //id
            null, //name
            "now", //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    /**
     * The property simple1970
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final UTCTimeProperty simple1970 =
        new UTCTimeProperty(
            "simple1970", //id
            null, //name
            "1970:01:01::00:00:00", //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property simpleSeqDefNow
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final UTCTimeSequenceProperty simpleSeqDefNow =
        new UTCTimeSequenceProperty(
            "simpleSeqDefNow", //id
            null, //name
            UTCTimeSequenceProperty.asList("now"), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property simpleSeqNoDef
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final UTCTimeSequenceProperty simpleSeqNoDef =
        new UTCTimeSequenceProperty(
            "simpleSeqNoDef", //id
            null, //name
            UTCTimeSequenceProperty.asList(), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property simpleSeq1970
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final UTCTimeSequenceProperty simpleSeq1970 =
        new UTCTimeSequenceProperty(
            "simpleSeq1970", //id
            null, //name
            UTCTimeSequenceProperty.asList("1970:01:01::00:00:00"), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * @generated
     */
    public time_ja_now_base()
    {
        super();

        setLogger( logger, time_ja_now_base.class.getName() );


        // Properties
        addProperty(rightnow);

        addProperty(simple1970);

        addProperty(simpleSeqDefNow);

        addProperty(simpleSeqNoDef);

        addProperty(simpleSeq1970);

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
        time_ja_now.configureOrb(orbProps);

        try {
            Component.start_component(time_ja_now.class, args, orbProps);
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
