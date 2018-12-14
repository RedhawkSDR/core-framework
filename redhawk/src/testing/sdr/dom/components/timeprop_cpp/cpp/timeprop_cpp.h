#ifndef TIMEPROP_CPP_I_IMPL_H
#define TIMEPROP_CPP_I_IMPL_H

#include "timeprop_cpp_base.h"

class timeprop_cpp_i : public timeprop_cpp_base
{
    ENABLE_LOGGING
    public:
        timeprop_cpp_i(const char *uuid, const char *label);
        ~timeprop_cpp_i();

        void constructor();

        int serviceFunction();
};

#endif // TIMEPROP_CPP_I_IMPL_H
