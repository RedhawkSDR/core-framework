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

#include <string>
#include <algorithm>
#include <stdexcept>

#include <omnijni/class.h>

jclass omnijni::loadClass (JNIEnv* env, const std::string& name)
{
    std::string desc;
    std::replace_copy(name.begin(), name.end(), std::back_insert_iterator<std::string>(desc), '.', '/');

    jclass cls = env->FindClass(desc.c_str());
    if (cls == NULL) {
        throw std::runtime_error(name + " not found");
    }
    jclass ref = (jclass)env->NewWeakGlobalRef(cls);
    if (ref == NULL) {
        throw std::runtime_error("Failed to create global reference for " + name);
    }
    return ref;
}

