#ifndef cpp_wb_receiver_BASE_IMPL_BASE_H
#define cpp_wb_receiver_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <CF/cf.h>
#include <CF/AggregateDevices.h>
#include <ossie/Device_impl.h>
#include <ossie/AggregateDevice_impl.h>
#include <ossie/ThreadedComponent.h>
#include <ossie/DynamicComponent.h>

#include "supersimple/supersimple.h"
#include "anothersimple/anothersimple.h"

#define BOOL_VALUE_HERE 0

class cpp_wb_receiver_base : public Device_impl, public virtual POA_CF::AggregatePlainDevice, public AggregateDevice_impl, protected ThreadedComponent, public virtual DynamicComponent
{
    public:
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        cpp_wb_receiver_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~cpp_wb_receiver_base();

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

    private:
        void construct();
};
#endif // cpp_wb_receiver_BASE_IMPL_BASE_H
