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
package TestLoggingAPI.java;

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
 * Source: TestLoggingAPI.spd.xml
 *
 * @generated
 */
public abstract class TestLoggingAPI_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestLoggingAPI_base.class.getName());

    /**
     * The property new_log_level
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final LongProperty new_log_level =
        new LongProperty(
            "new_log_level", //id
            "new_log_level", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property new_log_cfg
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final StringProperty new_log_cfg =
        new StringProperty(
            "new_log_cfg", //id
            "new_log_cfg", //name
            null, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );

    /**
     * The property DISABLE_CB
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final BooleanProperty disable_cb =
        new BooleanProperty(
            "DISABLE_CB", //id
            "disable_cb", //name
            false, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );

    /**
     * @generated
     */
    public TestLoggingAPI_base()
    {
        super();

        setLogger( logger, TestLoggingAPI_base.class.getName() );

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
        TestLoggingAPI.configureOrb(orbProps);

        try {
            ThreadedResource.start_component(TestLoggingAPI.class, args, orbProps);
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
