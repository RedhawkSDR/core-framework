#ifndef STREAM_SRC_BASE_IMPL_BASE_H
#define STREAM_SRC_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class stream_src_base : public Component, protected ThreadedComponent
{
    public:
        stream_src_base(const char *uuid, const char *label);
        ~stream_src_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

        // Ports
        /// Port: dataFloat_out
        bulkio::OutFloatPort *dataFloat_out;

    private:
};
#endif // STREAM_SRC_BASE_IMPL_BASE_H
