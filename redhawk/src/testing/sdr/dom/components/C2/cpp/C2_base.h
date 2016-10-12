#ifndef C2_BASE_IMPL_BASE_H
#define C2_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class C2_base : public Component, protected ThreadedComponent
{
    public:
        C2_base(const char *uuid, const char *label);
        ~C2_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // C2_BASE_IMPL_BASE_H
