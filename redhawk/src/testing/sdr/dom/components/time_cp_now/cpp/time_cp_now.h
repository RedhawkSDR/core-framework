#ifndef TIME_CP_NOW_I_IMPL_H
#define TIME_CP_NOW_I_IMPL_H

#include "time_cp_now_base.h"

class time_cp_now_i : public time_cp_now_base
{
    ENABLE_LOGGING
    public:
        time_cp_now_i(const char *uuid, const char *label);
        ~time_cp_now_i();

        void constructor();

        int serviceFunction();
};

#endif // TIME_CP_NOW_I_IMPL_H
