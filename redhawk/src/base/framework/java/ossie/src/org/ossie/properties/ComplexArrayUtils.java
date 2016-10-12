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
 * Identification: $Revision: 5763 $
 */
package org.ossie.properties;

import java.lang.reflect.Array;

import org.apache.commons.lang.ArrayUtils;

/**
 * @since 3.0
 */
public final class ComplexArrayUtils {

    private ComplexArrayUtils() {}

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexFloat[] 
    convertToComplexFloatArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexFloat[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexFloat[])array;
        }

        final CF.complexFloat[] newArray = 
            new CF.complexFloat[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexFloat) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexDouble[] 
    convertToComplexDoubleArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexDouble[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexDouble[])array;
        }

        final CF.complexDouble[] newArray = 
            new CF.complexDouble[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexDouble) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexBoolean[] 
    convertToComplexBooleanArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexBoolean[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexBoolean[])array;
        }

        final CF.complexBoolean[] newArray = 
            new CF.complexBoolean[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexBoolean) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexChar[] 
    convertToComplexCharArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexChar[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexChar[])array;
        }

        final CF.complexChar[] newArray = 
            new CF.complexChar[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexChar) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexOctet[] 
    convertToComplexOctetArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexOctet[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexOctet[])array;
        }

        final CF.complexOctet[] newArray = 
            new CF.complexOctet[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexOctet) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexShort[] 
    convertToComplexShortArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexShort[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexShort[])array;
        }

        final CF.complexShort[] newArray = 
            new CF.complexShort[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexShort) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexUShort[] 
    convertToComplexUShortArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexUShort[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexUShort[])array;
        }

        final CF.complexUShort[] newArray = 
            new CF.complexUShort[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexUShort) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexLong[] 
    convertToComplexLongArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexLong[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexLong[])array;
        }

        final CF.complexLong[] newArray = 
            new CF.complexLong[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexLong) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexULong[] 
    convertToComplexULongArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexULong[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexULong[])array;
        }

        final CF.complexULong[] newArray = 
            new CF.complexULong[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexULong) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexLongLong[] 
    convertToComplexLongLongArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexLongLong[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexLongLong[])array;
        }

        final CF.complexLongLong[] newArray = 
            new CF.complexLongLong[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexLongLong) Array.get(array, i);
        }
        return newArray;
    }

    /*
     * Create an array of the correct type by type-casting
     * each element of the array.
     */ 
    public static CF.complexULongLong[] 
    convertToComplexULongLongArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof CF.complexULongLong[]) {
            // No need for conversion.  Need to type-cast
            // so that the compiler knows that this matches
            // the return type.
            return (CF.complexULongLong[])array;
        }

        final CF.complexULongLong[] newArray = 
            new CF.complexULongLong[Array.getLength(array)];

        for (int i = 0; i < newArray.length; i++) {
            newArray[i] = (CF.complexULongLong) Array.get(array, i);
        }
        return newArray;
    }
}
