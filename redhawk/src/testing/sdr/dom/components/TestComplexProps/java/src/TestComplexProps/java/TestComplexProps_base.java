package TestComplexProps.java;


import java.util.List;
import java.util.List;
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
 * Source: TestComplexProps.spd.xml
 *
 * @generated
 */

public abstract class TestComplexProps_base extends Component {
    /**
     * @generated
     */
    public final static Logger logger = Logger.getLogger(TestComplexProps_base.class.getName());

    /**
     * The property complexBooleanProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexBooleanProperty complexBooleanProp =
        new ComplexBooleanProperty(
            "complexBooleanProp", //id
            null, //name
            new CF.complexBoolean(false,true), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexULongProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexULongProperty complexULongProp =
        new ComplexULongProperty(
            "complexULongProp", //id
            null, //name
            new CF.complexULong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexShortProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexShortProperty complexShortProp =
        new ComplexShortProperty(
            "complexShortProp", //id
            null, //name
            new CF.complexShort((short)4,(short)5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexFloatProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexFloatProperty complexFloatProp =
        new ComplexFloatProperty(
            "complexFloatProp", //id
            null, //name
            new CF.complexFloat(4.0F,5.0F), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexOctetProp
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexOctetProperty complexOctetProp =
        new ComplexOctetProperty(
            "complexOctetProp", //id
            null, //name
            new CF.complexOctet((byte)4,(byte)5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexUShort
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexUShortProperty complexUShort =
        new ComplexUShortProperty(
            "complexUShort", //id
            null, //name
            new CF.complexUShort((short)4,(short)5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexDouble
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexDoubleProperty complexDouble =
        new ComplexDoubleProperty(
            "complexDouble", //id
            null, //name
            new CF.complexDouble(4.0,5.0), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexLong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexLongProperty complexLong =
        new ComplexLongProperty(
            "complexLong", //id
            null, //name
            new CF.complexLong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexLongLong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexLongLongProperty complexLongLong =
        new ComplexLongLongProperty(
            "complexLongLong", //id
            null, //name
            new CF.complexLongLong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexULongLong
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexULongLongProperty complexULongLong =
        new ComplexULongLongProperty(
            "complexULongLong", //id
            null, //name
            new CF.complexULongLong(4,5), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property complexFloatSequence
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    public final ComplexFloatSequenceProperty complexFloatSequence =
        new ComplexFloatSequenceProperty(
            "complexFloatSequence", //id
            null, //name
            ComplexFloatSequenceProperty.asList(new CF.complexFloat(6.0F,7.0F),new CF.complexFloat(4.0F,5.0F),new CF.complexFloat(4.0F,5.0F)), //default value
            Mode.READWRITE, //mode
            Action.EXTERNAL, //action
            new Kind[] {Kind.PROPERTY}
            );
    
    /**
     * The property FloatStruct
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property FloatStruct
     * 
     * @generated
     */
    public static class FloatStruct_struct extends StructDef {
        public final FloatProperty FloatStructMember =
            new FloatProperty(
                "FloatStructMember", //id
                null, //name
                6.0F, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.PROPERTY}
                );
    
        /**
         * @generated
         */
        public FloatStruct_struct(Float FloatStructMember) {
            this();
            this.FloatStructMember.setValue(FloatStructMember);
        }
    
        /**
         * @generated
         */
        public void set_FloatStructMember(Float FloatStructMember) {
            this.FloatStructMember.setValue(FloatStructMember);
        }
        public Float get_FloatStructMember() {
            return this.FloatStructMember.getValue();
        }
    
        /**
         * @generated
         */
        public FloatStruct_struct() {
            addElement(this.FloatStructMember);
        }
    
        public String getId() {
            return "FloatStruct";
        }
    };
    
    public final StructProperty<FloatStruct_struct> FloatStruct =
        new StructProperty<FloatStruct_struct>(
            "FloatStruct", //id
            null, //name
            FloatStruct_struct.class, //type
            new FloatStruct_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.PROPERTY} //kind
            );
    
    /**
     * The property complexFloatStruct
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property complexFloatStruct
     * 
     * @generated
     */
    public static class complexFloatStruct_struct extends StructDef {
        public final ComplexFloatProperty complexFloatStructMember =
            new ComplexFloatProperty(
                "complexFloatStructMember", //id
                null, //name
                new CF.complexFloat(6.0F,7.0F), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.PROPERTY}
                );
        public final ComplexFloatSequenceProperty complex_float_seq =
            new ComplexFloatSequenceProperty(
                "complexFloatStruct::complex_float_seq", //id
                "complex_float_seq", //name
                ComplexFloatSequenceProperty.asList(new CF.complexFloat(3.0F,2.0F)), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public complexFloatStruct_struct(CF.complexFloat complexFloatStructMember, List<CF.complexFloat> complex_float_seq) {
            this();
            this.complexFloatStructMember.setValue(complexFloatStructMember);
            this.complex_float_seq.setValue(complex_float_seq);
        }
    
        /**
         * @generated
         */
        public void set_complexFloatStructMember(CF.complexFloat complexFloatStructMember) {
            this.complexFloatStructMember.setValue(complexFloatStructMember);
        }
        public CF.complexFloat get_complexFloatStructMember() {
            return this.complexFloatStructMember.getValue();
        }
        public void set_complex_float_seq(List<CF.complexFloat> complex_float_seq) {
            this.complex_float_seq.setValue(complex_float_seq);
        }
        public List<CF.complexFloat> get_complex_float_seq() {
            return this.complex_float_seq.getValue();
        }
    
        /**
         * @generated
         */
        public complexFloatStruct_struct() {
            addElement(this.complexFloatStructMember);
            addElement(this.complex_float_seq);
        }
    
        public String getId() {
            return "complexFloatStruct";
        }
    };
    
    public final StructProperty<complexFloatStruct_struct> complexFloatStruct =
        new StructProperty<complexFloatStruct_struct>(
            "complexFloatStruct", //id
            null, //name
            complexFloatStruct_struct.class, //type
            new complexFloatStruct_struct(), //default value
            Mode.READWRITE, //mode
            new Kind[] {Kind.PROPERTY} //kind
            );
    
    /**
     * The property FloatStructSequence
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property FloatStructSequenceMember
     * 
     * @generated
     */
    public static class FloatStructSequenceMember_struct extends StructDef {
        public final FloatProperty FloatStructSequenceMemberMemember =
            new FloatProperty(
                "FloatStructSequenceMemberMemember", //id
                null, //name
                6.0F, //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final FloatSequenceProperty float_seq =
            new FloatSequenceProperty(
                "FloatStructSequence::float_seq", //id
                "float_seq", //name
                FloatSequenceProperty.asList(3.0F), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public FloatStructSequenceMember_struct(Float FloatStructSequenceMemberMemember, List<Float> float_seq) {
            this();
            this.FloatStructSequenceMemberMemember.setValue(FloatStructSequenceMemberMemember);
            this.float_seq.setValue(float_seq);
        }
    
        /**
         * @generated
         */
        public void set_FloatStructSequenceMemberMemember(Float FloatStructSequenceMemberMemember) {
            this.FloatStructSequenceMemberMemember.setValue(FloatStructSequenceMemberMemember);
        }
        public Float get_FloatStructSequenceMemberMemember() {
            return this.FloatStructSequenceMemberMemember.getValue();
        }
        public void set_float_seq(List<Float> float_seq) {
            this.float_seq.setValue(float_seq);
        }
        public List<Float> get_float_seq() {
            return this.float_seq.getValue();
        }
    
        /**
         * @generated
         */
        public FloatStructSequenceMember_struct() {
            addElement(this.FloatStructSequenceMemberMemember);
            addElement(this.float_seq);
        }
    
        public String getId() {
            return "FloatStructSequenceMember";
        }
    };
    
    public final StructSequenceProperty<FloatStructSequenceMember_struct> FloatStructSequence =
        new StructSequenceProperty<FloatStructSequenceMember_struct> (
            "FloatStructSequence", //id
            null, //name
            FloatStructSequenceMember_struct.class, //type
            StructSequenceProperty.<FloatStructSequenceMember_struct>asList(), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.PROPERTY } //kind
        );
    
    /**
     * The property complexFloatStructSequence
     * If the meaning of this property isn't clear, a description should be added.
     *
     * @generated
     */
    /**
     * The structure for property complexFloatStructSequenceMember
     * 
     * @generated
     */
    public static class complexFloatStructSequenceMember_struct extends StructDef {
        public final ComplexFloatProperty complexFloatStructSequenceMemberMemember =
            new ComplexFloatProperty(
                "complexFloatStructSequenceMemberMemember", //id
                null, //name
                new CF.complexFloat(6.0F,5.0F), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
        public final ComplexFloatSequenceProperty complex_float_seq =
            new ComplexFloatSequenceProperty(
                "complexFloatStructSequence::complex_float_seq", //id
                "complex_float_seq", //name
                ComplexFloatSequenceProperty.asList(new CF.complexFloat(3.0F,2.0F)), //default value
                Mode.READWRITE, //mode
                Action.EXTERNAL, //action
                new Kind[] {Kind.CONFIGURE}
                );
    
        /**
         * @generated
         */
        public complexFloatStructSequenceMember_struct(CF.complexFloat complexFloatStructSequenceMemberMemember, List<CF.complexFloat> complex_float_seq) {
            this();
            this.complexFloatStructSequenceMemberMemember.setValue(complexFloatStructSequenceMemberMemember);
            this.complex_float_seq.setValue(complex_float_seq);
        }
    
        /**
         * @generated
         */
        public void set_complexFloatStructSequenceMemberMemember(CF.complexFloat complexFloatStructSequenceMemberMemember) {
            this.complexFloatStructSequenceMemberMemember.setValue(complexFloatStructSequenceMemberMemember);
        }
        public CF.complexFloat get_complexFloatStructSequenceMemberMemember() {
            return this.complexFloatStructSequenceMemberMemember.getValue();
        }
        public void set_complex_float_seq(List<CF.complexFloat> complex_float_seq) {
            this.complex_float_seq.setValue(complex_float_seq);
        }
        public List<CF.complexFloat> get_complex_float_seq() {
            return this.complex_float_seq.getValue();
        }
    
        /**
         * @generated
         */
        public complexFloatStructSequenceMember_struct() {
            addElement(this.complexFloatStructSequenceMemberMemember);
            addElement(this.complex_float_seq);
        }
    
        public String getId() {
            return "complexFloatStructSequenceMember";
        }
    };
    
    public final StructSequenceProperty<complexFloatStructSequenceMember_struct> complexFloatStructSequence =
        new StructSequenceProperty<complexFloatStructSequenceMember_struct> (
            "complexFloatStructSequence", //id
            null, //name
            complexFloatStructSequenceMember_struct.class, //type
            StructSequenceProperty.<complexFloatStructSequenceMember_struct>asList(), //defaultValue
            Mode.READWRITE, //mode
            new Kind[] { Kind.PROPERTY } //kind
        );
    
    /**
     * @generated
     */
    public TestComplexProps_base()
    {
        super();

        setLogger( logger, TestComplexProps_base.class.getName() );


        // Properties
        addProperty(complexBooleanProp);

        addProperty(complexULongProp);

        addProperty(complexShortProp);

        addProperty(complexFloatProp);

        addProperty(complexOctetProp);

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
        TestComplexProps.configureOrb(orbProps);

        try {
            Component.start_component(TestComplexProps.class, args, orbProps);
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
