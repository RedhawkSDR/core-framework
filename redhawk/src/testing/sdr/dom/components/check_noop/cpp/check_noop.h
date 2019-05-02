#ifndef CHECK_NOOP_I_IMPL_H
#define CHECK_NOOP_I_IMPL_H

#include "check_noop_base.h"

class check_noop_i : public check_noop_base
{
    ENABLE_LOGGING
    public:
        check_noop_i(const char *uuid, const char *label);
        ~check_noop_i();

        void constructor();

        int serviceFunction();

        int iteration_number;

        struct timespec ts_start;
        struct timespec ts_end;
};

#endif // CHECK_NOOP_I_IMPL_H
