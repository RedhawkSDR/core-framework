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

package omnijni.impl;

import org.omg.CORBA.portable.OutputStream;
import org.omg.CORBA.portable.InputStream;

public class DefaultStreamConverter extends omnijni.StreamConverter
{
    public byte[] toBytes (final OutputStream out)
    {
        // Fallback method: we are unable to determine the number of bytes in
        // advance, so read byte by byte; we can assume that anything larger
        // than 2MB would raise a CORBA MARSHALL error, anyway
        InputStream in = out.create_input_stream();
        byte[] buffer = new byte[2097152];
        int length = 0;
        for (; length < buffer.length; length++) {
            try {
                buffer[length] = in.read_octet();
            } catch (final Exception ex) {
                break;
            }
        }

        // Copy the data into an exactly-sized buffer
        byte[] data = new byte[length];
        System.arraycopy(buffer, 0, data, 0, data.length);
        return data;
    }
}

