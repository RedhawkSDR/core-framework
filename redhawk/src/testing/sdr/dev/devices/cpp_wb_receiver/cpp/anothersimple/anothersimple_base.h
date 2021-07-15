#ifndef ANOTHERSIMPLE_BASE_IMPL_BASE_H
#define ANOTHERSIMPLE_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/AggregateDevice_impl.h>
#include <ossie/ThreadedComponent.h>
#include <ossie/DynamicComponent.h>

#define BOOL_VALUE_HERE 0

class anothersimple_base : public Device_impl, public virtual POA_CF::AggregatePlainDevice, public AggregateDevice_impl, protected ThreadedComponent, public virtual DynamicComponent
{
    public:
        anothersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        anothersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        anothersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        anothersimple_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~anothersimple_base();

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
        /// Property: defg
        std::string defg;

    private:
        void construct();
};
#endif // ANOTHERSIMPLE_BASE_IMPL_BASE_H
