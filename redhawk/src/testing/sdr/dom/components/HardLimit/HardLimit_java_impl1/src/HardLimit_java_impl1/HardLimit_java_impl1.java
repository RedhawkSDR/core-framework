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

package HardLimit_java_impl1;


import java.util.Properties;
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import CF.PropertiesHolder;
import CF.ResourceHelper;
import CF.UnknownProperties;
import CF.LifeCyclePackage.InitializeError;
import CF.LifeCyclePackage.ReleaseError;
import CF.InvalidObjectReference;
import CF.PropertySetPackage.InvalidConfiguration;
import CF.PropertySetPackage.PartialConfiguration;
import CF.ResourcePackage.StartError;
import CF.ResourcePackage.StopError;
import CF.DataType;
import org.omg.CORBA.UserException;
import org.omg.CosNaming.NameComponent;
import org.apache.log4j.Logger;
import org.ossie.component.*;
import org.ossie.properties.*;
import javaDep.javaDep;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 */
public class HardLimit_java_impl1 extends Resource implements Runnable {
 
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(HardLimit_java_impl1.class.getName());

    /**
     * The property DCE:06281569-8b63-4034-961e-665b36b33cf1.
     * Sets the upper limit threshold before modification of the input if triggered.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final DoubleProperty upper_limit =
       new DoubleProperty(
           "DCE:06281569-8b63-4034-961e-665b36b33cf1", // id
           "upper_limit", // name
           0.5, // default value
           Mode.READWRITE, // mode
           Action.EXTERNAL, // action
           new Kind[] {Kind.CONFIGURE,} // kind
           );
            
    /**
     * The property DCE:18a49108-e4c0-40b7-a193-aac98eb52a1d.
     * Sets the lower limit threshold before modification of the input if triggered.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final DoubleProperty lower_limit =
       new DoubleProperty(
           "DCE:18a49108-e4c0-40b7-a193-aac98eb52a1d", // id
           "lower_limit", // name
           -0.5, // default value
           Mode.READWRITE, // mode
           Action.EXTERNAL, // action
           new Kind[] {Kind.CONFIGURE,} // kind
           );
            
    /**
     * The property DCE:a49d55c0-ad66-4605-aa42-08728f2a2aa3.
     * The upper output value if the 'upper_limit' is exceeded.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final DoubleProperty upper_val =
       new DoubleProperty(
           "DCE:a49d55c0-ad66-4605-aa42-08728f2a2aa3", // id
           "upper_val", // name
           1.0, // default value
           Mode.READWRITE, // mode
           Action.EXTERNAL, // action
           new Kind[] {Kind.CONFIGURE,} // kind
           );
            
    /**
     * The property DCE:565a1d4f-ee69-48ae-89d7-403ccf399b2b.
     * The lower output value if the 'lower_limit' is exceeded.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final DoubleProperty lower_val =
       new DoubleProperty(
           "DCE:565a1d4f-ee69-48ae-89d7-403ccf399b2b", // id
           "lower_val", // name
           -1.0, // default value
           Mode.READWRITE, // mode
           Action.EXTERNAL, // action
           new Kind[] {Kind.CONFIGURE,} // kind
           );
           

    /**
     * @generated
     */
    public HardLimit_java_impl1() 
    {
        super();
        addProperty(upper_limit);
        addProperty(lower_limit);
        addProperty(upper_val);
        addProperty(lower_val);
        javaDep mydep = new javaDep();

       //begin-user-code
       //end-user-code
    }
    
    /**
     *
     * Main processing thread
     *
     * <!-- begin-user-doc -->
     * 
     * General functionality:
     * 
     *       This function is running as a separate thread from the component's main thread. 
     *    
     *    The IDE uses JMerge during the generation (and re-generation) process.  To keep
     *    customizations to this file from being over-written during subsequent generations,
     *    put your customization in between the following tags:
     *        - //begin-user-code
     *      - //end-user-code
     *    or, alternatively, set the @generated flag located before the code you wish to 
     *    modify, in the following way:
     *      - "@generated NOT"
     * 
     * Ports:
     * 
     *       Each port instance is accessed through members of the following form: this.port_<PORT NAME>
     * 
     *       Data is obtained in the run function through the getPacket call (BULKIO only) on a
     *    provides port member instance. The getPacket function call is non-blocking; it takes
     *    one argument which is the time to wait on new data. If you pass 0, it will return
     *    immediately if no data available (won't wait).
     *    
     *    To send data, call the appropriate function in the port directly. In the case of BULKIO,
     *    convenience functions have been added in the port classes that aid in output.
     *    
     *    Interactions with non-BULKIO ports are left up to the component developer's discretion.
     *    
     * Properties:
     * 
     *       Properties are accessed through members of the same name with helper functions. If the 
     *       property name is baudRate, then reading the value is achieved by: this.baudRate.getValue();
     *    and writing a new value is achieved by: this.baudRate.setValue(new_value);
     *    
     * Example:
     * 
     *       // This example assumes that the component has two ports:
     *    //     - A provides (input) port of type BULKIO::dataShort called dataShort_in
     *    //     - A uses (output) port of type BULKIO::dataFloat called dataFloat_out
     *    // The mapping between the port and the class is found the class of the same name.
     *    // This example also makes use of the following Properties:
     *    //    - A float value called amplitude
     *    //    - A boolean called increaseAmplitude
     *    
     *    BULKIO_dataShortInPort.Packet<short[]> data = this.port_dataShort_in.getPacket(125);
     *    
     *    float[] outData = new float[data.getData().length];
     *    if (data != null) {
     *          for (int i = 0; i < data.getData().length; i++) {
     *              if (this.increaseAmplitude.getValue()) {
     *                  outData[i] = (float)data[i] * this.amplitude;
     *            } else {
     *                outData[i] = (float)data[i];
     *            }
     *          }
     *            
     *        this.port_dataFloat_out.pushPacket(outData, data.getTime(), data.getEndOfStream(), data.getStreamID());
     *    }
     * 
     *      
     * <!-- end-user-doc -->
     * 
     * @generated
     */
    public void run() 
    {
        //begin-user-code
        //end-user-code
        
        while(this.started())
        {
            //begin-user-code
            // Process data here
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
                break;
            }
            //end-user-code
        }
        
        //begin-user-code
        //end-user-code
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

        //begin-user-code
        // TODO You may add extra startup code here, for example:
        // orbProps.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
        //end-user-code

        try {
            Resource.start_component(HardLimit_java_impl1.class, args, orbProps);
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

        //begin-user-code
        // TODO You may add extra shutdown code here
        //end-user-code
    }
}
