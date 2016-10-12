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
 * Identification: $Revision: 5706 $
 */
package org.ossie.properties;

import java.io.ByteArrayOutputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.math.BigInteger;

/**
 * Utility class to convert between signed and unsigned type. <B>Note<B> probably only supports 32-bit systems. 
 * @since 3.0
 * 
 */
public final class UnsignedUtils {

    public static final short MAX_OCTET = 255;
    public static final int MAX_USHORT = 65535;
    public static final long MAX_ULONG = 4294967295L;
    public static final BigInteger MAX_ULONGLONG = new BigInteger("18446744073709551615");

    private UnsignedUtils() {

    }

    public static byte parseOctet(final String str) {
        short retVal = Short.decode(str);
        if (retVal < 0 || retVal > MAX_OCTET) {
            throw new IllegalArgumentException("octet value must be greater than '0' and less than " + MAX_OCTET);
        }
        return (byte)retVal;
    }

    public static int parseULong(final String str) {
        final long retVal = Long.decode(str);
        if (retVal < 0 || retVal > MAX_ULONG) {
            throw new IllegalArgumentException("ulong value must be greater than '0' and less than " + MAX_ULONG);
        }
        return (int)retVal;
    }

    public static short parseUShort(final String str) {
        final int retVal = Integer.decode(str);
        if (retVal < 0 || retVal > MAX_USHORT) {
            throw new IllegalArgumentException("ushort value must be greater than '0' and less than " + MAX_USHORT);
        }
        return (short)retVal;
    }

    public static long parseULongLong(final String str) {
        final BigInteger retVal = AnyUtils.bigIntegerDecode(str);
        if (retVal.compareTo(BigInteger.ZERO) < 0 || retVal.compareTo(MAX_ULONGLONG) > 0) {
            throw new IllegalArgumentException("ulonglong value must be greater than '0' and less than " + MAX_ULONGLONG);
        }
        return retVal.longValue();
    }

    public static int compareOctet(final byte lhs, final byte rhs) {
        return ((Short)toSigned(lhs)).compareTo(toSigned(rhs));
    }

    public static int compareUShort(final short lhs, final short rhs) {
        return ((Integer)toSigned(lhs)).compareTo(toSigned(rhs));
    }

    public static int compareULong(final int lhs, final int rhs) {
        return ((Long)toSigned(lhs)).compareTo(toSigned(rhs));
    }

    public static int compareULongLong(final long lhs, final long rhs) {
        return toSigned(lhs).compareTo(toSigned(rhs));
    }

    public static int[] toSigned(final short[] ushort) {
        final int[] retVal = new int[ushort.length];
        for (int i = 0; i < retVal.length; i++) {
            retVal[i] = toSigned(ushort[i]);
        }
        return retVal;
    }

    public static short[] toUnsigned(final int[] ushort) {
        final short[] retVal = new short[ushort.length];
        for (int i = 0; i < retVal.length; i++) {
            retVal[i] = toUnsigned(ushort[i]);
        }
        return retVal;
    }

    public static long[] toSigned(final int[] uint) {
        final long[] retVal = new long[uint.length];
        for (int i = 0; i < retVal.length; i++) {
            retVal[i] = toSigned(uint[i]);
        }
        return retVal;
    }

    public static int[] toUnsigned(final long[] uint) {
        final int[] retVal = new int[uint.length];
        for (int i = 0; i < retVal.length; i++) {
            retVal[i] = toUnsigned(uint[i]);
        }
        return retVal;
    }

    public static BigInteger[] toSigned(final long[] ulong) {
        final BigInteger[] retVal = new BigInteger[ulong.length];
        for (int i = 0; i < retVal.length; i++) {
            retVal[i] = toSigned(ulong[i]);
        }
        return retVal;
    }

    public static long[] toUnsigned(final BigInteger[] ulong) {
        final long[] retVal = new long[ulong.length];
        for (int i = 0; i < retVal.length; i++) {
            retVal[i] = toUnsigned(ulong[i]);
        }
        return retVal;
    }

    public static byte toUnsigned(final short ubyte) {
        return (byte) ubyte;
    }

    public static short toSigned(final byte ubyte) {
        return (short)(MAX_OCTET & ubyte);
    }

    public static int toSigned(final short ushort) {
        return MAX_USHORT & ushort;
    }

    public static short toUnsigned(final int ushort) {
        return (short) ushort;
    }

    public static long toSigned(final int uint) {
        return MAX_ULONG & uint;
    }

    public static int toUnsigned(final long uint) {
        return (int) uint;
    }

    public static BigInteger toSigned(final long ulong) {
        final ByteArrayOutputStream array = new ByteArrayOutputStream();
        final DataOutputStream stream = new DataOutputStream(array);
        try {
            stream.writeLong(ulong);
            array.flush();
        } catch (final IOException e) {
            // PASS, will never happen
        } finally {
            try {
                array.close();
            } catch (final IOException e) {
                // PASS
            }
        }

        return new BigInteger(1, array.toByteArray());
    }

    public static long toUnsigned(final BigInteger ulong) {
        return ulong.longValue();
    }
}
