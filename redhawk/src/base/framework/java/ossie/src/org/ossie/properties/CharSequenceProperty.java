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
import java.util.ArrayList;

import org.omg.CORBA.Any;
import org.omg.CORBA.CharSeqHelper;

public class CharSequenceProperty extends AbstractSequenceProperty<Character> {
    public CharSequenceProperty(String id, String name, List<Character> value,
                                Mode mode, Action action, Kind[] kinds) {
        super(id, name, "char", value, mode, action, kinds);
    }

    public CharSequenceProperty(String id, String name, List<Character> value,
                                Mode mode, Action action, Kind[] kinds, boolean optional) {
        super(id, name, "char", value, mode, action, kinds, optional);
    }

    public static List<Character> asList(char... array) {
        return PrimitiveArrayUtils.convertToCharList(array);
    }

    protected List<Character> extract(Any any) {
        char[] array = CharSeqHelper.extract(any);
        List<Character> list = new ArrayList<Character>(array.length);
        for (Character item : array) {
            list.add(item);
        }
        return list;
    }

    protected void insert(Any any, List<Character> value) {
        char[] array = PrimitiveArrayUtils.convertToCharArray(value);
        CharSeqHelper.insert(any, array);
    }
}
