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

#ifndef __OMNIJNI_STRING_H__
#define __OMNIJNI_STRING_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

namespace omnijni {
    char* stringCopy (JNIEnv* env, jstring jstr);
}

namespace CORBA {

    namespace jni {

        class String {
        public:
            static inline void fromJObject (CORBA::String_out out, JNIEnv* env, jobject obj)
            {
                out = omnijni::stringCopy(env, (jstring)obj);
            }

            static jstring toJObject (const char* in, JNIEnv* env)
            {
                return env->NewStringUTF(in);
            }
        };

    } // namespace jni

} // namespace CORBA

inline void fromJObject(CORBA::String_out out, JNIEnv* env, jobject obj)
{
    CORBA::jni::String::fromJObject(out, env, obj);
}

inline jstring toJObject(const char* in, JNIEnv* env)
{
    return CORBA::jni::String::toJObject(in, env);
}

#endif // __OMNIJNI_STRING_H__
