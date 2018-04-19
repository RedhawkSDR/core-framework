#ifndef SVC_ERROR_CPP_I_IMPL_H
#define SVC_ERROR_CPP_I_IMPL_H

#include "svc_error_cpp_base.h"

class svc_error_cpp_i : public svc_error_cpp_base
{
    ENABLE_LOGGING
    public:
        svc_error_cpp_i(const char *uuid, const char *label);
        ~svc_error_cpp_i();

        void constructor();

        int serviceFunction();
};

#endif // SVC_ERROR_CPP_I_IMPL_H
