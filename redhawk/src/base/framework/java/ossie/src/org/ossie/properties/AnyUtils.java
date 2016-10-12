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
            import org.apache.log4j.Logger;

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
import CF.complexFloatHelper;
import CF.complexDoubleHelper;
import CF.complexShortHelper;
import CF.complexUShortHelper;
import CF.complexBooleanHelper;
import CF.complexCharHelper;
import CF.complexOctetHelper;
import CF.complexLongHelper;
import CF.complexULongHelper;
import CF.complexLongLongHelper;
import CF.complexULongLongHelper;

import CF.complexFloatSeqHelper;
import CF.complexDoubleSeqHelper;
import CF.complexShortSeqHelper;
import CF.complexUShortSeqHelper;
import CF.complexBooleanSeqHelper;
import CF.complexCharSeqHelper;
import CF.complexOctetSeqHelper;
import CF.complexLongSeqHelper;
import CF.complexULongSeqHelper;
import CF.complexLongLongSeqHelper;
import CF.complexULongLongSeqHelper;


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
            return UnsignedUtils.parseULong(stringValue);
        } else if (type.equals("ushort")) {
            return UnsignedUtils.parseUShort(stringValue);
        } else if (type.equals("ulonglong")) {
            return UnsignedUtils.parseULongLong(stringValue);
        } else if (type.equals("objref")) {
            List<String> objrefPrefix = Arrays.asList("IOR:", "corbaname:", "corbaloc:");
            for (String prefix : objrefPrefix) {
                if (stringValue.startsWith(prefix)) {
                    return stringValue;
                }
            }
            throw new IllegalArgumentException(stringValue + " is not a valid objref value");
        } else if (type.equals("octet")) {
            return UnsignedUtils.parseOctet(stringValue);
        } else if (type.equals("complexFloat")) {
            return ComplexUtils.parseComplexFloat(stringValue);
        } else if (type.equals("complexDouble")) {
            return ComplexUtils.parseComplexDouble(stringValue);
        } else if (type.equals("complexBoolean")) {
            return ComplexUtils.parseComplexBoolean(stringValue);
        // TODO: complexChar
        /*
        } else if (type.equals("complexChar")) {
            String[] realAndImag = new String[2];
            realAndImag = ComplexUtils.getRealImagStringsFromComplex(stringValue);
            CF.complexChar returnVal = new CF.complexChar(
                (String)AnyUtils.convertString(realAndImag[0], "char"),
                (String)AnyUtils.convertString(realAndImag[1], "char"));
            return returnVal;
        */
        } else if (type.equals("complexOctet")) {
            return ComplexUtils.parseComplexOctet(stringValue);
        } else if (type.equals("complexShort")) {
            return ComplexUtils.parseComplexShort(stringValue);
        } else if (type.equals("complexUShort")) {
            return ComplexUtils.parseComplexUShort(stringValue);
        } else if (type.equals("complexLong")) {
            return ComplexUtils.parseComplexLong(stringValue);
        } else if (type.equals("complexULong")) {
            return ComplexUtils.parseComplexULong(stringValue);
        } else if (type.equals("complexLongLong")) {
            return ComplexUtils.parseComplexLongLong(stringValue);
        } else if (type.equals("complexULongLong")) {
            return ComplexUtils.parseComplexULongLong(stringValue);
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
        
        final TCKind kind = typeCode.kind();

        // Do this check because extract doesn't throw correctly
        try {
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
                if (typeCode.name().equals("complexFloat")) {
                    return complexFloatHelper.extract(theAny);
                } else if (typeCode.name().equals("complexDouble")) {
                    return complexDoubleHelper.extract(theAny);
                } else if (typeCode.name().equals("complexShort")) {
                    return complexShortHelper.extract(theAny);
                } else if (typeCode.name().equals("complexUShort")) {
                    return complexUShortHelper.extract(theAny);
                } else if (typeCode.name().equals("complexBoolean")) {
                    return complexBooleanHelper.extract(theAny);
                } else if (typeCode.name().equals("complexOctet")) {
                    return complexOctetHelper.extract(theAny);
                } else if (typeCode.name().equals("complexChar")) {
                    return complexCharHelper.extract(theAny);
                } else if (typeCode.name().equals("complexLong")) {
                    return complexLongHelper.extract(theAny);
                } else if (typeCode.name().equals("complexULong")) {
                    return complexULongHelper.extract(theAny);
                } else if (typeCode.name().equals("complexLongLong")) {
                    return complexLongLongHelper.extract(theAny);
                } else if (typeCode.name().equals("complexULongLong")) {
                    return complexULongLongHelper.extract(theAny);
                }
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
                throw new IllegalArgumentException("Only primitive types supported, unknown conversion: " + typeCode.name());//typeCode);
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
    private static Object[] extractSequence(final Any theAny, 
                                            final TypeCode contentType) {
        final TCKind kind = contentType.kind();

        // Do this check because extract doesn't throw correctly
        try {
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
                } else if (complexFloatHelper.type().equivalent(contentType)) {
                    return complexFloatSeqHelper.extract(theAny);
                } else if (complexDoubleHelper.type().equivalent(contentType)) {
                    return complexDoubleSeqHelper.extract(theAny);
                } else if (complexBooleanHelper.type().equivalent(contentType)) {
                    return complexBooleanSeqHelper.extract(theAny);
                } else if (complexCharHelper.type().equivalent(contentType)) {
                    return complexCharSeqHelper.extract(theAny);
                } else if (complexOctetHelper.type().equivalent(contentType)) {
                    return complexOctetSeqHelper.extract(theAny);
                } else if (complexShortHelper.type().equivalent(contentType)) {
                    return complexShortSeqHelper.extract(theAny);
                } else if (complexUShortHelper.type().equivalent(contentType)) {
                    return complexShortSeqHelper.extract(theAny);
                } else if (complexLongHelper.type().equivalent(contentType)) {
                    return complexLongSeqHelper.extract(theAny);
                } else if (complexULongHelper.type().equivalent(contentType)) {
                    return complexULongSeqHelper.extract(theAny);
                } else if (complexLongLongHelper.type().equivalent(contentType)) {
                    return complexLongSeqHelper.extract(theAny);
                } else if (complexULongLongHelper.type().equivalent(contentType)) {
                    return complexULongLongSeqHelper.extract(theAny);
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
        return getPrimitiveTypeCode(type).name();
    }

    public static TypeCode getPrimitiveTypeCode(TCKind kind) {
        return ORB.init().get_primitive_tc( kind );
    }

    /*
     * Call ORB.get_primative_tc with the appropriate TCKind
     * corresponding to kindStr.  Return the TypeCode if supported; 
     * otherwise, return null.
     */
    public static TypeCode getPrimitiveTypeCode(final String kindStr) {
        if (kindStr == null || "".equals(kindStr)) {
            return getPrimitiveTypeCode( TCKind.tk_null );
        } else if (kindStr.equals("any")) {
            return getPrimitiveTypeCode( TCKind.tk_any );
        } else if (kindStr.equals("boolean")) {
            return getPrimitiveTypeCode( TCKind.tk_boolean );
        } else if (kindStr.equals("char")) {
            return getPrimitiveTypeCode( TCKind.tk_char );
        } else if (kindStr.equals("double")) {
            return getPrimitiveTypeCode( TCKind.tk_double );
        } else if (kindStr.equals("fixed")) {
            return getPrimitiveTypeCode( TCKind.tk_fixed );
        } else if (kindStr.equals("float")) {
            return getPrimitiveTypeCode( TCKind.tk_float );
        } else if (kindStr.equals("long")) {
            return getPrimitiveTypeCode( TCKind.tk_long );
        } else if (kindStr.equals("longlong")) {
            return getPrimitiveTypeCode( TCKind.tk_longlong );
        } else if (kindStr.equals("objref")) {
            return getPrimitiveTypeCode( TCKind.tk_objref );
        } else if (kindStr.equals("octet")) {
            return getPrimitiveTypeCode( TCKind.tk_octet );
        } else if (kindStr.equals("short")) {
            return getPrimitiveTypeCode( TCKind.tk_short );
        } else if (kindStr.equals("string")) {
            return getPrimitiveTypeCode( TCKind.tk_string );
        } else if (kindStr.equals("ulong")) {
            return getPrimitiveTypeCode( TCKind.tk_ulong );
        } else if (kindStr.equals("ulonglong")) {
            return getPrimitiveTypeCode( TCKind.tk_ulonglong );
        } else if (kindStr.equals("ushort")) {
            return getPrimitiveTypeCode( TCKind.tk_ushort );
        } else if (kindStr.equals("value")) {
            return getPrimitiveTypeCode( TCKind.tk_value );
        } else if (kindStr.equals("wchar")) {
            return getPrimitiveTypeCode( TCKind.tk_wchar );
        } else if (kindStr.equals("wstring")) {
            return getPrimitiveTypeCode( TCKind.tk_wstring );
        } else {
            return null;
        }
    }
        
    /*
     * Call type() on the appropriate complex helper.
     * Return the TypeCode if supported; otherwise, return null.
     */
    public static TypeCode getComplexTypeCode(final String kindStr) {
        if ("complexShort".equals(kindStr)) {
            return complexShortHelper.type();
        } else if (kindStr.equals("complexUShort")) {
            return complexUShortHelper.type();
        } else if (kindStr.equals("complexBoolean")) {
            return complexBooleanHelper.type();
        } else if (kindStr.equals("complexDouble")) {
            return complexDoubleHelper.type();
        } else if (kindStr.equals("complexFloat")) {
            return complexFloatHelper.type();
        } else if (kindStr.equals("complexChar")) {
            return complexCharHelper.type();
        } else if (kindStr.equals("complexOctet")) {
            return complexOctetHelper.type();
        } else if (kindStr.equals("complexLong")) {
            return complexLongHelper.type();
        } else if (kindStr.equals("complexLongLong")) {
            return complexLongLongHelper.type();
        } else if (kindStr.equals("complexULong")) {
            return complexULongHelper.type();
        } else if (kindStr.equals("complexULongLong")) {
            return complexULongLongHelper.type();
        } else {
            return null;
        }
    }

    public static TypeCode convertToTypeCode(final String typeStr) {
        TypeCode type = getPrimitiveTypeCode(typeStr);
        if (type == null) {
            type = getComplexTypeCode(typeStr);
        }
        if (type == null) {
            throw new IllegalArgumentException("Unknown type: " + typeStr);
        }
        return type;
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

    public static Any toAny(final String value) {
        Any any = ORB.init().create_any();
        if (value != null) {
            any.insert_string(value);
        }
        return any;
    }

    public static Any toAny(final Object value, final TypeCode type) {
        final Any retVal = ORB.init().create_any();
        AnyUtils.insertInto(retVal, value, type);
        return retVal;
    }

    /*
     * This only works for primitive types.  For structs, such as 
     * complex values, use toAny(final Object value, final TypeCode type) 
     * instead.
     */ 
    public static Any toAny(final Object value, final TCKind kind) {
        return toAny(value, getPrimitiveTypeCode(kind));
    }

    public static Any insertInto(final Any retVal, 
                                 final Object value, 
                                 final TCKind kind) {
        insertInto(retVal, value, getPrimitiveTypeCode(kind));
        return retVal;
    }

    /**
     * @since 3.0
     */
    public static Any insertInto(final Any retVal, 
                                 final Object value, 
                                 final TypeCode type) {
        if (value == null) {
            return retVal;
        }

        switch (type.kind().value()) {
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
            
        case TCKind._tk_struct:
            if (type.equals(complexFloatHelper.type())) {
                complexFloatHelper.insert(retVal, (CF.complexFloat) value);
            } else if (type.equals(complexDoubleHelper.type())) {
                complexDoubleHelper.insert(retVal, (CF.complexDouble) value);
            } else if (type.equals(complexShortHelper.type())) {
                complexShortHelper.insert(retVal, (CF.complexShort) value);
            } else if (type.equals(complexUShortHelper.type())) {
                complexUShortHelper.insert(retVal, (CF.complexUShort) value);
            } else if (type.equals(complexBooleanHelper.type())) {
                complexBooleanHelper.insert(retVal, (CF.complexBoolean) value);
            } else if (type.equals(complexOctetHelper.type())) {
                complexOctetHelper.insert(retVal, (CF.complexOctet) value);
            } else if (type.equals(complexCharHelper.type())) {
                complexCharHelper.insert(retVal, (CF.complexChar) value);
            } else if (type.equals(complexLongHelper.type())) {
                complexLongHelper.insert(retVal, (CF.complexLong) value);
            } else if (type.equals(complexULongHelper.type())) {
                complexULongHelper.insert(retVal, (CF.complexULong) value);
            } else if (type.equals(complexLongLongHelper.type())) {
                complexLongLongHelper.insert(retVal, 
                                             (CF.complexLongLong) value);
            } else if (type.equals(complexULongLongHelper.type())) {
                complexULongLongHelper.insert(retVal, 
                                              (CF.complexULongLong) value);
            }
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
        final TypeCode typeCode = AnyUtils.convertToTypeCode(type);

        if ((value instanceof String) && (kind != TCKind.tk_string)) {
            return AnyUtils.stringToAny((String) value, type);
        }

        return AnyUtils.toAny(value, typeCode);
    }

    public static Any toAny(final Object[] value, final String type) {
        final TCKind kind = AnyUtils.convertToTCKind(type);
        final TypeCode typeCode = AnyUtils.convertToTypeCode(type);
        final Object[] convArray = AnyUtils.convertStringArray(value, type);

        return AnyUtils.toAny(value, typeCode);
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

    /*
     * This only works for primitive types.  For structs, such as 
     * complex values, use toAny(final Object[] value, final TypeCode type) 
     * instead.
     */ 
    public static Any toAny(final Object[] value, final TCKind kind) {
        return AnyUtils.toAnySequence(value, getPrimitiveTypeCode(kind));
    }

    public static Any toAnySequence(final Object value, final TCKind kind) {
        return AnyUtils.toAnySequence(value, getPrimitiveTypeCode(kind));
    }

    /**
     * @since 3.0
     */
    public static Any toAnySequence(final Object value, TypeCode type) {
        if (type.kind().value() == TCKind._tk_sequence) {
            // for sequences, use the typecode kind of the sequence items
            TypeCode contentType;
            try {
                contentType = type.content_type();
            } catch (final BadKind e) {
                throw new IllegalArgumentException("Unable to resolve sequence type");
            }
            return toAnySequence_(value, contentType);
        } else {
            return toAnySequence_(value, type);
        }
    }

    static Any toAnySequence_(final Object value, TypeCode type) {
        final Any retVal = ORB.init().create_any();
        final TCKind kind = type.kind();

        if (kind == TCKind.tk_any ) {
            org.omg.DynamicAny.AnySeqHelper.insert(retVal, (Any[]) value);
        } else if (kind == TCKind.tk_boolean ){
            AnyUtils.handleBoolean(retVal, value);
        } else if (kind == TCKind.tk_char ){
            AnyUtils.handleChar(retVal, value);
        } else if (kind == TCKind.tk_double ){
            AnyUtils.handleDouble(retVal, value);
        } else if (kind == TCKind.tk_float ){
            AnyUtils.handleFloat(retVal, value);
        } else if (kind == TCKind.tk_long ){
            AnyUtils.handleLong(retVal, value);
        } else if (kind == TCKind.tk_longlong ){
            AnyUtils.handleLongLong(retVal, value);
        } else if (kind == TCKind.tk_octet ){
            AnyUtils.handleOctet(retVal, value);
        } else if (kind == TCKind.tk_short ){
            AnyUtils.handleShort(retVal, value);
        } else if (kind == TCKind.tk_ulong ){
            AnyUtils.handleULong(retVal, value);
        } else if (kind == TCKind.tk_ulonglong ){
            AnyUtils.handleULongLong(retVal, value);
        } else if (kind == TCKind.tk_ushort ){
            AnyUtils.handleUShort(retVal, value);
        } else if (kind == TCKind.tk_wchar ){
            AnyUtils.handleWChar(retVal, value);
        } else if (kind == TCKind.tk_string ){
            StringSeqHelper.insert(retVal, AnyUtils.convertStringArray(value));
        } else if (kind == TCKind.tk_wstring ){
            WStringSeqHelper.insert(retVal, AnyUtils.convertStringArray(value));
        } else if (type.equals(complexFloatHelper.type())) {
            complexFloatSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexFloatArray(value));
        } else if (type.equals(complexDoubleHelper.type())) {
            complexDoubleSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexDoubleArray(value));
        } else if (type.equals(complexBooleanHelper.type())) {
            complexBooleanSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexBooleanArray(value));
        } else if (type.equals(complexCharHelper.type())) {
            complexCharSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexCharArray(value));
        } else if (type.equals(complexOctetHelper.type())) {
            complexOctetSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexOctetArray(value));
        } else if (type.equals(complexShortHelper.type())) {
            complexShortSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexShortArray(value));
        } else if (type.equals(complexUShortHelper.type())) {
            complexUShortSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexUShortArray(value));
        } else if (type.equals(complexLongHelper.type())) {
            complexLongSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexLongArray(value));
        } else if (type.equals(complexULongHelper.type())) {
            complexULongSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexULongArray(value));
        } else if (type.equals(complexLongLongHelper.type())) {
            complexLongLongSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexLongLongArray(value));
        } else if (type.equals(complexULongLongHelper.type())) {
            complexULongLongSeqHelper.insert(
                retVal, 
                ComplexArrayUtils.convertToComplexULongLongArray(value));
        } else {
            throw new IllegalArgumentException("Unknown target type: " + kind);
        }
        return retVal;
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
            result = AnyUtils.performAction(a.extract_boolean(), b.extract_boolean(), action, a.type());
            break;
        case TCKind._tk_char:
            result = AnyUtils.performAction(a.extract_char(), b.extract_char(), action, a.type());
            break;
        case TCKind._tk_wchar:
            result = AnyUtils.performAction(a.extract_wchar(), b.extract_wchar(), action, a.type());
            break;
        case TCKind._tk_octet:
            result = AnyUtils.performAction(a.extract_octet(), b.extract_octet(), action, a.type());
            break;
        case TCKind._tk_ushort:
            result = AnyUtils.performAction(a.extract_ushort(), b.extract_ushort(), action, a.type());
            break;
        case TCKind._tk_short:
            result = AnyUtils.performAction(a.extract_short(), b.extract_short(), action, a.type());
            break;
        case TCKind._tk_float:
            result = AnyUtils.performAction(a.extract_float(), b.extract_float(), action, a.type());
            break;
        case TCKind._tk_double:
            result = AnyUtils.performAction(a.extract_double(), b.extract_double(), action, a.type());
            break;
        case TCKind._tk_ulong:
            result = AnyUtils.performAction(a.extract_ulong(), b.extract_ulong(), action, a.type());
            break;
        case TCKind._tk_long:
            result = AnyUtils.performAction(a.extract_long(), b.extract_long(), action, a.type());
            break;
        case TCKind._tk_ulonglong:
            result = AnyUtils.performAction(a.extract_ulonglong(), b.extract_ulonglong(), action, a.type());
            break;
        case TCKind._tk_longlong:
            result = AnyUtils.performAction(a.extract_longlong(), b.extract_longlong(), action, a.type());
            break;
        case TCKind._tk_string:
            result = AnyUtils.performAction(a.extract_string(), b.extract_string(), action, a.type());
            break;
        case TCKind._tk_wstring:
            result = AnyUtils.performAction(a.extract_wstring(), b.extract_wstring(), action, a.type());
            break;
        case TCKind._tk_fixed:
            result = AnyUtils.performAction(a.extract_fixed(), b.extract_fixed(), action, a.type());
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
                                if (dtsA[i].id.equals(dtsB[j].id)) {
                                    if (compareAnys(dtsA[i].value, dtsB[j].value, action)) {
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

        // Note that performAction may not support gt, lt, ge, le for
        // complex numbers.  We are relying on perfromAction to check
        // the validity of the requested action and throw an exception
        // if appropriate.

        if (a.type().equivalent(complexFloatHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexFloatHelper.extract(a), 
                CF.complexFloatHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexDoubleHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexDoubleHelper.extract(a), 
                CF.complexDoubleHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexBooleanHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexBooleanHelper.extract(a), 
                CF.complexBooleanHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexOctetHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexOctetHelper.extract(a), 
                CF.complexOctetHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexCharHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexCharHelper.extract(a), 
                CF.complexCharHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexShortHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexShortHelper.extract(a), 
                CF.complexShortHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexUShortHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexUShortHelper.extract(a), 
                CF.complexUShortHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexLongHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexLongHelper.extract(a), 
                CF.complexLongHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexULongHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexULongHelper.extract(a), 
                CF.complexULongHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexLongLongHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexLongLongHelper.extract(a), 
                CF.complexLongLongHelper.extract(b), 
                action, 
                a.type());
        } else if (a.type().equivalent(complexULongLongHelper.type())) {
            result = AnyUtils.performAction(
                CF.complexULongLongHelper.extract(a), 
                CF.complexULongLongHelper.extract(b), 
                action, 
                a.type());
        } else {
            result = false;
        }
        }
        return result;
    }

    private static boolean performAction(final Object val1, 
                                         final Object val2, 
                                         final String action, 
                                         final TypeCode typeCode) {
        if ( val1 == null && val2 != null) {
            return false;
        }
        if ( val1 != null && val2 == null) {
            return false;
        }
                
        final boolean eq = val1.equals(val2);

        if ("eq".equals(action)) {
            return eq;
        } else if ("ne".equals(action)) {
            return !eq;
        }

        boolean gt = false;
        boolean lt = false;
        int comp = 0;

        final int typeKind = typeCode.kind().value();
        switch (typeKind) {
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
            if ("gt".equals(action) || "lt".equals(action) || "ge".equals(action) || "le".equals(action)) {
                // If the user is attempting to do gt, lt, ge, or le on 
                // a complex number, throw an exception.
                //
                // We could implement greater-than/less-than for complex 
                // numbers as a comparison between magnitudes, but the 
                // "or-equal-to" results would be misleading.
                if (typeCode.equivalent(complexFloatHelper.type()) ||
                    typeCode.equivalent(complexDoubleHelper.type()) ||
                    typeCode.equivalent(complexBooleanHelper.type()) ||
                    typeCode.equivalent(complexCharHelper.type()) ||
                    typeCode.equivalent(complexOctetHelper.type()) ||
                    typeCode.equivalent(complexShortHelper.type()) ||
                    typeCode.equivalent(complexUShortHelper.type()) ||
                    typeCode.equivalent(complexLongHelper.type()) ||
                    typeCode.equivalent(complexLongLongHelper.type()) ||
                    typeCode.equivalent(complexULongHelper.type()) ||
                    typeCode.equivalent(complexULongLongHelper.type())
                ) {
                    throw new IllegalArgumentException("Greater-than/less-than not supported for complex types.");
                }
            }

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

    public static boolean isNull(Any any) {
        return any.type().kind().value() == TCKind._tk_null;
    }
}
