/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package ECM_JAVA.java;

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
 * Source: ECM_JAVA.spd.xml
 *
 * @generated
 */

public abstract class ECM_JAVA_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(ECM_JAVA_base.class.getName());

    /**
     * The property msg_limit
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongProperty msg_limit =
        new LongProperty(
            "msg_limit", //id
            "msg_limit", //name
            5, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property msg_recv
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongProperty msg_recv =
        new LongProperty(
            "msg_recv", //id
            "msg_recv", //name
            0, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property msg_xmit
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongProperty msg_xmit =
        new LongProperty(
            "msg_xmit", //id
            "msg_xmit", //name
            0, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property enablecb
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final BooleanProperty enablecb =
        new BooleanProperty(
            "enablecb", //id
            "enablecb", //name
            false, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE,Kind.PROPERTY} //kind} //kind
            );
    
    /**
     * @generated
     */
    public ECM_JAVA_base()
    {
        super();

        // Properties
        addProperty(msg_limit);

        addProperty(msg_recv);

        addProperty(msg_xmit);

        addProperty(enablecb);

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
        ECM_JAVA.configureOrb(orbProps);

        try {
            Component.start_component(ECM_JAVA.class, args, orbProps);
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
