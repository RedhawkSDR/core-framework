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

package EventSend_java_impl1;


import java.util.Properties;
import java.util.ArrayList;
import java.util.List;
import org.omg.CORBA.ORB;
import org.omg.PortableServer.POA;
import org.omg.PortableServer.POAPackage.ServantNotActive;
import org.omg.PortableServer.POAPackage.WrongPolicy;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;
import org.omg.CosEventChannelAdmin.ProxyPushConsumerOperations;
import CF.PropertiesHelper;
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
import org.ossie.events.*;
import org.omg.CORBA.Any;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 * @generated
 */
public class EventSend_java_impl1 extends Resource implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(EventSend_java_impl1.class.getName());
        
    
    /**
     * The structure for property test_message.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public class test_message_struct extends StructDef { 
        /**
         * The struct field item_float.
         * If the meaning of this field isn't clear a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final SimpleProperty<Float> item_float =
           new SimpleProperty<Float>(
               "item_float", // id
               "item_float", // name
               "float", // type
               null, // default value
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
        public final SimpleProperty<String> item_string =
           new SimpleProperty<String>(
               "item_string", // id
               "item_string", // name
               "string", // type
               null, // default value
               "readwrite", // mode
               "external", // action
               new String[] {"configure",} // kind
              );
        
        /**
         * @generated
         */
        public test_message_struct() {
            addElement(item_float);
            addElement(item_string);
            //begin-user-code
            //end-user-code
        }
    };
    
    /**
     * The property test_message.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructProperty<test_message_struct> test_message =
       new StructProperty<test_message_struct>(
           "test_message", // id
           "test_message", // name
           new test_message_struct(), // type
           new test_message_struct(), // type
           "readwrite", // mode
           new String[] { "configure" }// kind
           );
    

    /**
     * @generated
     */
    public MessageSupplierPort port_message_out;


    // Provides/inputs

    // Uses/outputs

    /**
     * @generated
     */
    public EventSend_java_impl1() 
    {
        super();
        addProperty(test_message);

        // Provides/input

        // Uses/output
        this.port_message_out = new MessageSupplierPort( "message_out");
        this.addPort("message_out", this.port_message_out);
       //begin-user-code
       //end-user-code
    }

    
    public void sendMessage(MessageSupplierPort tmp_port, final test_message_struct message) {
        final List<DataType> outProps = new ArrayList<DataType>();
        final List<DataType> propStruct = new ArrayList<DataType>();
        Any propStruct_any = ORB.init().create_any();
        propStruct.add(new DataType("item_float", message.item_float.toAny()));
        propStruct.add(new DataType("item_string", message.item_string.toAny()));
        PropertiesHelper.insert(propStruct_any, propStruct.toArray(new DataType[propStruct.size()]));
        outProps.add(new DataType("test_message", propStruct_any));
        Any outProps_any = ORB.init().create_any();
        PropertiesHelper.insert(outProps_any, outProps.toArray(new DataType[outProps.size()]));
        tmp_port.push(outProps_any);
    }
    
    public void sendMessages(MessageSupplierPort tmp_port, final ArrayList<test_message_struct> messages) {
        final List<DataType> outProps = new ArrayList<DataType>();
        for (test_message_struct message : messages) {
            final List<DataType> propStruct = new ArrayList<DataType>();
            Any propStruct_any = ORB.init().create_any();
            propStruct.add(new DataType("item_float", message.item_float.toAny()));
            propStruct.add(new DataType("item_string", message.item_string.toAny()));
            PropertiesHelper.insert(propStruct_any, propStruct.toArray(new DataType[propStruct.size()]));
            outProps.add(new DataType("test_message", propStruct_any));
        }
        Any outProps_any = ORB.init().create_any();
        PropertiesHelper.insert(outProps_any, outProps.toArray(new DataType[outProps.size()]));
        tmp_port.push(outProps_any);
    }
    
    public void start() throws StartError {
        super.start();
        test_message_struct tmp = new test_message_struct();
        tmp.item_float.setValue((float)1.0);
        tmp.item_string.setValue("some string");
        sendMessage(port_message_out, tmp);
        
        ArrayList<test_message_struct> messages = new ArrayList<test_message_struct>();
        test_message_struct tmp_2 = new test_message_struct();
        tmp_2.item_float.setValue((float)2.0);
        tmp_2.item_string.setValue("another string");
        messages.add(tmp_2);
        test_message_struct tmp_3 = new test_message_struct();
        tmp_3.item_float.setValue((float)3.0);
        tmp_3.item_string.setValue("yet another string");
        messages.add(tmp_3);
        sendMessages(port_message_out, messages);
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
     *        for (int i = 0; i < data.getData().length; i++) {
     *            if (this.increaseAmplitude.getValue()) {
     *                outData[i] = (float)data.getData()[i] * this.amplitude;
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
            Resource.start_component(EventSend_java_impl1.class, args, orbProps);
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
