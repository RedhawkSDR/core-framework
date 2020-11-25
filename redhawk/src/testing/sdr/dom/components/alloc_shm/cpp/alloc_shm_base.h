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

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:

    private:
};
#endif // ALLOC_SHM_BASE_IMPL_BASE_H
