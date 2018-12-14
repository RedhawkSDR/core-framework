#include "time_cp_now_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

time_cp_now_base::time_cp_now_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    setThreadName(label);

    loadProperties();
}

time_cp_now_base::~time_cp_now_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void time_cp_now_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void time_cp_now_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void time_cp_now_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void time_cp_now_base::loadProperties()
{
    addProperty(rightnow,
                "now",
                "rightnow",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(simple1970,
                "1970:01:01::00:00:00",
                "simple1970",
                "",
                "readwrite",
                "",
                "external",
                "property");

    // Set the sequence with its initial values
    simpleSeqDefNow.push_back(redhawk::time::utils::convert("now"));
    addProperty(simpleSeqDefNow,
                simpleSeqDefNow,
                "simpleSeqDefNow",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(simpleSeqNoDef,
                "simpleSeqNoDef",
                "",
                "readwrite",
                "",
                "external",
                "property");

    // Set the sequence with its initial values
    simpleSeq1970.push_back(redhawk::time::utils::convert("1970:01:01::00:00:00"));
    addProperty(simpleSeq1970,
                simpleSeq1970,
                "simpleSeq1970",
                "",
                "readwrite",
                "",
                "external",
                "property");

}


