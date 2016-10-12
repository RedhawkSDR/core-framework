#ifndef C1_IMPL_H
#define C1_IMPL_H

#include "C1_base.h"

class C1_i : public C1_base
{
    ENABLE_LOGGING
    public:
        C1_i(const char *uuid, const char *label);
        ~C1_i();
        int serviceFunction();
};

#endif // C1_IMPL_H
