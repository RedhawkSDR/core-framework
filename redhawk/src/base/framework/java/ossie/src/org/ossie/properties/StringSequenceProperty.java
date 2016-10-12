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

import java.util.Arrays;
import java.util.ArrayList;
import java.util.List;

import org.omg.CORBA.Any;
import org.omg.CORBA.StringSeqHelper;

public class StringSequenceProperty extends AbstractSequenceProperty<String> {
    public StringSequenceProperty(String id, String name, List<String> value, Mode mode,
                                  Action action, Kind[] kinds) {
        super(id, name, "string", value, mode, action, kinds);
    }

    public StringSequenceProperty(String id, String name, List<String> value, Mode mode,
                                  Action action, Kind[] kinds, boolean optional) {
        super(id, name, "string", value, mode, action, kinds, optional);
    }

    public static List<String> asList(String... array) {
        return new ArrayList<String>(Arrays.asList(array));
    }

    protected List<String> extract(Any any) {
        String[] array = StringSeqHelper.extract(any);
        List<String> list = new ArrayList<String>(array.length);
        for (String item : array) {
            list.add(item);
        }
        return list;
    }

    protected void insert(Any any, List<String> value) {
        String[] array = value.toArray(new String[value.size()]);
        StringSeqHelper.insert(any, array);
    }
}
