#ifndef C2_I_IMPL_H
#define C2_I_IMPL_H

#include "C2_base.h"

class C2_i : public C2_base
{
    ENABLE_LOGGING
    public:
        C2_i(const char *uuid, const char *label);
        ~C2_i();

        void constructor();

        int serviceFunction();
};

#endif // C2_I_IMPL_H
