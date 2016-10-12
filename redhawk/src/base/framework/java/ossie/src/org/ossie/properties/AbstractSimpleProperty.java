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

abstract class AbstractSimpleProperty<T> extends Property<T> {

    private final String type;

    protected AbstractSimpleProperty(String id,
                                     String name,
                                     String type,
                                     T value,
                                     Mode mode,
                                     Action action,
                                     Kind[] kinds) {
        super(id, name, value, mode, action, kinds);
        this.type = type;
    }

    protected AbstractSimpleProperty(String id,
                                     String name,
                                     String type,
                                     T value,
                                     Mode mode,
                                     Action action,
                                     Kind[] kinds,
                                     boolean optional) {
        super(id, name, value, mode, action, kinds, optional);
        this.type = type;
    }

    @Override
    public void fromString(String str) {
        this.value = parseString(str);
    }

    @Override
    protected T fromAny_(Any any) {
        T newValue = null;
        if (!AnyUtils.isNull(any)) {
            newValue = extract(any);
        }
        return newValue;
    }

    @Override
    public Any toAny() {
        Any any = ORB.init().create_any();
        if (this.value != null) {
            insert(any, this.value);
        }
        return any;
    }

    @Override
    public String toString() {
        return this.getId() + "/" + this.getName() + " = " + this.value;
    }

    public String getType() {
        return this.type;
    }

    protected abstract T parseString(String str);
    protected abstract T extract(Any any);
    protected abstract void insert(Any any, T value);
}
