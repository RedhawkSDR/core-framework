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
import CF.UTCTime;
import CF.UTCTimeHelper;
import org.ossie.redhawk.time.utils;

public class UTCTimeProperty extends AbstractSimpleProperty<UTCTime> {
    public UTCTimeProperty(String id, String name, UTCTime value, Mode mode,
                          Action action, Kind[] kinds) {
        super(id, name, "utctime", value, mode, action, kinds);
    }

    public UTCTimeProperty(String id, String name, UTCTime value, Mode mode,
                          Action action, Kind[] kinds, boolean optional) {
        super(id, name, "utctime", value, mode, action, kinds, optional);
    }

    public UTCTimeProperty(String id, String name, String value, Mode mode,
                          Action action, Kind[] kinds) {
        super(id, name, "utctime", utils.convert(value), mode, action, kinds);
    }

    public UTCTimeProperty(String id, String name, String value, Mode mode,
                          Action action, Kind[] kinds, boolean optional) {
        super(id, name, "utctime", utils.convert(value), mode, action, kinds, optional);
    }

    protected UTCTime extract(Any any) {
        try {
            return (UTCTime)AnyUtils.convertAny(any);
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }

    protected void insert(Any any, UTCTime value) {
        UTCTimeHelper.insert(any, value);
    }

    protected UTCTime parseString(String time) {
        return utils.convert(time);
    }
}
