#ifndef PROPERTY_CPP_BASE_IMPL_BASE_H
#define PROPERTY_CPP_BASE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Component.h>
#include <ossie/ThreadedComponent.h>

#include "struct_props.h"

class Property_CPP_base : public Component, protected ThreadedComponent
{
    public:
        Property_CPP_base(const char *uuid, const char *label);
        ~Property_CPP_base();

        void start() throw (CF::Resource::StartError, CORBA::SystemException);

        void stop() throw (CF::Resource::StopError, CORBA::SystemException);

        void releaseObject() throw (CF::LifeCycle::ReleaseError, CORBA::SystemException);

        void loadProperties();

    protected:
        // Member variables exposed as properties
        std::string p1;
        double p2;
        CORBA::Long p3;
        p4_struct p4;

    private:
};
#endif // PROPERTY_CPP_BASE_IMPL_BASE_H
