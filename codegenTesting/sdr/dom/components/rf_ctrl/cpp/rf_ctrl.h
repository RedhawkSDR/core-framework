#ifndef RF_CTRL_I_IMPL_H
#define RF_CTRL_I_IMPL_H

#include "rf_ctrl_base.h"

class rf_ctrl_i : public rf_ctrl_base
{
    ENABLE_LOGGING
    public:
        rf_ctrl_i(const char *uuid, const char *label);
        ~rf_ctrl_i();

        void constructor();

        int serviceFunction();
};

#endif // RF_CTRL_I_IMPL_H
