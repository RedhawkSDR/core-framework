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

package TestJavaPropsRange.java;


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

/**
 * This is the component code. This file contains all the access points
 * you need to use to be able to access all input and output ports,
 * respond to incoming data, and perform general component housekeeping
 *
 * Source: TestJavaPropsRange.spd.xml
 * Generated on: Thu Dec 20 16:51:13 EST 2012
 * Redhawk IDE
 * Version:M.1.8.2
 * Build id: v201211201139RC3 
 
 * @generated
 */
public class TestJavaPropsRange extends Resource implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestJavaPropsRange.class.getName());
    
	/**
	 * The property my_octet
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final OctetProperty my_octet_name =
		new OctetProperty(
			"my_octet", //id
			"my_octet_name", //name
			(byte)49, //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property my_short
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final ShortProperty my_short_name =
		new ShortProperty(
			"my_short", //id
			"my_short_name", //name
			(short)2, //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property my_ushort
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final UShortProperty my_ushort_name =
		new UShortProperty(
			"my_ushort", //id
			"my_ushort_name", //name
			(short)3, //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property my_long
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final LongProperty my_long_name =
		new LongProperty(
			"my_long", //id
			"my_long_name", //name
			4, //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property my_ulong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final ULongProperty my_ulong_name =
		new ULongProperty(
			"my_ulong", //id
			"my_ulong_name", //name
			5, //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property my_longlong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final LongLongProperty my_longlong_name =
		new LongLongProperty(
			"my_longlong", //id
			"my_longlong_name", //name
			6L, //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The structure for property my_struct
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public static class my_struct_name_struct extends StructDef {

		/**
		 * The property struct_octet
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final OctetProperty struct_octet_name =
			new OctetProperty(
				"struct_octet", //id
				"struct_octet_name", //name
				(byte)49, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {} //kind
				);
		/**
		 * The property struct_short
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final ShortProperty struct_short_name =
			new ShortProperty(
				"struct_short", //id
				"struct_short_name", //name
				(short)2, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property struct_ushort
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final UShortProperty struct_ushort_name =
			new UShortProperty(
				"struct_ushort", //id
				"struct_ushort_name", //name
				(short)3, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property struct_long
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final LongProperty struct_long_name =
			new LongProperty(
				"struct_long", //id
				"struct_long_name", //name
				4, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property struct_ulong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final ULongProperty struct_ulong_name =
			new ULongProperty(
				"struct_ulong", //id
				"struct_ulong_name", //name
				5, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property struct_longlong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final LongLongProperty struct_longlong_name =
			new LongLongProperty(
				"struct_longlong", //id
				"struct_longlong_name", //name
				6L, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * @generated
		 */
		public my_struct_name_struct() {
			addElement(struct_octet_name);
			addElement(struct_short_name);
			addElement(struct_ushort_name);
			addElement(struct_long_name);
			addElement(struct_ulong_name);
			addElement(struct_longlong_name);
			//begin-user-code
			//end-user-code
		}
	};
	
	/**
	 * The property my_struct
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final StructProperty<my_struct_name_struct> my_struct_name =
		new StructProperty<my_struct_name_struct> (
			"my_struct", //id
			"my_struct_name", //name
			my_struct_name_struct.class, //type
			new my_struct_name_struct(), // tmp type
			Mode.READWRITE, //mode
			new Kind[] { Kind.CONFIGURE } //kind
			);
    
	/**
	 * The property seq_octet
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final OctetSequenceProperty seq_octet_name =
		new OctetSequenceProperty(
			"seq_octet", //id
			"seq_octet_name", //name
			OctetSequenceProperty.asList(49, 50), //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property seq_short
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final ShortSequenceProperty seq_short_name =
		new ShortSequenceProperty(
			"seq_short", //id
			"seq_short_name", //name
			ShortSequenceProperty.asList(1, 2), //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property seq_ushort
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final UShortSequenceProperty seq_ushort_name =
		new UShortSequenceProperty(
			"seq_ushort", //id
			"seq_ushort_name", //name
			UShortSequenceProperty.asList(1, 2), //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property seq_long
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final LongSequenceProperty seq_long_name =
		new LongSequenceProperty(
			"seq_long", //id
			"seq_long_name", //name
			LongSequenceProperty.asList(1,2), //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property seq_ulong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final ULongSequenceProperty seq_ulong_name =
		new ULongSequenceProperty(
			"seq_ulong", //id
			"seq_ulong_name", //name
			ULongSequenceProperty.asList(1,2), //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property seq_longlong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final LongLongSequenceProperty seq_longlong_name =
		new LongLongSequenceProperty(
			"seq_longlong", //id
			"seq_longlong_name", //name
			LongLongSequenceProperty.asList(1L,2L), //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The property seq_char
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final CharSequenceProperty seq_char_name =
		new CharSequenceProperty(
			"seq_char", //id
			"seq_char_name", //name
			CharSequenceProperty.asList('a','b'), //default value
			Mode.READWRITE, //mode
			Action.EXTERNAL, //action
			new Kind[] {Kind.CONFIGURE} //kind
			);
    
	/**
	 * The structure for property ss_struct
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public static class ss_struct_name_struct extends StructDef {

		/**
		 * The property ss_octet
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final OctetProperty ss_octet_name =
			new OctetProperty(
				"ss_octet", //id
				"ss_octet_name", //name
				null, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {} //kind
				);
		/**
		 * The property ss_short
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final ShortProperty ss_short_name =
			new ShortProperty(
				"ss_short", //id
				"ss_short_name", //name
				null, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property ss_ushort
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final UShortProperty ss_ushort_name =
			new UShortProperty(
				"ss_ushort", //id
				"ss_ushort_name", //name
				null, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property ss_long
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final LongProperty ss_long_name =
			new LongProperty(
				"ss_long", //id
				"ss_long_name", //name
				null, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property ss_ulong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final ULongProperty ss_ulong_name =
			new ULongProperty(
				"ss_ulong", //id
				"ss_ulong_name", //name
				null, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * The property ss_longlong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final LongLongProperty ss_longlong_name =
			new LongLongProperty(
				"ss_longlong", //id
				"ss_longlong_name", //name
				null, //default value
				Mode.READWRITE, //mode
				Action.EXTERNAL, //action
				new Kind[] {Kind.CONFIGURE} //kind
				);
		/**
		 * @generated
		 */
		public ss_struct_name_struct() {
			addElement(ss_octet_name);
			addElement(ss_short_name);
			addElement(ss_ushort_name);
			addElement(ss_long_name);
			addElement(ss_ulong_name);
			addElement(ss_longlong_name);
			//begin-user-code
			//end-user-code
		}
	};

	/**
	 * The property my_structseq
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final StructSequenceProperty<ss_struct_name_struct> my_structseq_name; 
    // Provides/inputs

    // Uses/outputs

    /**
     * @generated
     */
    public TestJavaPropsRange() 
    {
        super();                            
        ArrayList<ss_struct_name_struct> structVals_ss_struct_name_struct = new ArrayList<ss_struct_name_struct>();
		ss_struct_name_struct ss_struct_name1 = new ss_struct_name_struct();
		ss_struct_name1.ss_octet_name.setValue((byte)48);
		ss_struct_name1.ss_short_name.setValue((short)1);
		ss_struct_name1.ss_ushort_name.setValue(2);
		ss_struct_name1.ss_long_name.setValue(3);
		ss_struct_name1.ss_ulong_name.setValue(4L);
		ss_struct_name1.ss_longlong_name.setValue(5L);
		structVals_ss_struct_name_struct.add(ss_struct_name1);
		ss_struct_name_struct ss_struct_name2 = new ss_struct_name_struct();
		ss_struct_name2.ss_octet_name.setValue((byte)55);
		ss_struct_name2.ss_short_name.setValue((short)8);
		ss_struct_name2.ss_ushort_name.setValue(9);
		ss_struct_name2.ss_long_name.setValue(10);
		ss_struct_name2.ss_ulong_name.setValue(11L);
		ss_struct_name2.ss_longlong_name.setValue(12L);
		structVals_ss_struct_name_struct.add(ss_struct_name2);
		
        this.my_structseq_name = new StructSequenceProperty<ss_struct_name_struct> (
			"my_structseq", //id
			"my_structseq_name", //name
			ss_struct_name_struct.class, //type
			structVals_ss_struct_name_struct, //defaultValue
			Mode.READWRITE, //mode
			new Kind[] { Kind.CONFIGURE } //kind
			);    
        addProperty(my_octet_name);
        addProperty(my_short_name);
        addProperty(my_ushort_name);
        addProperty(my_long_name);
        addProperty(my_ulong_name);
        addProperty(my_longlong_name);
        addProperty(my_struct_name);
        addProperty(seq_octet_name);
        addProperty(seq_short_name);
        addProperty(seq_ushort_name);
        addProperty(seq_long_name);
        addProperty(seq_ulong_name);
        addProperty(seq_longlong_name);
        addProperty(seq_char_name);
        addProperty(my_structseq_name);

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
            Resource.start_component(TestJavaPropsRange.class, args, orbProps);
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
