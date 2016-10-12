#ifndef PROP_TRIGGER_TIMING_BASE_IMPL_BASE_H
#define PROP_TRIGGER_TIMING_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class prop_trigger_timing_base : public Component, protected ThreadedComponent
{
    public:
        prop_trigger_timing_base(const char *uuid, const char *label);
        ~prop_trigger_timing_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        /// Property: prop_1
        std::string prop_1;
        /// Property: prop_1_trigger
        bool prop_1_trigger;
        /// Property: prop_2_trigger
        bool prop_2_trigger;
        /// Property: prop_3_trigger
        bool prop_3_trigger;
        /// Property: prop_4_trigger
        bool prop_4_trigger;
        /// Property: prop_2
        std::vector<std::string> prop_2;
        /// Property: prop_3
        prop_3_struct prop_3;
        /// Property: prop_4
        std::vector<prop_4_a_struct> prop_4;

    private:
};
#endif // PROP_TRIGGER_TIMING_BASE_IMPL_BASE_H
