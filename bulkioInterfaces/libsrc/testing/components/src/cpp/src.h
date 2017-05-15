#ifndef SRC_I_IMPL_H
#define SRC_I_IMPL_H

#include "src_base.h"

class src_i : public src_base
{
    ENABLE_LOGGING
    public:
        src_i(const char *uuid, const char *label);
        ~src_i();

        void constructor();

        int serviceFunction();
        bulkio::OutFloatStream stream;
};

#endif // SRC_I_IMPL_H
