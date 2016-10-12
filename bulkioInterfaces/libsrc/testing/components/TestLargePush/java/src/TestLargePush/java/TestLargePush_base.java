/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package TestLargePush.java;

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

import bulkio.*;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: TestLargePush.spd.xml
 *
 * @generated
 */
public abstract class TestLargePush_base extends Resource implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestLargePush_base.class.getName());

    /**
     * Return values for service function.
     */
    public final static int FINISH = -1;
    public final static int NOOP   = 0;
    public final static int NORMAL = 1;

    /**
     * The property numSamples
     * number of samples to send in the push.  should be large enough to exceed the max message size for a CORBA packet, which will force the packet chunking to be exercised.
     *
     * @generated
     */
    public final ULongLongProperty numSamples =
        new ULongLongProperty(
            "numSamples", //id
            null, //name
            3000000L, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    // Provides/inputs
    // Uses/outputs
    /**
     * @generated
     */
    public OutCharPort port_dataChar;

    /**
     * @generated
     */
    public OutFilePort port_dataFile;

    /**
     * @generated
     */
    public OutShortPort port_dataShort;

    /**
     * @generated
     */
    public OutULongPort port_dataUlong;

    /**
     * @generated
     */
    public OutULongLongPort port_dataUlongLong;

    /**
     * @generated
     */
    public OutUShortPort port_dataUshort;

    /**
     * @generated
     */
    public OutXMLPort port_dataXML;

    /**
     * @generated
     */
    public OutLongPort port_dataLong;

    /**
     * @generated
     */
    public OutLongLongPort port_dataLongLong;

    /**
     * @generated
     */
    public OutOctetPort port_dataOctet;

    /**
     * @generated
     */
    public OutFloatPort port_dataFloat;

    /**
     * @generated
     */
    public OutSDDSPort port_dataSDDS;

    /**
     * @generated
     */
    public OutDoublePort port_dataDouble;

    /**
     * @generated
     */
    public TestLargePush_base()
    {
        super();
        addProperty(numSamples);

        // Provides/input

        // Uses/output
        this.port_dataChar = new OutCharPort("dataChar");
        this.addPort("dataChar", this.port_dataChar);
        this.port_dataFile = new OutFilePort("dataFile");
        this.addPort("dataFile", this.port_dataFile);
        this.port_dataShort = new OutShortPort("dataShort");
        this.addPort("dataShort", this.port_dataShort);
        this.port_dataUlong = new OutULongPort("dataUlong");
        this.addPort("dataUlong", this.port_dataUlong);
        this.port_dataUlongLong = new OutULongLongPort("dataUlongLong");
        this.addPort("dataUlongLong", this.port_dataUlongLong);
        this.port_dataUshort = new OutUShortPort("dataUshort");
        this.addPort("dataUshort", this.port_dataUshort);
        this.port_dataXML = new OutXMLPort("dataXML");
        this.addPort("dataXML", this.port_dataXML);
        this.port_dataLong = new OutLongPort("dataLong");
        this.addPort("dataLong", this.port_dataLong);
        this.port_dataLongLong = new OutLongLongPort("dataLongLong");
        this.addPort("dataLongLong", this.port_dataLongLong);
        this.port_dataOctet = new OutOctetPort("dataOctet");
        this.addPort("dataOctet", this.port_dataOctet);
        this.port_dataFloat = new OutFloatPort("dataFloat");
        this.addPort("dataFloat", this.port_dataFloat);
        this.port_dataSDDS = new OutSDDSPort("dataSDDS");
        this.addPort("dataSDDS", this.port_dataSDDS);
        this.port_dataDouble = new OutDoublePort("dataDouble");
        this.addPort("dataDouble", this.port_dataDouble);
    }

    public void run() 
    {
        while(this.started())
        {
            int state = this.serviceFunction();
            if (state == NOOP) {
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    break;
                }
            } else if (state == FINISH) {
                return;
            }
        }
    }

    protected abstract int serviceFunction();

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
        TestLargePush.configureOrb(orbProps);

        try {
            Resource.start_component(TestLargePush.class, args, orbProps);
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
