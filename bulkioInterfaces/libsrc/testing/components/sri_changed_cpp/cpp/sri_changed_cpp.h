#ifndef SRI_CHANGED_CPP_IMPL_H
#define SRI_CHANGED_CPP_IMPL_H

#include "sri_changed_cpp_base.h"

class sri_changed_cpp_i : public sri_changed_cpp_base
{
    ENABLE_LOGGING
    public:
        sri_changed_cpp_i(const char *uuid, const char *label);
        ~sri_changed_cpp_i();
        int serviceFunction();
};

#endif // SRI_CHANGED_CPP_IMPL_H
