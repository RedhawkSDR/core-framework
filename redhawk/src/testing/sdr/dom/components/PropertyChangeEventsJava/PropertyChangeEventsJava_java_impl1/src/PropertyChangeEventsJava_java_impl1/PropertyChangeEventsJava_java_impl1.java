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

package PropertyChangeEventsJava_java_impl1;

import java.util.Properties;
import java.util.ArrayList;
import java.util.Arrays;
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
import org.ossie.events.*;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 * @generated
 */
public class PropertyChangeEventsJava_java_impl1 extends Resource implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(PropertyChangeEventsJava_java_impl1.class.getName());
        
     
    /**
     * The property myprop.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<Integer> myprop =
       new SimpleProperty<Integer>(
           "myprop", // id
           "myprop", // name
           "long", // type
           null, // default value
           "readwrite", // mode
           "external", // action
           new String[] {"configure","event"} // kind
           );
            
    /**
     * The property anotherprop.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleProperty<Integer> anotherprop =
       new SimpleProperty<Integer>(
           "anotherprop", // id
           "anotherprop", // name
           "long", // type
           null, // default value
           "readwrite", // mode
           "external", // action
           new String[] {"configure","event"} // kind
           );
            
    /**
     * The property anotherprop.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final SimpleSequenceProperty<Float> seqprop =
       new SimpleSequenceProperty<Float>(
           "seqprop", // id
           "seqprop", // name
           "float", // type
           null, // default value
           "readwrite", // mode
           "external", // action
           new String[] {"configure","event"} // kind
           );
           
    /**
     * The structure for property some_struct.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public static class some_struct_struct extends StructDef { 
        /**
         * The struct field some_number.
         * If the meaning of this field isn't clear a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final SimpleProperty<Double> some_number =
           new SimpleProperty<Double>(
               "some_number", // id
               "some_number", // name
               "double", // type
               null, // default value
               "readwrite", // mode
               "external", // action
               new String[] {"configure",} // kind
              ); 
        /**
         * The struct field some_string.
         * If the meaning of this field isn't clear a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final SimpleProperty<String> some_string =
           new SimpleProperty<String>(
               "some_string", // id
               "some_string", // name
               "string", // type
               null, // default value
               "readwrite", // mode
               "external", // action
               new String[] {"configure",} // kind
              );
        
        /**
         * @generated
         */
        public some_struct_struct() {
            addElement(some_number);
            addElement(some_string);
            //begin-user-code
            //end-user-code
        }
    }

    /**
     * The property some_struct.
     * If the meaning of this property isn't clear a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructProperty<some_struct_struct> some_struct =
       new StructProperty<some_struct_struct>(
           "some_struct", // id
           "some_struct", // name
           new some_struct_struct(), // type
           new some_struct_struct(), // type_tmp
           "readwrite", // mode
           new String[] { "configure","event" } // kind
           );

    public final StructSequenceProperty<some_struct_struct> structseq_prop =
       new StructSequenceProperty<some_struct_struct>(
           "structseq_prop", // id
           "structseq_prop", // name
           some_struct_struct.class, // type
           new ArrayList<some_struct_struct>(Arrays.asList(new some_struct_struct(), new some_struct_struct())), // default
           "readwrite", // mode
           new String[] { "configure","event" } // kind
           );

    /**
     * @generated
     */
    public PropertyEventSupplier port_propEvent;


    // Provides/inputs

    // Uses/outputs

    /**
     * @generated
     */
    public PropertyChangeEventsJava_java_impl1() 
    {
        super();
        addProperty(myprop);
        addProperty(anotherprop);
        addProperty(seqprop);
        addProperty(some_struct);
        addProperty(structseq_prop);

        // Provides/input

        // Uses/output
        this.port_propEvent = new PropertyEventSupplier("propEvent");
        this.addPort("propEvent", this.port_propEvent);
       //begin-user-code
       //end-user-code
    }
    
    public CF.Resource setup(final String compId, final String compName, final ORB orb, final POA poa) throws ServantNotActive, WrongPolicy {
        CF.Resource retval = super.setup(compId, compName, orb, poa);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.anotherprop);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.myprop);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.seqprop);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.some_struct);
        this.port_propEvent.registerProperty(this.compId, this.compName, this.structseq_prop);
        this.registerPropertyChangePort(this.port_propEvent);
        return retval;
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
            Resource.start_component(PropertyChangeEventsJava_java_impl1.class, args, orbProps);
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
