#ifndef DEVC_I_IMPL_H
#define DEVC_I_IMPL_H

#include "DevC_base.h"

class DevC_i : public DevC_base
{
    ENABLE_LOGGING
    public:
        DevC_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        DevC_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        DevC_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        DevC_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~DevC_i();

        void constructor();

        int serviceFunction();

    protected:
        void updateUsageState();
};

#endif // DEVC_I_IMPL_H
