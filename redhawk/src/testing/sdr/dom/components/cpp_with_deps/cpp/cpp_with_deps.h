#ifndef CPP_WITH_DEPS_I_IMPL_H
#define CPP_WITH_DEPS_I_IMPL_H

#include "cpp_with_deps_base.h"

class cpp_with_deps_i : public cpp_with_deps_base
{
    ENABLE_LOGGING
    public:
        cpp_with_deps_i(const char *uuid, const char *label);
        ~cpp_with_deps_i();

        void constructor();

        int serviceFunction();
};

#endif // CPP_WITH_DEPS_I_IMPL_H
