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

#ifndef __OMNIJNI_STRUCT_H__
#define __OMNIJNI_STRUCT_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

namespace omnijni {

    inline void fromJObject (_CORBA_String_member& str, JNIEnv* env, jobject obj)
    {
        jstring jstr = (jstring)obj;
        const char* utf = env->GetStringUTFChars(jstr, NULL);
        str = utf;
        env->ReleaseStringUTFChars(jstr, utf);
    }

    inline void fromJObject (CORBA::Object_Member& member, JNIEnv* env, jobject obj)
    {
        // TODO
        member = CORBA::Object::_nil();
    }

    template <class T, class T_Helper>
    void fromJObject(_CORBA_ObjRef_Member<T,T_Helper>& member, JNIEnv* env, jobject obj)
    {
        // TODO
    }

    template <class T>
    void getObjectField (T& out, JNIEnv* env, jobject obj, jfieldID fid) {
        jobject field = env->GetObjectField(obj, fid);
        fromJObject(out, env, field);
        env->DeleteLocalRef(field);
    }

}

#endif // __OMNIJNI_STRUCT_H__
