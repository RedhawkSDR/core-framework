#ifndef ECM_CPP_IMPL_BASE_H
#define ECM_CPP_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class ECM_CPP_base : public Component, protected ThreadedComponent
{
    public:
        ECM_CPP_base(const char *uuid, const char *label);
        ~ECM_CPP_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        CORBA::Long msg_recv;
        CORBA::Long msg_xmit;
        CORBA::Long msg_limit;

    private:
};
#endif // ECM_CPP_IMPL_BASE_H
