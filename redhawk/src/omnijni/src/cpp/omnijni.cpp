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

#include <jni.h>

#include <omnijni/orb.h>
#include <omnijni/threading.h>

namespace {
    // Internal pointer to shared mutex; using a pointer gives predictable
    // instantiation by deferring creation to JNI_OnLoad.
    static omni_mutex* sharedMutex_ = 0;
}

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad (JavaVM* jvm, void* reserved)
{
    // Only initialize if the shared mutex doesn't exist
    if (!sharedMutex_) {
        // Map to the JVM enviroment.
        JNIEnv* env;
        if (jvm->GetEnv((void**)&env, JNI_VERSION_1_4)) {
            return false;
        }

        // Must initialize the CORBA ORB before anything else happens.
        omnijni::ORB::Init(env);

        // Initialize the omniORB/JVM thread interface code.
        omnijni::threading::Init(env);

        // Create the shared mutex at JNI initialization time (rather than whenever
        // static initializers are called)
        sharedMutex_ = new omni_mutex();
    }

    return JNI_VERSION_1_4;
}

namespace omnijni {
    omni_mutex& sharedMutex ()
    {
        return *sharedMutex_;
    }
}
