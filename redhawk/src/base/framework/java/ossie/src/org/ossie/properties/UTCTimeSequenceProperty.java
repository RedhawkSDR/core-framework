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
import CF.UTCTime;
import CF.UTCTimeSequenceHelper;
import org.ossie.redhawk.time.utils;

public class UTCTimeSequenceProperty extends AbstractSequenceProperty<CF.UTCTime> {
    public UTCTimeSequenceProperty(String id, String name, List<CF.UTCTime> value, Mode mode,
                                  Action action, Kind[] kinds) {
        super(id, name, "utctime", value, mode, action, kinds);
    }

    public UTCTimeSequenceProperty(String id, String name, List<CF.UTCTime> value, Mode mode,
                                  Action action, Kind[] kinds, boolean optional) {
        super(id, name, "utctime", value, mode, action, kinds, optional);
    }

    public static List<CF.UTCTime> asList() {
        return new ArrayList<CF.UTCTime>();
    }

    public static List<CF.UTCTime> asList(CF.UTCTime... array) {
        return new ArrayList<CF.UTCTime>(Arrays.asList(array));
    }

    public static List<CF.UTCTime> asList(String... array) {
        List<CF.UTCTime> list = new ArrayList<CF.UTCTime>(0);
        for (String item : array) {
            list.add(utils.convert(item));
        }
        return list;
    }

    protected List<CF.UTCTime> extract(Any any) {
        CF.UTCTime[] array = CF.UTCTimeSequenceHelper.extract(any);
        List<CF.UTCTime> list = new ArrayList<CF.UTCTime>(array.length);
        for (CF.UTCTime item : array) {
            list.add(item);
        }
        return list;
    }

    protected void insert(Any any, List<CF.UTCTime> value) {
        CF.UTCTime[] array = value.toArray(new CF.UTCTime[value.size()]);
        CF.UTCTimeSequenceHelper.insert(any, array);
    }
}
