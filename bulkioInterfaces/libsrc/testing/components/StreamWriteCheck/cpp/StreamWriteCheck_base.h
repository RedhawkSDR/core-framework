#ifndef STREAMWRITECHECK_BASE_IMPL_BASE_H
#define STREAMWRITECHECK_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class StreamWriteCheck_base : public Component, protected ThreadedComponent
{
    public:
        StreamWriteCheck_base(const char *uuid, const char *label);
        ~StreamWriteCheck_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

        // Ports
        /// Port: input
        bulkio::InLongPort *input;
        /// Port: output
        bulkio::OutLongPort *output;

    private:
};
#endif // STREAMWRITECHECK_BASE_IMPL_BASE_H
