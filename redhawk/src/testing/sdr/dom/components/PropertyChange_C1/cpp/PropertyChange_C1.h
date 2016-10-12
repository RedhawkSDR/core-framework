#ifndef PropertyChange_C1_IMPL_H
#define PropertyChange_C1_IMPL_H

#include "PropertyChange_C1_base.h"

class PropertyChange_C1_i : public PropertyChange_C1_base
{
    ENABLE_LOGGING
    public:
        PropertyChange_C1_i(const char *uuid, const char *label);
        ~PropertyChange_C1_i();
        int serviceFunction();
};

#endif // PropertyChange_C1_IMPL_H
