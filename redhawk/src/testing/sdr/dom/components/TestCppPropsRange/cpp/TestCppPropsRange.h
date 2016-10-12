#ifndef TESTCPPPROPSRANGE_IMPL_H
#define TESTCPPPROPSRANGE_IMPL_H

#include "TestCppPropsRange_base.h"

class TestCppPropsRange_i : public TestCppPropsRange_base
{
    ENABLE_LOGGING
    public:
        TestCppPropsRange_i(const char *uuid, const char *label);
        ~TestCppPropsRange_i();
        int serviceFunction();
};

#endif // TESTCPPPROPSRANGE_IMPL_H
