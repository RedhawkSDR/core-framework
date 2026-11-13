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
#ifndef GPP_IMPL_BASE_H
#define GPP_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/ExecutableDevice_impl.h>
#include <ossie/ThreadedComponent.h>

#include <ossie/PropertyInterface.h>
#include <ossie/MessageInterface.h>
#include "struct_props.h"

class GPP_base : public ExecutableDevice_impl, protected ThreadedComponent
{
    public:
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        GPP_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~GPP_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        std::string device_kind;
        std::string device_model;
        std::string processor_name;
        std::string os_name;
        std::string os_version;
        std::string hostName;
        std::string componentOutputLog;
        bool useScreen;
        advanced_struct advanced;

        std::vector<std::string> nic_interfaces;
        std::vector<std::string> available_nic_interfaces;
        nic_allocation_struct nic_allocation;
        std::string mcastnicInterface;
        CORBA::Long mcastnicIngressTotal;
        CORBA::Long mcastnicEgressTotal;
        CORBA::Long mcastnicIngressCapacity;
        CORBA::Long mcastnicEgressCapacity;
        CORBA::Long mcastnicIngressFree;
        CORBA::Long mcastnicEgressFree;
        CORBA::Long mcastnicThreshold;
        std::vector<CORBA::Long> mcastnicVLANs;
        std::vector<nic_allocation_status_struct_struct> nic_allocation_status;
        std::vector<nic_metrics_struct_struct> nic_metrics;
        std::vector<interfaces_struct> networkMonitor;
        std::vector<component_monitor_struct> component_monitor;

        // reporting struct when a threshold is broke
        threshold_event_struct threshold_event;
        // threshold items to watch
        thresholds_struct thresholds;
        /// Property  to annotate why the system is busy 
        std::string busy_reason;
        /// Property  to select a cache directory other than the default
        std::string cacheDirectory;
        /// Property  to select a working directory other than the default
        std::string workingDirectory;
        // time between cycles to refresh threshold metrics
        CORBA::ULong threshold_cycle_time;
        // ulimits for the GPP process
        ulimit_struct gpp_limits;
        // ulimits for the system as a whole
        sys_limits_struct sys_limits;
        /// Property: memFree
        CORBA::LongLong memFree;
        /// Property: memCapacity
        CORBA::LongLong memCapacity;
        /// Property: loadTotal
        double loadTotal;
        /// Property: loadThreshold
        CORBA::Long   loadThreshold;
        /// Property: loadCapacityPerCore
        double loadCapacityPerCore;
        /// Property: loadFree
        double loadFree;
        /// Property: loadCapacity
        double loadCapacity;
        /// Property: loadAverage
        loadAverage_struct loadAverage;
        /// Property: reserved capacity per core for reservation schema
        float  reserved_capacity_per_component;
        /// Property  processor_cores  - number of cores the machine supports
        short  processor_cores;
        /// Property: redhawk__reservation_request
        redhawk__reservation_request_struct redhawk__reservation_request;
        /// Property  processor_monitor_list - list of the cores we are watching..
        std::string processor_monitor_list;
        // Property affinity  - controls affinity processing for the GPP
        affinity_struct affinity;

        // Ports
        PropertyEventSupplier *propEvent;
        MessageSupplierPort *MessageEvent_out;
        std::vector<utilization_entry_struct> utilization;

    private:
        void construct();
};
#endif // GPP_IMPL_BASE_H
