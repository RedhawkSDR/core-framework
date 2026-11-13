/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#include "multiout_attachable_base.h"

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

    The following class functions are for the base class for the component class. To
    customize any of these functions, do not modify them here. Instead, overload them
    on the child class

******************************************************************************************/

multiout_attachable_base::multiout_attachable_base(const char *uuid, const char *label) :
    Resource_impl(uuid, label),
    ThreadedComponent()
{
    loadProperties();

    dataSDDS_in = new bulkio::InSDDSPort("dataSDDS_in");
    addPort("dataSDDS_in", dataSDDS_in);
    dataVITA49_in = new bulkio::InVITA49Port("dataVITA49_in");
    addPort("dataVITA49_in", dataVITA49_in);
    dataFloat_in = new bulkio::InFloatPort("dataFloat_in");
    addPort("dataFloat_in", dataFloat_in);
    dataSDDS_out = new bulkio::OutSDDSPort("dataSDDS_out");
    addPort("dataSDDS_out", dataSDDS_out);
    dataVITA49_out = new bulkio::OutVITA49Port("dataVITA49_out");
    addPort("dataVITA49_out", dataVITA49_out);

    this->addPropertyChangeListener("connectionTable", this, &multiout_attachable_base::connectionTableChanged);
}

multiout_attachable_base::~multiout_attachable_base()
{
    delete dataSDDS_in;
    dataSDDS_in = 0;
    delete dataVITA49_in;
    dataVITA49_in = 0;
    delete dataFloat_in;
    dataFloat_in = 0;
    delete dataSDDS_out;
    dataSDDS_out = 0;
    delete dataVITA49_out;
    dataVITA49_out = 0;
}

/*******************************************************************************************
    Framework-level functions
    These functions are generally called by the framework to perform housekeeping.
*******************************************************************************************/
void multiout_attachable_base::start() 
{
    Resource_impl::start();
    ThreadedComponent::startThread();
}

void multiout_attachable_base::stop() 
{
    Resource_impl::stop();
    if (!ThreadedComponent::stopThread()) {
        throw CF::Resource::StopError(CF::CF_NOTSET, "Processing thread did not die");
    }
}

void multiout_attachable_base::releaseObject() 
{
    // This function clears the component running condition so main shuts down everything
    try {
        stop();
    } catch (CF::Resource::StopError& ex) {
        // TODO - this should probably be logged instead of ignored
    }

    Resource_impl::releaseObject();
}

void multiout_attachable_base::connectionTableChanged(const std::vector<connection_descriptor_struct>* oldValue, const std::vector<connection_descriptor_struct>* newValue)
{
    dataSDDS_out->updateConnectionFilter(*newValue);
    dataVITA49_out->updateConnectionFilter(*newValue);
}

void multiout_attachable_base::loadProperties()
{
    addProperty(packets_ingested,
                0,
                "packets_ingested",
                "packets_ingested",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(callback_stats,
                callback_stats_struct(),
                "callback_stats",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(connectionTable,
                "connectionTable",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(SDDSStreamDefinitions,
                "SDDSStreamDefinitions",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(VITA49StreamDefinitions,
                "VITA49StreamDefinitions",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(received_sdds_attachments,
                "received_sdds_attachments",
                "",
                "readwrite",
                "",
                "external",
                "configure");

    addProperty(received_vita49_attachments,
                "received_vita49_attachments",
                "",
                "readwrite",
                "",
                "external",
                "configure");

}


