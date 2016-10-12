#include "Property_CPP_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

Property_CPP_base::Property_CPP_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    loadProperties();
}

Property_CPP_base::~Property_CPP_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void Property_CPP_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void Property_CPP_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void Property_CPP_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void Property_CPP_base::loadProperties()
{
    addProperty(p1,
                "p1",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(p2,
                "p2",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(p3,
                "p3",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(p4,
                p4_struct(),
                "p4",
                "",
                "readwrite",
                "",
                "external",
                "property");

}


