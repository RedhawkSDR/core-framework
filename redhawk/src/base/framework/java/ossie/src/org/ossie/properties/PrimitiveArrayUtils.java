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
import java.util.List;
import java.util.ArrayList;

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
            return convertToShortArray((Short[]) array);
        }
        final short[] newArray = new short[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.shortValue();
        }
        return newArray;
    }

    public static short[] convertToShortArray(final Short[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static short[] convertToShortArray(final List<Short> array) {
        final Short[] newArray = array.toArray(new Short[array.size()]);
        return convertToShortArray(newArray);
    }

    public static List<Short> convertToShortList(final short[] array) {
        final List<Short> newList = new ArrayList<Short>(array.length);
        for (short item : array) {
            newList.add(item);
        }
        return newList;
    }

    public static List<Short> convertToShortList(final Number[] array) {
        final List<Short> newList = new ArrayList<Short>(array.length);
        for (Number num : array) {
            newList.add(num.shortValue());
        }
        return newList;
    }

    public static byte[] convertToByteArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof byte[]) {
            return (byte[]) array;
        }
        if (array instanceof Byte[]) {
            return convertToByteArray((Byte[]) array);
        }
        final byte[] newArray = new byte[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.byteValue();
        }
        return newArray;
    }

    public static byte[] convertToByteArray(final Byte[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static byte[] convertToByteArray(final List<Byte> array) {
        final Byte[] newArray = array.toArray(new Byte[array.size()]);
        return convertToByteArray(newArray);
    }

    public static List<Byte> convertToByteList(final byte[] array) {
        final List<Byte> newList = new ArrayList<Byte>(array.length);
        for (byte item: array) {
            newList.add(item);
        }
        return newList;
    }

    public static List<Byte> convertToByteList(final Number[] array) {
        final List<Byte> newList = new ArrayList<Byte>(array.length);
        for (Number num : array) {
            newList.add(num.byteValue());
        }
        return newList;
    }

    public static long[] convertToLongArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof long[]) {
            return (long[]) array;
        }
        if (array instanceof Long[]) {
            return convertToLongArray((Long[]) array);
        }
        final long[] newArray = new long[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.longValue();
        }
        return newArray;
    }

    public static long[] convertToLongArray(final Long[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static long[] convertToLongArray(final List<Long> array) {
        final Long[] newArray = array.toArray(new Long[array.size()]);
        return convertToLongArray(newArray);
    }

    public static List<Long> convertToLongList(final long[] array) {
        final List<Long> newList = new ArrayList<Long>(array.length);
        for (long item : array) {
            newList.add(item);
        }
        return newList;
    }

    public static List<Long> convertToLongList(final Number[] array) {
        final List<Long> newList = new ArrayList<Long>(array.length);
        for (Number num : array) {
            newList.add(num.longValue());
        }
        return newList;
    }

    public static int[] convertToIntArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof int[]) {
            return (int[]) array;
        }
        if (array instanceof Integer[]) {
            return convertToIntArray((Integer[]) array);
        }
        final int[] newArray = new int[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.intValue();
        }
        return newArray;
    }

    public static int[] convertToIntArray(final Integer[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static int[] convertToIntArray(final List<Integer> array) {
        final Integer[] newArray = array.toArray(new Integer[array.size()]);
        return convertToIntArray(newArray);
    }

    public static List<Integer> convertToIntList(final int[] array) {
        final List<Integer> newList = new ArrayList<Integer>(array.length);
        for (int item: array) {
            newList.add(item);
        }
        return newList;
    }

    public static List<Integer> convertToIntList(final Number[] array) {
        final List<Integer> newList = new ArrayList<Integer>(array.length);
        for (Number num : array) {
            newList.add(num.intValue());
        }
        return newList;
    }

    public static float[] convertToFloatArray(final Object array) {
        if (array == null) {
            return null;
        }
        if (array instanceof float[]) {
            return (float[]) array;
        }
        if (array instanceof Float[]) {
            return convertToFloatArray((Float[]) array);
        }
        final float[] newArray = new float[Array.getLength(array)];
        for (int i = 0; i < newArray.length; i++) {
            final Number val = (Number) Array.get(array, i);
            newArray[i] = val.floatValue();
        }
        return newArray;
    }

    public static float[] convertToFloatArray(final Float[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static float[] convertToFloatArray(final List<Float> array) {
        final Float[] newArray = array.toArray(new Float[array.size()]);
        return convertToFloatArray(newArray);
    }

    public static List<Float> convertToFloatList(final float[] array) {
        final List<Float> newList = new ArrayList<Float>(array.length);
        for (float item : array) {
            newList.add(item);
        }
        return newList;
    }

    public static List<Float> convertToFloatList(final Number[] array) {
        final List<Float> newList = new ArrayList<Float>(array.length);
        for (Number num : array) {
            newList.add(num.floatValue());
        }
        return newList;
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

    public static double[] convertToDoubleArray(final Double[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static double[] convertToDoubleArray(final List<Double> array) {
        final Double[] newArray = array.toArray(new Double[array.size()]);
        return convertToDoubleArray(newArray);
    }

    public static List<Double> convertToDoubleList(final double[] array) {
        final List<Double> newList = new ArrayList<Double>(array.length);
        for (double item : array) {
            newList.add(item);
        }
        return newList;
    }

    public static List<Double> convertToDoubleList(final Number[] array) {
        final List<Double> newList = new ArrayList<Double>(array.length);
        for (Number num : array) {
            newList.add(num.doubleValue());
        }
        return newList;
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

    public static char[] convertToCharArray(final Character[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static char[] convertToCharArray(final List<Character> array) {
        final Character[] newArray = array.toArray(new Character[array.size()]);
        return convertToCharArray(newArray);
    }

    public static List<Character> convertToCharList(final char[] array) {
        final List<Character> newList = new ArrayList<Character>(array.length);
        for (char item : array) {
            newList.add(item);
        }
        return newList;
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

    public static boolean[] convertToBooleanArray(final Boolean[] array) {
        return ArrayUtils.toPrimitive(array);
    }

    public static boolean[] convertToBooleanArray(final List<Boolean> array) {
        final Boolean[] newArray = array.toArray(new Boolean[array.size()]);
        return convertToBooleanArray(newArray);
    }

    public static List<Boolean> convertToBooleanList(final boolean[] array) {
        final List<Boolean> newList = new ArrayList<Boolean>(array.length);
        for (boolean item : array) {
            newList.add(item);
        }
        return newList;
    }
}
