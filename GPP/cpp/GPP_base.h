#ifndef GPP_BASE_IMPL_BASE_H
#define GPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/ExecutableDevice_impl.h>
#include <ossie/ThreadedComponent.h>

#include <ossie/MessageInterface.h>
#include <ossie/PropertyInterface.h>
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
        /// Property: device_kind
        std::string device_kind;
        /// Property: device_model
        std::string device_model;
        /// Property: processor_name
        std::string processor_name;
        /// Property: os_name
        std::string os_name;
        /// Property: os_version
        std::string os_version;
        /// Property: hostName
        std::string hostName;
        /// Property: useScreen
        bool useScreen;
        /// Property: componentOutputLog
        std::string componentOutputLog;
        /// Property: mcastnicInterface
        std::string mcastnicInterface;
        /// Property: mcastnicIngressTotal
        CORBA::Long mcastnicIngressTotal;
        /// Property: mcastnicEgressTotal
        CORBA::Long mcastnicEgressTotal;
        /// Property: mcastnicIngressCapacity
        CORBA::Long mcastnicIngressCapacity;
        /// Property: mcastnicEgressCapacity
        CORBA::Long mcastnicEgressCapacity;
        /// Property: mcastnicIngressFree
        CORBA::Long mcastnicIngressFree;
        /// Property: mcastnicEgressFree
        CORBA::Long mcastnicEgressFree;
        /// Property: mcastnicThreshold
        CORBA::Long mcastnicThreshold;
        /// Property: threshold_cycle_time
        CORBA::ULong threshold_cycle_time;
        /// Property: busy_reason
        std::string busy_reason;
        /// Property: cacheDirectory
        std::string cacheDirectory;
        /// Property: workingDirectory
        std::string workingDirectory;
        /// Property: memFree
        CORBA::LongLong memFree;
        /// Property: memCapacity
        CORBA::LongLong memCapacity;
        /// Property: shmFree
        CORBA::LongLong shmFree;
        /// Property: shmCapacity
        CORBA::LongLong shmCapacity;
        /// Property: loadTotal
        double loadTotal;
        /// Property: loadCapacityPerCore
        double loadCapacityPerCore;
        /// Property: loadThreshold
        CORBA::Long loadThreshold;
        /// Property: loadFree
        double loadFree;
        /// Property: loadCapacity
        double loadCapacity;
        /// Property: reserved_capacity_per_component
        float reserved_capacity_per_component;
        /// Property: processor_cores
        short processor_cores;
        /// Property: processor_monitor_list
        std::string processor_monitor_list;
        /// Property: mcastnicVLANs
        std::vector<CORBA::Long> mcastnicVLANs;
        /// Property: nic_interfaces
        std::vector<std::string> nic_interfaces;
        /// Property: available_nic_interfaces
        std::vector<std::string> available_nic_interfaces;
        /// Property: nic_allocation
        nic_allocation_struct nic_allocation;
        /// Property: advanced
        advanced_struct advanced;
        /// Message structure definition for threshold_event
        threshold_event_struct threshold_event;
        /// Property: thresholds
        thresholds_struct thresholds;
        /// Property: loadAverage
        loadAverage_struct loadAverage;
        /// Property: gpp_limits
        gpp_limits_struct gpp_limits;
        /// Property: sys_limits
        sys_limits_struct sys_limits;
        /// Property: redhawk__reservation_request
        redhawk__reservation_request_struct redhawk__reservation_request;
        /// Property: affinity
        affinity_struct affinity;
        /// Message structure definition for plugin_registration
        plugin_registration_struct plugin_registration;
        /// Message structure definition for plugin_heartbeat
        plugin_heartbeat_struct plugin_heartbeat;
        /// Message structure definition for plugin_message
        plugin_message_struct plugin_message;
        /// Property: plugin_update_metric
        plugin_update_metric_struct plugin_update_metric;
        /// Message structure definition for plugin_set_threshold
        plugin_set_threshold_struct plugin_set_threshold;
        /// Property: nic_allocation_status
        std::vector<nic_allocation_status_struct_struct> nic_allocation_status;
        /// Property: nic_metrics
        std::vector<nic_metrics_struct_struct> nic_metrics;
        /// Property: networkMonitor
        std::vector<interfaces_struct> networkMonitor;
        /// Property: utilization
        std::vector<utilization_entry_struct> utilization;
        /// Property: component_monitor
        std::vector<component_monitor_struct> component_monitor;
        /// Property: plugin_status
        std::vector<plugin_status_template_struct> plugin_status;
        /// Property: plugin_metric_status
        std::vector<plugin_metric_status_template_struct> plugin_metric_status;

        // Ports
        /// Port: metrics_in
        MessageConsumerPort *metrics_in;
        /// Port: propEvent
        PropertyEventSupplier *propEvent;
        /// Port: MessageEvent_out
        MessageSupplierPort *MessageEvent_out;

    private:
        void construct();
};
#endif // GPP_BASE_IMPL_BASE_H
