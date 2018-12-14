#ifndef DEV_ALLOC_CPP_I_IMPL_H
#define DEV_ALLOC_CPP_I_IMPL_H

#include "dev_alloc_cpp_base.h"

class dev_alloc_cpp_i : public dev_alloc_cpp_base
{
    ENABLE_LOGGING
    public:
        dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        dev_alloc_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~dev_alloc_cpp_i();

        void constructor();

        int serviceFunction();
        bool alloc_s_prop(const s_prop_struct &value);
        void dealloc_s_prop(const s_prop_struct &value);
        bool alloc_si_prop(const short &value);
        void dealloc_si_prop(const short &value);
        bool alloc_se_prop(const std::vector<float> &value);
        void dealloc_se_prop(const std::vector<float> &value);
        bool alloc_sq_prop(const std::vector<sq_prop_s_struct> &value);
        void dealloc_sq_prop(const std::vector<sq_prop_s_struct> &value);

    protected:
        void updateUsageState();
};

#endif // DEV_ALLOC_CPP_I_IMPL_H
