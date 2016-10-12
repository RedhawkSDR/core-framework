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

public class BooleanProperty extends AbstractSimpleProperty<Boolean> {
    public BooleanProperty(String id, String name, Boolean value, Mode mode,
                           Action action, Kind[] kinds) {
        super(id, name, "boolean", value, mode, action, kinds);
    }

    protected Boolean extract(org.omg.CORBA.Any any) {
        try {
            return (Boolean)AnyUtils.convertAny(any);
        } catch (ClassCastException ex) {
            throw new IllegalArgumentException("Incorrect any type recevied");
        }
    }

    protected void insert(org.omg.CORBA.Any any, Boolean value) {
        any.insert_boolean(value);
     }

    protected Boolean parseString(String str) {
        return Boolean.valueOf(str);
    }
}
