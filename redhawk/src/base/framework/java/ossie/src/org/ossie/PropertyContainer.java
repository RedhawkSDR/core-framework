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

/**
 *
 * Identification: $Revision$
 */
package org.ossie;

import java.util.Vector;

import org.omg.CORBA.Any;
import org.omg.CORBA.ORB;
import org.omg.CORBA.TCKind;

import CF.DataType;

public class PropertyContainer {
    private String id;
    private String name;
    private short type;
    private String mode;
    private String units;
    private String action;
    private Vector<String> kinds;
    private String stringVal;
    private DataType baseProperty;

    public static < E extends Comparable<E>> int compareProps(final E tmp1, final E tmp2) {
        return tmp1.compareTo(tmp2);
    }

    /**
     * This creates a property container given the property's parameters.
     * 
     * @param id id of the property
     * @param name name of the property
     * @param type SCA type of the property
     * @param mode the SCA mode of the property
     * @param initialValue the initial value for the property
     * @param units the SCA units for the property
     * @param action the SCA action type for the property
     * @param kinds the SCA kinds for the property
     */
    public PropertyContainer(final String id, final String name, final short type, final String mode, final Any initialValue, final String units, // SUPPRESS CHECKSTYLE Parameters
            final String action, final Vector<String> kinds) {
        this.id = id;
        this.name = name;
        this.type = type;
        this.mode = mode;
        this.units = units;
        this.action = action;
        this.kinds = kinds;
        this.baseProperty = new DataType();
        this.baseProperty.id = id;
        setValue(initialValue);
    }

    /**
     * The default constructor. Initializes everything to empty and a default,
     * tk_null any.
     */
    public PropertyContainer() {
        this.id = "";
        this.name = "";
        this.type = TCKind._tk_null;
        this.mode = "";
        this.units = "";
        this.action = "";
        this.kinds = new Vector<String>();
        this.baseProperty = new DataType();
        this.baseProperty.id = "";
        this.baseProperty.value = ORB.init().create_any();
    }

    /**
     * Sets the ID of the property.
     * 
     * @param id the new property ID
     */
    public void setId(final String id) {
        this.id = id;
        this.baseProperty.id = id;
    }

    /**
     * Sets the name of the property
     * 
     * @param name the new property name
     */
    public void setName(final String name) {
        this.name = name;
    }

    /**
     * Sets the CORBA type of the property
     * 
     * @param type the new CORBA type of the property
     */
    public void setType(final short type) {
        this.type = type;
    }

    /**
     * This converts a string type value to a valid CORBA type for the property
     * 
     * @param type The new SCA string type of the property
     */
    public void setType(final String type) {
        if ("short".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_short;
        } else if ("ushort".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_ushort;
        } else if ("long".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_long;
        } else if ("ulong".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_ulong;
        } else if ("longlong".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_longlong;
        } else if ("ulonglong".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_ulonglong;
        } else if ("float".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_float;
        } else if ("double".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_double;
        } else if ("boolean".equalsIgnoreCase(type) || "bool".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_boolean;
        } else if ("char".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_char;
        } else if ("wchar".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_wchar;
        } else if ("octet".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_octet;
        } else if ("objref".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_objref;
        } else if ("string".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_string;
        } else if ("wstring".equalsIgnoreCase(type)) {
            this.type = TCKind._tk_wstring;
        }
    }

    /**
     * This sets the SCA mode of the property
     * 
     * @param mode the new SCA Mode
     */
    public void setMode(final String mode) {
        this.mode = mode;
    }

    /**
     * This sets the SCA Units for the property
     * 
     * @param units the new SCA Units
     */
    public void setUnits(final String units) {
        this.units = units;
    }

    /**
     * This sets the SCA action for the property
     * 
     * @param action the new SCA Action
     */
    public void setAction(final String action) {
        this.action = action;
    }

    /**
     * This sets the SCA kinds for the property
     * 
     * @param kinds the new SCA kinds
     */
    public void setKinds(final Vector<String> kinds) {
        this.kinds = kinds;
    }

    /**
     * This sets the base property for the container
     * 
     * @param baseProperty the new base property
     */
    public void setBaseProperty(final DataType baseProperty) {
        this.baseProperty = baseProperty;
    }

    /**
     * This returns the ID of the property
     * 
     * @return the property's ID
     */
    public String getId() {
        return this.id;
    }

    /**
     * This returns the name of the property.
     * 
     * @return the property's name
     */
    public String getName() {
        return this.name;
    }

    /**
     * This returns the CORBA type of the property
     * 
     * @return the property's CORBA type
     */
    public short getType() {
        return this.type;
    }

    /**
     * This returns the SCA mode of the property
     * 
     * @return the property's SCA mode
     */
    public String getMode() {
        return this.mode;
    }

    /**
     * This returns the SCA units for the property
     * 
     * @return the property's SCA units
     */
    public String getUnits() {
        return this.units;
    }

    /**
     * This returns the SCA action for the property
     * 
     * @return the property's SCA Action
     */
    public String getAction() {
        return this.action;
    }

    /**
     * This returns the SCA Kinds for this property
     * 
     * @return the property's SCA Kinds values
     */
    public Vector<String> getKinds() {
        return this.kinds;
    }

    /**
     * This returns the underlying CF::DataType for the property
     * 
     * @return the base property object for this property
     */
    public DataType getBaseProperty() {
        return this.baseProperty;
    }

    /**
     * This returns the value of the property as a char if supported(char,
     * wchar), otherwise Character.MIN_VALUE
     * 
     * @return the char value of this property
     */
    public char getCharValue() {
        char tmp = Character.MIN_VALUE;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_char)) {
            tmp = this.baseProperty.value.extract_char();
        } else if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_wchar)) {
            tmp = this.baseProperty.value.extract_wchar();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as a short if supported(short,
     * ushort), otherwise Short.MIN_VALUE
     * 
     * @return the char value of this property
     */
    public short getShortValue() {
        short tmp = Short.MIN_VALUE;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_short)) {
            tmp = this.baseProperty.value.extract_short();
        } else if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_ushort)) {
            tmp = this.baseProperty.value.extract_ushort();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as an int if supported(long,
     * ulong), otherwise Integer.MIN_VALUE
     * 
     * @return the int value of this property
     */
    public int getIntegerValue() {
        int tmp = Integer.MIN_VALUE;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_long)) {
            tmp = this.baseProperty.value.extract_long();
        } else if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_ulong)) {
            tmp = this.baseProperty.value.extract_ulong();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as a long if supported(longlong or
     * ulonglong), otherwise Long.MIN_VALUE
     * 
     * @return the long value of this property
     */
    public long getLongValue() {
        long tmp = Long.MIN_VALUE;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_longlong)) {
            tmp = this.baseProperty.value.extract_longlong();
        } else if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_ulonglong)) {
            tmp = this.baseProperty.value.extract_ulonglong();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as a float if supported, otherwise
     * Float.MIN_VALUE
     * 
     * @return the char value of this property
     */
    public float getFloatValue() {
        float tmp = Float.MIN_VALUE;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_float)) {
            tmp = this.baseProperty.value.extract_float();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as a double if supported,
     * otherwise Double.MIN_VALUE
     * 
     * @return the double value of this property
     */
    public double getDoubleValue() {
        double tmp = Double.MIN_VALUE;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_double)) {
            tmp = this.baseProperty.value.extract_double();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as a boolean if supported,
     * otherwise false
     * 
     * @return the boolean value of this property
     */
    public boolean getBooleanValue() {
        boolean tmp = false;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_boolean)) {
            tmp = this.baseProperty.value.extract_boolean();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as a byte if supported, otherwise
     * Byte.MIN_VALUE
     * 
     * @return the byte value of this property
     */
    public byte getOctetValue() {
        byte tmp = Byte.MIN_VALUE;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_octet)) {
            tmp = this.baseProperty.value.extract_octet();
        }
        return tmp;
    }

    /**
     * This returns the value of the property as a CORBA Object if supported,
     * otherwise null
     * 
     * @return the CORBA object value of this property
     */
    public org.omg.CORBA.Object getObjectValue() {
        org.omg.CORBA.Object tmp = null;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_objref)) {
            tmp = this.baseProperty.value.extract_Object();
        }
        return tmp;
    }

    /**
     * This returns the string value of the property if supported(string,
     * wstring), otherwise null
     * 
     * @return the string value of this property
     */
    public String getStringValue() {
        String tmp = null;
        if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_string)) {
            tmp = this.baseProperty.value.extract_string();
        } else if ((this.baseProperty.value != null) && (this.baseProperty.value.type().kind().value() == TCKind._tk_wstring)) {
            tmp = this.baseProperty.value.extract_wstring();
        }
        return tmp;
    }

    /**
     * This will set the saved value to the given CORBA Any value
     * 
     * @param val the new CORBA value for the property
     */
    public void setValue(final Any val) {
        if (val == null) {
            this.baseProperty.value = ORB.init().create_any();
            this.stringVal = null;
        } else {
            this.baseProperty.value = val;
        }

        //if (!this.isBasePropertyValueValid()) {
        //this.baseProperty.value.type(ORB.init().get_primitive_tc(TCKind.from_int(this.type)));
        //this.setValue(this.getDefaultValue(), this.type);
        //}

        if (this.isBasePropertyValueValid()) {
            switch (this.type) {
            case TCKind._tk_short:
                setValue(this.baseProperty.value.extract_short());
                break;
            case TCKind._tk_long:
                setValue(this.baseProperty.value.extract_long());
                break;
            case TCKind._tk_ushort:
                setValue(this.baseProperty.value.extract_ushort());
                break;
            case TCKind._tk_ulong:
                setValue(this.baseProperty.value.extract_ulong());
                break;
            case TCKind._tk_float:
                setValue(this.baseProperty.value.extract_float());
                break;
            case TCKind._tk_double:
                setValue(this.baseProperty.value.extract_double());
                break;
            case TCKind._tk_boolean:
                setValue(this.baseProperty.value.extract_boolean());
                break;
            case TCKind._tk_char:
                setValue(this.baseProperty.value.extract_char());
                break;
            case TCKind._tk_octet:
                setValue(this.baseProperty.value.extract_octet());
                break;
            case TCKind._tk_objref:
                setValue(this.baseProperty.value.extract_Object());
                break;
            case TCKind._tk_string:
                setValue(this.baseProperty.value.extract_string());
                break;
            case TCKind._tk_longlong:
                setValue(this.baseProperty.value.extract_longlong());
                break;
            case TCKind._tk_ulonglong:
                setValue(this.baseProperty.value.extract_ulonglong());
                break;
            case TCKind._tk_wchar:
                setValue(this.baseProperty.value.extract_wchar());
                break;
            case TCKind._tk_wstring:
                setValue(this.baseProperty.value.extract_wstring());
                break;
            default:
                this.stringVal = null;
                break;
            }
        }
    }

    /**
     * This will set the value of the property to the given string, based on the
     * given type
     * 
     * @param val the new value of the property
     * @param type the type of the property
     */
    public void setValue(final String val, final short type) {
        if (val == null) {
            this.baseProperty.value = ORB.init().create_any();
            this.stringVal = null;
            return;
        }

        switch (type) {
        case TCKind._tk_short:
            setValue(Short.parseShort(val));
            break;
        case TCKind._tk_long:
            setValue(Integer.parseInt(val));
            break;
        case TCKind._tk_ushort:
            this.type = TCKind._tk_ushort;
            setValue(Short.parseShort(val));
            break;
        case TCKind._tk_ulong:
            this.type = TCKind._tk_ulong;
            setValue(Integer.parseInt(val));
            break;
        case TCKind._tk_float:
            setValue(Float.parseFloat(val));
            break;
        case TCKind._tk_double:
            setValue(Double.parseDouble(val));
            break;
        case TCKind._tk_boolean:
            setValue(Boolean.parseBoolean(val));
            break;
        case TCKind._tk_char:
            setValue((val.length() > 0) ? val.charAt(0) : '\0'); // SUPPRESS CHECKSTYLE AvoidInline
            break;
        case TCKind._tk_octet:
            setValue(Byte.parseByte(val));
            break;
        case TCKind._tk_objref:
            setValue(ORB.init().string_to_object(val));
            break;
        case TCKind._tk_string:
            setValue(val);
            break;
        case TCKind._tk_longlong:
            setValue(Long.parseLong(val));
            break;
        case TCKind._tk_ulonglong:
            this.type = TCKind._tk_ulonglong;
            setValue(Long.parseLong(val));
            break;
        case TCKind._tk_wchar:
            this.type = TCKind._tk_wchar;
            setValue((val.length() > 0) ? val.charAt(0) : '\0'); // SUPPRESS CHECKSTYLE AvoidInline
            break;
        case TCKind._tk_wstring:
            this.type = TCKind._tk_wstring;
            setValue(val);
            break;
        default:
            break;
        }
    }

    /**
     * This converts the property to a short with the given value. It also
     * stores the string version of the property's value NOTE: It first checks
     * the base property for a type, if it's ushort, it sets the value to a
     * ushort, otherwise it overrides to a short.
     * 
     * @param val the new value
     */
    public void setValue(final short val) {
        if (this.isType(TCKind._tk_ushort)) {
            this.baseProperty.value.insert_ushort(val);
            this.type = TCKind._tk_ushort;
        } else {
            this.baseProperty.value.insert_long(val);
            this.type = TCKind._tk_short;
        }
        this.stringVal = Short.toString(val);
    }

    /**
     * This converts the property to a long with the given value. It also stores
     * the string version of the property's value NOTE: It first checks the base
     * property for a type, if it's ulong, it sets the value to a ulong,
     * otherwise it overrides to a long.
     * 
     * @param val the new value
     */
    public void setValue(final int val) {
        if (this.isType(TCKind._tk_ulong)) {
            this.baseProperty.value.insert_ulong(val);
            this.type = TCKind._tk_ulong;
        } else {
            this.baseProperty.value.insert_long(val);
            this.type = TCKind._tk_long;
        }
        this.stringVal = Integer.toString(val);
    }

    /**
     * This converts the property to a long with the given value. It also stores
     * the string version of the property's value NOTE: It first checks the base
     * property for a type, if it's ulonglong, it sets the value to a ulonglong,
     * otherwise it overrides to a longlong.
     * 
     * @param val the new value
     */
    public void setValue(final long val) {
        if (this.isType(TCKind._tk_ulonglong)) {
            this.baseProperty.value.insert_ulonglong(val);
            this.type = TCKind._tk_ulonglong;
        } else {
            this.baseProperty.value.insert_longlong(val);
            this.type = TCKind._tk_longlong;
        }
        this.stringVal = Long.toString(val);
    }

    /**
     * This converts the property to a float with the given value. It also
     * stores the string version of the property's value
     * 
     * @param val the new value
     */
    public void setValue(final float val) {
        this.baseProperty.value.insert_float(val);
        this.type = TCKind._tk_float;
        this.stringVal = Float.toString(val);
    }

    /**
     * This converts the property to a double with the given value. It also
     * stores the string version of the property's value
     * 
     * @param val the new value
     */
    public void setValue(final double val) {
        this.baseProperty.value.insert_double(val);
        this.type = TCKind._tk_double;
        this.stringVal = Double.toString(val);
    }

    /**
     * This converts the property to a boolean with the given value. It also
     * stores the string version of the property's value
     * 
     * @param val the new value
     */
    public void setValue(final boolean val) {
        this.baseProperty.value.insert_boolean(val);
        this.type = TCKind._tk_boolean;
        this.stringVal = Boolean.toString(val);
    }

    /**
     * This converts the property to a char with the given value. It also stores
     * the string version of the property's value NOTE: It first checks the base
     * property for a type, if it's wchar, it sets the value to a wchar,
     * otherwise it overrides to a char.
     * 
     * @param val the new value
     */
    public void setValue(final char val) {
        if (this.isType(TCKind._tk_wchar)) {
            this.baseProperty.value.insert_wchar(val);
            this.type = TCKind._tk_wchar;
        } else {
            this.baseProperty.value.insert_char(val);
            this.type = TCKind._tk_char;
        }
        this.stringVal = Character.toString(val);
    }

    /**
     * This converts the property to a byte with the given value. It also stores
     * the string version of the property's value
     * 
     * @param val the new value
     */
    public void setValue(final byte val) {
        this.baseProperty.value.insert_octet(val);
        this.type = TCKind._tk_octet;
        this.stringVal = Byte.toString(val);
    }

    /**
     * This converts the property to a CORBA Object with the given value. It
     * also stores the string version of the property's value
     * 
     * @param val the new value
     */
    public void setValue(final org.omg.CORBA.Object val) {
        this.baseProperty.value.insert_Object(val);
        this.type = TCKind._tk_objref;
        this.stringVal = val.toString();
    }

    /**
     * This converts the property to a string with the given value. It also
     * stores the string version of the property's value. NOTE: It first checks
     * the base property for a type, if it's wstring, it sets the value to a
     * wstring, otherwise it overrides to a string.
     * 
     * @param val the new value
     */
    public void setValue(final String val) {
        if (this.isType(TCKind._tk_wstring)) {
            this.baseProperty.value.insert_wstring(val);
            this.type = TCKind._tk_wstring;
        } else {
            this.baseProperty.value.insert_string(val);
            this.type = TCKind._tk_string;
        }
        this.stringVal = val;
    }

    public int compare(final Any a) {
        if (!this.isBasePropertyValueValid()) {
            return 1;
        }
        switch (this.type) {
        case TCKind._tk_short:
            return PropertyContainer.compareProps(a.extract_short(), this.baseProperty.value.extract_short());
        case TCKind._tk_long:
            return PropertyContainer.compareProps(a.extract_long(), this.baseProperty.value.extract_long());
        case TCKind._tk_ushort:
            return PropertyContainer.compareProps(a.extract_ushort(), this.baseProperty.value.extract_ushort());
        case TCKind._tk_ulong:
            return PropertyContainer.compareProps(a.extract_ulong(), this.baseProperty.value.extract_ulong());
        case TCKind._tk_float:
            return PropertyContainer.compareProps(a.extract_float(), this.baseProperty.value.extract_float());
        case TCKind._tk_double:
            return PropertyContainer.compareProps(a.extract_double(), this.baseProperty.value.extract_double());
        case TCKind._tk_boolean:
            return PropertyContainer.compareProps(a.extract_boolean(), this.baseProperty.value.extract_boolean());
        case TCKind._tk_char:
            return PropertyContainer.compareProps(a.extract_char(), this.baseProperty.value.extract_char());
        case TCKind._tk_octet:
            return PropertyContainer.compareProps(a.extract_octet(), this.baseProperty.value.extract_octet());
        case TCKind._tk_string:
            final String tmp1 = a.extract_string();
            if (tmp1 != null && this.stringVal != null) {
                return tmp1.compareTo(this.stringVal);
            } else {
                return 1;
            }
        case TCKind._tk_longlong:
            return PropertyContainer.compareProps(a.extract_longlong(), this.baseProperty.value.extract_longlong());
        case TCKind._tk_ulonglong:
            return PropertyContainer.compareProps(a.extract_ulonglong(), this.baseProperty.value.extract_ulonglong());
        case TCKind._tk_wchar:
            return PropertyContainer.compareProps(a.extract_wchar(), this.baseProperty.value.extract_wchar());
        case TCKind._tk_wstring:
            final String tmpw = a.extract_wstring();
            if (tmpw != null && this.stringVal != null) {
                return tmpw.compareTo(this.stringVal);
            } else {
                return 1;
            }
        default:
            return 1;
        }
    }

    public void increment(final Any a) {
        if (!this.isBasePropertyValueValid()) {
            throw new IllegalStateException("The value of the base property is not initialized");
        }
        switch (this.type) {
        case TCKind._tk_short:
            setValue((short) (this.baseProperty.value.extract_short() + a.extract_short()));
            break;
        case TCKind._tk_long:
            setValue(this.baseProperty.value.extract_long() + a.extract_long());
            break;
        case TCKind._tk_ushort:
            setValue((short) (this.baseProperty.value.extract_ushort() + a.extract_ushort()));
            break;
        case TCKind._tk_ulong:
            setValue(this.baseProperty.value.extract_ulong() + a.extract_ulong());
            break;
        case TCKind._tk_float:
            setValue(this.baseProperty.value.extract_float() + a.extract_float());
            break;
        case TCKind._tk_double:
            setValue(this.baseProperty.value.extract_double() + a.extract_double());
            break;
        case TCKind._tk_char:
            setValue(this.baseProperty.value.extract_char() + a.extract_char());
            break;
        case TCKind._tk_octet:
            setValue(this.baseProperty.value.extract_octet() + a.extract_octet());
            break;
        case TCKind._tk_longlong:
            setValue(this.baseProperty.value.extract_longlong() + a.extract_longlong());
            break;
        case TCKind._tk_ulonglong:
            setValue(this.baseProperty.value.extract_ulonglong() + a.extract_ulonglong());
            break;
        case TCKind._tk_wchar:
            setValue(this.baseProperty.value.extract_wchar() + a.extract_wchar());
            break;
        default:
            return;
        }
    }

    public void decrement(final Any a) {
        if (!this.isBasePropertyValueValid()) {
            throw new IllegalStateException("The value of the base property is not initialized");
        }
        switch (this.type) {
        case TCKind._tk_short:
            setValue((short) (this.baseProperty.value.extract_short() - a.extract_short()));
            break;
        case TCKind._tk_long:
            setValue(this.baseProperty.value.extract_long() - a.extract_long());
            break;
        case TCKind._tk_ushort:
            setValue((short) (this.baseProperty.value.extract_ushort() - a.extract_ushort()));
            break;
        case TCKind._tk_ulong:
            setValue(this.baseProperty.value.extract_ulong() - a.extract_ulong());
            break;
        case TCKind._tk_float:
            setValue(this.baseProperty.value.extract_float() - a.extract_float());
            break;
        case TCKind._tk_double:
            setValue(this.baseProperty.value.extract_double() - a.extract_double());
            break;
        case TCKind._tk_char:
            setValue(this.baseProperty.value.extract_char() - a.extract_char());
            break;
        case TCKind._tk_octet:
            setValue(this.baseProperty.value.extract_octet() - a.extract_octet());
            break;
        case TCKind._tk_longlong:
            setValue(this.baseProperty.value.extract_longlong() - a.extract_longlong());
            break;
        case TCKind._tk_ulonglong:
            setValue(this.baseProperty.value.extract_ulonglong() - a.extract_ulonglong());
            break;
        case TCKind._tk_wchar:
            setValue(this.baseProperty.value.extract_wchar() - a.extract_wchar());
            break;
        default:
            return;
        }
    }

    private boolean isType(final int newType) {
        return ((this.baseProperty.value.type().kind().value() == newType) || (this.type == newType));
    }

    private final boolean isBasePropertyValueValid() {
        return this.baseProperty.value.type().kind().value() != TCKind._tk_null;
    }

}
