#include "sri_changed_cpp_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

sri_changed_cpp_base::sri_changed_cpp_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    ThreadedComponent()
{
    loadProperties();

    input = new bulkio::InShortPort("input");
    addPort("input", input);
}

sri_changed_cpp_base::~sri_changed_cpp_base()
{
    delete input;
    input = 0;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void sri_changed_cpp_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Resource_impl::start();
    ThreadedComponent::startThread();
}

void sri_changed_cpp_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Resource_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void sri_changed_cpp_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Resource_impl::releaseObject();
}

void sri_changed_cpp_base::loadProperties()
{
    addProperty(changed,
                false,
                "changed",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(verified,
                false,
                "verified",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}


