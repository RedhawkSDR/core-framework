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
import java.util.List;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;

abstract class AbstractSequenceProperty<T> extends Property<List<T>> {

    private final String type;

    protected AbstractSequenceProperty(String id, String name, String type, List<T> value,
                                       Mode mode,  Action action, Kind[] kinds) {
        super(id, name, value, mode, action, kinds);
        this.type = type;
    }

    @Override
    public void fromString(String str) {
        throw new IllegalArgumentException("Only simple properties can be initialized with strings");
    }

    @Override
    protected List<T> fromAny_(Any any) {
        if (AnyUtils.isNull(any)) {
            return new ArrayList<T>();
        } else {
            return extract(any);
        }
    }

    @Override
    public Any toAny() {
        Any any = ORB.init().create_any();
        if (this.value != null) {
            insert(any, this.value);
        }
        return any;
    }

    public String getType() {
        return this.type;
    }

    protected abstract List<T> extract(Any any);
    protected abstract void insert(Any any, List<T> value);
}
