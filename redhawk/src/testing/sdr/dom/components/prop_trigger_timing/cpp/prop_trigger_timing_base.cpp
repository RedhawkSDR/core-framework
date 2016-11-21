/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK core.
 *
 * REDHAWK core is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "prop_trigger_timing_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

prop_trigger_timing_base::prop_trigger_timing_base(const char *uuid, const char *label) :
    Component(uuid, label),
    ThreadedComponent()
{
    loadProperties();
}

prop_trigger_timing_base::~prop_trigger_timing_base()
{
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void prop_trigger_timing_base::start() throw (CORBA::SystemException, CF::Resource::StartError)
{
    Component::start();
    ThreadedComponent::startThread();
}

void prop_trigger_timing_base::stop() throw (CORBA::SystemException, CF::Resource::StopError)
{
    Component::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void prop_trigger_timing_base::releaseObject() throw (CORBA::SystemException, CF::LifeCycle::ReleaseError)
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Component::releaseObject();
}

void prop_trigger_timing_base::loadProperties()
{
    addProperty(prop_1,
                "prop_1",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(prop_1_trigger,
                "prop_1_trigger",
                "",
                "readonly",
                "",
                "external",
                "property");

    addProperty(prop_2_trigger,
                "prop_2_trigger",
                "",
                "readonly",
                "",
                "external",
                "property");

    addProperty(prop_3_trigger,
                "prop_3_trigger",
                "",
                "readonly",
                "",
                "external",
                "property");

    addProperty(prop_4_trigger,
                "prop_4_trigger",
                "",
                "readonly",
                "",
                "external",
                "property");

    addProperty(prop_2,
                "prop_2",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(prop_3,
                prop_3_struct(),
                "prop_3",
                "",
                "readwrite",
                "",
                "external",
                "property");

    addProperty(prop_4,
                "prop_4",
                "",
                "readwrite",
                "",
                "external",
                "property");

}


