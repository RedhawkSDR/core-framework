#ifndef DEVCPP_I_IMPL_H
#define DEVCPP_I_IMPL_H

#include "devcpp_base.h"

class devcpp_i : public devcpp_base
{
    ENABLE_LOGGING
    public:
        devcpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        devcpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        devcpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        devcpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~devcpp_i();

        void constructor();

        int serviceFunction();
        void set_foobar( int value );


    protected:
        void updateUsageState();
};

#endif // DEVCPP_I_IMPL_H
