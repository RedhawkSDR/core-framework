package TestJavaPropsRange.java;

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
 * Source: TestJavaPropsRange.spd.xml
 *
 * @generated
 */

public abstract class TestJavaPropsRange_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestJavaPropsRange_base.class.getName());

    /**
     * The property my_octet
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final OctetProperty my_octet_name =
        new OctetProperty(
            "my_octet", //id
            "my_octet_name", //name
            (byte)1, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property my_short
     * If the meaning of this property isn't clear, a description should be added.
     *
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
     * The property my_ulonglong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ULongLongProperty my_ulonglong_name =
        new ULongLongProperty(
            "my_ulonglong", //id
            "my_ulonglong_name", //name
            7L, //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property seq_octet
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final OctetSequenceProperty seq_octet_name =
        new OctetSequenceProperty(
            "seq_octet", //id
            "seq_octet_name", //name
            OctetSequenceProperty.asList((byte)1,(byte)2), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property seq_short
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ShortSequenceProperty seq_short_name =
        new ShortSequenceProperty(
            "seq_short", //id
            "seq_short_name", //name
            ShortSequenceProperty.asList((short)1,(short)2), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property seq_ushort
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final UShortSequenceProperty seq_ushort_name =
        new UShortSequenceProperty(
            "seq_ushort", //id
            "seq_ushort_name", //name
            UShortSequenceProperty.asList((short)1,(short)2), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property seq_long
     * If the meaning of this property isn't clear, a description should be added.
     *
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
     * The property seq_ulonglong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ULongLongSequenceProperty seq_ulonglong_name =
        new ULongLongSequenceProperty(
            "seq_ulonglong", //id
            "seq_ulonglong_name", //name
            ULongLongSequenceProperty.asList(1L,2L), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.CONFIGURE} //kind
            );
    
    /**
     * The property seq_char
     * If the meaning of this property isn't clear, a description should be added.
     *
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
                (byte)1, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ShortProperty struct_short_name =
            new ShortProperty(
                "struct_short", //id
                "struct_short_name", //name
                (short)2, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortProperty struct_ushort_name =
            new UShortProperty(
                "struct_ushort", //id
                "struct_ushort_name", //name
                (short)3, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty struct_long_name =
            new LongProperty(
                "struct_long", //id
                "struct_long_name", //name
                4, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty struct_ulong_name =
            new ULongProperty(
                "struct_ulong", //id
                "struct_ulong_name", //name
                5, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongLongProperty struct_longlong_name =
            new LongLongProperty(
                "struct_longlong", //id
                "struct_longlong_name", //name
                6L, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongLongProperty struct_ulonglong_name =
            new ULongLongProperty(
                "struct_ulonglong", //id
                "struct_ulonglong_name", //name
                7L, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final OctetSequenceProperty struct_seq_octet_name =
            new OctetSequenceProperty(
                "struct_seq_octet", //id
                "struct_seq_octet_name", //name
                OctetSequenceProperty.asList((byte)1,(byte)2), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ShortSequenceProperty struct_seq_short_name =
            new ShortSequenceProperty(
                "struct_seq_short", //id
                "struct_seq_short_name", //name
                ShortSequenceProperty.asList((short)1,(short)2), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortSequenceProperty struct_seq_ushort_name =
            new UShortSequenceProperty(
                "struct_seq_ushort", //id
                "struct_seq_ushort_name", //name
                UShortSequenceProperty.asList((short)1,(short)2), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongSequenceProperty struct_seq_long_name =
            new LongSequenceProperty(
                "struct_seq_long", //id
                "struct_seq_long_name", //name
                LongSequenceProperty.asList(1,2), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongSequenceProperty struct_seq_ulong_name =
            new ULongSequenceProperty(
                "struct_seq_ulong", //id
                "struct_seq_ulong_name", //name
                ULongSequenceProperty.asList(1,2), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongLongSequenceProperty struct_seq_longlong_name =
            new LongLongSequenceProperty(
                "struct_seq_longlong", //id
                "struct_seq_longlong_name", //name
                LongLongSequenceProperty.asList(1L,2L), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongLongSequenceProperty struct_seq_ulonglong_name =
            new ULongLongSequenceProperty(
                "struct_seq_ulonglong", //id
                "struct_seq_ulonglong_name", //name
                ULongLongSequenceProperty.asList(1L,2L), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public my_struct_name_struct(Byte struct_octet_name, Short struct_short_name, Short struct_ushort_name, Integer struct_long_name, Integer struct_ulong_name, Long struct_longlong_name, Long struct_ulonglong_name, List<Byte> struct_seq_octet_name, List<Short> struct_seq_short_name, List<Short> struct_seq_ushort_name, List<Integer> struct_seq_long_name, List<Integer> struct_seq_ulong_name, List<Long> struct_seq_longlong_name, List<Long> struct_seq_ulonglong_name) {
            this();
            this.struct_octet_name.setValue(struct_octet_name);
            this.struct_short_name.setValue(struct_short_name);
            this.struct_ushort_name.setValue(struct_ushort_name);
            this.struct_long_name.setValue(struct_long_name);
            this.struct_ulong_name.setValue(struct_ulong_name);
            this.struct_longlong_name.setValue(struct_longlong_name);
            this.struct_ulonglong_name.setValue(struct_ulonglong_name);
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
     * The property my_structseq
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property ss_struct
     * 
     * @generated
     */
    public static class ss_struct_name_struct extends StructDef {
        public final OctetProperty ss_octet_name =
            new OctetProperty(
                "ss_octet", //id
                "ss_octet_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ShortProperty ss_short_name =
            new ShortProperty(
                "ss_short", //id
                "ss_short_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortProperty ss_ushort_name =
            new UShortProperty(
                "ss_ushort", //id
                "ss_ushort_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongProperty ss_long_name =
            new LongProperty(
                "ss_long", //id
                "ss_long_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongProperty ss_ulong_name =
            new ULongProperty(
                "ss_ulong", //id
                "ss_ulong_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongLongProperty ss_longlong_name =
            new LongLongProperty(
                "ss_longlong", //id
                "ss_longlong_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongLongProperty ss_ulonglong_name =
            new ULongLongProperty(
                "ss_ulonglong", //id
                "ss_ulonglong_name", //name
                null, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final OctetSequenceProperty ss_seq_octet_name =
            new OctetSequenceProperty(
                "ss_seq_octet", //id
                "ss_seq_octet_name", //name
                OctetSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ShortSequenceProperty ss_seq_short_name =
            new ShortSequenceProperty(
                "ss_seq_short", //id
                "ss_seq_short_name", //name
                ShortSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final UShortSequenceProperty ss_seq_ushort_name =
            new UShortSequenceProperty(
                "ss_seq_ushort", //id
                "ss_seq_ushort_name", //name
                UShortSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongSequenceProperty ss_seq_long_name =
            new LongSequenceProperty(
                "ss_seq_long", //id
                "ss_seq_long_name", //name
                LongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongSequenceProperty ss_seq_ulong_name =
            new ULongSequenceProperty(
                "ss_seq_ulong", //id
                "ss_seq_ulong_name", //name
                ULongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final LongLongSequenceProperty ss_seq_longlong_name =
            new LongLongSequenceProperty(
                "ss_seq_longlong", //id
                "ss_seq_longlong_name", //name
                LongLongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
        public final ULongLongSequenceProperty ss_seq_ulonglong_name =
            new ULongLongSequenceProperty(
                "ss_seq_ulonglong", //id
                "ss_seq_ulonglong_name", //name
                ULongLongSequenceProperty.asList(), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE} //kind
                );
    
        /**
         * @generated
         */
        public ss_struct_name_struct(Byte ss_octet_name, Short ss_short_name, Short ss_ushort_name, Integer ss_long_name, Integer ss_ulong_name, Long ss_longlong_name, Long ss_ulonglong_name, List<Byte> ss_seq_octet_name, List<Short> ss_seq_short_name, List<Short> ss_seq_ushort_name, List<Integer> ss_seq_long_name, List<Integer> ss_seq_ulong_name, List<Long> ss_seq_longlong_name, List<Long> ss_seq_ulonglong_name) {
            this();
            this.ss_octet_name.setValue(ss_octet_name);
            this.ss_short_name.setValue(ss_short_name);
            this.ss_ushort_name.setValue(ss_ushort_name);
            this.ss_long_name.setValue(ss_long_name);
            this.ss_ulong_name.setValue(ss_ulong_name);
            this.ss_longlong_name.setValue(ss_longlong_name);
            this.ss_ulonglong_name.setValue(ss_ulonglong_name);
            this.ss_seq_octet_name.setValue(ss_seq_octet_name);
            this.ss_seq_short_name.setValue(ss_seq_short_name);
            this.ss_seq_ushort_name.setValue(ss_seq_ushort_name);
            this.ss_seq_long_name.setValue(ss_seq_long_name);
            this.ss_seq_ulong_name.setValue(ss_seq_ulong_name);
            this.ss_seq_longlong_name.setValue(ss_seq_longlong_name);
            this.ss_seq_ulonglong_name.setValue(ss_seq_ulonglong_name);
        }
    
        /**
         * @generated
         */
        public ss_struct_name_struct() {
            addElement(this.ss_octet_name);
            addElement(this.ss_short_name);
            addElement(this.ss_ushort_name);
            addElement(this.ss_long_name);
            addElement(this.ss_ulong_name);
            addElement(this.ss_longlong_name);
            addElement(this.ss_ulonglong_name);
            addElement(this.ss_seq_octet_name);
            addElement(this.ss_seq_short_name);
            addElement(this.ss_seq_ushort_name);
            addElement(this.ss_seq_long_name);
            addElement(this.ss_seq_ulong_name);
            addElement(this.ss_seq_longlong_name);
            addElement(this.ss_seq_ulonglong_name);
        }
    
        public String getId() {
            return "ss_struct";
        }
    };
    
    public final StructSequenceProperty<ss_struct_name_struct> my_structseq_name =
        new StructSequenceProperty<ss_struct_name_struct> (
            "my_structseq", //id
            "my_structseq_name", //name
            ss_struct_name_struct.class, //type
            StructSequenceProperty.asList(
                new ss_struct_name_struct((byte)0, (short)1, (short)2, 3, 4, 5L, 6L, OctetSequenceProperty.asList((byte)1,(byte)2), ShortSequenceProperty.asList((short)3,(short)4), UShortSequenceProperty.asList((short)5,(short)6), LongSequenceProperty.asList(7,8), ULongSequenceProperty.asList(9,10), LongLongSequenceProperty.asList(11L,12L), ULongLongSequenceProperty.asList(13L,14L)),
                            new ss_struct_name_struct((byte)7, (short)8, (short)9, 10, 11, 12L, 13L, OctetSequenceProperty.asList((byte)15,(byte)16), ShortSequenceProperty.asList((short)17,(short)18), UShortSequenceProperty.asList((short)19,(short)20), LongSequenceProperty.asList(21,22), ULongSequenceProperty.asList(23,24), LongLongSequenceProperty.asList(25L,26L), ULongLongSequenceProperty.asList(27L,28L))
                ), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.CONFIGURE } //kind
        );
    
    /**
     * @generated
     */
    public TestJavaPropsRange_base()
    {
        super();

        // Properties
        addProperty(my_octet_name);

        addProperty(my_short_name);

        addProperty(my_ushort_name);

        addProperty(my_long_name);

        addProperty(my_ulong_name);

        addProperty(my_longlong_name);

        addProperty(my_ulonglong_name);

        addProperty(seq_octet_name);

        addProperty(seq_short_name);

        addProperty(seq_ushort_name);

        addProperty(seq_long_name);

        addProperty(seq_ulong_name);

        addProperty(seq_longlong_name);

        addProperty(seq_ulonglong_name);

        addProperty(seq_char_name);

        addProperty(my_struct_name);

        addProperty(my_structseq_name);

    }

    public void start() throws CF.ResourcePackage.StartError
    {
        super.start();
    }

    public void stop() throws CF.ResourcePackage.StopError
    {
        super.stop();
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
        TestJavaPropsRange.configureOrb(orbProps);

        try {
            Component.start_component(TestJavaPropsRange.class, args, orbProps);
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
