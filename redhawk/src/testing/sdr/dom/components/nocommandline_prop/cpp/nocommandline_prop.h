#ifndef NOCOMMANDLINE_PROP_I_IMPL_H
#define NOCOMMANDLINE_PROP_I_IMPL_H

#include "nocommandline_prop_base.h"

class nocommandline_prop_i : public nocommandline_prop_base
{
    ENABLE_LOGGING
    public:
        nocommandline_prop_i(const char *uuid, const char *label);
        ~nocommandline_prop_i();

        void constructor();

        int serviceFunction();
};

#endif // NOCOMMANDLINE_PROP_I_IMPL_H
