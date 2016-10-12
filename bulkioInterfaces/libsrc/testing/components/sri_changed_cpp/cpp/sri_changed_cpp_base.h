#ifndef SRI_CHANGED_CPP_IMPL_BASE_H
#define SRI_CHANGED_CPP_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class sri_changed_cpp_base : public Resource_impl, protected ThreadedComponent
{
    public:
        sri_changed_cpp_base(const char *uuid, const char *label);
        ~sri_changed_cpp_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        bool changed;
        bool verified;

        // Ports
        bulkio::InShortPort *input;

    private:
};
#endif // SRI_CHANGED_CPP_IMPL_BASE_H
