#ifndef DEVCPP_BASE_IMPL_BASE_H
#define DEVCPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>


class devcpp_base : public Device_impl, protected ThreadedComponent
{
    public:
        devcpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        devcpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        devcpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        devcpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~devcpp_base();

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
        /// Property: foobar
        CORBA::Long foobar;
        /// Property: busy_state
        bool busy_state;
        /// Property: a_number
        short a_number;

    private:
        void construct();
};
#endif // DEVCPP_BASE_IMPL_BASE_H
