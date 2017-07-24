#ifndef BUSYCOMP_I_IMPL_H
#define BUSYCOMP_I_IMPL_H

#include "busycomp_base.h"

class busycomp_i : public busycomp_base
{
    ENABLE_LOGGING
    public:
        busycomp_i(const char *uuid, const char *label);
        ~busycomp_i();

        void constructor();

        int serviceFunction();
};

#endif // BUSYCOMP_I_IMPL_H
