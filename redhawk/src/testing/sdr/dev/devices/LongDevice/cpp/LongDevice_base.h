#ifndef LONGDEVICE_BASE_IMPL_BASE_H
#define LONGDEVICE_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>


class LongDevice_base : public Device_impl, protected ThreadedComponent
{
    public:
        LongDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        LongDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        LongDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        LongDevice_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~LongDevice_base();

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
        /// Property: disable_delay
        bool disable_delay;
        /// Property: delay
        CORBA::Long delay;

    private:
        void construct();
};
#endif // LONGDEVICE_BASE_IMPL_BASE_H
