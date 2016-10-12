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

#ifndef __OMNIJNI_OBJECT_H__
#define __OMNIJNI_OBJECT_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

#include "class.h"

namespace CORBA {

    namespace jni {

        class Object {
        public:
            static void fromJObject (CORBA::Object_var& out, JNIEnv* env, jobject bj);
            static jobject toJObject (CORBA::Object_ptr in, JNIEnv* env);

            static jclass getJClass (JNIEnv* env)
            {
                OnLoad(env);
                return cls_;
            }

            static void OnLoad (JNIEnv* env);
            static void OnUnload (JNIEnv* env);
        private:
            static jclass cls_;
            static jmethodID ctor_;
        };


    } // namespace jni

} // namespace CORBA

inline void fromJObject (CORBA::Object_var& out, JNIEnv* env, jobject obj)
{
    CORBA::jni::Object::fromJObject(out, env, obj);
}

inline jobject toJObject (CORBA::Object_ptr in, JNIEnv* env)
{
    return CORBA::jni::Object::toJObject(in, env);
}

template<>
inline jclass getJClass<CORBA::ObjectRef> (JNIEnv* env)
{
    return CORBA::jni::Object::getJClass(env);
}

#endif // __OMNIJNI_OBJECT_H__
