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


#include "BasicDevWithExecParam_cpp_impl1_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

/**************************************************************************************
    Main processing thread

    General functionality:
    this class call a service function in the main component class. When the return
    value is the string "NOOP", then the thread class waits for a set amount of time
    before making another call

**************************************************************************************/
ProcessThread::ProcessThread(BasicDevWithExecParam_cpp_impl1_base *_target, float _delay)
{
    target = _target;
    udelay = (__useconds_t)(_delay * 1000000);
    _thread_running = true;
    _thread_exited = new omni_condition(&_thread_exited_mutex);
}

ProcessThread::~ProcessThread()
{
    if (_thread_exited != 0) {
        delete _thread_exited;
    }
}

bool ProcessThread::release(unsigned long secs, unsigned long nanosecs)
{
    _thread_running = false;
    unsigned long abs_secs = 0;
    unsigned long abs_nsec = 0;
    get_time(&abs_secs, &abs_nsec, secs, nanosecs);
    return _thread_exited->timedwait(abs_secs, abs_nsec);
}

void ProcessThread::run(void *args)
{
    int state = NORMAL;
    while (_thread_running and (state != FINISH)) {
        state = target->serviceFunction();
        if (state == NOOP) {
            usleep(udelay);
        }
    }
    _thread_exited->signal();
}

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

BasicDevWithExecParam_cpp_impl1_base::BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

BasicDevWithExecParam_cpp_impl1_base::BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

BasicDevWithExecParam_cpp_impl1_base::BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

BasicDevWithExecParam_cpp_impl1_base::BasicDevWithExecParam_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

void BasicDevWithExecParam_cpp_impl1_base::construct() {

    loadProperties();
    serviceThread = 0;

}

BasicDevWithExecParam_cpp_impl1_base::~BasicDevWithExecParam_cpp_impl1_base(void)
{
    usleep(1);
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void BasicDevWithExecParam_cpp_impl1_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void BasicDevWithExecParam_cpp_impl1_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread(this, 0.1);
        serviceThread->start();
    }
}

void BasicDevWithExecParam_cpp_impl1_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
}


void BasicDevWithExecParam_cpp_impl1_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    // deactivate ports
    releaseInPorts();
    releaseOutPorts();


    Device_impl::releaseObject();
}

void BasicDevWithExecParam_cpp_impl1_base::configure(const CF::Properties& props) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration)
{
    PropertySet_impl::configure(props);
}

void BasicDevWithExecParam_cpp_impl1_base::loadProperties()
{
    addProperty(execparam_only,
                "execparam default value", 
               "DCE:68dc0d3b-deb2-4fae-b898-62273b74614b",
               "execparam_only",
               "readwrite",
               "null",
               "external",
               "execparam");

    addProperty(config_execparam,
                "configure-execparam default value", 
               "DCE:07350439-e917-45ef-b71f-e387a737fd9c",
               "config_execparam",
               "readwrite",
               "null",
               "external",
               "configure,execparam");

}
