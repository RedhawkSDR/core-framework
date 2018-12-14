#ifndef SNK_SLOW_I_IMPL_H
#define SNK_SLOW_I_IMPL_H

#include "snk_slow_base.h"

class snk_slow_i : public snk_slow_base
{
    ENABLE_LOGGING
    public:
        snk_slow_i(const char *uuid, const char *label);
        ~snk_slow_i();

        void constructor();

        int serviceFunction();
};

#endif // SNK_SLOW_I_IMPL_H
