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
package TestComplexProps.java;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
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

import CF.complexBoolean;
import CF.complexULong;
import CF.complexShort;
import CF.complexFloat;
import CF.complexOctet;
import CF.complexChar;
import CF.complexUShort;
import CF.complexDouble;
import CF.complexLong;
import CF.complexLongLong;
import CF.complexULongLong;

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: TestComplexProps.spd.xml
 *
 * @generated
 */
public class TestComplexProps extends Resource implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestComplexProps.class.getName());

    /**
     * The property complexBooleanProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexBooleanProperty complexBooleanProp =
        new ComplexBooleanProperty(
            "complexBooleanProp", //id
            null, //name
            new complexBoolean(false,true), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexULongProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexULongProperty complexULongProp =
        new ComplexULongProperty(
            "complexULongProp", //id
            null, //name
            new complexULong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexShortProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexShortProperty complexShortProp =
        new ComplexShortProperty(
            "complexShortProp", //id
            null, //name
            new complexShort((short)4,(short)5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexFloatProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexFloatProperty complexFloatProp =
        new ComplexFloatProperty(
            "complexFloatProp", //id
            null, //name
            new complexFloat(4.0F,5.0F), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexOctetProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexOctetProperty complexOctetProp =
        new ComplexOctetProperty(
            "complexOctetProp", //id
            null, //name
            new complexOctet((byte)4,(byte)5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexCharProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexCharProperty complexCharProp =
        new ComplexCharProperty(
            "complexCharProp", //id
            null, //name
            new complexChar('4','5'), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexUShort
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexUShortProperty complexUShort =
        new ComplexUShortProperty(
            "complexUShort", //id
            null, //name
            new complexUShort((short)4,(short)5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexDouble
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexDoubleProperty complexDouble =
        new ComplexDoubleProperty(
            "complexDouble", //id
            null, //name
            new complexDouble(4.0,5.0), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexLong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexLongProperty complexLong =
        new ComplexLongProperty(
            "complexLong", //id
            null, //name
            new complexLong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexLongLong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexLongLongProperty complexLongLong =
        new ComplexLongLongProperty(
            "complexLongLong", //id
            null, //name
            new complexLongLong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexULongLong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexULongLongProperty complexULongLong =
        new ComplexULongLongProperty(
            "complexULongLong", //id
            null, //name
            new complexULongLong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The property complexFloatSequence
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final ComplexFloatSequenceProperty complexFloatSequence =
        new ComplexFloatSequenceProperty(
            "complexFloatSequence", //id
            null, //name
            new ArrayList<complexFloat>(Arrays.asList(new complexFloat(4.0F,5.0F),new complexFloat(4.0F,5.0F),new complexFloat(4.0F,5.0F))), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );

    /**
     * The structure for property FloatStruct
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public static class FloatStruct_struct extends StructDef {
        /**
         * The property FloatStructMember
         * If the meaning of this property isn't clear, a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final FloatProperty FloatStructMember =
            new FloatProperty(
                "FloatStructMember", //id
                null, //name
                4.0F, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public FloatStruct_struct() {
            addElement(FloatStructMember);
            //begin-user-code
            //end-user-code
        }
    };
    
    /**
     * The property FloatStruct
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructProperty<FloatStruct_struct> FloatStruct =
        new StructProperty<FloatStruct_struct>(
            "FloatStruct", //id
            null, //name
            new FloatStruct_struct(), //type
            new FloatStruct_struct(), // tmp type
            "readwrite", //mode
            new String[] {"configure"} //kind
            );

    /**
     * The structure for property complexFloatStruct
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public static class complexFloatStruct_struct extends StructDef {
        /**
         * The property complexFloatStructMember
         * If the meaning of this property isn't clear, a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final ComplexFloatProperty complexFloatStructMember =
            new ComplexFloatProperty(
                "complexFloatStructMember", //id
                null, //name
                new complexFloat(4.0F,5.0F), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public complexFloatStruct_struct() {
            addElement(complexFloatStructMember);
            //begin-user-code
            //end-user-code
        }
    };
    
    /**
     * The property complexFloatStruct
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructProperty<complexFloatStruct_struct> complexFloatStruct =
        new StructProperty<complexFloatStruct_struct>(
            "complexFloatStruct", //id
            null, //name
            new complexFloatStruct_struct(), //type
            new complexFloatStruct_struct(), // tmp type
            "readwrite", //mode
            new String[] {"configure"} //kind
            );

    /**
     * The structure for property FloatStructSequenceMember
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public static class FloatStructSequenceMember_struct extends StructDef {
        /**
         * The property FloatStructSequenceMemberMemember
         * If the meaning of this property isn't clear, a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final FloatProperty FloatStructSequenceMemberMemember =
            new FloatProperty(
                "FloatStructSequenceMemberMemember", //id
                null, //name
                4.0F, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public FloatStructSequenceMember_struct() {
            addElement(FloatStructSequenceMemberMemember);
            //begin-user-code
            //end-user-code
        }
    };
    
    /**
     * The property FloatStructSequence
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructSequenceProperty<FloatStructSequenceMember_struct> FloatStructSequence;

    /**
     * The structure for property complexFloatStructSequenceMember
     * 
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public static class complexFloatStructSequenceMember_struct extends StructDef {
        /**
         * The property complexFloatStructSequenceMemberMemember
         * If the meaning of this property isn't clear, a description should be added.
         *
         * <!-- begin-user-doc -->
         * <!-- end-user-doc -->
         * @generated
         */
        public final ComplexFloatProperty complexFloatStructSequenceMemberMemember =
            new ComplexFloatProperty(
                "complexFloatStructSequenceMemberMemember", //id
                null, //name
                new complexFloat(4.0F,5.0F), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public complexFloatStructSequenceMember_struct() {
            addElement(complexFloatStructSequenceMemberMemember);
            //begin-user-code
            //end-user-code
        }
    };
    
    /**
     * The property complexFloatStructSequence
     * If the meaning of this property isn't clear, a description should be added.
     *
     * <!-- begin-user-doc -->
     * <!-- end-user-doc -->
     * @generated
     */
    public final StructSequenceProperty<complexFloatStructSequenceMember_struct> complexFloatStructSequence;

    // Provides/inputs
    // Uses/outputs
    /**
     * @generated
     */
    public TestComplexProps()
    {
        super();
        ArrayList<FloatStructSequenceMember_struct> structVals_FloatStructSequenceMember_struct = new ArrayList<FloatStructSequenceMember_struct>();
        
        this.FloatStructSequence = new StructSequenceProperty<FloatStructSequenceMember_struct> (
            "FloatStructSequence", //id
            null, //name
            FloatStructSequenceMember_struct.class, //type
            structVals_FloatStructSequenceMember_struct, //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
            );
        ArrayList<complexFloatStructSequenceMember_struct> structVals_complexFloatStructSequenceMember_struct = new ArrayList<complexFloatStructSequenceMember_struct>();
        
        this.complexFloatStructSequence = new StructSequenceProperty<complexFloatStructSequenceMember_struct> (
            "complexFloatStructSequence", //id
            null, //name
            complexFloatStructSequenceMember_struct.class, //type
            structVals_complexFloatStructSequenceMember_struct, //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
            );
        addProperty(complexBooleanProp);
        addProperty(complexULongProp);
        addProperty(complexShortProp);
        addProperty(complexFloatProp);
        addProperty(complexOctetProp);
        addProperty(complexCharProp);
        addProperty(complexUShort);
        addProperty(complexDouble);
        addProperty(complexLong);
        addProperty(complexLongLong);
        addProperty(complexULongLong);
        addProperty(complexFloatSequence);
        addProperty(FloatStruct);
        addProperty(complexFloatStruct);
        addProperty(FloatStructSequence);
        addProperty(complexFloatStructSequence);

        // Provides/input

        // Uses/output

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
     * StreamSRI:
     *    To create a StreamSRI object, use the following code:
     *        this.stream_id = "stream";
     * 		  StreamSRI sri = new StreamSRI();
     * 		  sri.mode = 0;
     * 		  sri.xdelta = 0.0;
     * 		  sri.ydelta = 1.0;
     * 		  sri.subsize = 0;
     * 		  sri.xunits = 1; // TIME_S
     * 		  sri.streamID = (this.stream_id.getValue() != null) ? this.stream_id.getValue() : "";
     * 
     * PrecisionUTCTime:
     *    To create a PrecisionUTCTime object, use the following code:
     * 		  long tmp_time = System.currentTimeMillis();
     * 		  double wsec = tmp_time / 1000;
     * 		  double fsec = tmp_time % 1000;
     * 		  PrecisionUTCTime tstamp = new PrecisionUTCTime(BULKIO.TCM_CPU.value, (short)1, (short)0, wsec, fsec);
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
            Resource.start_component(TestComplexProps.class, args, orbProps);
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
