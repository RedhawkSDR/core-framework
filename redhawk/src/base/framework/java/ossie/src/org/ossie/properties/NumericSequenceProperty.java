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

import java.util.List;

import org.omg.CORBA.Any;
import org.omg.CORBA.TCKind;
import org.omg.CORBA.ORB;

abstract class NumericSequenceProperty<T extends Number> extends AbstractSequenceProperty<T> {
    public NumericSequenceProperty(String id, String name, String type, List<T> value, Mode mode,
                                   Action action, Kind[] kinds) {
        super(id, name, type, value, mode, action, kinds);
    }

    protected List<T> extract(Any any) {
        try {
            return fromNumberArray((Number[])AnyUtils.convertAny(any));
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }
    protected abstract List<T> fromNumberArray(Number[] array);
}
