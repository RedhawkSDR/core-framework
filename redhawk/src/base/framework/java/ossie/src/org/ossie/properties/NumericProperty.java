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
import org.omg.CORBA.ORB;

abstract class NumericProperty<T extends Number> extends AbstractSimpleProperty<T> {

    public NumericProperty(String id, String name, String type, T value, Mode mode,
                           Action action, Kind[] kinds) {
        super(id, name, type, value, mode, action, kinds);
    }

    public NumericProperty(String id, String name, String type, T value, Mode mode,
                           Action action, Kind[] kinds, boolean optional) {
        super(id, name, type, value, mode, action, kinds, optional);
    }

    public void setValue(Number value) {
        this.value = fromNumber(value);
        for (PropertyListener<Object> listener : voidListeners) {
            listener.valueChanged(value, this.value);
        }
    }

    protected T extract(Any any) {
        try {
            return fromNumber((Number)AnyUtils.convertAny(any));
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }
    protected abstract T fromNumber(Number value);

    public boolean allocate(T capacity) {
        if (compare(this.value, capacity) < 0) {
            return false;
        }
        this.value = subtract(this.value, capacity);
        return true;
    }

    public void deallocate(T capacity) {
        this.value = add(this.value, capacity);
    }

    protected abstract int compare(T lhs, T rhs);
    protected abstract T subtract(T lhs, T rhs);
    protected abstract T add(T lhs, T rhs);
}
