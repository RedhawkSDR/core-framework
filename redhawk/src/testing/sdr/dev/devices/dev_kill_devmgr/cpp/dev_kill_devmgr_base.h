#ifndef DEV_KILL_DEVMGR_BASE_IMPL_BASE_H
#define DEV_KILL_DEVMGR_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>


class dev_kill_devmgr_base : public Device_impl, protected ThreadedComponent
{
    public:
        dev_kill_devmgr_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        dev_kill_devmgr_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        dev_kill_devmgr_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        dev_kill_devmgr_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~dev_kill_devmgr_base();

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
#endif // DEV_KILL_DEVMGR_BASE_IMPL_BASE_H
