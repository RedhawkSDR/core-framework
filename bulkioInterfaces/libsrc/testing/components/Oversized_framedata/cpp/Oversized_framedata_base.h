#ifndef OVERSIZED_FRAMEDATA_IMPL_BASE_H
#define OVERSIZED_FRAMEDATA_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio/bulkio.h>

class Oversized_framedata_base : public Resource_impl, protected ThreadedComponent
{
    public:
        Oversized_framedata_base(const char *uuid, const char *label);
        ~Oversized_framedata_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

        // Ports
        bulkio::OutShortPort *dataShort_out;

    private:
};
#endif // OVERSIZED_FRAMEDATA_IMPL_BASE_H
