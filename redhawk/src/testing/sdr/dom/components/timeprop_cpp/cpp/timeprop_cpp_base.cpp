#include "timeprop_cpp_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

timeprop_cpp_base::timeprop_cpp_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    setThreadName(label);

    loadProperties();
}

timeprop_cpp_base::~timeprop_cpp_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void timeprop_cpp_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void timeprop_cpp_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void timeprop_cpp_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void timeprop_cpp_base::loadProperties()
{
    addProperty(prop,
                "value",
                "prop",
                "",
                "readwrite",
                "",
                "external",
                "property");

}


