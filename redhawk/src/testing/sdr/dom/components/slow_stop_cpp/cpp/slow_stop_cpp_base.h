#ifndef SLOW_STOP_CPP_BASE_IMPL_BASE_H
#define SLOW_STOP_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class slow_stop_cpp_base : public Component, protected ThreadedComponent
{
    public:
        slow_stop_cpp_base(const char *uuid, const char *label);
        ~slow_stop_cpp_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // SLOW_STOP_CPP_BASE_IMPL_BASE_H
