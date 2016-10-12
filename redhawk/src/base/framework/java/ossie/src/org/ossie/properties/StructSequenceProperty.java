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
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.omg.CORBA.Any;
import org.omg.CORBA.AnySeqHelper;
import org.omg.CORBA.ORB;

public class StructSequenceProperty<T extends StructDef> extends AbstractSequenceProperty<T> {

    protected Class<T> structClass;

    public StructSequenceProperty(String id, String name, Class<T> structClass, List<T> value, Mode mode, Kind[] kinds) {
        super(id, name, "struct", value, mode, null, kinds);
        this.structClass = structClass;
    }

    @Deprecated
    public StructSequenceProperty(String id, String name, Class<T> structClass, List<T> value, String mode, String[] kinds) {
        this(id, name, structClass, value, Mode.get(mode), Kind.get(kinds));
    }

    public static <E extends StructDef> List<E> asList(E... array) {
        return new ArrayList<E>(Arrays.asList(array));
    }

    protected void insert(Any any, List<T> values) {
        final Any[] array = new Any[values.size()];
        for (int ii = 0; ii < values.size(); ++ii) {
            array[ii] = values.get(ii).toAny();
        }

        AnySeqHelper.insert(any, array);
    }

    @Override
    public String toString() {
        return this.id + "/" + this.name;
    }

    protected List<T> extract(Any any) {
        List<T> structVals = new ArrayList<T>();

        for (final Any item : AnySeqHelper.extract(any)) {
            T itemValue;
            try {
                itemValue = this.structClass.newInstance();
            } catch (Exception ex) {
                throw new IllegalArgumentException("Unable to construct new struct value: " + ex.getMessage(), ex);
            }

            itemValue.fromAny(item);
            structVals.add(itemValue);
        }
        return structVals;
    }
}
