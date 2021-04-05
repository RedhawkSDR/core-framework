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

 
#ifndef BASICDEVWITHEXECPARAM_CPP_IMPL1_IMPL_BASE_H
#define BASICDEVWITHEXECPARAM_CPP_IMPL1_IMPL_BASE_H

#include <boost/thread/mutex.hpp>
#include <ossie/Resource_impl.h>

#include "ossie/Device_impl.h"
#include "port_impl.h"
#include "struct_props.h"

class BasicDevWithExecParam_cpp_impl1_base;

class ProcessThread : public omni_thread
{
    public:
        ProcessThread(BasicDevWithExecParam_cpp_impl1_base *_target, float _delay);

        bool release(unsigned long secs, unsigned long nanosecs = 0);

        // main omni_thread function
        void run(void *args);
        
    protected:
        // Per omni_thread guidance:
        // ... a thread object must be allocated with new - it cannot 
        // be statically or automatically allocated. The destructor of a 
        // class that inherits from omni_thread shouldn't be public either 
        // (otherwise the thread object can be destroyed while the 
        //  underlying thread is still running).
        // 
        // Basically you should never call "delete" on an omni_thread as
        // it will delete itself when the thread exits.  Making the destructor
        // protected prevents calling "delete"
        virtual ~ProcessThread();
        
    private:
        bool _thread_running;
        omni_mutex _thread_exited_mutex;
        omni_condition* _thread_exited;
        BasicDevWithExecParam_cpp_impl1_base *target;
        __useconds_t udelay;
};

class BasicDevWithExecParam_cpp_impl1_base : public Device_impl
{

    public:
        BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        
   

        ~BasicDevWithExecParam_cpp_impl1_base(void);

        void start();

        void stop();

        void releaseObject();

        void initialize();

        void configure(const CF::Properties&);

        void loadProperties();

        virtual int serviceFunction() = 0;
    
    protected:
        ProcessThread *serviceThread; 
        boost::mutex serviceThreadLock;  
    
        // Member variables exposed as properties
        std::string execparam_only;
        std::string config_execparam;



    private:
        void construct();
};
#endif
