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

import org.omg.CORBA.Any;
import org.omg.CORBA.TCKind;

public class SimpleProperty<T extends Object> extends Property<T> {
    protected T value;
    
    final private T defaultValue;
    final private String type;
    final private TCKind corbaKind;
    
    public SimpleProperty(String id, String name, String type, T value, String mode,
            String action, String[] kinds) {
        super(id, name, mode, action, kinds);
        this.defaultValue = value;
        this.value = value;
        this.type = type;
        corbaKind = convertToTCKind(type);
    }
    
    @Override
    public T getValue() {
        // TODO Auto-generated method stub
        return value;
    }
    @Override
    public void setValue(T value) {
        this.value = value;
    }
    
    
    public Any toAny() {
        return AnyUtils.toAny(value, corbaKind);
    }
    
    
    public void fromAny(Any any) {
        try {
            this.setValue((T)AnyUtils.convertAny(any));
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }
    
    @Override
    public String toString() {
        return this.id + "/" + this.name + " = " + this.value;
    }
    
    
    public void fromString(String str) {
        value = (T)convertString(str, type);
    }

    public String getType() {
        return this.type;
    }

}
