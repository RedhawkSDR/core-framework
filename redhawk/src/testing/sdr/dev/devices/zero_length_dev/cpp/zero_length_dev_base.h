#ifndef ZERO_LENGTH_DEV_BASE_IMPL_BASE_H
#define ZERO_LENGTH_DEV_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Device_impl.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class zero_length_dev_base : public Device_impl, protected ThreadedComponent
{
    public:
        zero_length_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        zero_length_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        zero_length_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        zero_length_dev_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~zero_length_dev_base();

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
        /// Property: mystruct
        mystruct_struct mystruct;

    private:
        void construct();
};
#endif // ZERO_LENGTH_DEV_BASE_IMPL_BASE_H
