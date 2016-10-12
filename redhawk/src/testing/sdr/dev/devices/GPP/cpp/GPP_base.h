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

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        std::string device_kind;
        std::string device_model;
        std::string processor_name;
        std::string os_name;
        std::string os_version;
        std::string hostName;
        bool useScreen;
        double loadCapacity;
        CORBA::Long mcastnicIngressCapacity;
        CORBA::LongLong memCapacity;
        double loadCapacityPerCore;
        float reserved_capacity_per_component;
        short processor_cores;
        CORBA::Long loadThreshold;
        std::vector<std::string> nic_interfaces;
        std::vector<std::string> available_nic_interfaces;
        nic_allocation_struct nic_allocation;
        advanced_struct advanced;
        threshold_event_struct threshold_event;
        thresholds_struct thresholds;
        std::vector<nic_allocation_status_struct_struct> nic_allocation_status;
        std::vector<nic_metrics_struct_struct> nic_metrics;
        std::vector<interfaces_struct> networkMonitor;
        std::string processor_monitor_list;
        affinity_struct affinity;
        CORBA::ULong threshold_cycle_time;

        // Ports
        PropertyEventSupplier *propEvent;
        MessageSupplierPort *MessageEvent_out;

    private:
        void construct();
};
#endif // GPP_IMPL_BASE_H
