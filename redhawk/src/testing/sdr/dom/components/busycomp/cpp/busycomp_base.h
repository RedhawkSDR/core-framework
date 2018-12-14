#ifndef BUSYCOMP_BASE_IMPL_BASE_H
#define BUSYCOMP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class busycomp_base : public Component, protected ThreadedComponent
{
    public:
        busycomp_base(const char *uuid, const char *label);
        ~busycomp_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // BUSYCOMP_BASE_IMPL_BASE_H
