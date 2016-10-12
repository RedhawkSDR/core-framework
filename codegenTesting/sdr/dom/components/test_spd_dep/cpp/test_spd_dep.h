#ifndef TEST_SPD_DEP_IMPL_H
#define TEST_SPD_DEP_IMPL_H

#include "test_spd_dep_base.h"

class test_spd_dep_i : public test_spd_dep_base
{
    ENABLE_LOGGING
    public:
        test_spd_dep_i(const char *uuid, const char *label);
        ~test_spd_dep_i();
        int serviceFunction();
};

#endif
