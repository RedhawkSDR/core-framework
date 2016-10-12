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

public class StructProperty<T extends StructDef> extends Property<T> {
    
    final protected Class<T> structDef;

    public StructProperty(String id, String name, Class<T> structDef, T value, Mode mode, Kind[] kinds) {
        super(id, name, value, mode, null, kinds);
        this.structDef = structDef;
    }

    @Deprecated
    public StructProperty(String id, String name, T structDef, T structDef_tmp, String mode, String[] kinds) {
        super(id, name, structDef, Mode.get(mode), null, Kind.get(kinds));
        @SuppressWarnings("unchecked") Class<T> clazz = (Class<T>)structDef.getClass();
        this.structDef = clazz;
    }
    
    public Any toAny() {
        if (this.value == null) {
            return ORB.init().create_any();
        } else {
            return this.value.toAny();
        }
    }
    
    @Override
    public String toString() {
        return this.id + "/" + this.name +  " = " + this.value;
    }
    
    protected T fromAny_(Any any) {
        T tmp;
        try {
            tmp = this.structDef.newInstance();
        } catch (Exception ex) {
            throw new IllegalArgumentException("Unable to construct new struct value: " + ex.getMessage(), ex);
        }
        
        tmp.fromAny(any); 
        return tmp;
    }
    
    public void fromString(String str) {
        throw new IllegalArgumentException("Only simple properties can be initialized with strings");
    }
}
