#ifndef DEV_ALLOC_CPP_BASE_IMPL_BASE_H
#define DEV_ALLOC_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class dev_alloc_cpp_base : public Device_impl, protected ThreadedComponent
{
    public:
        dev_alloc_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        dev_alloc_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        dev_alloc_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        dev_alloc_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~dev_alloc_cpp_base();

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
        /// Property: si_prop
        short si_prop;
        /// Property: se_prop
        std::vector<float> se_prop;
        /// Property: s_prop
        s_prop_struct s_prop;
        /// Property: sq_prop
        std::vector<sq_prop_s_struct> sq_prop;

    private:
        void construct();
};
#endif // DEV_ALLOC_CPP_BASE_IMPL_BASE_H
