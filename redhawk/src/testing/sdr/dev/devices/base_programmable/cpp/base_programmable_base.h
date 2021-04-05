#ifndef BASE_PROGRAMMABLE_BASE_IMPL_BASE_H
#define BASE_PROGRAMMABLE_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/ExecutableDevice_impl.h>
#include <CF/AggregateDevices.h>
#include <ossie/AggregateDevice_impl.h>
#include <ossie/ThreadedComponent.h>


class base_programmable_base : public ExecutableDevice_impl, public virtual POA_CF::AggregateExecutableDevice, public AggregateDevice_impl, protected ThreadedComponent
{
    public:
        base_programmable_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        base_programmable_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        base_programmable_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        base_programmable_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~base_programmable_base();

        void start();

        void stop();

        void releaseObject();

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

    private:
        void construct();
};
#endif // BASE_PROGRAMMABLE_BASE_IMPL_BASE_H
