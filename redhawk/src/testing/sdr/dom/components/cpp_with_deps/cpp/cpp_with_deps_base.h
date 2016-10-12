#ifndef CPP_WITH_DEPS_BASE_IMPL_BASE_H
#define CPP_WITH_DEPS_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class cpp_with_deps_base : public Component, protected ThreadedComponent
{
    public:
        cpp_with_deps_base(const char *uuid, const char *label);
        ~cpp_with_deps_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // CPP_WITH_DEPS_BASE_IMPL_BASE_H
