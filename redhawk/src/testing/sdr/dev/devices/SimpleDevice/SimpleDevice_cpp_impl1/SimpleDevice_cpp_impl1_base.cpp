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


#include "SimpleDevice_cpp_impl1_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

/******************************************************************************************

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

SimpleDevice_cpp_impl1_base::SimpleDevice_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

SimpleDevice_cpp_impl1_base::SimpleDevice_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

SimpleDevice_cpp_impl1_base::SimpleDevice_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl) , serviceThread(0){
    construct();
}

SimpleDevice_cpp_impl1_base::SimpleDevice_cpp_impl1_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
          ExecutableDevice_impl (devMgr_ior, id, lbl, sftwrPrfl, compDev) , serviceThread(0){
    construct();
}

void SimpleDevice_cpp_impl1_base::construct()
{
    loadProperties();
    serviceThread = 0;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void SimpleDevice_cpp_impl1_base::initialize() throw (CF::LifeCycle::InitializeError, CORBA::SystemException)
{
}

void SimpleDevice_cpp_impl1_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    boost::mutex::scoped_lock lock(serviceThreadLock);
    if (serviceThread == 0) {
        serviceThread = new ProcessThread<SimpleDevice_cpp_impl1_base>(this, 0.1);
        serviceThread->start();
    }
}

void SimpleDevice_cpp_impl1_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
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


void SimpleDevice_cpp_impl1_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
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


    ExecutableDevice_impl::releaseObject();
}

void SimpleDevice_cpp_impl1_base::configure(const CF::Properties& props) throw (CORBA::SystemException, CF::PropertySet::InvalidConfiguration, CF::PropertySet::PartialConfiguration)
{
    PropertySet_impl::configure(props);
}

void SimpleDevice_cpp_impl1_base::loadProperties()
{
    addProperty(os_name,
                "Linux", 
               "DCE:4a23ad60-0b25-4121-a630-68803a498f75",
               "os_name",
               "readonly",
               "null",
               "eq",
               "allocation");

    addProperty(os_version,
               "DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192",
               "os_version",
               "readonly",
               "null",
               "eq",
               "allocation");

    addProperty(memTotal,
               "DCE:329d9304-839e-4fec-a36f-989e3b4d311d",
               "memTotal",
               "readonly",
               "megabytes",
               "external",
               "configure");

    addProperty(memFree,
               "DCE:6565bffd-cb09-4927-9385-2ecac68035c7",
               "memFree",
               "readonly",
               "megabytes",
               "external",
               "configure");

    addProperty(memCapacity,
               "DCE:8dcef419-b440-4bcf-b893-cab79b6024fb",
               "memCapacity",
               "readwrite",
               "megabytes",
               "external",
               "allocation");

    addProperty(memThreshold,
                80, 
               "DCE:fc24e19d-eda9-4200-ae96-8ba2ed953128",
               "memThreshold",
               "readwrite",
               "percentage",
               "external",
               "configure");

    addProperty(processor_name,
                "x86_64", 
               "DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b",
               "processor_name",
               "readonly",
               "null",
               "eq",
               "allocation");

    addProperty(bogomipsPerCPU,
               "DCE:0592a90e-9ffe-4aaa-83a2-11627259a1b8",
               "bogomipsPerCPU",
               "readonly",
               "bogomips",
               "external",
               "configure,allocation");

    addProperty(bogomipsTotal,
               "DCE:5f41fd25-a8ae-49b4-b13c-ebc3812dc0ea",
               "bogomipsTotal",
               "readonly",
               "bogomips",
               "external",
               "configure");

    addProperty(bogomipsCapacity,
               "DCE:5636c210-0346-4df7-a5a3-8fd34c5540a8",
               "bogomipsCapacity",
               "readwrite",
               "bogomips",
               "external",
               "allocation");

    addProperty(bogomipsThreshold,
                80, 
               "DCE:65be2582-b95d-45b5-8486-546286f421c4",
               "bogomipsThreshold",
               "readwrite",
               "percentage",
               "external",
               "configure");

    addProperty(mcastnicTotal,
               "DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec",
               "mcastnicTotal",
               "readwrite",
               "mbps",
               "external",
               "execparam");

    addProperty(mcastnicInterface,
               "DCE:4e416acc-3144-47eb-9e38-97f1d24f7700",
               "mcastnicInterface",
               "readonly",
               "null",
               "external",
               "configure");

    addProperty(mcastnicCapacity,
               "DCE:506102d6-04a9-4532-9420-a323d818ddec",
               "mcastnicCapacity",
               "readwrite",
               "mbps",
               "external",
               "allocation");

    addProperty(mcastnicHasVLAN,
               "DCE:b2582bde-859a-4407-ba26-18ccec9b26f1",
               "mcastnicHasVLAN",
               "readonly",
               "null",
               "external",
               "allocation");

    addProperty(mcastnicThreshold,
                80, 
               "DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1",
               "mcastnicThreshold",
               "readwrite",
               "percentage",
               "external",
               "configure");

    addProperty(diskTotal,
               "DCE:901776c6-5708-40d7-8e2e-6e0c8d020d19",
               "diskTotal",
               "readonly",
               "Mbytes",
               "external",
               "configure");

    addProperty(diskFree,
               "DCE:5b584d54-2f20-4db3-ac95-33006933eceb",
               "diskFree",
               "readonly",
               "Mbytes",
               "external",
               "configure");

    addProperty(diskCapacity,
               "DCE:56b2eda0-1dea-47c3-8392-93e496a8a9b5",
               "diskCapacity",
               "readwrite",
               "Mbytes",
               "external",
               "allocation");

    addProperty(diskThreshold,
                80, 
               "DCE:b911fa00-e411-4eb6-93d4-fff12dcf03bc",
               "diskThreshold",
               "readwrite",
               "percentage",
               "external",
               "configure");

    addProperty(diskrateCapacity,
               "DCE:708b0ab0-b953-433d-9040-0ab9a5359c7f",
               "diskrateCapacity",
               "readonly",
               "MBps",
               "ge",
               "allocation");

    addProperty(diskHasMountPoint,
               "DCE:ab2e3139-e933-45f6-8144-cf3e0a02bda5",
               "diskHasMountPoint",
               "readonly",
               "null",
               "external",
               "allocation");

    addProperty(hostName,
               "DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39",
               "hostName",
               "readwrite",
               "null",
               "external",
               "configure");

    addProperty(DeviceKind,
                "GPP", 
               "DCE:7f36cdfb-f828-4e4f-b84f-446e17f1a85b",
               "DeviceKind",
               "readonly",
               "null",
               "eq",
               "configure,allocation");

}
