#ifndef STREAM_SNK_I_IMPL_H
#define STREAM_SNK_I_IMPL_H

#include "stream_snk_base.h"

class stream_snk_i : public stream_snk_base
{
    ENABLE_LOGGING
    public:
        stream_snk_i(const char *uuid, const char *label);
        ~stream_snk_i();

        void constructor();

        int serviceFunction();
};

#endif // STREAM_SNK_I_IMPL_H
