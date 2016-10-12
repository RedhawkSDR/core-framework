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
public final class PrimitiveArrayUtils {

    private PrimitiveArrayUtils() {

    }

    public static short[] convertToShortArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof short[]) {
            return (short[]) array;
        }
        if (array instanceof Short[]) {
            return ArrayUtils.toPrimitive((Short[]) array);
        }
        final short[] newArray = new short[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.shortValue();
        }
        return newArray;
    }

    public static byte[] convertToByteArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof byte[]) {
            return (byte[]) array;
        }
        if (array instanceof Byte[]) {
            return ArrayUtils.toPrimitive((Byte[]) array);
        }
        final byte[] newArray = new byte[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.byteValue();
        }
        return newArray;
    }

    public static long[] convertToLongArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof long[]) {
            return (long[]) array;
        }
        if (array instanceof Long[]) {
            return ArrayUtils.toPrimitive((Long[]) array);
        }
        final long[] newArray = new long[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.longValue();
        }
        return newArray;
    }

    public static int[] convertToIntArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof int[]) {
            return (int[]) array;
        }
        if (array instanceof Integer[]) {
            return ArrayUtils.toPrimitive((Integer[]) array);
        }
        final int[] newArray = new int[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.intValue();
        }
        return newArray;
    }

    public static float[] convertToFloatArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof float[]) {
            return (float[]) array;
        }
        if (array instanceof Float[]) {
            return ArrayUtils.toPrimitive((Float[]) array);
        }
        final float[] newArray = new float[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.floatValue();
        }
        return newArray;
    }

    public static double[] convertToDoubleArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof double[]) {
            return (double[]) array;
        }
        if (array instanceof Double[]) {
            return ArrayUtils.toPrimitive((Double[]) array);
        }
        final double[] newArray = new double[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.doubleValue();
        }
        return newArray;
    }

    public static char[] convertToCharArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof char[]) {
            return (char[]) array;
        }
        if (array instanceof Character[]) {
            return ArrayUtils.toPrimitive((Character[]) array);
        }
        final char[] newArray = new char[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            Object obj = Array.get(array, i);
            if (obj instanceof Character) {
                newArray[i] = (Character) Array.get(array, i);
            } else {
                newArray[i] = (char) ((Number) Array.get(array, i)).byteValue();
            }
        }
        return newArray;
    }

    public static boolean[] convertToBooleanArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof boolean[]) {
            return (boolean[]) array;
        }
        if (array instanceof Boolean[]) {
            return ArrayUtils.toPrimitive((Boolean[]) array);
        }
        final boolean[] newArray = new boolean[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            Object obj = Array.get(array, i);
            if (obj instanceof Boolean) {
                newArray[i] = (Boolean) Array.get(array, i);
            } else {
                newArray[i] = ((Number) Array.get(array, i)).byteValue() != 0;
            }
        }
        return newArray;
    }
}
