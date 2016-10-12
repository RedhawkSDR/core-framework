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
import org.omg.CORBA.BooleanSeqHelper;

public class BooleanSequenceProperty extends AbstractSequenceProperty<Boolean> {
    public BooleanSequenceProperty(String id, String name, List<Boolean> value,
                                   Mode mode, Action action, Kind[] kinds) {
        super(id, name, "boolean", value, mode, action, kinds);
    }

    public static List<Boolean> asList(boolean... array) {
        return PrimitiveArrayUtils.convertToBooleanList(array);
    }

    protected List<Boolean> extract(Any any) {
        boolean[] array = BooleanSeqHelper.extract(any);
        List<Boolean> list = new ArrayList<Boolean>(array.length);
        for (Boolean item : array) {
            list.add(item);
        }
        return list;
    }

    protected void insert(Any any, List<Boolean> value) {
        boolean[] array = PrimitiveArrayUtils.convertToBooleanArray(value);
        BooleanSeqHelper.insert(any, array);
    }
}
