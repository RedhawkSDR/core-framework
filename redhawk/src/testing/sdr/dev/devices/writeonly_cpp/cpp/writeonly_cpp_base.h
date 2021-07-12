#ifndef WRITEONLY_CPP_BASE_IMPL_BASE_H
#define WRITEONLY_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class writeonly_cpp_base : public Device_impl, protected ThreadedComponent
{
    public:
        writeonly_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        writeonly_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        writeonly_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        writeonly_cpp_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~writeonly_cpp_base();

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
        /// Property: foo
        std::string foo;
        /// Property: foo_seq
        std::vector<std::string> foo_seq;
        /// Property: foo_struct
        foo_struct_struct foo_struct;
        /// Property: foo_struct_seq
        std::vector<ghi_struct> foo_struct_seq;

    private:
        void construct();
};
#endif // WRITEONLY_CPP_BASE_IMPL_BASE_H
