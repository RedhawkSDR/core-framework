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


#include "issue_111_cpp_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY
    
     Source: issue_111_cpp.spd.xml
     Generated on: Fri Oct 26 16:20:16 EDT 2012
     Redhawk IDE
     Version:@buildLabel@
     Build id: @buildId@

*******************************************************************************************/

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

issue_111_cpp_base::issue_111_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

issue_111_cpp_base::issue_111_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

issue_111_cpp_base::issue_111_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

issue_111_cpp_base::issue_111_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
          Device_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

void issue_111_cpp_base::construct()
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
void issue_111_cpp_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void issue_111_cpp_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<issue_111_cpp_base>(this, 0.1);
        serviceThread->start();
    }
    
    if (!Resource_impl::started()) {
        Resource_impl::start();
    }
}

void issue_111_cpp_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
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


void issue_111_cpp_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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

void issue_111_cpp_base::loadProperties()
{
    addProperty(device_kind,
               "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
               "device_kind",
               "readonly",
               "",
               "eq",
               "configure,allocation");

    addProperty(device_model,
               "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
               "device_model",
               "readonly",
               "",
               "eq",
               "configure,allocation");

    addProperty(test_cap,
               "test_cap",
               "",
               "readwrite",
               "",
               "external",
               "allocation");

}
