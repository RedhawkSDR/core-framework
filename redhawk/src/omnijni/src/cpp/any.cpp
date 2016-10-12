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

#include <omnijni/any.h>

namespace {
    namespace AnyUtils {
        jclass cls_ = NULL;
        jmethodID from_bytes_ = 0;
        jmethodID to_bytes_ = 0;

        void OnLoad (JNIEnv* env)
        {
            if (cls_) return;

            cls_ = omnijni::loadClass(env, "omnijni.AnyUtils");
            from_bytes_ = env->GetStaticMethodID(cls_, "from_bytes", "([B)Lorg/omg/CORBA/Any;");
            to_bytes_ = env->GetStaticMethodID(cls_, "to_bytes", "(Lorg/omg/CORBA/Any;)[B");
        }

        jobject from_bytes (JNIEnv* env, jbyteArray data)
        {
            return env->CallStaticObjectMethod(cls_, from_bytes_, data);
        }

        jbyteArray to_bytes (JNIEnv* env, jobject any)
        {
            return (jbyteArray)env->CallStaticObjectMethod(cls_, to_bytes_, any);
        }
    }
}

jclass CORBA::jni::Any::cls_ = NULL;

void CORBA::jni::Any::OnLoad (JNIEnv* env)
{
    if (cls_) return;

    AnyUtils::OnLoad(env);
    cls_ = omnijni::loadClass(env, "org.omg.CORBA.Any");
}

void CORBA::jni::Any::fromJObject (CORBA::Any& any, JNIEnv* env, jobject obj)
{
    OnLoad(env);

    if (obj == NULL) {
        any = CORBA::Any();
        return;
    }

    jbyteArray data = AnyUtils::to_bytes(env, obj);
    if (!data) {
        // Unable to get Java Any as raw byte data
        env->ExceptionDescribe();
        return;
    }
    jint size = env->GetArrayLength(data);
    jbyte* buffer = env->GetByteArrayElements(data, NULL);
    
    cdrMemoryStream stream(buffer, size);
    stream.setByteSwapFlag(false);
    any <<= stream;

    env->ReleaseByteArrayElements(data, buffer, JNI_ABORT);
}

jobject CORBA::jni::Any::toJObject (const CORBA::Any& any, JNIEnv* env)
{
    OnLoad(env);
    cdrMemoryStream output;
    output.setByteSwapFlag(false);
    any >>= output;
    jbyteArray data = env->NewByteArray(output.bufSize());
    env->SetByteArrayRegion(data, 0, output.bufSize(), (jbyte*)output.bufPtr());
    jobject out = AnyUtils::from_bytes(env, data);

    env->DeleteLocalRef(data);

    return out;
}
