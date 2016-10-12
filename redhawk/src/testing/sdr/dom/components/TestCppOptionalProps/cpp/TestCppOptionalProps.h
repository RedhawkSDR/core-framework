#ifndef TESTCPPOPTIONALPROPS_IMPL_H
#define TESTCPPOPTIONALPROPS_IMPL_H

#include "TestCppOptionalProps_base.h"

class TestCppOptionalProps_i : public TestCppOptionalProps_base
{
    ENABLE_LOGGING
    public:
        TestCppOptionalProps_i(const char *uuid, const char *label);
        ~TestCppOptionalProps_i();
        int serviceFunction();

};

#endif // TESTCPPOPTIONALPROPS_IMPL_H
