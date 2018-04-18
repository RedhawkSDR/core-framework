#ifndef ZERO_LENGTH_DEV_I_IMPL_H
#define ZERO_LENGTH_DEV_I_IMPL_H

#include "zero_length_dev_base.h"

class zero_length_dev_i : public zero_length_dev_base
{
    ENABLE_LOGGING
    public:
        zero_length_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        zero_length_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        zero_length_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        zero_length_dev_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~zero_length_dev_i();

        void constructor();

        int serviceFunction();

    protected:
        void updateUsageState();
};

#endif // ZERO_LENGTH_DEV_I_IMPL_H
