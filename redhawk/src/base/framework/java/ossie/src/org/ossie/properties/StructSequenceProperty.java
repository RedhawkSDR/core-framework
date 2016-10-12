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

package org.ossie.properties;

import java.lang.reflect.Array;
import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.omg.CORBA.Any;
import org.omg.CORBA.AnySeqHelper;
import org.omg.CORBA.ORB;

import CF.DataType;
import CF.PropertiesHelper;

public class StructSequenceProperty< T extends StructDef > extends Property<List<T>> {

    protected final List<T> defaultValue;
    protected List<T> value;
    protected Class< ? > structClass;
    protected Constructor< ? > structCtor = null;
    protected Object[] ctorArgs = null;

    protected StructSequenceProperty(String id, String name, List<T> value, String mode, String[] kinds) {
        super(id, name, mode, null, kinds);
        this.defaultValue = Collections.unmodifiableList(value);
        this.value = value;
    }

    public StructSequenceProperty(String id, String name, Class<T> structClass, List<T> value, String mode, String[] kinds) {
        this(id, name, value, mode, kinds);
        this.structClass = structClass;
    }

    /**
     * @deprecated
     */
    public StructSequenceProperty(String id, String name, T structDef, List<T> value, String mode, String[] kinds) {
        this(id, name, value, mode, kinds);
        this.structClass = structDef.getClass();

        Constructor< ? > ctor = null;
        Object[] args = null;
        try {
            // Look for a no-argument constructor for the struct class.
            ctor = this.structClass.getConstructor();
            args = null;
        } catch (NoSuchMethodException nsm) {
            try {
                // If the struct is an inner class, it probably has a one-argument
                // constructor that takes the parent class. Try to find the parent
                // pointer from the structDef instance so that it can be reused to
                // construct other struct values.
                final Field parentField = this.structClass.getDeclaredField("this$0");
                parentField.setAccessible(true);
                final Object parent = parentField.get(structDef);
                ctor = this.structClass.getConstructors()[0];
                args = new Object[] { parent };
            } catch (Exception ex) {
                // We have no way of creating struct values.
                throw new IllegalArgumentException("Cannot construct struct values of this type: " + this.structClass.getName());
            }
        }
        this.structCtor = ctor;
        this.ctorArgs = args;
    }

    @Override
    public List<T> getValue() {
        return this.value;
    }

    @Override
    public void setValue(List<T> value) {
        this.value = value;
    }

    public Any toAny() {
        final Any retVal = ORB.init().create_any();

        final List<Any> structVals = new ArrayList<Any>();
        for (T item : this.value) {
            final List<DataType> props = new ArrayList<DataType>();
            for (IProperty prop : item.getElements()) {
                props.add(new DataType(prop.getId(), prop.toAny()));
            }
            Any itemAny = ORB.init().create_any();
            PropertiesHelper.insert(itemAny, props.toArray(new DataType[props.size()]));
            structVals.add(itemAny);
        }

        AnySeqHelper.insert(retVal, structVals.toArray(new Any[structVals.size()]));
        return retVal;
    }

    @Override
    public String toString() {
        return this.id + "/" + this.name;
    }

    public void fromAny(Any any) {
        try {
            List<T> structVals = new ArrayList<T>();

            for (final Any item : AnySeqHelper.extract(any)) {
                T itemValue = this.newStructInstance();
                DataType[] struct = PropertiesHelper.extract(item);
                if (struct != null) {
                    for (final DataType prop : struct) {
                        IProperty structProp = itemValue.getElement(prop.id);
                        if (structProp != null) {
                            structProp.fromAny(prop.value);
                        }
                    }
                }
                structVals.add(itemValue);
            }
            this.setValue(structVals);
        } catch (ClassCastException ex) {
            ex.printStackTrace();
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }

    public void fromString(String str) {
        throw new IllegalArgumentException("Only simple properties can be initialized with strings");
    }

    private T newStructInstance() {
        try {
            if (this.structCtor == null) {
                return (T) this.structClass.getConstructor().newInstance();
            } else {
                return (T) this.structCtor.newInstance(ctorArgs);
            }
        } catch (Exception ex) {
            throw new IllegalArgumentException("Unable to construct new struct value: " + ex.getMessage(), ex);
        }
    }

    private T[] newStructArray(int size) {
        return (T[]) Array.newInstance(this.structClass, size);
    }
}
