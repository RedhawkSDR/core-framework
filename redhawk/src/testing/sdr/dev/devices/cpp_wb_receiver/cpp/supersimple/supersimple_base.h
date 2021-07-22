#ifndef SUPERSIMPLE_BASE_IMPL_BASE_H
#define SUPERSIMPLE_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <CF/AggregateDevices.h>
#include <ossie/AggregateDevice_impl.h>
#include <ossie/ThreadedComponent.h>
#include <ossie/DynamicComponent.h>


namespace supersimple_ns {
class supersimple_base : public Device_impl, public virtual POA_CF::AggregatePlainDevice, public AggregateDevice_impl, protected ThreadedComponent, public virtual DynamicComponent
{
    public:
        supersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        supersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        supersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        supersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~supersimple_base();

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
        /// Property: abc
        std::string abc;

    private:
        void construct();
};
};
#endif // SUPERSIMPLE_BASE_IMPL_BASE_H
