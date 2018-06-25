/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

package helpers;

public class ArrayData {

    public static void ramp(char[] data)
    {
        // Fun fact: char is an unsigned 16-bit integer type in Java, but
        // REDHAWK uses it for signed 8-bit integers. So, in order to generate
        // valid data, we have to explicitly mask off the upper bits.
        for (int ii = 0; ii < data.length; ii++) {
            data[ii] = (char) (ii & 0xFF);
        }
    }

    public static void ramp(byte[] data)
    {
        for (int ii = 0; ii < data.length; ii++) {
            data[ii] = (byte) ii;
        }
    }

    public static void ramp(short[] data)
    {
        for (int ii = 0; ii < data.length; ii++) {
            data[ii] = (short) ii;
        }
    }

    public static void ramp(int[] data)
    {
        for (int ii = 0; ii < data.length; ii++) {
            data[ii] = ii;
        }
    }

    public static void ramp(long[] data)
    {
        for (int ii = 0; ii < data.length; ii++) {
            data[ii] = (long) ii;
        }
    }

    public static void ramp(float[] data)
    {
        for (int ii = 0; ii < data.length; ii++) {
            data[ii] = (float) ii;
        }
    }

    public static void ramp(double[] data)
    {
        for (int ii = 0; ii < data.length; ii++) {
            data[ii] = (double) ii;
        }
    }

    // Cannot instantiate
    private ArrayData()
    {
    }
}
