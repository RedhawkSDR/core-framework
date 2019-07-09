#ifndef STREAM_SNK_BASE_IMPL_BASE_H
#define STREAM_SNK_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class stream_snk_base : public Component, protected ThreadedComponent
{
    public:
        stream_snk_base(const char *uuid, const char *label);
        ~stream_snk_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

        // Ports
        /// Port: dataFloat_in
        bulkio::InFloatPort *dataFloat_in;

    private:
};
#endif // STREAM_SNK_BASE_IMPL_BASE_H
