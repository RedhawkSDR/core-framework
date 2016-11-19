#ifndef CHECK_CWD_CPP_I_IMPL_H
#define CHECK_CWD_CPP_I_IMPL_H

#include "check_cwd_cpp_base.h"

class check_cwd_cpp_i : public check_cwd_cpp_base
{
    ENABLE_LOGGING
    public:
        check_cwd_cpp_i(const char *uuid, const char *label);
        ~check_cwd_cpp_i();

        void constructor();

        int serviceFunction();
};

#endif // CHECK_CWD_CPP_I_IMPL_H
