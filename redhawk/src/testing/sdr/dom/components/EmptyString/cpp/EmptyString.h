#ifndef EMPTYSTRING_I_IMPL_H
#define EMPTYSTRING_I_IMPL_H

#include "EmptyString_base.h"

class EmptyString_i : public EmptyString_base
{
    ENABLE_LOGGING
    public:
        EmptyString_i(const char *uuid, const char *label);
        ~EmptyString_i();

        void constructor();

        int serviceFunction();
};

#endif // EMPTYSTRING_I_IMPL_H
