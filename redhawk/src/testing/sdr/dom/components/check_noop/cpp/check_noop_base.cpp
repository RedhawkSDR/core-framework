#include "check_noop_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

check_noop_base::check_noop_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    setThreadName(label);

    loadProperties();
}

check_noop_base::~check_noop_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void check_noop_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void check_noop_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void check_noop_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void check_noop_base::loadProperties()
{
    addProperty(noop_delay,
                -1,
                "noop_delay",
                "noop_delay",
                "readwrite",
                "ms",
                "external",
                "property");

    addProperty(evaluate,
                "done",
                "evaluate",
                "evaluate",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(iterations,
                30,
                "iterations",
                "iterations",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(average_delay,
                "average_delay",
                "average_delay",
                "readonly",
                "ms",
                "external",
                "property");

}


