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

public enum Kind {
    ALLOCATION("allocation"),
    PROPERTY("property"),
    CONFIGURE("configure"),
    TEST("test"),
    EXECPARAM("execparam"),
    FACTORYPARAM("factoryparam"),
    EVENT("event"),
    MESSAGE("message");
            
    public String toString() {
        return this.name;
    }

    public static Kind get(String name) {
        for (Kind kind : Kind.values()) {
            if (kind.name.equals(name)) {
                return kind;
            }
        }
        return null;
    }

    public static Kind[] get(String[] kinds) {
        Kind[] retval = new Kind[kinds.length];
        for (int ii = 0; ii < kinds.length; ++ii) {
            retval[ii] = Kind.get(kinds[ii]);
        }
        return retval;
    }

    private Kind (String name) {
        this.name = name;
    }
    private final String name;
}
