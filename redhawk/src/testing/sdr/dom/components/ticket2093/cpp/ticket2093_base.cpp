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


#include "ticket2093_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY
    
     Source: ticket2093.spd.xml
     Generated on: Thu Apr 12 13:06:33 EDT 2012
     Redhawk IDE
     Version:T.1.X.X
     Build id: v201204121229-r7961

*******************************************************************************************/

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/
 
ticket2093_base::ticket2093_base(const char *uuid, const char *label) :
                                     Resource_impl(uuid, label), serviceThread(0) {
    construct();
}

void ticket2093_base::construct()
{
    Resource_impl::_started = false;
    loadProperties();
    serviceThread = 0;
    
    PortableServer::ObjectId_var oid;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void ticket2093_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void ticket2093_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<ticket2093_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
        Resource_impl::start();
    }
}

void ticket2093_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    // release the child thread (if it exists)
    if (serviceThread != 0) {
        if (!serviceThread->release(2)) {
            throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
        }
        serviceThread = 0;
    }
    
    if (Resource_impl::started()) {
        Resource_impl::stop();
    }
}


void ticket2093_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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

 
    Resource_impl::releaseObject();
}

void ticket2093_base::configure(const CF::Properties& props) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration)
{
    PropertySet_impl::configure(props);
}

void ticket2093_base::query(CF::Properties& props) throw (CF::UnknownProperties, CORBA::SystemException)
{
    PropertySet_impl::query(props);
}

void ticket2093_base::loadProperties()
{
    addProperty(some_simple,
               "some_simple",
               "",
               "readwrite",
               "null",
               "external",
               "configure");

    addProperty(some_sequence,
               "some_sequence",
               "",
               "readwrite",
               "null",
               "external",
               "configure");

    addProperty(some_struct,
//                some_struct_struct(), 
               "some_struct",
               "",
               "readwrite",
               "",
               "external",
               "configure");
            
        some_struct_seq.resize(0);
    addProperty(some_struct_seq,
               "some_struct_seq",
               "",
               "readwrite",
               "",
               "external",
               "configure");

}
