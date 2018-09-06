#ifndef STREAMWRITECHECK_I_IMPL_H
#define STREAMWRITECHECK_I_IMPL_H

#include "StreamWriteCheck_base.h"

class StreamWriteCheck_i : public StreamWriteCheck_base
{
    ENABLE_LOGGING
    public:
        StreamWriteCheck_i(const char *uuid, const char *label);
        ~StreamWriteCheck_i();

        void constructor();

        int serviceFunction();
        bulkio::OutLongStream outputStream;
};

#endif // STREAMWRITECHECK_I_IMPL_H
