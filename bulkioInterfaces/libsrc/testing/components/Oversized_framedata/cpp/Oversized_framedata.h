#ifndef OVERSIZED_FRAMEDATA_IMPL_H
#define OVERSIZED_FRAMEDATA_IMPL_H

#include "Oversized_framedata_base.h"

class Oversized_framedata_i : public Oversized_framedata_base
{
    ENABLE_LOGGING
    public:
        Oversized_framedata_i(const char *uuid, const char *label);
        ~Oversized_framedata_i();
        int serviceFunction();
        BULKIO::StreamSRI sri;
};

#endif // OVERSIZED_FRAMEDATA_IMPL_H
