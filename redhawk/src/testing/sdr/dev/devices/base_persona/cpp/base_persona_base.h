#ifndef BASE_PERSONA_BASE_IMPL_BASE_H
#define BASE_PERSONA_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/ExecutableDevice_impl.h>
#include <ossie/ThreadedComponent.h>


class base_persona_base : public ExecutableDevice_impl, protected ThreadedComponent
{
    public:
        base_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl);
        base_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, char *compDev);
        base_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities);
        base_persona_base(char *devMgr_ior, char *id, char *lbl, char *sftwrPrfl, CF::Properties capacities, char *compDev);
        ~base_persona_base();

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
        /// Property: processor_name
        std::string processor_name;
        /// Property: os_name
        std::string os_name;
        /// Property: os_version
        std::string os_version;

    private:
        void construct();
};
#endif // BASE_PERSONA_BASE_IMPL_BASE_H
