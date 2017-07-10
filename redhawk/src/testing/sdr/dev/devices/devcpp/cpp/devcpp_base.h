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
        /// Property: foobar
        CORBA::Long foobar;

    private:
        void construct();
};
#endif // DEVCPP_BASE_IMPL_BASE_H
