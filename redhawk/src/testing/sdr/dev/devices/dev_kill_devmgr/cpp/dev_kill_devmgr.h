#ifndef DEV_KILL_DEVMGR_I_IMPL_H
#define DEV_KILL_DEVMGR_I_IMPL_H

#include "dev_kill_devmgr_base.h"

class dev_kill_devmgr_i : public dev_kill_devmgr_base
{
    ENABLE_LOGGING
    public:
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        dev_kill_devmgr_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~dev_kill_devmgr_i();

        void constructor();

        int serviceFunction();

    protected:
        void updateUsageState();
};

#endif // DEV_KILL_DEVMGR_I_IMPL_H
