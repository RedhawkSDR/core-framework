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

package omnijni;

import org.omg.CORBA.Any;
import org.omg.CORBA.portable.OutputStream;

public abstract class AnyUtils {

    public static Any from_bytes (byte[] buffer)
    {
        OutputStream out = ORB.create_output_stream();
        out.write_octet_array(buffer, 0, buffer.length);
        return out.create_input_stream().read_any();
    }

    public static byte[] to_bytes (Any any)
    {
        OutputStream out = ORB.create_output_stream();
        out.write_any(any);
        return StreamConverter.to_bytes(out);
    }
}
