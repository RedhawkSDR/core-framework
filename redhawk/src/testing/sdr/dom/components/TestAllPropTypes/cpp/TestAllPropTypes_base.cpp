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


#include "TestAllPropTypes_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY
    
 	Source: TestAllPropTypes.spd.xml
 	Generated on: Tue May 07 10:59:08 EDT 2013
 	REDHAWK IDE
 	Version: R.1.8.3
 	Build id: v201303122306

*******************************************************************************************/

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/
 
TestAllPropTypes_base::TestAllPropTypes_base(const char *uuid, const char *label) :
                                     Resource_impl(uuid, label), serviceThread(0) {
    construct();
}

void TestAllPropTypes_base::construct()
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
void TestAllPropTypes_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void TestAllPropTypes_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<TestAllPropTypes_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
    	Resource_impl::start();
    }
}

void TestAllPropTypes_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
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


void TestAllPropTypes_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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

void TestAllPropTypes_base::loadProperties()
{
    addProperty(simple_string,
               "simple_string",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_boolean,
               "simple_boolean",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_ulong,
               "simple_ulong",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_objref,
               "simple_objref",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_short,
               "simple_short",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_float,
               "simple_float",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_octet,
               "simple_octet",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_char,
               "simple_char",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_ushort,
               "simple_ushort",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_double,
               "simple_double",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_long,
               "simple_long",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_longlong,
               "simple_longlong",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_ulonglong,
               "simple_ulonglong",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_string,
               "simple_sequence_string",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_boolean,
               "simple_sequence_boolean",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_ulong,
               "simple_sequence_ulong",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_objref,
               "simple_sequence_objref",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_short,
               "simple_sequence_short",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_float,
               "simple_sequence_float",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_octet,
               "simple_sequence_octet",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_char,
               "simple_sequence_char",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_ushort,
               "simple_sequence_ushort",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_double,
               "simple_sequence_double",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_long,
               "simple_sequence_long",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_longlong,
               "simple_sequence_longlong",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(simple_sequence_ulonglong,
               "simple_sequence_ulonglong",
               "",
               "readwrite",
               "",
               "external",
               "configure");

    addProperty(struct_vars,
                struct_vars_struct(), 
               "struct_vars",
               "",
               "readwrite",
               "",
               "external",
               "configure");
            
        struct_seq.resize(0);
    addProperty(struct_seq,
               "struct_seq",
               "",
               "readwrite",
               "",
               "external",
               "configure");

}
