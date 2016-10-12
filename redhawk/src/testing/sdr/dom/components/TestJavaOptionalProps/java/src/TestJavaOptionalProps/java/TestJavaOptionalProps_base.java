package TestJavaOptionalProps.java;

import java.util.List;
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
 * Source: TestJavaOptionalProps.spd.xml
 *
 * @generated
 */

public abstract class TestJavaOptionalProps_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestJavaOptionalProps_base.class.getName());

    /**
     * The property my_struct
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property my_struct
     * 
     * @generated
     */
    public static class my_struct_name_struct extends StructDef {
        public final OctetProperty struct_octet_name =
            new OctetProperty(
                "struct_octet", //id
                "struct_octet_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final ShortProperty struct_short_name =
            new ShortProperty(
                "struct_short", //id
                "struct_short_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final UShortProperty struct_ushort_name =
            new UShortProperty(
                "struct_ushort", //id
                "struct_ushort_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final LongProperty struct_long_name =
            new LongProperty(
                "struct_long", //id
                "struct_long_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final ULongProperty struct_ulong_name =
            new ULongProperty(
                "struct_ulong", //id
                "struct_ulong_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final LongLongProperty struct_longlong_name =
            new LongLongProperty(
                "struct_longlong", //id
                "struct_longlong_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final ULongLongProperty struct_ulonglong_name =
            new ULongLongProperty(
                "struct_ulonglong", //id
                "struct_ulonglong_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final StringProperty struct_string_name =
            new StringProperty(
                "struct_string", //id
                "struct_string_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final OctetSequenceProperty struct_seq_octet_name =
            new OctetSequenceProperty(
                "struct_seq_octet", //id
                "struct_seq_octet_name", //name
                OctetSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final ShortSequenceProperty struct_seq_short_name =
            new ShortSequenceProperty(
                "struct_seq_short", //id
                "struct_seq_short_name", //name
                ShortSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final UShortSequenceProperty struct_seq_ushort_name =
            new UShortSequenceProperty(
                "struct_seq_ushort", //id
                "struct_seq_ushort_name", //name
                UShortSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final LongSequenceProperty struct_seq_long_name =
            new LongSequenceProperty(
                "struct_seq_long", //id
                "struct_seq_long_name", //name
                LongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final ULongSequenceProperty struct_seq_ulong_name =
            new ULongSequenceProperty(
                "struct_seq_ulong", //id
                "struct_seq_ulong_name", //name
                ULongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final LongLongSequenceProperty struct_seq_longlong_name =
            new LongLongSequenceProperty(
                "struct_seq_longlong", //id
                "struct_seq_longlong_name", //name
                LongLongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
        public final ULongLongSequenceProperty struct_seq_ulonglong_name =
            new ULongLongSequenceProperty(
                "struct_seq_ulonglong", //id
                "struct_seq_ulonglong_name", //name
                ULongLongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}, //kind
                true
                );
    
        /**
         * @generated
         */
        public my_struct_name_struct(Byte struct_octet_name, Short struct_short_name, Short struct_ushort_name, Integer struct_long_name, Integer struct_ulong_name, Long struct_longlong_name, Long struct_ulonglong_name, String struct_string_name, List<Byte> struct_seq_octet_name, List<Short> struct_seq_short_name, List<Short> struct_seq_ushort_name, List<Integer> struct_seq_long_name, List<Integer> struct_seq_ulong_name, List<Long> struct_seq_longlong_name, List<Long> struct_seq_ulonglong_name) {
            this();
            this.struct_octet_name.setValue(struct_octet_name);
            this.struct_short_name.setValue(struct_short_name);
            this.struct_ushort_name.setValue(struct_ushort_name);
            this.struct_long_name.setValue(struct_long_name);
            this.struct_ulong_name.setValue(struct_ulong_name);
            this.struct_longlong_name.setValue(struct_longlong_name);
            this.struct_ulonglong_name.setValue(struct_ulonglong_name);
            this.struct_string_name.setValue(struct_string_name);
            this.struct_seq_octet_name.setValue(struct_seq_octet_name);
            this.struct_seq_short_name.setValue(struct_seq_short_name);
            this.struct_seq_ushort_name.setValue(struct_seq_ushort_name);
            this.struct_seq_long_name.setValue(struct_seq_long_name);
            this.struct_seq_ulong_name.setValue(struct_seq_ulong_name);
            this.struct_seq_longlong_name.setValue(struct_seq_longlong_name);
            this.struct_seq_ulonglong_name.setValue(struct_seq_ulonglong_name);
        }
    
        /**
         * @generated
         */
        public my_struct_name_struct() {
            addElement(this.struct_octet_name);
            addElement(this.struct_short_name);
            addElement(this.struct_ushort_name);
            addElement(this.struct_long_name);
            addElement(this.struct_ulong_name);
            addElement(this.struct_longlong_name);
            addElement(this.struct_ulonglong_name);
            addElement(this.struct_string_name);
            addElement(this.struct_seq_octet_name);
            addElement(this.struct_seq_short_name);
            addElement(this.struct_seq_ushort_name);
            addElement(this.struct_seq_long_name);
            addElement(this.struct_seq_ulong_name);
            addElement(this.struct_seq_longlong_name);
            addElement(this.struct_seq_ulonglong_name);
        }
    
        public String getId() {
            return "my_struct";
        }
    };
    
    public final StructProperty<my_struct_name_struct> my_struct_name =
        new StructProperty<my_struct_name_struct>(
            "my_struct", //id
            "my_struct_name", //name
            my_struct_name_struct.class, //type
            new my_struct_name_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * @generated
     */
    public TestJavaOptionalProps_base()
    {
        super();

        // Properties
        addProperty(my_struct_name);

    }

    public void start() throws CF.ResourcePackage.StartError
    {
        super.start();
    }

    public void stop() throws CF.ResourcePackage.StopError
    {
        super.stop();
    }
    
    public void runTest(final int testid, final CF.PropertiesHolder testValues) 
    		throws CF.TestableObjectPackage.UnknownTest, CF.UnknownProperties {
        if (testid == 0) {
        	testOptional(testValues);
        }
    }
    
    private void testOptional(final CF.PropertiesHolder testValues) {
    	System.out.println("Test Optional");
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
        TestJavaOptionalProps.configureOrb(orbProps);

        try {
            Component.start_component(TestJavaOptionalProps.class, args, orbProps);
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
