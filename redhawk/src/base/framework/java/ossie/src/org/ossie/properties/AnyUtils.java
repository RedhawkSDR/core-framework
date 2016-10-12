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
 * Identification: $Revision: 6405 $
 */
package org.ossie.properties;

import java.io.Serializable;
import java.math.BigDecimal;
import java.math.BigInteger;
import java.util.Arrays;
import java.util.List;

import org.apache.commons.lang.ArrayUtils;
import org.omg.CORBA.Any;
import org.omg.CORBA.AnySeqHelper;
import org.omg.CORBA.BAD_OPERATION;
import org.omg.CORBA.BooleanSeqHelper;
import org.omg.CORBA.CharSeqHelper;
import org.omg.CORBA.DoubleSeqHelper;
import org.omg.CORBA.FloatSeqHelper;
import org.omg.CORBA.LongLongSeqHelper;
import org.omg.CORBA.LongSeqHelper;
import org.omg.CORBA.ORB;
import org.omg.CORBA.OctetSeqHelper;
import org.omg.CORBA.ShortSeqHelper;
import org.omg.CORBA.StringSeqHelper;
import org.omg.CORBA.TCKind;
import org.omg.CORBA.TypeCode;
import org.omg.CORBA.ULongLongSeqHelper;
import org.omg.CORBA.ULongSeqHelper;
import org.omg.CORBA.UShortSeqHelper;
import org.omg.CORBA.WCharSeqHelper;
import org.omg.CORBA.WStringSeqHelper;
import org.omg.CORBA.TypeCodePackage.BadKind;

import CF.DataTypeHelper;
import CF.PropertiesHelper;

public final class AnyUtils {

    private AnyUtils() {
    }

    /**
     * Attempts to convert the string value to the appropriate Java type.
     * 
     * @param stringValue the string form of the value
     * @param type the string form of the TypeCode
     * @return A Java object of theString corresponding to the typecode
     */
    public static Object convertString(final String stringValue, final String type) {
        if (stringValue == null) {
            return null;
        }
        if (type.equals("string")) {
            return stringValue;
        } else if (type.equals("wstring")) {
            return stringValue;
        } else if (type.equals("boolean")) {
            if ("true".equalsIgnoreCase(stringValue) || "false".equalsIgnoreCase(stringValue)) {
                return Boolean.parseBoolean(stringValue);
            }
            throw new IllegalArgumentException(stringValue + " is not a valid boolean value");
        } else if (type.equals("char")) {
            if (stringValue.length() == 1) {
                return stringValue.charAt(0);
            }
            throw new IllegalArgumentException(stringValue + " is not a valid char value");
        } else if (type.equals("wchar")) {
            return stringValue.charAt(0);
        } else if (type.equals("double")) {
            return Double.parseDouble(stringValue);
        } else if (type.equals("float")) {
            return Float.parseFloat(stringValue);
        } else if (type.equals("short")) {
            return Short.decode(stringValue);
        } else if (type.equals("long")) {
            return Integer.decode(stringValue);
        } else if (type.equals("longlong")) {
            return Long.decode(stringValue);
        } else if (type.equals("ulong")) {
            final long MAX_UINT = 2L * Integer.MAX_VALUE + 1L;
            final Long retVal = Long.decode(stringValue);
            if (retVal < 0 || retVal > MAX_UINT) {
                throw new IllegalArgumentException("ulong value must be greater than '0' and less than " + MAX_UINT);
            }
            return retVal;
        } else if (type.equals("ushort")) {
            final int MAX_USHORT = 2 * Short.MAX_VALUE + 1;
            final Integer retVal = Integer.decode(stringValue);
            if (retVal < 0 || retVal > MAX_USHORT) {
                throw new IllegalArgumentException("ushort value must be greater than '0' and less than " + MAX_USHORT);
            }
            return retVal;
        } else if (type.equals("ulonglong")) {
            final BigInteger MAX_ULONG_LONG = BigInteger.valueOf(Long.MAX_VALUE).multiply(BigInteger.valueOf(2)).add(BigInteger.ONE);
            final BigInteger retVal = bigIntegerDecode(stringValue);
            if (retVal.compareTo(BigInteger.ZERO) < 0 || retVal.compareTo(MAX_ULONG_LONG) > 0) {
                throw new IllegalArgumentException("ulonglong value must be greater than '0' and less than " + MAX_ULONG_LONG.toString());
            }
            return retVal;
        } else if (type.equals("objref")) {
            List<String> objrefPrefix = Arrays.asList("IOR:", "corbaname:", "corbaloc:");
            for (String prefix : objrefPrefix) {
                if (stringValue.startsWith(prefix)) {
                    return stringValue;
                }
            }
            throw new IllegalArgumentException(stringValue + " is not a valid objref value");
        } else if (type.equals("octet")) {
            final short MIN_OCTET = 0;
            final short MAX_OCTET = 0xFF;
            short val = Short.valueOf(stringValue);
            if (val <= MAX_OCTET && val >= MIN_OCTET) {
                return Short.valueOf(val).byteValue();
            }
            throw new IllegalArgumentException(stringValue + " is not a valid octet value");
        } else {
            throw new IllegalArgumentException("Unknown CORBA Type: " + type);
        }
    }

    /**
     * @since 3.0
     */
    public static BigInteger bigIntegerDecode(final String nm) throws NumberFormatException {
        int radix = 10;
        int index = 0;
        boolean negative = false;
        BigInteger result;

        // Handle minus sign, if present
        if (nm.startsWith("-")) {
            negative = true;
            index++;
        }

        // Handle radix specifier, if present
        if (nm.startsWith("0x", index) || nm.startsWith("0X", index)) {
            index += 2;
            radix = 16;
        } else if (nm.startsWith("#", index)) {
            index++;
            radix = 16;
        } else if (nm.startsWith("0", index) && nm.length() > 1 + index) {
            index++;
            radix = 8;
        }

        if (nm.startsWith("-", index)) {
            throw new NumberFormatException("Negative sign in wrong position");
        }
        try {
            result = new BigInteger(nm.substring(index), radix);
            result = (negative) ? result.negate() : result;
        } catch (final NumberFormatException e) {
            // If number is Long.MIN_VALUE, we'll end up here. The next line
            // handles this case, and causes any genuine format error to be
            // rethrown.
            final String constant = (negative) ? "-" + nm.substring(index) : nm.substring(index);
            result = new BigInteger(constant, radix);
        }
        return result;
    }

    /**
     * Attempts to convert the any to the appropriate Java type.
     * 
     * @param theAny the Any to convert
     * @return a Java object that corresponds to theAny's type
     */
    public static Object convertAny(final Any theAny) {
        if (theAny == null) {
            return null;
        }
        return AnyUtils.convertAny(theAny, theAny.type());
    }

    /**
     * Attempts to convert the any using the specified typeCode to the appropriate Java type.
     * 
     * @param theAny the Any to convert
     * @param typeCode the TypeCode of the desired value
     * @return a Java object from theAny that corresponds to the typeCode
     * @since 3.0
     */
    public static Object convertAny(final Any theAny, final TypeCode typeCode) {
        if (theAny == null) {
            return null;
        }
        // Do this check because extract doesn't throw correctly
        try {
            final TCKind kind = typeCode.kind();
            switch (kind.value()) {
            case TCKind._tk_any:
                return theAny.extract_any();
            case TCKind._tk_boolean:
                return theAny.extract_boolean();
            case TCKind._tk_char:
                return theAny.extract_char();
            case TCKind._tk_double:
                return theAny.extract_double();
            case TCKind._tk_fixed:
                return theAny.extract_fixed();
            case TCKind._tk_float:
                return theAny.extract_float();
            case TCKind._tk_long:
                return theAny.extract_long();
            case TCKind._tk_longlong:
                return theAny.extract_longlong();
            case TCKind._tk_objref:
                return theAny.extract_Object();
            case TCKind._tk_octet:
                return theAny.extract_octet();
            case TCKind._tk_short:
                return theAny.extract_short();
            case TCKind._tk_string:
                return theAny.extract_string();
            case TCKind._tk_TypeCode:
                return theAny.extract_TypeCode();
            case TCKind._tk_ulong:
                // return theAny.extract_ulong() & 0xffffffffL;
                // return Integer.valueOf(Integer.toBinaryString(theAny.extract_ulong()), 2);
                return UnsignedUtils.toSigned(theAny.extract_ulong());
            case TCKind._tk_ulonglong:
                //                return Long.toBinaryString(theAny.extract_ulonglong()), 2);
                return UnsignedUtils.toSigned(theAny.extract_ulonglong());
            case TCKind._tk_ushort:
                //                return (short) (theAny.extract_ushort() & AnyUtils.MAX_SHORT);
                return UnsignedUtils.toSigned(theAny.extract_ushort());
            case TCKind._tk_value:
                return theAny.extract_Value();
            case TCKind._tk_wchar:
                return theAny.extract_wchar();
            case TCKind._tk_wstring:
                return theAny.extract_wstring();
            case TCKind._tk_null:
                return null;
            case TCKind._tk_sequence:
                if (PropertiesHelper.type().equivalent(typeCode)) {
                    return PropertiesHelper.extract(theAny);
                } else {
                    return AnyUtils.extractSequence(theAny, typeCode.content_type());
                }
            case TCKind._tk_alias:
                final TypeCode contentType = typeCode.content_type();
                if (contentType.kind().value() == TCKind._tk_sequence) {
                    return AnyUtils.convertAny(theAny, contentType);
                } else {
                    return theAny.extract_Object();
                }
            case TCKind._tk_struct:
            case TCKind._tk_longdouble:
            case TCKind._tk_array:
            case TCKind._tk_abstract_interface:
            case TCKind._tk_enum:
            case TCKind._tk_except:
            case TCKind._tk_native:
            case TCKind._tk_Principal:
            case TCKind._tk_union:
            case TCKind._tk_value_box:
            case TCKind._tk_void:
            default:
                throw new IllegalArgumentException("Only primitive types supported, unknown conversion: " + typeCode);
            }
        } catch (final BAD_OPERATION ex) {
            return null;
        } catch (final BadKind e) {
            return null;
        }
    }

    /**
     * Internal function used to extract a sequence from an any.
     * 
     * @param theAny the Any to convert
     * @param contentType the TypeCode of the desired value
     * @return an array of Java objects from theAny that corresponds to the typeCode
     */
    private static Object[] extractSequence(final Any theAny, final TypeCode contentType) {
        try {
            final TCKind kind = contentType.kind();
            switch (kind.value()) {
            case TCKind._tk_any:
                return AnySeqHelper.extract(theAny);
            case TCKind._tk_boolean:
                return ArrayUtils.toObject(BooleanSeqHelper.extract(theAny));
            case TCKind._tk_char:
                return ArrayUtils.toObject(CharSeqHelper.extract(theAny));
            case TCKind._tk_double:
                return ArrayUtils.toObject(DoubleSeqHelper.extract(theAny));
            case TCKind._tk_float:
                return ArrayUtils.toObject(FloatSeqHelper.extract(theAny));
            case TCKind._tk_long:
                return ArrayUtils.toObject(LongSeqHelper.extract(theAny));
            case TCKind._tk_longlong:
                return ArrayUtils.toObject(LongLongSeqHelper.extract(theAny));
            case TCKind._tk_octet:
                return ArrayUtils.toObject(OctetSeqHelper.extract(theAny));
            case TCKind._tk_short:
                return ArrayUtils.toObject(ShortSeqHelper.extract(theAny));
            case TCKind._tk_string:
                return StringSeqHelper.extract(theAny);
            case TCKind._tk_ulong:
                return ArrayUtils.toObject(UnsignedUtils.toSigned(ULongSeqHelper.extract(theAny)));
            case TCKind._tk_ulonglong:
                return UnsignedUtils.toSigned(ULongLongSeqHelper.extract(theAny));
            case TCKind._tk_ushort:
                return ArrayUtils.toObject(UnsignedUtils.toSigned(UShortSeqHelper.extract(theAny)));
            case TCKind._tk_wchar:
                return ArrayUtils.toObject(WCharSeqHelper.extract(theAny));
            case TCKind._tk_wstring:
                return WStringSeqHelper.extract(theAny);
            case TCKind._tk_null:
                return null;
            case TCKind._tk_sequence:
                if (PropertiesHelper.type().equivalent(contentType)) {
                    return PropertiesHelper.extract(theAny);
                } else {
                    return AnyUtils.extractSequence(theAny, contentType.content_type());
                }
            case TCKind._tk_alias:
                final TypeCode innerContentType = contentType.content_type();
                if (innerContentType.kind().value() == TCKind._tk_sequence) {
                    return AnyUtils.extractSequence(theAny, innerContentType);
                } else {
                    throw new IllegalArgumentException("Unsupported alias content type: " + innerContentType);
                }
            case TCKind._tk_struct:
                if (DataTypeHelper.type().equivalent(contentType)) {
                    return PropertiesHelper.extract(theAny);
                } else {
                    throw new IllegalArgumentException("Unsupported struct content type: " + contentType);
                }
            default:
                throw new IllegalArgumentException("Only primitive sequence types supported, unknown conversion: " + contentType);
            }
        } catch (final BAD_OPERATION ex) {
            return null;
        } catch (final BadKind e) {
            return null;
        }
    }

    public static String convertToStringName(final TCKind type) throws BadKind {
        if (type == null) {
            throw new NullPointerException();
        }
        return ORB.init().get_primitive_tc(type).name();
    }

    public static TCKind convertToTCKind(final String type) {
        if (type == null || "".equals(type)) {
            return TCKind.tk_null;
        } else if (type.equals("boolean")) {
            return TCKind.tk_boolean;
        } else if (type.equals("char")) {
            return TCKind.tk_char;
        } else if (type.equals("double")) {
            return TCKind.tk_double;
        } else if (type.equals("fixed")) {
            return TCKind.tk_fixed;
        } else if (type.equals("float")) {
            return TCKind.tk_float;
        } else if (type.equals("long")) {
            return TCKind.tk_long;
        } else if (type.equals("longlong")) {
            return TCKind.tk_longlong;
        } else if (type.equals("objref")) {
            return TCKind.tk_objref;
        } else if (type.equals("octet")) {
            return TCKind.tk_octet;
        } else if (type.equals("short")) {
            return TCKind.tk_short;
        } else if (type.equals("string")) {
            return TCKind.tk_string;
        } else if (type.equals("typecode")) {
            return TCKind.tk_TypeCode;
        } else if (type.equals("ulong")) {
            return TCKind.tk_ulong;
        } else if (type.equals("ulonglong")) {
            return TCKind.tk_ulonglong;
        } else if (type.equals("ushort")) {
            return TCKind.tk_ushort;
        } else if (type.equals("value")) {
            return TCKind.tk_value;
        } else if (type.equals("wchar")) {
            return TCKind.tk_wchar;
        } else if (type.equals("wstring")) {
            return TCKind.tk_wstring;
        } else {
            throw new IllegalArgumentException("Unknown type: " + type);
        }
    }

    public static Any toAny(final Object value, final TCKind type) {
        final Any retVal = ORB.init().create_any();
        AnyUtils.insertInto(retVal, value, type);
        return retVal;
    }

    /**
     * @since 3.0
     */
    public static Any insertInto(final Any retVal, final Object value, final TCKind type) {
        if (value == null) {
            return retVal;
        }
        switch (type.value()) {
        case TCKind._tk_any:
            retVal.insert_any((Any) value);
            break;
        case TCKind._tk_boolean:
            retVal.insert_boolean((Boolean) value);
            break;
        case TCKind._tk_char:
            retVal.insert_char((Character) value);
            break;
        case TCKind._tk_double:
            retVal.insert_double(((Number) value).doubleValue());
            break;
        case TCKind._tk_fixed:
            retVal.insert_fixed((BigDecimal) value);
            break;
        case TCKind._tk_float:
            retVal.insert_float(((Number) value).floatValue());
            break;
        case TCKind._tk_long:
            retVal.insert_long(((Number) value).intValue());
            break;
        case TCKind._tk_longlong:
            retVal.insert_longlong(((Number) value).longValue());
            break;
        case TCKind._tk_objref:
            if (value instanceof org.omg.CORBA.Object) {
                retVal.insert_Object((org.omg.CORBA.Object) value);
            }
            break;
        case TCKind._tk_octet:
            retVal.insert_octet(((Number) value).byteValue());
            break;
        case TCKind._tk_short:
            retVal.insert_short(((Number) value).shortValue());
            break;
        case TCKind._tk_string:
            retVal.insert_string(value.toString());
            break;
        case TCKind._tk_TypeCode:
            retVal.insert_TypeCode((TypeCode) value);
            break;
        case TCKind._tk_ulong:
            retVal.insert_ulong(((Number) value).intValue());
            break;
        case TCKind._tk_ulonglong:
            retVal.insert_ulonglong(((Number) value).longValue());
            break;
        case TCKind._tk_ushort:
            retVal.insert_ushort(((Number) value).shortValue());
            break;
        case TCKind._tk_value:
            retVal.insert_Value((Serializable) value);
            break;
        case TCKind._tk_wchar:
            retVal.insert_wchar((Character) value);
            break;
        case TCKind._tk_wstring:
            retVal.insert_wstring(value.toString());
            break;
        default:
            throw new IllegalArgumentException("Unknown target type: " + type);
        }
        return retVal;
    }

    /**
     */
    public static Any toAny(final Object value, final String type) {
        final TCKind kind = AnyUtils.convertToTCKind(type);

        if ((value instanceof String) && (kind != TCKind.tk_string)) {
            return AnyUtils.stringToAny((String) value, type);
        }

        return AnyUtils.toAny(value, kind);
    }

    public static Any toAny(final Object[] value, final String type) {
        final TCKind kind = AnyUtils.convertToTCKind(type);
        final Object[] convArray = AnyUtils.convertStringArray(value, type);

        return AnyUtils.toAny(convArray, kind);
    }

    private static Object[] convertStringArray(final Object[] value, final String type) {
        Object[] retVal = value;
        if (value instanceof String[] && !"string".equals(type)) {
            retVal = new Object[value.length];
            for (int i = 0; i < value.length; ++i) {
                final String val = (String) value[i];
                retVal[i] = AnyUtils.convertString(val, type);
            }
        }

        return retVal;
    }

    /**
     * @since 3.0
     */
    public static Any toAnySequence(final Object value, TypeCode type) {
        if (type.kind().value() != TCKind._tk_sequence) {
            throw new IllegalArgumentException("Type is not a sequence");
        }
        
        try {
            return toAnySequence(value, type.content_type().kind());
        } catch (final BadKind e) {
            throw new IllegalArgumentException("Bad Kind for type: " + type);
        }
    }

    public static Any toAny(final Object[] value, final TCKind type) {
        return AnyUtils.toAnySequence(value, type);
    }

    /**
     * @since 3.0
     */
    public static Any toAnySequence(final Object value, TCKind type) {
        final Any retVal = ORB.init().create_any();
        if (type == null) {
            type = AnyUtils.deriveArrayType(value);
        }

        if (value == null || type == null) {
            return retVal;
        }

        switch (type.value()) {
        case TCKind._tk_any:
            org.omg.DynamicAny.AnySeqHelper.insert(retVal, (Any[]) value);
            break;
        case TCKind._tk_boolean:
            AnyUtils.handleBoolean(retVal, value);
            break;
        case TCKind._tk_char:
            AnyUtils.handleChar(retVal, value);
            break;
        case TCKind._tk_double:
            AnyUtils.handleDouble(retVal, value);
            break;
        case TCKind._tk_float:
            AnyUtils.handleFloat(retVal, value);
            break;
        case TCKind._tk_long:
            AnyUtils.handleLong(retVal, value);
            break;
        case TCKind._tk_longlong:
            AnyUtils.handleLongLong(retVal, value);
            break;
        case TCKind._tk_octet:
            AnyUtils.handleOctet(retVal, value);
            break;
        case TCKind._tk_short:
            AnyUtils.handleShort(retVal, value);
            break;
        case TCKind._tk_string:
            StringSeqHelper.insert(retVal, AnyUtils.convertStringArray(value));
            break;
        case TCKind._tk_ulong:
            AnyUtils.handleULong(retVal, value);
            break;
        case TCKind._tk_ulonglong:
            AnyUtils.handleULongLong(retVal, value);
            break;
        case TCKind._tk_ushort:
            AnyUtils.handleUShort(retVal, value);
            break;
        case TCKind._tk_wchar:
            AnyUtils.handleWChar(retVal, value);
            break;
        case TCKind._tk_wstring:
            WStringSeqHelper.insert(retVal, AnyUtils.convertStringArray(value));
            break;
        default:
            throw new IllegalArgumentException("Unknown target type: " + type);
        }
        return retVal;
    }

    private static TCKind deriveArrayType(final Object array) {
        if (array instanceof Any[]) {
            return TCKind.tk_any;
        } else if (array instanceof Boolean[] || array instanceof boolean[]) {
            return TCKind.tk_boolean;
        } else if (array instanceof Character[] || array instanceof char[]) {
            return TCKind.tk_char;
        } else if (array instanceof Double[] || array instanceof double[]) {
            return TCKind.tk_double;
        } else if (array instanceof Float[] || array instanceof float[]) {
            return TCKind.tk_float;
        } else if (array instanceof Integer[] || array instanceof int[]) {
            return TCKind.tk_long;
        } else if (array instanceof Long[] || array instanceof long[]) {
            return TCKind.tk_longlong;
        } else if (array instanceof Byte[] || array instanceof byte[]) {
            return TCKind.tk_octet;
        } else if (array instanceof Short[] || array instanceof short[]) {
            return TCKind.tk_short;
        } else if (array instanceof String[]) {
            return TCKind.tk_string;
        }
        return null;
    }

    private static String[] convertStringArray(final Object value) {
        if (value instanceof String[]) {
            return (String[]) value;
        }
        Object[] objArray;
        if (value instanceof Object[]) {
            objArray = (Object[]) value;
        } else {
            objArray = new Object[] { value };
        }
        try {
            return Arrays.asList(objArray).toArray(new String[objArray.length]);
        } catch (final ArrayStoreException e) {
            final String[] retVal = new String[objArray.length];
            for (int i = 0; i < retVal.length; i++) {
                if (objArray[i] == null) {
                    retVal[i] = "";
                } else {
                    retVal[i] = objArray[i].toString();
                }
            }
            return retVal;
        }
    }

    private static void handleWChar(final Any retVal, final Object value) {
        final char[] primValue = PrimitiveArrayUtils.convertToCharArray(value);
        if (primValue != null) {
            WCharSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleUShort(final Any retVal, final Object value) {
        final short[] primValue = PrimitiveArrayUtils.convertToShortArray(value);
        if (primValue != null) {
            UShortSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleULongLong(final Any retVal, final Object value) {
        final long[] primValue = PrimitiveArrayUtils.convertToLongArray(value);
        if (primValue != null) {
            ULongLongSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleULong(final Any retVal, final Object value) {
        final int[] primValue = PrimitiveArrayUtils.convertToIntArray(value);
        if (primValue != null) {
            ULongSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleShort(final Any retVal, final Object value) {
        final short[] primValue = PrimitiveArrayUtils.convertToShortArray(value);
        if (primValue != null) {
            ShortSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleOctet(final Any retVal, final Object value) {
        final byte[] primValue = PrimitiveArrayUtils.convertToByteArray(value);
        if (primValue != null) {
            OctetSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleLongLong(final Any retVal, final Object value) {
        final long[] primValue = PrimitiveArrayUtils.convertToLongArray(value);
        if (primValue != null) {
            LongLongSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleLong(final Any retVal, final Object value) {
        final int[] primValue = PrimitiveArrayUtils.convertToIntArray(value);
        if (primValue != null) {
            LongSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleFloat(final Any retVal, final Object value) {
        final float[] primValue = PrimitiveArrayUtils.convertToFloatArray(value);
        if (primValue != null) {
            FloatSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleDouble(final Any retVal, final Object value) {
        final double[] primValue = PrimitiveArrayUtils.convertToDoubleArray(value);
        if (primValue != null) {
            DoubleSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleChar(final Any retVal, final Object value) {
        final char[] primValue = PrimitiveArrayUtils.convertToCharArray(value);
        if (primValue != null) {
            CharSeqHelper.insert(retVal, primValue);
        }
    }

    private static void handleBoolean(final Any retVal, final Object value) {
        final boolean[] primValue = PrimitiveArrayUtils.convertToBooleanArray(value);
        if (primValue != null) {
            BooleanSeqHelper.insert(retVal, primValue);
        }
    }

    /**
     */
    public static Any stringToAny(final String value, final String type) {
        final Object newValue = AnyUtils.convertString(value, type);
        return AnyUtils.toAny(newValue, AnyUtils.convertToTCKind(type));
    }

    /**
     * This compares two Any objects with the given SCA action.
     * @param a the first Any
     * @param b the second Any
     * @param action the action to use for comparison
     * @return the corresponding result of the SCA defined action on the two
     * passed in Any objects
     * @since 3.0
     */
    public static boolean compareAnys(final Any a, final Any b, final String action) {
        int kindA = a.type().kind().value();
        int kindB = b.type().kind().value();

        // If the types don't match, the comparison will always be false, 
        // unless the action passed in is not equals
        if (kindA != kindB) {
            if ("ne".equals(action)) {
                return true;
            }
            return false;
        }
        
        boolean result = false;
        switch (kindA) {
        case TCKind._tk_boolean:
            result = AnyUtils.performAction(a.extract_boolean(), b.extract_boolean(), action, kindA);
            break;
        case TCKind._tk_char:
            result = AnyUtils.performAction(a.extract_char(), b.extract_char(), action, kindA);
            break;
        case TCKind._tk_wchar:
            result = AnyUtils.performAction(a.extract_wchar(), b.extract_wchar(), action, kindA);
            break;
        case TCKind._tk_octet:
            result = AnyUtils.performAction(a.extract_octet(), b.extract_octet(), action, kindA);
            break;
        case TCKind._tk_ushort:
            result = AnyUtils.performAction(a.extract_ushort(), b.extract_ushort(), action, kindA);
            break;
        case TCKind._tk_short:
            result = AnyUtils.performAction(a.extract_short(), b.extract_short(), action, kindA);
            break;
        case TCKind._tk_float:
            result = AnyUtils.performAction(a.extract_float(), b.extract_float(), action, kindA);
            break;
        case TCKind._tk_double:
            result = AnyUtils.performAction(a.extract_double(), b.extract_double(), action, kindA);
            break;
        case TCKind._tk_ulong:
            result = AnyUtils.performAction(a.extract_ulong(), b.extract_ulong(), action, kindA);
            break;
        case TCKind._tk_long:
            result = AnyUtils.performAction(a.extract_long(), b.extract_long(), action, kindA);
            break;
        case TCKind._tk_ulonglong:
            result = AnyUtils.performAction(a.extract_ulonglong(), b.extract_ulonglong(), action, kindA);
            break;
        case TCKind._tk_longlong:
            result = AnyUtils.performAction(a.extract_longlong(), b.extract_longlong(), action, kindA);
            break;
        case TCKind._tk_string:
            result = AnyUtils.performAction(a.extract_string(), b.extract_string(), action, kindA);
            break;
        case TCKind._tk_wstring:
            result = AnyUtils.performAction(a.extract_wstring(), b.extract_wstring(), action, kindA);
            break;
        case TCKind._tk_fixed:
            result = AnyUtils.performAction(a.extract_fixed(), b.extract_fixed(), action, kindA);
            break;
        case TCKind._tk_objref:
            result = false;
            break;
        case TCKind._tk_alias:
            try {
                final TypeCode innerContentTypeA = a.type().content_type();
                final TypeCode innerContentTypeB = b.type().content_type();
                
                // check that both are sequences and that both sequences are the same type
                if (innerContentTypeA.kind().value() != innerContentTypeB.kind().value() ||
                        innerContentTypeA.content_type().kind().value() != innerContentTypeB.content_type().kind().value()) {
                    if ("ne".equals(action)) {
                        return true;
                    }
                    return false;
                }
                
                // Sequences, Structs, and StructSequences will all match this
                if (innerContentTypeA.kind().value() == TCKind._tk_sequence) {
                    Object[] tmpA = extractSequence(a, innerContentTypeA.content_type());
                    Object[] tmpB = extractSequence(b, innerContentTypeB.content_type());
                    
                    // Here is the test to determine if we have a struct
                    if(tmpA.getClass() == CF.DataType[].class) {
                        CF.DataType[] dtsA = (CF.DataType[])tmpA;
                        CF.DataType[] dtsB = (CF.DataType[])tmpB;
                        
                        // test if the structs have a different number of members
                        if (dtsA.length != dtsB.length) {
                            if ("ne".equals(action)) {
                                return true;
                            }
                            return false;
                        }
                        
                        // try to compare each member of the struct using the compareAnys recursively
                        for (int i = 0; i < dtsA.length; i++) {
                            for (int j = 0; j < dtsB.length; j++) {
                                if (dtsA[i].id.equals(dtsB[i].id)) {
                                    if (compareAnys(dtsA[i].value, dtsB[i].value, action)) {
                                        if ("ne".equals(action)) {
                                            return true;
                                        }
                                    } else {
                                        if ("eq".equals(action)) {
                                            return false;
                                        }
                                    }
                                    break;
                                }
                            }
                        }
                        if ("eq".equals(action)) {
                            return true;
                        } 
                        return false;
                    }
                    
                    // Here is the test to determine if we have a struct sequence
                    if(tmpA.getClass() == Any[].class) {
                        Any[] anysA = (Any[])tmpA;
                        Any[] anysB = (Any[])tmpB;
                        
                        // test if the sequences are of different length
                        if (anysA.length != anysB.length) {
                            if ("ne".equals(action)) {
                                return true;
                            }     
                            return false;
                        } else {
                            // try to compare each struct using the compareAnys recursively
                            for (int i = 0; i < anysA.length; i++) {
                                if (compareAnys(anysA[i], anysB[i], action)) {
                                    if ("ne".equals(action)) {
                                        return true;
                                    }
                                } else {
                                    if ("eq".equals(action)) {
                                        return false;
                                    }
                                }
                            }
                            if ("eq".equals(action)) {
                                return true;
                            }
                            return false;
                        }
                    }
                    
                    // Here is the comparison for a normal sequence type
                    if (tmpA.length != tmpB.length) {
                        if ("ne".equals(action)) {
                            return true;
                        } 
                        return false;
                    } else {
                        //compare each value
                        for (int i = 0; i < tmpA.length; i++) {
                            if (!tmpA[i].equals(tmpB[i])) {
                                if ("ne".equals(action)) {
                                    return true;
                                } 
                                return false;
                            }
                        }
                        if ("eq".equals(action)) {
                            return true;
                        } 
                        return false;
                    }
                } else {
                    return false;
                }
            } catch (final BadKind e) {
                return false;
            }

            
        default:
            result = false;
        }
        return result;
    }

    private static boolean performAction(final Object val1, final Object val2, final String action, final int kindValue) {
        // TODO need to account for null val1 or val2
        final boolean eq = val1.equals(val2);

        if ("eq".equals(action)) {
            return eq;
        } else if ("ne".equals(action)) {
            return !eq;
        }

        boolean gt = false;
        boolean lt = false;
        int comp = 0;
        switch (kindValue) {
        case TCKind._tk_boolean:
            gt = (Boolean) val1 && !(Boolean) val2;
            lt = !(Boolean) val1 && (Boolean) val2;
            break;
        case TCKind._tk_char:
        case TCKind._tk_wchar:
            comp = ((Character) val1).compareTo((Character) val2);
            gt = comp > 0;
            lt = comp < 0;
            break;
        case TCKind._tk_octet:
            gt = (Byte) val1 > (Byte) val2;
            lt = (Byte) val1 < (Byte) val2;
            break;
        case TCKind._tk_ushort:
        case TCKind._tk_short:
            gt = (Short) val1 > (Short) val2;
            lt = (Short) val1 < (Short) val2;
            break;
        case TCKind._tk_float:
            gt = (Float) val1 > (Float) val2;
            lt = (Float) val1 < (Float) val2;
            break;
        case TCKind._tk_double:
            gt = (Double) val1 > (Double) val2;
            lt = (Double) val1 < (Double) val2;
            break;
        case TCKind._tk_ulong:
        case TCKind._tk_long:
            gt = (Integer) val1 > (Integer) val2;
            lt = (Integer) val1 < (Integer) val2;
            break;
        case TCKind._tk_ulonglong:
        case TCKind._tk_longlong:
            gt = (Long) val1 > (Long) val2;
            lt = (Long) val1 < (Long) val2;
            break;
        case TCKind._tk_string:
        case TCKind._tk_wstring:
            comp = ((String) val1).compareTo((String) val2);
            gt = comp > 0;
            lt = comp < 0;
            break;
        case TCKind._tk_fixed:
            final int compare = ((BigDecimal) val1).compareTo((BigDecimal) val2);
            gt = compare == 1;
            lt = compare == -1;
            break;
        default:
            break;
        }

        if ("gt".equals(action)) {
            return gt;
        } else if ("lt".equals(action)) {
            return lt;
        } else if ("ge".equals(action)) {
            return gt || eq;
        } else if ("le".equals(action)) {
            return lt || eq;
        }
        return false;
    }

}
