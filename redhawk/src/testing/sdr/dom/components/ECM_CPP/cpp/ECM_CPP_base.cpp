#include "ECM_CPP_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

ECM_CPP_base::ECM_CPP_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    loadProperties();
}

ECM_CPP_base::~ECM_CPP_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void ECM_CPP_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void ECM_CPP_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void ECM_CPP_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void ECM_CPP_base::loadProperties()
{
    addProperty(msg_recv,
                0,
                "msg_recv",
                "msg_recv",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(msg_xmit,
                0,
                "msg_xmit",
                "msg_xmit",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(msg_limit,
                5,
                "msg_limit",
                "msg_limit",
                "readwrite",
                "",
                "external",
                "configure");

}


