#ifndef LOG_TEST_CPP_I_IMPL_H
#define LOG_TEST_CPP_I_IMPL_H

#include "log_test_cpp_base.h"

class log_test_cpp_i : public log_test_cpp_base
{
    ENABLE_LOGGING
    public:
        log_test_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        log_test_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        log_test_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        log_test_cpp_i(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~log_test_cpp_i();

        void constructor();

        int serviceFunction();
        rh_logger::LoggerPtr baseline_1_logger;
        rh_logger::LoggerPtr baseline_2_logger;
        rh_logger::LoggerPtr namespaced_logger;
        rh_logger::LoggerPtr basetree_logger;

    protected:
        void updateUsageState();
};

#endif // LOG_TEST_CPP_I_IMPL_H
