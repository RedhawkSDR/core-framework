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

#ifndef __OMNIJNI_ANY_H__
#define __OMNIJNI_ANY_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

#include "class.h"

namespace CORBA {

    namespace jni {

        class Any {
        public:
            static void fromJObject (CORBA::Any& out, JNIEnv* env, jobject obj);
            static jobject toJObject (const CORBA::Any& in, JNIEnv* env);

            static jclass getJClass (JNIEnv* env)
            {
                OnLoad(env);
                return cls_;
            }

            static void OnLoad (JNIEnv* env);
            static void OnUnload (JNIEnv* env);
        private:
            static jclass cls_;

        };

    } // namespace jni

} // namespace CORBA

inline void fromJObject(CORBA::Any& out, JNIEnv* env, jobject obj)
{
    CORBA::jni::Any::fromJObject(out, env, obj);
}

inline jobject toJObject(const CORBA::Any& in, JNIEnv* env)
{
    return CORBA::jni::Any::toJObject(in, env);
}

template<>
inline jclass getJClass<CORBA::Any> (JNIEnv* env)
{
    return CORBA::jni::Any::getJClass(env);
}

#endif // __OMNIJNI_ANY_H__
