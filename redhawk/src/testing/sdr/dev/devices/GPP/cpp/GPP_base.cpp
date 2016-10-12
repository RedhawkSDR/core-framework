/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "GPP_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the device class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

GPP_base::GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl) :
    ExecutableDevice_impl(devMgr_ior, id, lbl, sftwrPrfl),
    ThreadedComponent()
{
    construct();
}

GPP_base::GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev) :
    ExecutableDevice_impl(devMgr_ior, id, lbl, sftwrPrfl, compDev),
    ThreadedComponent()
{
    construct();
}

GPP_base::GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities) :
    ExecutableDevice_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities),
    ThreadedComponent()
{
    construct();
}

GPP_base::GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev) :
    ExecutableDevice_impl(devMgr_ior, id, lbl, sftwrPrfl, capacities, compDev),
    ThreadedComponent()
{
    construct();
}

GPP_base::~GPP_base()
{
  delete propEvent;
  propEvent = 0;
  delete MessageEvent_out;
  MessageEvent_out = 0;
}

void GPP_base::construct()
{
    loadProperties();

    propEvent = new PropertyEventSupplier("propEvent");
    addPort("propEvent", propEvent);
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:22a60339-b66e-4309-91ae-e9bfed6f0490"));
    this->registerPropertyChangePort(propEvent);
    MessageEvent_out = new MessageSupplierPort("MessageEvent_out");
    addPort("MessageEvent_out", MessageEvent_out);

}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void GPP_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    ExecutableDevice_impl::start();
    ThreadedComponent::startThread();
}

void GPP_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    ExecutableDevice_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void GPP_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the device running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    ExecutableDevice_impl::releaseObject();
}

void GPP_base::loadProperties()
{
    addProperty(device_kind,
                "GPP",
                "DCE:cdc5ee18-7ceb-4ae6-bf4c-31f983179b4d",
                "device_kind",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(device_model,
                "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                "device_model",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(processor_name,
                "DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b",
                "processor_name",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(os_name,
                "DCE:4a23ad60-0b25-4121-a630-68803a498f75",
                "os_name",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(os_version,
                "DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192",
                "os_version",
                "readonly",
                "",
                "eq",
                "allocation,configure");

    addProperty(hostName,
                "DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39",
                "hostName",
                "readonly",
                "",
                "external",
                "configure,event");

    addProperty(useScreen,
                false,
                "DCE:218e612c-71a7-4a73-92b6-bf70959aec45",
                "useScreen",
                "readwrite",
                "",
                "external",
                "execparam");

    addProperty(loadCapacity,
                "DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056",
                "loadCapacity",
                "readwrite",
                "",
                "external",
                "allocation");

    addProperty(mcastnicIngressCapacity,
                "DCE:506102d6-04a9-4532-9420-a323d818ddec",
                "mcastnicIngressCapacity",
                "readwrite",
                "Mb/s",
                "external",
                "allocation");

    addProperty(memCapacity,
                "DCE:8dcef419-b440-4bcf-b893-cab79b6024fb",
                "memCapacity",
                "readwrite",
                "MiB",
                "external",
                "allocation");

    addProperty(loadCapacityPerCore,
                1.0,
                "DCE:3bf07b37-0c00-4e2a-8275-52bd4e391f07",
                "loadCapacityPerCore",
                "readwrite",
                "",
                "gt",
                "allocation,execparam");

    addProperty(reserved_capacity_per_component,
                0.25,
                "reserved_capacity_per_component",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(processor_cores,
                "processor_cores",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(loadThreshold,
                80,
                "DCE:22a60339-b66e-4309-91ae-e9bfed6f0490",
                "loadThreshold",
                "readwrite",
                "%",
                "external",
                "configure,event");

    // Set the sequence with its initial values
    nic_interfaces.push_back("e.*");
    addProperty(nic_interfaces,
                nic_interfaces,
                "nic_interfaces",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(available_nic_interfaces,
                "available_nic_interfaces",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(nic_allocation,
                nic_allocation_struct(),
                "nic_allocation",
                "nic_allocation",
                "readwrite",
                "",
                "external",
                "allocation");

    addProperty(advanced,
                advanced_struct(),
                "advanced",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(threshold_event,
                threshold_event_struct(),
                "threshold_event",
                "",
                "readonly",
                "",
                "external",
                "message");

    addProperty(thresholds,
                thresholds_struct(),
                "thresholds",
                "",
                "readwrite",
                "",
                "external",
                "property,configure");

    addProperty(nic_allocation_status,
                "nic_allocation_status",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(nic_metrics,
                "nic_metrics",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(networkMonitor,
                "networkMonitor",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(processor_monitor_list,
                "processor_monitor_list",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(affinity,
                affinity_struct(),
                "affinity",
                "",
                "readwrite",
                "",
                "external",
                "configure,property");


    addProperty(threshold_cycle_time,
                500,
                "threshold_cycle_time",
                "threshold_cycle_time",
                "readwrite",
                "milliseconds",
                "external",
                "property,configure");

}


