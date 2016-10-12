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
#include <sstream>

#include <omnijni/threading.h>
#include <omnijni/class.h>

namespace {
    namespace Threading {
        jobject group_ = NULL;
        int count_ = 0;

        std::string getThreadName()
        {
            ++Threading::count_;
            std::ostringstream name;
            name << "omnijni-Thread-" << Threading::count_;
            return name.str();
        }

        class JVMAttachment : public omni_thread::value_t
        {
        public:
            JVMAttachment(JavaVM* jvm) :
                jvm_(jvm)
            {
            }

            ~JVMAttachment()
            {
                jvm_->DetachCurrentThread();
            }
        private:
            JavaVM* jvm_;
        };
    }
}

void omnijni::threading::Init(JNIEnv* env)
{
    if (!Threading::group_) {
        jclass cls = omnijni::loadClass(env, "java.lang.ThreadGroup");
        jmethodID ctor = env->GetMethodID(cls, "<init>", "(Ljava/lang/String;)V");
        jstring name = env->NewStringUTF("omnijni");
        jobject group = env->NewObject(cls, ctor, name);
        Threading::group_ = env->NewWeakGlobalRef(group);
        env->DeleteLocalRef(name);
        env->DeleteWeakGlobalRef(cls);
    }
}

JNIEnv* omnijni::attachThread (JavaVM* jvm)
{
    JNIEnv* env = NULL;
    jint status = jvm->GetEnv((void**)&env, JNI_VERSION_1_4);
    if (JNI_EDETACHED == status) {
        // This thread is not attached to the JVM; attach it, using a unique,
        // omnijni-specific name and the omnijni thread group
        JavaVMAttachArgs args;
        args.version = JNI_VERSION_1_4;
        const std::string name = Threading::getThreadName();
        args.name = const_cast<char*>(name.c_str());
        args.group = Threading::group_;

        jvm->AttachCurrentThreadAsDaemon((void**)&env, &args);

        // Save the JVM attachment state in thread-local storage; when the
        // thread exits, this ensures that it detaches the thread from the JVM
        omni_thread::key_t key = omni_thread::allocate_key();
        omni_thread::self()->set_value(key, new Threading::JVMAttachment(jvm));
    }
    return env;
}
