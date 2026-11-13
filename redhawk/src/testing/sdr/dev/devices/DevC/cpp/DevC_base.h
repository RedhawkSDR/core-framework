#ifndef DEVC_BASE_IMPL_BASE_H
#define DEVC_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>


class DevC_base : public Device_impl, protected ThreadedComponent
{
    public:
        DevC_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        DevC_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        DevC_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        DevC_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~DevC_base();

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
        /// Property: myulong
        CORBA::ULong myulong;

    private:
        void construct();
};
#endif // DEVC_BASE_IMPL_BASE_H
