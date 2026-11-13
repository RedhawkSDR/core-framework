#ifndef LOG_TEST_CPP_BASE_IMPL_BASE_H
#define LOG_TEST_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>


class log_test_cpp_base : public Device_impl, protected ThreadedComponent
{
    public:
        log_test_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        log_test_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        log_test_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        log_test_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~log_test_cpp_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: device_kind
        std::string device_kind;
        /// Property: device_model
        std::string device_model;

    private:
        void construct();
};
#endif // LOG_TEST_CPP_BASE_IMPL_BASE_H
