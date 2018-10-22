#ifndef ALLOC_SHM_BASE_IMPL_BASE_H
#define ALLOC_SHM_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>


class alloc_shm_base : public Component, protected ThreadedComponent
{
    public:
        alloc_shm_base(const char *uuid, const char *label);
        ~alloc_shm_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:

    private:
};
#endif // ALLOC_SHM_BASE_IMPL_BASE_H
