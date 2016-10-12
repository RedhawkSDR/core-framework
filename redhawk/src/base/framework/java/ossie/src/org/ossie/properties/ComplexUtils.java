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

import java.math.BigInteger;

import CF.complexBoolean;
import CF.complexChar;
import CF.complexDouble;
import CF.complexFloat;
import CF.complexLong;
import CF.complexLongLong;
import CF.complexOctet;
import CF.complexShort;
import CF.complexULong;
import CF.complexULongLong;
import CF.complexUShort;

public final class ComplexUtils {
    private ComplexUtils() {
    }

    /*
     * Get A and B from A+jB string.
     *
     * Can handle:
     *      A
     *      A-jB
     *      A+jB
     *      0+jB
     *      0-jB
     */ 
    public static String[] getRealImagStringsFromComplex(final String ajbString)
    {
        String[] output     = new String[2];
        String   realString = "0";
        String   imagString = "0";

        int jIndex = ajbString.indexOf("j");
        if (jIndex != -1) {
            // found j

            // split A+jB into [A+][B]
            String[] aSignB = ajbString.split("j");

            // get A out of [A-][B] by selecting [A-] and then
            // dropping the last character in [A-]
            realString = aSignB[0].substring(0, aSignB[0].length()-1);

            imagString = aSignB[1];

            if (ajbString.substring(jIndex-1, jIndex).equals("-")) {
                // found "-" sign directly before "j"
                imagString = "-" + imagString; 
            } 
        } else {
            // no j: real data only
            realString = ajbString;
        }
        output[0] = realString;
        output[1] = imagString;
        return output;
    }

    public static complexBoolean parseComplexBoolean(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        boolean real = Boolean.parseBoolean(parts[0]);
        boolean imag = Boolean.parseBoolean(parts[1]);
        return new complexBoolean(real, imag);
    }

    public static complexChar parseComplexChar(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        char real = parts[0].charAt(0);
        char imag = parts[1].charAt(0);
        return new complexChar(real, imag);
    }

    public static complexDouble parseComplexDouble(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        double real = Double.parseDouble(parts[0]);
        double imag = Double.parseDouble(parts[1]);
        return new complexDouble(real, imag);
    }

    public static complexFloat parseComplexFloat(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        float real = Float.parseFloat(parts[0]);
        float imag = Float.parseFloat(parts[1]);
        return new complexFloat(real, imag);
    }

    public static complexLong parseComplexLong(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        int real = Integer.parseInt(parts[0]);
        int imag = Integer.parseInt(parts[1]);
        return new complexLong(real, imag);
    }

    public static complexLongLong parseComplexLongLong(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        long real = Long.parseLong(parts[0]);
        long imag = Long.parseLong(parts[1]);
        return new complexLongLong(real, imag);
    }

    public static complexOctet parseComplexOctet(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        byte real = Byte.decode(parts[0]);
        byte imag = Byte.decode(parts[1]);
        return new complexOctet(real, imag);
    }

    public static complexShort parseComplexShort(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        short real = Short.parseShort(parts[0]);
        short imag = Short.parseShort(parts[1]);
        return new complexShort(real, imag);
    }

    public static complexULong parseComplexULong(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        int real = UnsignedUtils.parseULong(parts[0]);
        int imag = UnsignedUtils.parseULong(parts[1]);
        return new complexULong(real, imag);
    }

    public static complexULongLong parseComplexULongLong(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        long real = UnsignedUtils.parseULongLong(parts[0]);
        long imag = UnsignedUtils.parseULongLong(parts[1]);
        return new complexULongLong(real, imag);
    }

    public static complexUShort parseComplexUShort(String str) {
        String[] parts = getRealImagStringsFromComplex(str);
        short real = UnsignedUtils.parseUShort(parts[0]);
        short imag = UnsignedUtils.parseUShort(parts[1]);
        return new complexUShort(real, imag);
    }
}
