#ifndef STREAM_SRC_I_IMPL_H
#define STREAM_SRC_I_IMPL_H

#include "stream_src_base.h"

class stream_src_i : public stream_src_base
{
    ENABLE_LOGGING
    public:
        stream_src_i(const char *uuid, const char *label);
        ~stream_src_i();

        void constructor();

        int serviceFunction();
};

#endif // STREAM_SRC_I_IMPL_H
