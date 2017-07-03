#ifndef CHECK_CWD_CPP_SO_I_IMPL_H
#define CHECK_CWD_CPP_SO_I_IMPL_H

#include "check_cwd_cpp_so_base.h"

class check_cwd_cpp_so_i : public check_cwd_cpp_so_base
{
    ENABLE_LOGGING
    public:
        check_cwd_cpp_so_i(const char *uuid, const char *label);
        ~check_cwd_cpp_so_i();

        void constructor();

        int serviceFunction();
};

#endif // CHECK_CWD_CPP_SO_I_IMPL_H
