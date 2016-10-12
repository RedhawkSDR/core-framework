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

import org.omg.CORBA.portable.OutputStream;

import omnijni.impl.ArrayStreamConverter;
import omnijni.impl.DefaultStreamConverter;
import omnijni.impl.SunStreamConverter;

public abstract class StreamConverter {
    
    private static StreamConverter impl = null;

    private static StreamConverter check_impl ()
    {
        if (SunStreamConverter.test()) {
            return new SunStreamConverter();
        } else if (ArrayStreamConverter.test()) {
            return new ArrayStreamConverter();
        } else {
            return new DefaultStreamConverter();
        }
    }

    private static synchronized StreamConverter get_impl ()
    {
        if (impl == null) {
            impl = check_impl();
        }
        return impl;
    }

    public static byte[] to_bytes (final OutputStream out)
    {
        return get_impl().toBytes(out);
    }

    public abstract byte[] toBytes (final OutputStream out);
}
