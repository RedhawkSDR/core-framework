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

#ifndef __OMNIJNI_EXCEPTION_H__
#define __OMNIJNI_EXCEPTION_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

namespace CORBA {

    namespace jni {

        class SystemException
        {
        public:
            static void throwJava (const CORBA::SystemException& ex, JNIEnv* env);
            static void throwNative (JNIEnv* env, jobject obj);
            static void OnLoad (JNIEnv* env);
            static void OnUnload (JNIEnv* env);

        private:
            SystemException();
            ~SystemException();
            static jclass cls_;
            static jfieldID minor_;
            static jfieldID completed_;
            static jmethodID get_name_;
        };

    }

}

#endif // __OMNIJNI_EXCEPTION_H__
