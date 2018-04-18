#ifndef ZERO_LENGTH_I_IMPL_H
#define ZERO_LENGTH_I_IMPL_H

#include "zero_length_base.h"

class zero_length_i : public zero_length_base
{
    ENABLE_LOGGING
    public:
        zero_length_i(const char *uuid, const char *label);
        ~zero_length_i();

        void constructor();

        int serviceFunction();
};

#endif // ZERO_LENGTH_I_IMPL_H
