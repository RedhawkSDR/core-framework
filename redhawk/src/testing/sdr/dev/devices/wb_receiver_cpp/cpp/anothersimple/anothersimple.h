#ifndef ANOTHERSIMPLE_I_IMPL_H
#define ANOTHERSIMPLE_I_IMPL_H

#include "anothersimple_base.h"

class anothersimple_i : public anothersimple_base
{
    ENABLE_LOGGING
    public:
        anothersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        anothersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        anothersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        anothersimple_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~anothersimple_i();

        void constructor();

        int serviceFunction();

    protected:

        void updateUsageState();
    private:
};

#endif // ANOTHERSIMPLE_I_IMPL_H
