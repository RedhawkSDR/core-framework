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
#include <omnijni/exception.h>

#include "jni_corba.h"

jclass CORBA::jni::SystemException::cls_ = NULL;
jfieldID CORBA::jni::SystemException::minor_ = 0;
jfieldID CORBA::jni::SystemException::completed_ = 0;
jmethodID CORBA::jni::SystemException::get_name_ = 0;

void CORBA::jni::SystemException::OnLoad (JNIEnv* env)
{
  if (cls_) return;
  CORBA::jni::CompletionStatus::OnLoad(env);

  cls_ = omnijni::loadClass(env, "org.omg.CORBA.SystemException");
  minor_ = env->GetFieldID(cls_, "minor", "I");
  completed_ = env->GetFieldID(cls_, "completed", "Lorg/omg/CORBA/CompletionStatus;");

  jclass clazz = env->FindClass("java/lang/Class");
  get_name_ = env->GetMethodID(clazz, "getSimpleName", "()Ljava/lang/String;");
  env->DeleteLocalRef(clazz);
}

void CORBA::jni::SystemException::OnUnload (JNIEnv* env)
{
}

void CORBA::jni::SystemException::throwNative (JNIEnv* env, jobject obj)
{
    CORBA::jni::SystemException::OnLoad(env);

    // Get the minor number and completion status to be passed along as-is.
    int minor = env->GetIntField(obj, minor_);
    CORBA::CompletionStatus completed;
    omnijni::getObjectField(completed, env, obj, completed_);

    // Get the name of the exception's class
    jclass clazz = env->GetObjectClass(obj);
    jstring str = (jstring)env->CallObjectMethod(clazz, get_name_);
    env->DeleteLocalRef(clazz);
    const char* cname = env->GetStringUTFChars(str, NULL);
    const std::string name = cname;
    env->ReleaseStringUTFChars(str, cname);

    // Check the class name against all known CORBA SystemExceptions.
#define THROW_IF_MATCHES(exname) if (name == #exname) throw CORBA::exname (minor, completed);
    OMNIORB_FOR_EACH_SYS_EXCEPTION(THROW_IF_MATCHES);
#undef THROW_IF_MATCHES

    // If all else fails, throw CORBA::UNKNOWN.
    throw CORBA::UNKNOWN(minor, completed);
}

void CORBA::jni::SystemException::throwJava (const CORBA::SystemException& ex, JNIEnv* env)
{
    CORBA::jni::SystemException::OnLoad(env);

    // Look up the equivalent Java class for this exception.
    std::string name = std::string("org/omg/CORBA/") + ex._name();
    jclass clazz = env->FindClass(name.c_str());
    jmethodID ctor = env->GetMethodID(clazz, "<init>", "(ILorg/omg/CORBA/CompletionStatus;)V");

    // Create and throw an instance of the exception type with the minor code
    // and completion status, which are all we are guaranteed to have.
    jint minor = ex.minor();
    jobject status = CORBA::jni::CompletionStatus::toJObject(ex.completed(), env);
    jthrowable exc = (jthrowable)env->NewObject(clazz, ctor, minor, status);
    env->DeleteLocalRef(clazz);
    env->Throw(exc);
}
