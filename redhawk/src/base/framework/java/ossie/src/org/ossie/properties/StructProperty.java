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

import java.util.ArrayList;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

import CF.DataType;
import CF.PropertiesHelper;

public class StructProperty<T extends StructDef> extends Property<T> {
    
    protected T value;
    protected T tmp;
    final protected T structDef;
    
    public StructProperty(String id, String name, T structDef, T structDef_tmp, String mode, String[] kinds) {
        super(id, name, mode, null, kinds);
        this.structDef = structDef;
        this.value = structDef;
        this.tmp = structDef_tmp;
    }
    
    @Override
    public T getValue() {
        // TODO Auto-generated method stub
        return value;
    }
    @Override
    public void setValue(final T value) {
        //this.value = value;
        for (final IProperty prop : value.getElements()) {
            this.value.getElement(prop.getId()).fromAny(prop.toAny());
        }
    }
    
    
    public Any toAny() {
        final Any retVal = ORB.init().create_any();
        
        final ArrayList<DataType> props = new ArrayList<DataType>();
        for (IProperty prop : this.value.getElements()) {
            props.add(new DataType(prop.getId(), prop.toAny()));
        }
        
        PropertiesHelper.insert(retVal, props.toArray(new DataType[props.size()]));
        return retVal;
    }
    
    @Override
    public String toString() {
        return this.id + "/" + this.name +  " = " + this.value;
    }
    
    
    public void fromAny(Any any) {
        try {
            DataType[] struct  = (DataType[])AnyUtils.convertAny(any);
            
            for (final DataType prop : struct) {
                tmp.getElement(prop.id).fromAny(prop.value);
            }
            this.setValue(tmp);
        } catch (ClassCastException ex) {
            ex.printStackTrace();
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }
    
    
    public void fromString(String str) {
        throw new IllegalArgumentException("Only simple properties can be initialized with strings");
    }

}
