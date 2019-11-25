#ifndef SUPERSIMPLE_I_IMPL_H
#define SUPERSIMPLE_I_IMPL_H

#include "supersimple_base.h"

class supersimple_i : public supersimple_base
{
    ENABLE_LOGGING
    public:
        supersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        supersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        supersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        supersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~supersimple_i();

        void constructor();

        int serviceFunction();

    protected:

        void updateUsageState();
    private:
};

#endif // SUPERSIMPLE_I_IMPL_H
