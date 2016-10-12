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

package BasicTestDevice_java.java;


import java.util.ArrayList;
import java.util.HashMap;
import java.util.Properties;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import CF.DataType;
import CF.InvalidObjectReference;
import CF.PropertiesHolder;
import CF.UnknownProperties;
import CF.DevicePackage.AdminType;
import CF.DevicePackage.OperationalType;
import CF.DevicePackage.UsageType;
import CF.LifeCyclePackage.InitializeError;
import CF.TestableObjectPackage.UnknownTest;
import org.apache.log4j.Logger;
import org.ossie.component.*;
import org.ossie.properties.*;

/**
 * This is the Device code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general device housekeeping
 *
 * Source: BasicTestDevice_java.spd.xml
 * Generated on: Wed Oct 17 14:45:54 EDT 2012
 * Redhawk IDE
 * Version:M.1.8.1
 * Build id: v201209290832 
 
 * @generated
 */
public class BasicTestDevice_java extends Device implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(BasicTestDevice_java.class.getName());
    
    
    /**
     * The property DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<String> device_kind =
        new SimpleProperty<String>(
            "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d", //id
            "device_kind", //name
            "string", //type
            null, //default value
            "readonly", //mode
            "eq", //action
            new String[] {"configure","allocation"} //kind
            );
    
    /**
     * The property DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<String> device_model =
        new SimpleProperty<String>(
            "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb", //id
            "device_model", //name
            "string", //type
            null, //default value
            "readonly", //mode
            "eq", //action
            new String[] {"configure","allocation"} //kind
            );
    
    /**
     * The property DCE:0f99b2e4-9903-4631-9846-ff349d18eaaa
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<Integer> int_prop =
        new SimpleProperty<Integer>(
            "DCE:0f99b2e4-9903-4631-9846-ff349d18eaaa", //id
            "int_prop", //name
            "long", //type
            10, //default value
            "readwrite", //mode
            "external", //action
            new String[] {"configure","allocation"} //kind
            );
    
    /**
     * The property DCE:d3aa6040-b731-4110-b814-376967264728
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<Integer> short_prop =
        new SimpleProperty<Integer>(
            "DCE:d3aa6040-b731-4110-b814-376967264728", //id
            "short_prop", //name
            "long", //type
            22, //default value
            "readwrite", //mode
            "external", //action
            new String[] {"configure","allocation"} //kind
            );
    
    /**
     * The property no_allocation
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<Integer> no_allocation =
        new SimpleProperty<Integer>(
            "no_allocation", //id
            "no_allocation", //name
            "long", //type
            100, //default value
            "readwrite", //mode
            "external", //action
            new String[] {"configure"} //kind
            );
    
    /**
     * The property not_external
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<Float> not_external =
        new SimpleProperty<Float>(
            "not_external", //id
            "not_external", //name
            "float", //type
            66.6F, //default value
            "readwrite", //mode
            "eq", //action
            new String[] {"configure","allocation"} //kind
            );
    
    /**
     * The property non_simple1
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleSequenceProperty< String> non_simple1 =
        new SimpleSequenceProperty< String>(
            "non_simple1", //id
            "non_simple1", //name
            "string", //type
            new ArrayList<String>(), //default value
            "readwrite", //mode
            "external", //action
            new String[] {"configure","allocation"} //kind
            );
    
    /**
     * The structure for property non_simple2
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public static class non_simple2_struct extends StructDef {

        /**
         * The property prop
         * If the meaning of this property isn't clear, a description should be added.
         * 
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final SimpleProperty<String> prop =
            new SimpleProperty<String>(
                "prop", //id
                null, //name
                "string", //type
                null, //default value
                "readwrite", //mode
                "null", //action
                new String[] {} //kind
                );


        /**
         * @generated
         */
        public non_simple2_struct() {
            addElement(prop);
            //begin-user-code
            //end-user-code
        }
    };
    
    /**
     * The property non_simple2
     * If the meaning of this property isn't clear, a description should be added.
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructProperty<non_simple2_struct> non_simple2 =
        new StructProperty<non_simple2_struct> (
            "non_simple2", //id
            "non_simple2", //name
            new non_simple2_struct(), //type
            new non_simple2_struct(), // tmp type
            "readwrite", //mode
            new String[] { "configure","allocation" } //kind
            ); 

    /**
     * The property test_execparam
     * None
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<String> test_execparam =
        new SimpleProperty<String>(
            "test_execparam", //id
            "test_execparam", //name
            "string", //type
            null, //default value
            "readwrite", //mode
            "external", //action
            new String[] {"execparam"} //kind
            );

    // Provides/inputs

    // Uses/outputs

    /**
     * @generated
     */
    public BasicTestDevice_java() 
    {
        super();
           this.usageState = UsageType.IDLE;
          this.operationState = OperationalType.ENABLED;
          this.adminState = AdminType.UNLOCKED;
          this.callbacks = new HashMap<String, AllocCapacity>();                  
        addProperty(device_kind);
        addProperty(device_model);
        addProperty(int_prop);
        addProperty(short_prop);
        addProperty(no_allocation);
        addProperty(not_external);
        addProperty(non_simple1);
        addProperty(non_simple2);
        addProperty(test_execparam);

        // Inits the call back classes
        int_prop_callback int_prop_call_var = new int_prop_callback();
        short_prop_callback short_prop_call_var = new short_prop_callback();
        no_allocation_callback no_allocation_call_var = new no_allocation_callback();
        not_external_callback not_external_call_var = new not_external_callback();

        // Adds call backs to map
        callbacks.put(int_prop.getId(), int_prop_call_var);
        callbacks.put(short_prop.getId(), short_prop_call_var);
        callbacks.put(no_allocation.getId(), no_allocation_call_var);
        callbacks.put(not_external.getId(), not_external_call_var);

        // Provides/input

        // Uses/output
    
       //begin-user-code
       //end-user-code
    }
    
    
     /**
      * Call back class for int_prop
      *
      * -User can custom design allocation and deallocation
      * -Otherwise basic auto alloc / dealloc will be user
      *
      * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
      */
    public class int_prop_callback implements AllocCapacity {
        public boolean allocate(DataType t) {
            /* - TODO User defined allocate functions go here
             * - Returns:
             *        true  -> successful allocate
             *        false -> failure to allocate
             */
        logger.trace("Attempting user defined allocate");
            return false;
        }
        public boolean deallocate(DataType t){
            /* - TODO User defined deallocate functions go here
             * - Returns :
             *      true  -> if user specifies their own function
             *      false -> if no deallocate function is defined
             */
        logger.trace("Attempting user defined deallocate");
             return false;
        }
    }
    
     /**
      * Call back class for short_prop
      *
      * -User can custom design allocation and deallocation
      * -Otherwise basic auto alloc / dealloc will be user
      *
      * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
      */
    public class short_prop_callback implements AllocCapacity {
        public boolean allocate(DataType t) {
            /* - TODO User defined allocate functions go here
             * - Returns:
             *        true  -> successful allocate
             *        false -> failure to allocate
             */
        logger.trace("Attempting user defined allocate");
            return false;
        }
        public boolean deallocate(DataType t){
            /* - TODO User defined deallocate functions go here
             * - Returns :
             *      true  -> if user specifies their own function
             *      false -> if no deallocate function is defined
             */
        logger.trace("Attempting user defined deallocate");
             return false;
        }
    }
    
     /**
      * Call back class for no_allocation
      *
      * -User can custom design allocation and deallocation
      * -Otherwise basic auto alloc / dealloc will be user
      *
      * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
      */
    public class no_allocation_callback implements AllocCapacity {
        public boolean allocate(DataType t) {
            /* - TODO User defined allocate functions go here
             * - Returns:
             *        true  -> successful allocate
             *        false -> failure to allocate
             */
        logger.trace("Attempting user defined allocate");
            return false;
        }
        public boolean deallocate(DataType t){
            /* - TODO User defined deallocate functions go here
             * - Returns :
             *      true  -> if user specifies their own function
             *      false -> if no deallocate function is defined
             */
        logger.trace("Attempting user defined deallocate");
             return false;
        }
    }
    
     /**
      * Call back class for not_external
      *
      * -User can custom design allocation and deallocation
      * -Otherwise basic auto alloc / dealloc will be user
      *
      * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
      */
    public class not_external_callback implements AllocCapacity {
        public boolean allocate(DataType t) {
            /* - TODO User defined allocate functions go here
             * - Returns:
             *        true  -> successful allocate
             *        false -> failure to allocate
             */
        logger.trace("Attempting user defined allocate");
            return false;
        }
        public boolean deallocate(DataType t){
            /* - TODO User defined deallocate functions go here
             * - Returns :
             *      true  -> if user specifies their own function
             *      false -> if no deallocate function is defined
             */
        logger.trace("Attempting user defined deallocate");
             return false;
        }
    }
    

    
    /**
     *
     * Main processing thread
     *
     * <!-- begin-user-doc -->
     * 
     * General functionality:
     * 
     *    This function is running as a separate thread from the device's main thread. 
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
     * StreamSRI:
     *    To create a StreamSRI object, use the following code:
     *        this.stream_id = "stream";
     *           StreamSRI sri = new StreamSRI();
     *           sri.mode = 0;
     *           sri.xdelta = 0.0;
     *           sri.ydelta = 1.0;
     *           sri.subsize = 0;
     *           sri.xunits = 1; // TIME_S
     *           sri.streamID = (this.stream_id.getValue() != null) ? this.stream_id.getValue() : "";
     * 
     * PrecisionUTCTime:
     *    To create a PrecisionUTCTime object, use the following code:
     *           long tmp_time = System.currentTimeMillis();
     *           double wsec = tmp_time / 1000;
     *           double fsec = tmp_time % 1000;
     *           PrecisionUTCTime tstamp = new PrecisionUTCTime(BULKIO.TCM_CPU.value, (short)1, (short)0, wsec, fsec);
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
     *    Interactions with non-BULKIO ports are left up to the device developer's discretion.
     *    
     * Properties:
     * 
     *    Properties are accessed through members of the same name with helper functions. If the 
     *    property name is baudRate, then reading the value is achieved by: this.baudRate.getValue();
     *    and writing a new value is achieved by: this.baudRate.setValue(new_value);
     *    
     * Example:
     * 
     *    This example assumes that the device has two ports:
     *        - A provides (input) port of type BULKIO::dataShort called dataShort_in
     *        - A uses (output) port of type BULKIO::dataFloat called dataFloat_out
     *    The mapping between the port and the class is found the class of the same name.
     *    This example also makes use of the following Properties:
     *        - A float value called amplitude with a default value of 2.0
     *        - A boolean called increaseAmplitude with a default value of true
     *    
     *    BULKIO_dataShortInPort.Packet<short[]> data = this.port_dataShort_in.getPacket(125);
     *
     *    if (data != null) {
     *        float[] outData = new float[data.getData().length];
     *        for (int i = 0; i < data.getData().length; i++) {
     *            if (this.increaseAmplitude.getValue()) {
     *                outData[i] = (float)data.getData()[i] * this.amplitude.getValue();
     *            } else {
     *                outData[i] = (float)data.getData()[i];
     *            }
     *        }
     *
     *        // NOTE: You must make at least one valid pushSRI call
     *        if (data.sriChanged()) {
     *            this.port_dataFloat_out.pushSRI(data.getSRI());
     *        }
     *        this.port_dataFloat_out.pushPacket(outData, data.getTime(), data.getEndOfStream(), data.getStreamID());
     *    }
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
                logger.debug("run() example log message");
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
     * The main function of your device.  If no args are provided, then the
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
            Device.start_device(BasicTestDevice_java.class, args, orbProps);
        } catch (InvalidObjectReference e) {
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

    @Override
    public void initialize() throws InitializeError {
        super.initialize();

        // Save the current value of "test_execparam" to check whether
        // execparams are available in initialize (they should be).
        this.test_execparam_at_init = this.test_execparam.getValue();
    }

    @Override
    public void runTest(int testid, PropertiesHolder testValues) throws UnknownTest, UnknownProperties {
        if (testid == 561) {
            // Issue #561: Execparams are not set in Java device when initialize() is called
            testValues.value = new DataType[1];
            testValues.value[0] = new DataType("test_execparam", AnyUtils.toAny(this.test_execparam_at_init, "string"));
        } else {
            throw new UnknownTest("Invalid test " + testid);
        }
    }

    private String test_execparam_at_init;
}
