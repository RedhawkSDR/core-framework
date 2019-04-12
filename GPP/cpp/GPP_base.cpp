/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
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
    propEvent->_remove_ref();
    propEvent = 0;
    MessageEvent_out->_remove_ref();
    MessageEvent_out = 0;
}

void GPP_base::construct()
{
    loadProperties();

    propEvent = new PropertyEventSupplier("propEvent");
    propEvent->setLogger(this->_baseLog->getChildLogger("propEvent", "ports"));
    addPort("propEvent", propEvent);
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:506102d6-04a9-4532-9420-a323d818ddec"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:eb08e43f-11c7-45a0-8750-edff439c8b24"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:0b57a27a-8fa2-412b-b0ae-010618b8f40e"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:9b5bbdcb-1894-4b95-847c-787f121c05ae"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:6565bffd-cb09-4927-9385-2ecac68035c7"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:22a60339-b66e-4309-91ae-e9bfed6f0490"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:6c000787-6fea-4765-8686-2e051e6c24b0"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056"));
    propEvent->registerProperty(this->_identifier, this->naming_service_name, this->getPropertyFromId("DCE:9da85ebc-6503-48e7-af36-b77c7ad0c2b4"));
    this->registerPropertyChangePort(propEvent);
    MessageEvent_out = new MessageSupplierPort("MessageEvent_out");
    MessageEvent_out->setLogger(this->_baseLog->getChildLogger("MessageEvent_out", "ports"));
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
                "REDHAWK GPP",
                "DCE:0f99b2e4-9903-4631-9846-ff349d18ecfb",
                "device_model",
                "readonly",
                "",
                "eq",
                "allocation,property,configure");

    addProperty(processor_name,
                "DCE:fefb9c66-d14a-438d-ad59-2cfd1adb272b",
                "processor_name",
                "readonly",
                "",
                "eq",
                "allocation,property,configure");

    addProperty(os_name,
                "DCE:4a23ad60-0b25-4121-a630-68803a498f75",
                "os_name",
                "readonly",
                "",
                "eq",
                "allocation,property,configure");

    addProperty(os_version,
                "DCE:0f3a9a37-a342-43d8-9b7f-78dc6da74192",
                "os_version",
                "readonly",
                "",
                "eq",
                "allocation,property,configure");

    addProperty(hostName,
                "DCE:9190eb70-bd1e-4556-87ee-5a259dcfee39",
                "hostName",
                "readonly",
                "",
                "external",
                "property,configure,event");

    addProperty(useScreen,
                false,
                "DCE:218e612c-71a7-4a73-92b6-bf70959aec45",
                "useScreen",
                "readwrite",
                "",
                "external",
                "execparam");

    addProperty(componentOutputLog,
                "",
                "DCE:c80f6c5a-e3ea-4f57-b0aa-46b7efac3176",
                "componentOutputLog",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(mcastnicInterface,
                "",
                "DCE:4e416acc-3144-47eb-9e38-97f1d24f7700",
                "mcastnicInterface",
                "readwrite",
                "",
                "external",
                "execparam");

    addProperty(mcastnicIngressTotal,
                0,
                "DCE:5a41c2d3-5b68-4530-b0c4-ae98c26c77ec",
                "mcastnicIngressTotal",
                "readwrite",
                "Mb/s",
                "external",
                "execparam");

    addProperty(mcastnicEgressTotal,
                0,
                "DCE:442d5014-2284-4f46-86ae-ce17e0749da0",
                "mcastnicEgressTotal",
                "readwrite",
                "Mb/s",
                "external",
                "execparam");

    addProperty(mcastnicIngressCapacity,
                "DCE:506102d6-04a9-4532-9420-a323d818ddec",
                "mcastnicIngressCapacity",
                "readwrite",
                "Mb/s",
                "external",
                "allocation,event");

    addProperty(mcastnicEgressCapacity,
                "DCE:eb08e43f-11c7-45a0-8750-edff439c8b24",
                "mcastnicEgressCapacity",
                "readwrite",
                "Mb/s",
                "external",
                "allocation,event");

    addProperty(mcastnicIngressFree,
                0,
                "DCE:0b57a27a-8fa2-412b-b0ae-010618b8f40e",
                "mcastnicIngressFree",
                "readonly",
                "Mb/s",
                "external",
                "configure,event");

    addProperty(mcastnicEgressFree,
                0,
                "DCE:9b5bbdcb-1894-4b95-847c-787f121c05ae",
                "mcastnicEgressFree",
                "readonly",
                "Mb/s",
                "external",
                "configure,event");

    addProperty(mcastnicThreshold,
                80,
                "DCE:89be90ae-6a83-4399-a87d-5f4ae30ef7b1",
                "mcastnicThreshold",
                "readwrite",
                "%",
                "external",
                "configure,event");

    addProperty(threshold_cycle_time,
                500,
                "threshold_cycle_time",
                "threshold_cycle_time",
                "readwrite",
                "milliseconds",
                "external",
                "property");

    addProperty(busy_reason,
                "",
                "busy_reason",
                "busy_reason",
                "readonly",
                "",
                "external",
                "property");

    addProperty(cacheDirectory,
                "",
                "cacheDirectory",
                "cacheDirectory",
                "readonly",
                "",
                "external",
                "property");

    addProperty(workingDirectory,
                "",
                "workingDirectory",
                "workingDirectory",
                "readonly",
                "",
                "external",
                "property");

    addProperty(memFree,
                "DCE:6565bffd-cb09-4927-9385-2ecac68035c7",
                "memFree",
                "readonly",
                "MiB",
                "external",
                "configure,event");

    addProperty(memCapacity,
                "DCE:8dcef419-b440-4bcf-b893-cab79b6024fb",
                "memCapacity",
                "readwrite",
                "MiB",
                "external",
                "allocation");

    addProperty(shmFree,
                "shmFree",
                "shmFree",
                "readonly",
                "MB",
                "external",
                "property");

    addProperty(shmCapacity,
                "shmCapacity",
                "shmCapacity",
                "readonly",
                "MB",
                "external",
                "property");

    addProperty(loadTotal,
                "DCE:28b23bc8-e4c0-421b-9c52-415a24715209",
                "loadTotal",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(loadCapacityPerCore,
                1.0,
                "DCE:3bf07b37-0c00-4e2a-8275-52bd4e391f07",
                "loadCapacityPerCore",
                "readwrite",
                "",
                "gt",
                "allocation,execparam");

    addProperty(loadThreshold,
                80,
                "DCE:22a60339-b66e-4309-91ae-e9bfed6f0490",
                "loadThreshold",
                "readwrite",
                "%",
                "external",
                "configure,event");

    addProperty(loadFree,
                "DCE:6c000787-6fea-4765-8686-2e051e6c24b0",
                "loadFree",
                "readonly",
                "",
                "external",
                "configure,event");

    addProperty(loadCapacity,
                "DCE:72c1c4a9-2bcf-49c5-bafd-ae2c1d567056",
                "loadCapacity",
                "readwrite",
                "",
                "external",
                "allocation,event");

    addProperty(reserved_capacity_per_component,
                0.1,
                "reserved_capacity_per_component",
                "reserved_capacity_per_component",
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

    addProperty(processor_monitor_list,
                "processor_monitor_list",
                "",
                "readonly",
                "",
                "external",
                "configure");

    addProperty(mcastnicVLANs,
                "DCE:65544aad-4c73-451f-93de-d4d76984025a",
                "mcastnicVLANs",
                "readwrite",
                "",
                "external",
                "allocation");

    // Set the sequence with its initial values
    nic_interfaces.push_back("e.*");
    addProperty(nic_interfaces,
                nic_interfaces,
                "nic_interfaces",
                "",
                "readwrite",
                "",
                "external",
                "property,configure");

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
                "property");

    addProperty(loadAverage,
                loadAverage_struct(),
                "DCE:9da85ebc-6503-48e7-af36-b77c7ad0c2b4",
                "loadAverage",
                "readonly",
                "",
                "external",
                "configure,event");

    addProperty(gpp_limits,
                gpp_limits_struct(),
                "gpp_limits",
                "",
                "readonly",
                "",
                "external",
                "property");

    addProperty(sys_limits,
                sys_limits_struct(),
                "sys_limits",
                "",
                "readonly",
                "",
                "external",
                "property");

    addProperty(redhawk__reservation_request,
                redhawk__reservation_request_struct(),
                "redhawk::reservation_request",
                "",
                "readwrite",
                "",
                "external",
                "allocation");

    addProperty(affinity,
                affinity_struct(),
                "affinity",
                "",
                "readwrite",
                "",
                "external",
                "property");

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

    addProperty(utilization,
                "utilization",
                "",
                "readonly",
                "",
                "external",
                "property");

    addProperty(component_monitor,
                "component_monitor",
                "",
                "readonly",
                "",
                "external",
                "property");

}


