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

package TestAllPropTypes.java;


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
 * Source: TestAllPropTypes.spd.xml
 * Generated on: Tue May 07 10:59:41 EDT 2013
 * REDHAWK IDE
 * Version: R.1.8.3
 * Build id: v201303122306 
 
 * @generated
 */
public class TestAllPropTypes extends Resource implements Runnable {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestAllPropTypes.class.getName());
    
	/**
	 * The property simple_string
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<String> simple_string =
		new SimpleProperty<String>(
			"simple_string", //id
			null, //name
			"string", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_boolean
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Boolean> simple_boolean =
		new SimpleProperty<Boolean>(
			"simple_boolean", //id
			null, //name
			"boolean", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_ulong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Long> simple_ulong =
		new SimpleProperty<Long>(
			"simple_ulong", //id
			null, //name
			"ulong", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_objref
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 *
	public final SimpleProperty<String> simple_objref =
		new SimpleProperty<String>(
			"simple_objref", //id
			null, //name
			"objref", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    */
	/**
	 * The property simple_short
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Short> simple_short =
		new SimpleProperty<Short>(
			"simple_short", //id
			null, //name
			"short", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_float
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Float> simple_float =
		new SimpleProperty<Float>(
			"simple_float", //id
			null, //name
			"float", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_octet
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Byte> simple_octet =
		new SimpleProperty<Byte>(
			"simple_octet", //id
			null, //name
			"octet", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_char
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Character> simple_char =
		new SimpleProperty<Character>(
			"simple_char", //id
			null, //name
			"char", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_ushort
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Integer> simple_ushort =
		new SimpleProperty<Integer>(
			"simple_ushort", //id
			null, //name
			"ushort", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_double
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Double> simple_double =
		new SimpleProperty<Double>(
			"simple_double", //id
			null, //name
			"double", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_long
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Integer> simple_long =
		new SimpleProperty<Integer>(
			"simple_long", //id
			null, //name
			"long", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_longlong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Long> simple_longlong =
		new SimpleProperty<Long>(
			"simple_longlong", //id
			null, //name
			"longlong", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_ulonglong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleProperty<Object> simple_ulonglong =
		new SimpleProperty<Object>(
			"simple_ulonglong", //id
			null, //name
			"ulonglong", //type
			null, //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_string
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< String> simple_sequence_string =
		new SimpleSequenceProperty< String>(
			"simple_sequence_string", //id
			null, //name
			"string", //type
			new ArrayList<String>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_boolean
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Boolean> simple_sequence_boolean =
		new SimpleSequenceProperty< Boolean>(
			"simple_sequence_boolean", //id
			null, //name
			"boolean", //type
			new ArrayList<Boolean>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_ulong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Long> simple_sequence_ulong =
		new SimpleSequenceProperty< Long>(
			"simple_sequence_ulong", //id
			null, //name
			"ulong", //type
			new ArrayList<Long>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_objref
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 *
	public final SimpleSequenceProperty< String> simple_sequence_objref =
		new SimpleSequenceProperty< String>(
			"simple_sequence_objref", //id
			null, //name
			"objref", //type
			new ArrayList<String>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    */
	/**
	 * The property simple_sequence_short
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Short> simple_sequence_short =
		new SimpleSequenceProperty< Short>(
			"simple_sequence_short", //id
			null, //name
			"short", //type
			new ArrayList<Short>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_float
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Float> simple_sequence_float =
		new SimpleSequenceProperty< Float>(
			"simple_sequence_float", //id
			null, //name
			"float", //type
			new ArrayList<Float>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_octet
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Byte> simple_sequence_octet =
		new SimpleSequenceProperty< Byte>(
			"simple_sequence_octet", //id
			null, //name
			"octet", //type
			new ArrayList<Byte>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_char
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Character> simple_sequence_char =
		new SimpleSequenceProperty< Character>(
			"simple_sequence_char", //id
			null, //name
			"char", //type
			new ArrayList<Character>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_ushort
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Integer> simple_sequence_ushort =
		new SimpleSequenceProperty< Integer>(
			"simple_sequence_ushort", //id
			null, //name
			"ushort", //type
			new ArrayList<Integer>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_double
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Double> simple_sequence_double =
		new SimpleSequenceProperty< Double>(
			"simple_sequence_double", //id
			null, //name
			"double", //type
			new ArrayList<Double>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_long
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Integer> simple_sequence_long =
		new SimpleSequenceProperty< Integer>(
			"simple_sequence_long", //id
			null, //name
			"long", //type
			new ArrayList<Integer>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_longlong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Long> simple_sequence_longlong =
		new SimpleSequenceProperty< Long>(
			"simple_sequence_longlong", //id
			null, //name
			"longlong", //type
			new ArrayList<Long>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The property simple_sequence_ulonglong
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final SimpleSequenceProperty< Object> simple_sequence_ulonglong =
		new SimpleSequenceProperty< Object>(
			"simple_sequence_ulonglong", //id
			null, //name
			"ulonglong", //type
			new ArrayList<Object>(), //default value
			"readwrite", //mode
			"external", //action
			new String[] {"configure"} //kind
			);
    
	/**
	 * The structure for property struct_vars
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public static class struct_vars_struct extends StructDef {

		/**
		 * The property struct_string
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<String> struct_string =
			new SimpleProperty<String>(
				"struct_string", //id
				null, //name
				"string", //type
				null, //default value
				"readwrite", //mode
				"null", //action
				new String[] {} //kind
				);
		/**
		 * The property struct_boolean
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Boolean> struct_boolean =
			new SimpleProperty<Boolean>(
				"struct_boolean", //id
				null, //name
				"boolean", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_ulong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Long> struct_ulong =
			new SimpleProperty<Long>(
				"struct_ulong", //id
				null, //name
				"ulong", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_objref
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 *
		public final SimpleProperty<String> struct_objref =
			new SimpleProperty<String>(
				"struct_objref", //id
				null, //name
				"objref", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
        */
		/**
		 * The property struct_short
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Short> struct_short =
			new SimpleProperty<Short>(
				"struct_short", //id
				null, //name
				"short", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_float
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Float> struct_float =
			new SimpleProperty<Float>(
				"struct_float", //id
				null, //name
				"float", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_octet
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Byte> struct_octet =
			new SimpleProperty<Byte>(
				"struct_octet", //id
				null, //name
				"octet", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_char
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Character> struct_char =
			new SimpleProperty<Character>(
				"struct_char", //id
				null, //name
				"char", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_ushort
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Integer> struct_ushort =
			new SimpleProperty<Integer>(
				"struct_ushort", //id
				null, //name
				"ushort", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_double
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Double> struct_double =
			new SimpleProperty<Double>(
				"struct_double", //id
				null, //name
				"double", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_long
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Integer> struct_long =
			new SimpleProperty<Integer>(
				"struct_long", //id
				null, //name
				"long", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_longlong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Long> struct_longlong =
			new SimpleProperty<Long>(
				"struct_longlong", //id
				null, //name
				"longlong", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_ulonglong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Object> struct_ulonglong =
			new SimpleProperty<Object>(
				"struct_ulonglong", //id
				null, //name
				"ulonglong", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);


		/**
		 * @generated
		 */
		public struct_vars_struct() {
			addElement(struct_string);
			addElement(struct_boolean);
			addElement(struct_ulong);
			//addElement(struct_objref);
			addElement(struct_short);
			addElement(struct_float);
			addElement(struct_octet);
			addElement(struct_char);
			addElement(struct_ushort);
			addElement(struct_double);
			addElement(struct_long);
			addElement(struct_longlong);
			addElement(struct_ulonglong);
			//begin-user-code
			//end-user-code
		}
	};
	
	/**
	 * The property struct_vars
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final StructProperty<struct_vars_struct> struct_vars =
		new StructProperty<struct_vars_struct> (
			"struct_vars", //id
			null, //name
			new struct_vars_struct(), //type
			new struct_vars_struct(), // tmp type
			"readwrite", //mode
			new String[] { "configure" } //kind
			);
    
	/**
	 * The structure for property struct_seq_vars
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public static class struct_seq_vars_struct extends StructDef {

		/**
		 * The property struct_seq_string
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<String> struct_seq_string =
			new SimpleProperty<String>(
				"struct_seq_string", //id
				null, //name
				"string", //type
				null, //default value
				"readwrite", //mode
				"null", //action
				new String[] {} //kind
				);
		/**
		 * The property struct_seq_boolean
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Boolean> struct_seq_boolean =
			new SimpleProperty<Boolean>(
				"struct_seq_boolean", //id
				null, //name
				"boolean", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_ulong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Long> struct_seq_ulong =
			new SimpleProperty<Long>(
				"struct_seq_ulong", //id
				null, //name
				"ulong", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_objref
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 *
		public final SimpleProperty<String> struct_seq_objref =
			new SimpleProperty<String>(
				"struct_seq_objref", //id
				null, //name
				"objref", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
        */
		/**
		 * The property struct_seq_short
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Short> struct_seq_short =
			new SimpleProperty<Short>(
				"struct_seq_short", //id
				null, //name
				"short", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_float
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Float> struct_seq_float =
			new SimpleProperty<Float>(
				"struct_seq_float", //id
				null, //name
				"float", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_octet
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Byte> struct_seq_octet =
			new SimpleProperty<Byte>(
				"struct_seq_octet", //id
				null, //name
				"octet", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_char
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Character> struct_seq_char =
			new SimpleProperty<Character>(
				"struct_seq_char", //id
				null, //name
				"char", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_ushort
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Integer> struct_seq_ushort =
			new SimpleProperty<Integer>(
				"struct_seq_ushort", //id
				null, //name
				"ushort", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_double
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Double> struct_seq_double =
			new SimpleProperty<Double>(
				"struct_seq_double", //id
				null, //name
				"double", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_long
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Integer> struct_seq_long =
			new SimpleProperty<Integer>(
				"struct_seq_long", //id
				null, //name
				"long", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_longlong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Long> struct_seq_longlong =
			new SimpleProperty<Long>(
				"struct_seq_longlong", //id
				null, //name
				"longlong", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);
		/**
		 * The property struct_seq_ulonglong
		 * If the meaning of this property isn't clear, a description should be added.
		 * 
		 * <!-- begin-user-doc -->
		 * <!-- end-user-doc -->
		 * @generated
		 */
		public final SimpleProperty<Object> struct_seq_ulonglong =
			new SimpleProperty<Object>(
				"struct_seq_ulonglong", //id
				null, //name
				"ulonglong", //type
				null, //default value
				"readwrite", //mode
				"external", //action
				new String[] {"configure"} //kind
				);


		/**
		 * @generated
		 */
		public struct_seq_vars_struct() {
			addElement(struct_seq_string);
			addElement(struct_seq_boolean);
			addElement(struct_seq_ulong);
			//addElement(struct_seq_objref);
			addElement(struct_seq_short);
			addElement(struct_seq_float);
			addElement(struct_seq_octet);
			addElement(struct_seq_char);
			addElement(struct_seq_ushort);
			addElement(struct_seq_double);
			addElement(struct_seq_long);
			addElement(struct_seq_longlong);
			addElement(struct_seq_ulonglong);
			//begin-user-code
			//end-user-code
		}
	};

	/**
	 * The property struct_seq
	 * If the meaning of this property isn't clear, a description should be added.
	 * 
	 * <!-- begin-user-doc -->
	 * <!-- end-user-doc -->
	 * @generated
	 */
	public final StructSequenceProperty<struct_seq_vars_struct> struct_seq; 
    // Provides/inputs

    // Uses/outputs

    /**
     * @generated
     */
    public TestAllPropTypes() 
    {
        super();                                                      
        ArrayList<struct_seq_vars_struct> structVals_struct_seq_vars_struct = new ArrayList<struct_seq_vars_struct>();
		
        this.struct_seq = new StructSequenceProperty<struct_seq_vars_struct> (
			"struct_seq", //id
			null, //name
			struct_seq_vars_struct.class, //type
			structVals_struct_seq_vars_struct, //defaultValue
			"readwrite", //mode
			new String[] { "configure" } //kind
			);    
        addProperty(simple_string);
        addProperty(simple_boolean);
        addProperty(simple_ulong);
        //addProperty(simple_objref);
        addProperty(simple_short);
        addProperty(simple_float);
        addProperty(simple_octet);
        addProperty(simple_char);
        addProperty(simple_ushort);
        addProperty(simple_double);
        addProperty(simple_long);
        addProperty(simple_longlong);
        addProperty(simple_ulonglong);
        addProperty(simple_sequence_string);
        addProperty(simple_sequence_boolean);
        addProperty(simple_sequence_ulong);
        //addProperty(simple_sequence_objref);
        addProperty(simple_sequence_short);
        addProperty(simple_sequence_float);
        addProperty(simple_sequence_octet);
        addProperty(simple_sequence_char);
        addProperty(simple_sequence_ushort);
        addProperty(simple_sequence_double);
        addProperty(simple_sequence_long);
        addProperty(simple_sequence_longlong);
        addProperty(simple_sequence_ulonglong);
        addProperty(struct_vars);
        addProperty(struct_seq);

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
            Resource.start_component(TestAllPropTypes.class, args, orbProps);
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
