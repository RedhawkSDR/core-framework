#ifndef C1_BASE_IMPL_BASE_H
#define C1_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class C1_base : public Component, protected ThreadedComponent
{
    public:
        C1_base(const char *uuid, const char *label);
        ~C1_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        float prop1;
        CORBA::Long prop2;
        float prop3;
        CORBA::Long memCapacity;

    private:
};
#endif // C1_BASE_IMPL_BASE_H
