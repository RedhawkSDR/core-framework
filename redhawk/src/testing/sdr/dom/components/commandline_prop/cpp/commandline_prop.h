#ifndef COMMANDLINE_PROP_I_IMPL_H
#define COMMANDLINE_PROP_I_IMPL_H

#include "commandline_prop_base.h"

class commandline_prop_i : public commandline_prop_base
{
    ENABLE_LOGGING
    public:
        commandline_prop_i(const char *uuid, const char *label);
        ~commandline_prop_i();

        void constructor();

        int serviceFunction();
};

#endif // COMMANDLINE_PROP_I_IMPL_H
