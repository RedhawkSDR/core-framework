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
#include <omnijni/class.h>

namespace {
    static CORBA::ORB_var orb;
} // namespace

jclass omnijni::ORB::cls_ = NULL;
jmethodID omnijni::ORB::object_to_string_ = 0;

void omnijni::ORB::Init (JNIEnv* env)
{
    // Initialize the ORB with no arguments.
    int argc = 0;
    char** argv = 0;
    orb = CORBA::ORB_init(argc, argv);

    // Automatically activate the root POA manager.
    CORBA::Object_var obj = orb->resolve_initial_references("RootPOA");
    PortableServer::POA_var root_poa = PortableServer::POA::_narrow(obj);
    PortableServer::POAManager_var manager = root_poa->the_POAManager();
    manager->activate();

    // Initialize JNI references
    cls_ = omnijni::loadClass(env, "omnijni.ORB");
    object_to_string_ = env->GetStaticMethodID(cls_, "object_to_string", "(Lorg/omg/CORBA/Object;)Ljava/lang/String;");
}

CORBA::Object_ptr omnijni::ORB::object_to_native (JNIEnv* env, jobject obj)
{
    jstring jior = (jstring)env->CallStaticObjectMethod(cls_, object_to_string_, obj);
    const char* ior = env->GetStringUTFChars(jior, NULL);
    CORBA::Object_ptr object = orb->string_to_object(ior);
    env->ReleaseStringUTFChars(jior, ior);
    return object;
}

extern "C" JNIEXPORT jlong JNICALL Java_omnijni_ORB_string_1to_1object_1ref (JNIEnv* env, jclass, jstring jior)
{
    const char* ior = env->GetStringUTFChars(jior, NULL);
    CORBA::Object_ptr object = orb->string_to_object(ior);
    env->ReleaseStringUTFChars(jior, ior);
    return reinterpret_cast<jlong>(object);
}

extern "C" JNIEXPORT jstring JNICALL Java_omnijni_ORB_objectref_1to_1string (JNIEnv* env, jclass, jlong ref)
{
    CORBA::Object_ptr object = reinterpret_cast<CORBA::Object_ptr>(ref);
    CORBA::String_var ior = orb->object_to_string(object);
    return env->NewStringUTF(ior);
}
