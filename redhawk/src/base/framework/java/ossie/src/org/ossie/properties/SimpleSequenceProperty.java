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
import java.util.Collections;
import java.util.List;
import java.util.ArrayList;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.TCKind;
import org.omg.CORBA.TypeCodePackage.BadKind;

public class SimpleSequenceProperty< T extends Object > extends Property< List<T> > {

    protected List<T> value;

    final private List<T> defaultValue;
    final private String type;
    private TCKind corbaKind;

    public SimpleSequenceProperty(final String id, final String name, final String type, final List<T> value, final String mode, final String action,
            final String[] kinds) {
        super(id, name, mode, action, kinds);
        this.defaultValue = value;
        this.value = value;
        this.type = type;
        
        try {
            if (type.equals("string")) {
                this.corbaKind = CF.StringSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("float")) {
                this.corbaKind = PortTypes.FloatSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("double")) {
                this.corbaKind = PortTypes.DoubleSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("char")) {
                this.corbaKind = PortTypes.CharSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("octet")) {
                this.corbaKind = CF.OctetSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("short")) {
                this.corbaKind = PortTypes.ShortSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("ushort")) {
                this.corbaKind = PortTypes.UshortSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("long")) {
                this.corbaKind = PortTypes.LongSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("ulong")) {
                this.corbaKind = PortTypes.UlongSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("longlong")) {
                this.corbaKind = PortTypes.LongLongSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("ulonglong")) {
                this.corbaKind = PortTypes.UlongLongSequenceHelper.type().content_type().content_type().kind();
            } else if (type.equals("boolean")) {
                this.corbaKind = PortTypes.BooleanSequenceHelper.type().content_type().content_type().kind();
            } else {
                this.corbaKind = convertToTCKind(type);
            }
        } catch (org.omg.CORBA.TypeCodePackage.BadKind ex) {
            this.corbaKind = org.omg.CORBA.TCKind.tk_null;
        }
    }

    @Override
    public List<T> getValue() {
        return this.value;
    }

    @Override
    public void setValue(final List<T> value) {
        this.value = value;
    }

    public Any toAny() {
        Any retval;
        if (this.value == null) {
            retval = ORB.init().create_any();
            return retval;
        }
        Object[] dataholder = ((List<T>)this.value).toArray(new Object[0]);
        retval = AnyUtils.toAnySequence(dataholder, this.corbaKind);
        return retval;
    }

    public void fromAny(final Any any) {
        if(any.type().kind().value() != TCKind._tk_null){
            try {
                this.setValue(new ArrayList<T>(Arrays.asList((T[])AnyUtils.convertAny(any))));
            } catch (final ClassCastException ex) {
                throw new IllegalArgumentException("Incorrect any type recevied");
            }
        } 
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
