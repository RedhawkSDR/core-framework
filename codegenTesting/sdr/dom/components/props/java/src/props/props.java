/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK codegenTesting.
 *
 * REDHAWK codegenTesting is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
package props;

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
 * Source: props.spd.xml
 *
 * @generated
 */
public class props extends props_base {
    public props()
    {
        super();
        this.stringSimple.addChangeListener(new PropertyListener<String>(){
                                       public void valueChanged(String old_value, String new_value){
                                           stringSimpleChanged(old_value, new_value);
                                       }
        });
        this.boolSimple.addChangeListener(new PropertyListener<Boolean>(){
                                       public void valueChanged(Boolean old_value, Boolean new_value){
                                           boolSimpleChanged(old_value, new_value);
                                       }
        });
        this.ulongSimple.addChangeListener(new PropertyListener<Integer>(){
                                       public void valueChanged(Integer old_value, Integer new_value){
                                           ulongSimpleChanged(old_value, new_value);
                                       }
        });
        this.shortSimple.addChangeListener(new PropertyListener<Short>(){
                                       public void valueChanged(Short old_value, Short new_value){
                                           shortSimpleChanged(old_value, new_value);
                                       }
        });
        this.floatSimple.addChangeListener(new PropertyListener<Float>(){
                                       public void valueChanged(Float old_value, Float new_value){
                                           floatSimpleChanged(old_value, new_value);
                                       }
        });
        this.octetSimple.addChangeListener(new PropertyListener<Byte>(){
                                       public void valueChanged(Byte old_value, Byte new_value){
                                           octetSimpleChanged(old_value, new_value);
                                       }
        });
        this.charSimple.addChangeListener(new PropertyListener<Character>(){
                                       public void valueChanged(Character old_value, Character new_value){
                                           charSimpleChanged(old_value, new_value);
                                       }
        });
        this.ushortSimple.addChangeListener(new PropertyListener<Short>(){
                                       public void valueChanged(Short old_value, Short new_value){
                                           ushortSimpleChanged(old_value, new_value);
                                       }
        });
        this.doubleSimple.addChangeListener(new PropertyListener<Double>(){
                                       public void valueChanged(Double old_value, Double new_value){
                                           doubleSimpleChanged(old_value, new_value);
                                       }
        });
        this.longSimple.addChangeListener(new PropertyListener<Integer>(){
                                       public void valueChanged(Integer old_value, Integer new_value){
                                           longSimpleChanged(old_value, new_value);
                                       }
        });
        this.longlongSimple.addChangeListener(new PropertyListener<Long>(){
                                       public void valueChanged(Long old_value, Long new_value){
                                           longlongSimpleChanged(old_value, new_value);
                                       }
        });
        this.ulonglongSimple.addChangeListener(new PropertyListener<Long>(){
                                       public void valueChanged(Long old_value, Long new_value){
                                           ulonglongSimpleChanged(old_value, new_value);
                                       }
        });
        this.stringSeq.addChangeListener(new PropertyListener<List<String>>(){
                                       public void valueChanged(List<String> old_value, List<String> new_value){
                                           stringSeqChanged(old_value, new_value);
                                       }
        });
        this.boolSeq.addChangeListener(new PropertyListener<List<Boolean>>(){
                                       public void valueChanged(List<Boolean> old_value, List<Boolean> new_value){
                                           boolSeqChanged(old_value, new_value);
                                       }
        });
        this.ulongSeq.addChangeListener(new PropertyListener<List<Integer>>(){
                                       public void valueChanged(List<Integer> old_value, List<Integer> new_value){
                                           ulongSeqChanged(old_value, new_value);
                                       }
        });
        this.longSeq.addChangeListener(new PropertyListener<List<Integer>>(){
                                       public void valueChanged(List<Integer> old_value, List<Integer> new_value){
                                           longSeqChanged(old_value, new_value);
                                       }
        });
        this.shortSeq.addChangeListener(new PropertyListener<List<Short>>(){
                                       public void valueChanged(List<Short> old_value, List<Short> new_value){
                                           shortSeqChanged(old_value, new_value);
                                       }
        });
        this.floatSeq.addChangeListener(new PropertyListener<List<Float>>(){
                                       public void valueChanged(List<Float> old_value, List<Float> new_value){
                                           floatSeqChanged(old_value, new_value);
                                       }
        });
        this.octetSeq.addChangeListener(new PropertyListener<List<Byte>>(){
                                       public void valueChanged(List<Byte> old_value, List<Byte> new_value){
                                           octetSeqChanged(old_value, new_value);
                                       }
        });
        this.charSeq.addChangeListener(new PropertyListener<List<Character>>(){
                                       public void valueChanged(List<Character> old_value, List<Character> new_value){
                                           charSeqChanged(old_value, new_value);
                                       }
        });
        this.ushortSeq.addChangeListener(new PropertyListener<List<Short>>(){
                                       public void valueChanged(List<Short> old_value, List<Short> new_value){
                                           ushortSeqChanged(old_value, new_value);
                                       }
        });
        this.ulonglongSeq.addChangeListener(new PropertyListener<List<Long>>(){
                                       public void valueChanged(List<Long> old_value, List<Long> new_value){
                                           ulonglongSeqChanged(old_value, new_value);
                                       }
        });
        this.longlongSeq.addChangeListener(new PropertyListener<List<Long>>(){
                                       public void valueChanged(List<Long> old_value, List<Long> new_value){
                                           longlongSeqChanged(old_value, new_value);
                                       }
        });
        this.doubleSeq.addChangeListener(new PropertyListener<List<Double>>(){
                                       public void valueChanged(List<Double> old_value, List<Double> new_value){
                                           doubleSeqChanged(old_value, new_value);
                                       }
        });
        this.structProp.addChangeListener(new PropertyListener<structProp_struct>(){
                                       public void valueChanged(structProp_struct old_value, structProp_struct new_value){
                                           structPropChanged(old_value, new_value);
                                       }
        });
        this.structSeqProp.addChangeListener(new PropertyListener<List<structSeqStruct_struct>>(){
                                       public void valueChanged(List<structSeqStruct_struct> old_value, List<structSeqStruct_struct> new_value){
                                           structSeqPropChanged(old_value, new_value);
                                       }
        });
    }

    protected int serviceFunction() 
    {
        //begin-user-code
        //end-user-code
        
            //begin-user-code
            // Process data here
            try {
                Thread.sleep(1000);
            } catch (InterruptedException e) {
            }
            return NOOP;    
        //begin-user-code
        //end-user-code
    }

    private void stringSimpleChanged(String old_value, String new_value)
    {
        stringSimple.setValue(new_value.concat(new_value));
    }

    private void boolSimpleChanged(Boolean old_value, Boolean new_value)
    {
        if (new_value != null)
            boolSimple.setValue(!new_value);
    }

    private void ulongSimpleChanged(Integer old_value, Integer new_value)
    {
        ulongSimple.setValue(new_value * 2);
    }

    private void shortSimpleChanged(Short old_value, Short new_value)
    {
        shortSimple.setValue((short) (new_value * 2));
    }

    private void floatSimpleChanged(Float old_value, Float new_value)
    {
        floatSimple.setValue(new_value * 2);
    }

    private void octetSimpleChanged(Byte old_value, Byte new_value)
    {
        octetSimple.setValue((byte) (new_value * 2));
    }

    private void charSimpleChanged(Character old_value, Character new_value)
    {
        charSimple.setValue(Character.toUpperCase(new_value));
    }

    private void ushortSimpleChanged(Short old_value, Short new_value)
    {
        ushortSimple.setValue(new_value * 2);
    }

    private void doubleSimpleChanged(Double old_value, Double new_value)
    {
        doubleSimple.setValue(new_value * 2);
    }

    private void longSimpleChanged(Integer old_value, Integer new_value)
    {
        longSimple.setValue(new_value * 2);
    }

    private void longlongSimpleChanged(Long old_value, Long new_value)
    {
        longlongSimple.setValue(new_value * 2);
    }

    private void ulonglongSimpleChanged(Long old_value, Long new_value)
    {
        ulonglongSimple.setValue(Integer.parseInt(new_value.toString()) * 2);
    }

    private void stringSeqChanged(List<String> old_value, List<String> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void boolSeqChanged(List<Boolean> old_value, List<Boolean> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void ulongSeqChanged(List<Integer> old_value, List<Integer> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void shortSeqChanged(List<Short> old_value, List<Short> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void floatSeqChanged(List<Float> old_value, List<Float> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void octetSeqChanged(List<Byte> old_value, List<Byte> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void charSeqChanged(List<Character> old_value, List<Character> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void ushortSeqChanged(List<Short> old_value, List<Short> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void ulonglongSeqChanged(List<Long> old_value, List<Long> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void longSeqChanged(List<Integer> old_value, List<Integer> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void longlongSeqChanged(List<Long> old_value, List<Long> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void doubleSeqChanged(List<Double> old_value, List<Double> new_value)
    {
        java.util.Collections.reverse(new_value);
    }

    private void structPropChanged(structProp_struct old_value, structProp_struct new_value)
    {
        try {
            structProp_struct value = new_value;
            value.structStringSimple.setValue(value.structStringSimple.getValue().concat(value.structStringSimple.getValue()));
            value.structBoolSimple.setValue(!value.structBoolSimple.getValue());
            value.structUlongSimple.setValue(value.structUlongSimple.getValue() * 2);
            value.structShortSimple.setValue((short) (value.structShortSimple.getValue() * 2));
            value.structFloatSimple.setValue(value.structFloatSimple.getValue() * 2);
            value.structOctetSimple.setValue((byte) (value.structOctetSimple.getValue() * 2));
            value.structCharSimple.setValue(Character.toUpperCase(value.structCharSimple.getValue()));
            value.structUshortSimple.setValue(value.structUshortSimple.getValue() * 2);
            value.structDoubleSimple.setValue(value.structDoubleSimple.getValue() * 2);
            value.structLongSimple.setValue(value.structLongSimple.getValue() * 2);
            value.structLonglongSimple.setValue(value.structLonglongSimple.getValue() * 2);
            value.structUlonglongSimple.setValue(Integer.parseInt(value.structUlonglongSimple.getValue().toString()) * 2);
        } catch(Exception e) {
            //pass
        }
    }

    private void structSeqPropChanged(List<structSeqStruct_struct> old_value, List<structSeqStruct_struct> new_value)
    {
        for (structSeqStruct_struct value : new_value) {
            value.structSeqBoolSimple.setValue(!value.structSeqBoolSimple.getValue());
            value.structSeqFloatSimple.setValue(value.structSeqFloatSimple.getValue() * 2);
            value.structSeqStringSimple.setValue(value.structSeqStringSimple.getValue().concat(value.structSeqStringSimple.getValue()));
            value.structSeqShortSimple.setValue((short) (value.structSeqShortSimple.getValue() * 2));
        }
    }

    /**
     * Set additional options for ORB startup. For example:
     *
     *   orbProps.put("com.sun.CORBA.giop.ORBFragmentSize", Integer.toString(fragSize));
     *
     * @param orbProps
     */
    public static void configureOrb(final Properties orbProps) {
    }
}
