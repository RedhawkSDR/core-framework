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
import java.util.ArrayList;
import java.util.List;
import java.util.Arrays;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 * @generated
 */
public class TestLegacyJavaProps extends Resource implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestLegacyJavaProps.class.getName());
        
     
    /**
     * The property DCE:dd8d450f-d377-4c2c-8c3c-207e42dae017.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<Long> ulong_prop =
       new SimpleProperty<Long>(
           "DCE:dd8d450f-d377-4c2c-8c3c-207e42dae017", // id
           "ulong_prop", // name
           "ulong", // type
           null, // default value
           "readwrite", // mode
           "external", // action
           new String[] {"configure",} // kind
           );
    
    public final SimpleProperty<Long> non_queryable =
       new SimpleProperty<Long>(
           "non_queryable", // id
           "non_queryable", // name
           "ulong", // type
           null, // default value
           "writeonly", // mode
           "external", // action
           new String[] {"configure",} // kind
           );
    
    public final SimpleProperty<Long> exec_param =
       new SimpleProperty<Long>(
           "exec_param", // id
           "exec_param", // name
           "ulong", // type
           null, // default value
           "readwrite", // mode
           "external", // action
           new String[] {"execparam",} // kind
           );
    
    public final SimpleProperty<Long> non_query_exec_param =
       new SimpleProperty<Long>(
           "non_query_exec_param", // id
           "non_query_exec_param", // name
           "ulong", // type
           null, // default value
           "writeonly", // mode
           "external", // action
           new String[] {"execparam",} // kind
           );
           
    /**
     * The property DCE:3303ee57-70bb-4325-84ad-fb7fd333c44a.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleSequenceProperty< Integer > long_seq =
       new SimpleSequenceProperty< Integer >(
           "DCE:3303ee57-70bb-4325-84ad-fb7fd333c44a", // id
           "long_seq", // name
           "long", // type
           new ArrayList<Integer> (), // default value
           "readwrite", // mode
           "external", // action
           new String[] {"configure",} // kind
           );
    
    /**
     * The structure for property DCE:23a6d333-55fb-4425-a102-185e6e998782.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public static class struct_prop_struct extends StructDef { 
        /**
         * The struct field item_long.
         * If the meaning of this field isn't clear a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final SimpleProperty<Integer> field =
           new SimpleProperty<Integer>(
               "item_long", // id
               null, // name
               "long", // type
               0, // default value
               "readwrite", // mode
               "external", // action
               new String[] {"configure",} // kind
              ); 
        /**
         * The struct field item_string.
         * If the meaning of this field isn't clear a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final SimpleProperty<String> field_2 =
           new SimpleProperty<String>(
               "item_string", // id
               null, // name
               "string", // type
               "default", // default value
               "readwrite", // mode
               "external", // action
               new String[] {"configure",} // kind
              );
        
        /**
         * @generated
         */
        public struct_prop_struct() {
            addElement(field);
            addElement(field_2);
            //begin-user-code
            //end-user-code
        }
    };
     
    /**
     * The property DCE:23a6d333-55fb-4425-a102-185e6e998782.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructProperty<struct_prop_struct> struct_prop =
       new StructProperty<struct_prop_struct>(
           "DCE:23a6d333-55fb-4425-a102-185e6e998782", // id
           "struct_prop", // name
           new struct_prop_struct(), // type
           new struct_prop_struct(), // type
           "readwrite", // mode
           new String[] { "configure" }// kind
           );
    

    public final StructSequenceProperty<struct_prop_struct> structseq_prop =
       new StructSequenceProperty<struct_prop_struct>(
           "DCE:d933f8ba-9e79-4d2c-a1b5-9fb9da0ea740", // id
           "structseq_prop", // name
           struct_prop_struct.class, // type
           new ArrayList<struct_prop_struct>(Arrays.asList(new struct_prop_struct(), new struct_prop_struct())), // default
           "readwrite", // mode
           new String[] { "configure" }// kind
           );

    // Project/input)

    // Uses/outputs

    /**
     * @generated
     */
    public TestLegacyJavaProps() 
    {
        super();
        addProperty(ulong_prop);
        addProperty(non_queryable);
        addProperty(exec_param);
        addProperty(non_query_exec_param);
        addProperty(long_seq);
        addProperty(struct_prop);
        addProperty(structseq_prop);

        // Project/input

        // Uses/outputs
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
     *    This function is running as a separate thread from the component's main thread. 
     *    
     *    The IDE uses JMerge during the generation (and re-generation) process.  To keep
     *    customizations to this file from being over-written during subsequent generations,
     *    put your customization in between the following tags:
     *      - //begin-user-code
     *      - //end-user-code
     *    or, alternatively, set the @generated flag located before the code you wish to 
     *    modify, in the following way:
     *      - "@generated NOT"
     * 
     * Ports:
     * 
     *    Each port instance is accessed through members of the following form: this.port_<PORT NAME>
     * 
     *    Data is obtained in the run function through the getPacket call (BULKIO only) on a
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
     *    Properties are accessed through members of the same name with helper functions. If the 
     *    property name is baudRate, then reading the value is achieved by: this.baudRate.getValue();
     *    and writing a new value is achieved by: this.baudRate.setValue(new_value);
     *    
     * Example:
     * 
     *    This example assumes that the component has two ports:
     *        - A provides (input) port of type BULKIO::dataShort called dataShort_in
     *        - A uses (output) port of type BULKIO::dataFloat called dataFloat_out
     *    The mapping between the port and the class is found the class of the same name.
     *    This example also makes use of the following Properties:
     *        - A float value called amplitude
     *        - A boolean called increaseAmplitude
     *    
     *    BULKIO_dataShortInPort.Packet<short[]> data = this.port_dataShort_in.getPacket(125);
     *    
     *    float[] outData = new float[data.getData().length];
     *    if (data != null) {
     *            for (int i = 0; i < data.getData().length; i++) {
     *                    if (this.increaseAmplitude.getValue()) {
     *                    outData[i] = (float)data[i] * this.amplitude;
     *            } else {
     *                outData[i] = (float)data[i];
     *            }
     *            }
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
                long_seq.getValue().clear();
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
            Resource.start_component(TestLegacyJavaProps.class, args, orbProps);
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
