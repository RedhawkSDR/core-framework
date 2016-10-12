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

#ifndef __OMNIJNI_SEQUENCE_H__
#define __OMNIJNI_SEQUENCE_H__

#include <omniORB4/CORBA.h>
#include <jni.h>

#include "class.h"

namespace omnijni {

    inline jobject toJObject (const char* str, JNIEnv* env)
    {
        return env->NewStringUTF(str);
    }

    inline void fromJObject (_CORBA_String_element str, JNIEnv* env, jobject obj)
    {
        jstring jstr = (jstring)obj;
        const char* utf = env->GetStringUTFChars(jstr, NULL);
        str = utf;
        env->ReleaseStringUTFChars(jstr, utf);
    }

    template <class T, class T_Helper>
    void fromJObject(_CORBA_ObjRef_Element<T,T_Helper> member, JNIEnv* env, jobject obj)
    {
        // TODO
    }

    template <class T>
    void fromJObjectArray_ (T& sequence, JNIEnv* env, jobject obj)
    {
        if (obj == NULL) {
            sequence.length(0);
            return;
        }
        jobjectArray array = (jobjectArray)obj;
        CORBA::ULong larray = env->GetArrayLength(array);
        sequence.length(larray);
        for (CORBA::ULong ii = 0; ii < larray; ++ii) {
            jobject element = env->GetObjectArrayElement(array, ii);
            fromJObject(sequence[ii], env, element);
        }
    }

    template <class T>
    void fromJObject (_CORBA_Sequence<T>& sequence, JNIEnv* env, jobject obj)
    {
        fromJObjectArray_(sequence, env, obj);
    }

    template <class T, class ElemT, class T_Helper>
    void fromJObject (_CORBA_Sequence_ObjRef<T,ElemT,T_Helper>& sequence, JNIEnv* env, jobject obj)
    {
        fromJObjectArray_(sequence, env, obj);
    }

    inline void fromJObject (_CORBA_Sequence_String& sequence, JNIEnv* env, jobject obj)
    {
        fromJObjectArray_(sequence, env, obj);
    }

    template <class T>
    jobjectArray toJObjectArray_ (const T& sequence, JNIEnv* env, jclass clazz)
    {
        jsize size = sequence.length();
        jobjectArray array = env->NewObjectArray(size, clazz, NULL);
        for (jsize ii = 0; ii < size; ++ii) {
            jobject item = toJObject(sequence[ii], env);
            env->SetObjectArrayElement(array, ii, item);
            env->DeleteLocalRef(item);
        }
        return array;
    }

    template <class T>
    jobjectArray toJObject (const _CORBA_Sequence<T>& sequence, JNIEnv* env)
    {
        return toJObjectArray_(sequence, env, getJClass<T>(env));
    }

    template <class T, class ElemT, class T_Helper>
    jobjectArray toJObject (const _CORBA_Sequence_ObjRef<T,ElemT,T_Helper>& sequence, JNIEnv* env)
    {
        return toJObjectArray_(sequence, env, getJClass<T*>(env));
    }

    jbooleanArray toJObject (const _CORBA_Sequence_Boolean& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence_Boolean& sequence, JNIEnv* env, jobject obj);

    jbyteArray toJObject (const _CORBA_Sequence_Octet& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence_Octet& sequence, JNIEnv* env, jobject obj);

    jcharArray toJObject (const _CORBA_Sequence_Char& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence_Char& sequence, JNIEnv* env, jobject obj);

    jshortArray toJObject (const _CORBA_Sequence<CORBA::Short>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::Short>& sequence, JNIEnv* env, jobject obj);

    jshortArray toJObject (const _CORBA_Sequence<CORBA::UShort>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::UShort>& sequence, JNIEnv* env, jobject obj);

    jintArray toJObject (const _CORBA_Sequence<CORBA::Long>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::Long>& sequence, JNIEnv* env, jobject obj);

    jintArray toJObject (const _CORBA_Sequence<CORBA::ULong>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::ULong>& sequence, JNIEnv* env, jobject obj);

    jlongArray toJObject (const _CORBA_Sequence<CORBA::LongLong>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::LongLong>& sequence, JNIEnv* env, jobject obj);

    jlongArray toJObject (const _CORBA_Sequence<CORBA::ULongLong>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::ULongLong>& sequence, JNIEnv* env, jobject obj);

    jfloatArray toJObject (const _CORBA_Sequence<CORBA::Float>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::Float>& sequence, JNIEnv* env, jobject obj);

    jdoubleArray toJObject (const _CORBA_Sequence<CORBA::Double>& sequence, JNIEnv* env);
    void fromJObject (_CORBA_Sequence<CORBA::Double>& sequence, JNIEnv* env, jobject obj);

    inline jobjectArray toJObject (const _CORBA_Sequence_String& sequence, JNIEnv* env)
    {
        jclass clazz = env->FindClass("java/lang/String");
        return toJObjectArray_(sequence, env, clazz);
    }

#define ARRAY_ELEMENT_METHODS(ELEM, NAME) \
    inline void releaseArrayElements(JNIEnv* env, ELEM##Array array, ELEM* elements) \
    {                                                                                \
        env->Release##NAME##ArrayElements(array, elements, JNI_ABORT);               \
    }                                                                                \
    inline ELEM* getArrayElements (JNIEnv* env, ELEM##Array array)                   \
    {                                                                                \
        return env->Get##NAME##ArrayElements(array, NULL);                           \
    }

    ARRAY_ELEMENT_METHODS(jbyte, Byte);
    ARRAY_ELEMENT_METHODS(jchar, Char);
    ARRAY_ELEMENT_METHODS(jshort, Short);
    ARRAY_ELEMENT_METHODS(jint, Int);
    ARRAY_ELEMENT_METHODS(jlong, Long);
    ARRAY_ELEMENT_METHODS(jfloat, Float);
    ARRAY_ELEMENT_METHODS(jdouble, Double);

#undef ARRAY_ELEMENT_METHODS

    // Wrapper class for automated release of Java array elements
    template <typename ElemT, typename ArrayT>
    class ArrayWrapper {
    public:
        typedef ArrayT array_type;
        typedef ElemT elem_type;

        ArrayWrapper (array_type array, JNIEnv* env) :
            array_(array),
            env_(env),
            data_(acquire())
        {
        }

        ~ArrayWrapper ()
        {
            release();
        }

        size_t size ()
        {
            return env_->GetArrayLength(array_);
        }

        elem_type* data ()
        {
            return data_;
        }

        operator elem_type* ()
        {
            return data();
        }

    private:
        elem_type* acquire ()
        {
            return getArrayElements(env_, array_);
        }

        void release ()
        {
            releaseArrayElements(env_, array_, data_);
        }

        array_type array_;
        JNIEnv* env_;
        elem_type* data_;
    };

    typedef ArrayWrapper<jbyte, jbyteArray> ByteArrayWrapper;
    typedef ArrayWrapper<jchar, jcharArray> CharArrayWrapper;
    typedef ArrayWrapper<jshort, jshortArray> ShortArrayWrapper;
    typedef ArrayWrapper<jint, jintArray> IntArrayWrapper;
    typedef ArrayWrapper<jlong, jlongArray> LongArrayWrapper;
    typedef ArrayWrapper<jfloat, jfloatArray> FloatArrayWrapper;
    typedef ArrayWrapper<jdouble, jdoubleArray> DoubleArrayWrapper;

    // Strings behave largely the same as arrays, so the same basic behavior
    // works, with minor adaptations.
    typedef ArrayWrapper<const char, jstring> StringWrapper;

    template<>
    inline size_t StringWrapper::size ()
    {
        return strlen(data_);
    }

    template<>
    inline const char* StringWrapper::acquire ()
    {
        return env_->GetStringUTFChars(array_, NULL);
    }

    template<>
    inline void StringWrapper::release ()
    {
        env_->ReleaseStringUTFChars(array_, data_);
    }
}

#endif // __OMNIJNI_SEQUENCE_H__
