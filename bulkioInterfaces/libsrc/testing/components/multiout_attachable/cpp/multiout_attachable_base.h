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
#ifndef MULTIOUT_ATTACHABLE_IMPL_BASE_H
#define MULTIOUT_ATTACHABLE_IMPL_BASE_H

#include <boost/thread.hpp>
#include <ossie/Resource_impl.h>
#include <ossie/ThreadedComponent.h>

#include <bulkio.h>
#include "struct_props.h"

class multiout_attachable_base : public Resource_impl, protected ThreadedComponent
{
    public:
        multiout_attachable_base(const char *uuid, const char *label);
        ~multiout_attachable_base();

        void start();

        void stop();

        void releaseObject();

        void loadProperties();

    protected:
        void connectionTableChanged(const std::vector<connection_descriptor_struct>* oldValue, const std::vector<connection_descriptor_struct>* newValue);

        // Member variables exposed as properties
        unsigned short packets_ingested;
        callback_stats_struct callback_stats;
        std::vector<connection_descriptor_struct> connectionTable;
        std::vector<SDDSStreamDefinition_struct> SDDSStreamDefinitions;
        std::vector<VITA49StreamDefinition_struct> VITA49StreamDefinitions;
        std::vector<sdds_attachment_struct> received_sdds_attachments;
        std::vector<vita49_attachment_struct> received_vita49_attachments;

        // Ports
        bulkio::InSDDSPort *dataSDDS_in;
        bulkio::InVITA49Port *dataVITA49_in;
        bulkio::InFloatPort *dataFloat_in;
        bulkio::OutSDDSPort *dataSDDS_out;
        bulkio::OutVITA49Port *dataVITA49_out;

    private:
};
#endif // MULTIOUT_ATTACHABLE_IMPL_BASE_H
