#ifndef CHECK_CWD_CPP_BASE_IMPL_BASE_H
#define CHECK_CWD_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class check_cwd_cpp_base : public Component, protected ThreadedComponent
{
    public:
        check_cwd_cpp_base(const char *uuid, const char *label);
        ~check_cwd_cpp_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: cwd
        std::string cwd;

    private:
};
#endif // CHECK_CWD_CPP_BASE_IMPL_BASE_H
