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
import java.util.List;
import java.util.ArrayList;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.TCKind;
import org.omg.CORBA.TypeCode;

@Deprecated
public class SimpleSequenceProperty< T extends Object > extends LegacyProperty< List<T> > {

    final private List<T> defaultValue;
    final private String type;
    final private TypeCode corbaType;

    public SimpleSequenceProperty(final String   id, 
                                  final String   name, 
                                  final String   type, 
                                  final List<T>  value, 
                                  final String   mode, 
                                  final String   action,
                                  final String[] kinds) {
        super(id, name, value, mode, action, kinds);
        this.type = type;
        this.defaultValue = value;
        this.corbaType = AnyUtils.convertToTypeCode(this.type);
    }

    public Any toAny() {
        Any retval;
        if (this.value == null) {
            retval = ORB.init().create_any();
            return retval;
        }
        Object[] dataholder = ((List<T>)this.value).toArray(new Object[0]);
        retval = AnyUtils.toAnySequence(dataholder, this.corbaType);
        return retval;
    }

    protected List<T> fromAny_(final Any any) {
        if(any.type().kind().value() != TCKind._tk_null){
            try {
                // Suppress unchecked cast warning; due to type erasure we do
                // not know the actual type of T, so this is an unsafe
                // operation, which is one of the reasons for deprecating this
                // class.
                @SuppressWarnings("unchecked") T[] array = (T[])AnyUtils.convertAny(any);
                return new ArrayList<T>(Arrays.asList(array));
            } catch (final ClassCastException ex) {
                throw new IllegalArgumentException("Incorrect any type recevied");
            }
        } 
        return new ArrayList<T>();
    }

    @Override
    public String toString() {
        return this.id + "/" + this.name + " = " + this.value;
    }

    public void fromString(final String str) {
        throw new IllegalArgumentException("Only simple properties can be initialized with strings");
    }

    public String getType() {
        return this.type;
    }

}
