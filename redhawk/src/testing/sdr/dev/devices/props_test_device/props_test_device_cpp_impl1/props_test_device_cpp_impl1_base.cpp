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


#include "props_test_device_cpp_impl1_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY
    
     Source: props_test_device.spd.xml
     Generated on: Wed Nov 16 15:33:54 EST 2011
     Redhawk IDE
     Version:T.1.X.X
     Build id: v201110201144-r6330

*******************************************************************************************/

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

props_test_device_cpp_impl1_base::props_test_device_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

props_test_device_cpp_impl1_base::props_test_device_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

props_test_device_cpp_impl1_base::props_test_device_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

props_test_device_cpp_impl1_base::props_test_device_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

void props_test_device_cpp_impl1_base::construct()
{
    loadProperties();
    serviceThread = 0;

}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void props_test_device_cpp_impl1_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void props_test_device_cpp_impl1_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<props_test_device_cpp_impl1_base>(this, 0.1);
        serviceThread->start();
    }
}

void props_test_device_cpp_impl1_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
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


void props_test_device_cpp_impl1_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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

void props_test_device_cpp_impl1_base::configure(const CF::Properties& props) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration)
{
    PropertySet_impl::configure(props);
}

void props_test_device_cpp_impl1_base::query(CF::Properties& props) throw (CF::UnknownProperties, CORBA::SystemException)
{
    PropertySet_impl::query(props);
}

void props_test_device_cpp_impl1_base::loadProperties()
{
    addProperty(device_kind,
               "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
               "device_kind",
               "readonly",
               "null",
               "eq",
               "configure,allocation");

    addProperty(device_model,
               "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
               "device_model",
               "readonly",
               "null",
               "eq",
               "configure,allocation");

    addProperty(writeonly_config,
               "writeonly_config",
               "writeonly_config",
               "writeonly",
               "null",
               "external",
               "configure");

    addProperty(writeonly_alloc,
               "DCE:446e722d-acd9-4079-8c70-0c156ff374a5",
               "writeonly_alloc",
               "writeonly",
               "null",
               "external",
               "allocation");

    addProperty(simple_config,
               "simple_config",
               "simple_config",
               "readwrite",
               "null",
               "external",
               "configure");

    addProperty(simple_alloc,
               "DCE:1d9bed5e-1e6c-4607-9391-0e692f8fd1ae",
               "simple_alloc",
               "readwrite",
               "null",
               "external",
               "allocation");

    addProperty(no_extern_alloc,
               "DCE:ee48d37f-ad9a-4678-86b5-8b568013f5f4",
               "no_extern_alloc",
               "readwrite",
               "null",
               "eq",
               "allocation");

    addProperty(execparam_readwrite,
               "execparam_readwrite",
               "execparam_readwrite",
               "readwrite",
               "null",
               "external",
               "execparam");

    addProperty(test_prop,
               "test_prop",
               "test_prop",
               "readwrite",
               "null",
               "external",
               "test");

}
