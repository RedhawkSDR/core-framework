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

public abstract class ORB {

    public static org.omg.CORBA.Object string_to_object (String ior)
    {
        long ref = string_to_object_ref(ior);
        return new CORBAObject(ref);
    }

    public static String object_to_string (org.omg.CORBA.Object obj)
    {
        if (obj instanceof omnijni.ObjectImpl) {
            return objectref_to_string(((omnijni.ObjectImpl)obj)._get_object_ref());
        } else {
            org.omg.CORBA.ORB orb = ((org.omg.CORBA.portable.ObjectImpl)obj)._orb();
            return orb.object_to_string(obj);
        }
    }

    static {
        System.loadLibrary("omnijni");
    }

    private static native long string_to_object_ref (String ior);
    private static native String objectref_to_string (long ref);
}