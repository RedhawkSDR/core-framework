#ifndef CPP_WB_RECEIVER_I_IMPL_H
#define CPP_WB_RECEIVER_I_IMPL_H

#include "cpp_wb_receiver_base.h"

class cpp_wb_receiver_i : public cpp_wb_receiver_base
{
    ENABLE_LOGGING
    public:
        cpp_wb_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        cpp_wb_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        cpp_wb_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        cpp_wb_receiver_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~cpp_wb_receiver_i();

        void constructor();

        int serviceFunction();

    protected:

        void updateUsageState();
};

#endif // CPP_WB_RECEIVER_I_IMPL_H
