#ifndef SVC_ERROR_CPP_BASE_IMPL_BASE_H
#define SVC_ERROR_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class svc_error_cpp_base : public Component, protected ThreadedComponent
{
    public:
        svc_error_cpp_base(const char *uuid, const char *label);
        ~svc_error_cpp_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // SVC_ERROR_CPP_BASE_IMPL_BASE_H
