#ifndef COMP_I_IMPL_H
#define COMP_I_IMPL_H

#include "comp_base.h"

class comp_i : public comp_base
{
    ENABLE_LOGGING
    public:
        comp_i(const char *uuid, const char *label);
        ~comp_i();
        int serviceFunction();
};

#endif // COMP_I_IMPL_H
