#ifndef SLOW_STOP_CPP_I_IMPL_H
#define SLOW_STOP_CPP_I_IMPL_H

#include "slow_stop_cpp_base.h"

class slow_stop_cpp_i : public slow_stop_cpp_base
{
    ENABLE_LOGGING
    public:
        slow_stop_cpp_i(const char *uuid, const char *label);
        ~slow_stop_cpp_i();

        void constructor();

        int serviceFunction();
};

#endif // SLOW_STOP_CPP_I_IMPL_H
