#ifndef COMP_BASE_IMPL_BASE_H
#define COMP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class comp_base : public Component, protected ThreadedComponent
{
    public:
        comp_base(const char *uuid, const char *label);
        ~comp_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // COMP_BASE_IMPL_BASE_H
