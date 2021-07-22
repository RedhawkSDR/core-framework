#ifndef CPP_WB_RECEIVER_BASE_IMPL_BASE_H
#define CPP_WB_RECEIVER_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <CF/AggregateDevices.h>
#include <ossie/AggregateDevice_impl.h>
#include <ossie/ThreadedComponent.h>
#include <ossie/DynamicComponent.h>

#include "struct_props.h"
#include "anothersimple/anothersimple.h"
#include "supersimple/supersimple.h"

class cpp_wb_receiver_base : public Device_impl, public virtual POA_CF::AggregatePlainDevice, public AggregateDevice_impl, protected ThreadedComponent, public virtual DynamicComponent
{
    public:
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~cpp_wb_receiver_base();

        /**
         * @throw CF::Resource::StartError
         * @throw CORBA::SystemException
         */
        void start();

        /**
         * @throw CF::Resource::StopError
         * @throw CORBA::SystemException
         */
        void stop();

        /**
         * @throw CF::LifeCycle::ReleaseError
         * @throw CORBA::SystemException
         */
        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: device_kind
        std::string device_kind;
        /// Property: device_model
        std::string device_model;
        /// Property: frontend_coherent_feeds
        std::vector<std::string> frontend_coherent_feeds;
        /// Property: frontend_listener_allocation
        frontend_listener_allocation_struct frontend_listener_allocation;
        /// Property: frontend_tuner_allocation
        frontend_tuner_allocation_struct frontend_tuner_allocation;
        /// Property: frontend_scanner_allocation
        frontend_scanner_allocation_struct frontend_scanner_allocation;
        /// Property: frontend_tuner_status
        std::vector<frontend_tuner_status_struct_struct> frontend_tuner_status;

    private:
        void construct();
};
#endif // CPP_WB_RECEIVER_BASE_IMPL_BASE_H
