#ifndef WRITEONLY_CPP_I_IMPL_H
#define WRITEONLY_CPP_I_IMPL_H

#include "writeonly_cpp_base.h"

class writeonly_cpp_i : public writeonly_cpp_base
{
    ENABLE_LOGGING
    public:
        writeonly_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        writeonly_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        writeonly_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        writeonly_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~writeonly_cpp_i();

        void constructor();

        int serviceFunction();

    protected:
        void updateUsageState();
};

#endif // WRITEONLY_CPP_I_IMPL_H
