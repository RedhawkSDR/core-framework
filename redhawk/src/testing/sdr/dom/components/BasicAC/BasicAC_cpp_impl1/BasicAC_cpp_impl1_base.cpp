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


#include "BasicAC_cpp_impl1_base.h"

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
ProcessThread::ProcessThread(BasicAC_cpp_impl1_base *_target, float _delay)
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
        _thread_exited = 0;
    }
}

bool ProcessThread::release(unsigned long secs, unsigned long nanosecs)
{
    unsigned long abs_secs = 0;
    unsigned long abs_nsec = 0;
    get_time(&abs_secs, &abs_nsec, secs, nanosecs);
    _thread_running = false;
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
 
BasicAC_cpp_impl1_base::BasicAC_cpp_impl1_base(const char *uuid, const char *label) :
                                     Resource_impl(uuid, label), serviceThread(0) {
    construct();
}

void BasicAC_cpp_impl1_base::construct() {

    loadProperties();
    serviceThread = 0;
    resourceOut = new CF_Resource_Out_i("resourceOut", this);
    registerOutPort(resourceOut, resourceOut->_this());
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void BasicAC_cpp_impl1_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void BasicAC_cpp_impl1_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread(this, 0.1);
        serviceThread->start();
    }
}

void BasicAC_cpp_impl1_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
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

CORBA::Object_ptr BasicAC_cpp_impl1_base::getPort(const char* _id) throw (CORBA::SystemException, CF::PortSupplier::UnknownPort)
{

    std::map<std::string, Port_Provides_base_impl *>::iterator p_in = inPorts.find(std::string(_id));
    if (p_in != inPorts.end()) {

    }

    std::map<std::string, CF::Port_var>::iterator p_out = outPorts_var.find(std::string(_id));
    if (p_out != outPorts_var.end()) {
        return CF::Port::_duplicate(p_out->second);
    }

    throw (CF::PortSupplier::UnknownPort());
}

void BasicAC_cpp_impl1_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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

    delete(resourceOut);
 
    Resource_impl::releaseObject();
}

void BasicAC_cpp_impl1_base::configure(const CF::Properties& props) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration)
{
    PropertySet_impl::configure(props);
}

void BasicAC_cpp_impl1_base::loadProperties()
{
}
