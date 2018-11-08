#ifndef ALLOC_SHM_I_IMPL_H
#define ALLOC_SHM_I_IMPL_H

#include "alloc_shm_base.h"

class alloc_shm_i : public alloc_shm_base
{
    ENABLE_LOGGING
    public:
        alloc_shm_i(const char *uuid, const char *label);
        ~alloc_shm_i();

        void constructor();

        int serviceFunction();
};

#endif // ALLOC_SHM_I_IMPL_H
