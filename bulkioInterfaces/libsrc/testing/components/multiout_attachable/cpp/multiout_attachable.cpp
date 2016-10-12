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
/**************************************************************************

    This is the component code. This file contains the child class where
    custom functionality can be added to the component. Custom
    functionality to the base class can be extended here. Access to
    the ports can also be done from this class

**************************************************************************/

#include "multiout_attachable.h"

PREPARE_LOGGING(multiout_attachable_i)

multiout_attachable_i::multiout_attachable_i(const char *uuid, const char *label) :
    multiout_attachable_base(uuid, label)
{
    addPropertyChangeListener("SDDSStreamDefinitions", this, &multiout_attachable_i::sddsStreamDefChanged);
    addPropertyChangeListener("VITA49StreamDefinitions", this, &multiout_attachable_i::vita49StreamDefChanged);

    this->dataSDDS_out->setLogger(this->__logger);
    this->dataSDDS_in->setLogger(this->__logger);
    this->dataSDDS_in->setNewAttachDetachCallback(this);
    this->dataSDDS_in->setNewSriListener(this, &multiout_attachable_i::newSriCallback);
    this->dataSDDS_in->setSriChangeListener(this, &multiout_attachable_i::sriChangeCallback);

    this->dataVITA49_out->setLogger(this->__logger);
    this->dataVITA49_in->setLogger(this->__logger);
    this->dataVITA49_in->setNewAttachDetachCallback(this);
    this->dataVITA49_in->setNewSriListener(this, &multiout_attachable_i::newSriCallback);
    this->dataVITA49_in->setSriChangeListener(this, &multiout_attachable_i::sriChangeCallback);
}

multiout_attachable_i::~multiout_attachable_i()
{
}

char* multiout_attachable_i::attach(const BULKIO::SDDSStreamDefinition& stream, const char* userid)
        throw (BULKIO::dataSDDS::AttachError, BULKIO::dataSDDS::StreamInputError) {

	this->callback_stats.num_sdds_attaches++;
	std::string aid = ossie::generateUUID();
	LOG_DEBUG(multiout_attachable_i, "Received SDDS::AttachCallback: ATTACH ID: " << aid);

	sdds_attachment_struct newAttachment;
	newAttachment.streamId = stream.id;
	newAttachment.attachId = aid;
	newAttachment.port = stream.port;
	this->received_sdds_attachments.push_back(newAttachment);

	return CORBA::string_dup(aid.c_str());
}

char* multiout_attachable_i::attach(const BULKIO::VITA49StreamDefinition& stream, const char* userid)
        throw (BULKIO::dataVITA49::AttachError, BULKIO::dataVITA49::StreamInputError) {
	this->callback_stats.num_vita49_attaches++;
	std::string aid = ossie::generateUUID();
	LOG_DEBUG(multiout_attachable_i, "Received VITA49::AttachCallback: ATTACH ID: " << aid);

    vita49_attachment_struct newAttachment;
	newAttachment.streamId = stream.id;
	newAttachment.attachId = aid;
	newAttachment.port = stream.port;
	this->received_vita49_attachments.push_back(newAttachment);

	return CORBA::string_dup(aid.c_str());
}

void multiout_attachable_i::detach(const char* attachId) {
	LOG_DEBUG(multiout_attachable_i, "Received DetachCallback: ATTACH ID: " << attachId);

	std::vector<sdds_attachment_struct>::iterator iter;
	for (iter = this->received_sdds_attachments.begin(); iter != this->received_sdds_attachments.end();) {
		if (iter->attachId == std::string(attachId)) {
			this->received_sdds_attachments.erase(iter);
			this->callback_stats.num_sdds_detaches++;
		} else {
			iter++;
		}
	}

	std::vector<vita49_attachment_struct>::iterator vIter;
	for (vIter = this->received_vita49_attachments.begin(); vIter != this->received_vita49_attachments.end();) {
		if (vIter->attachId == std::string(attachId)) {
			this->received_vita49_attachments.erase(vIter);
			this->callback_stats.num_vita49_detaches++;
		} else {
			vIter++;
		}
	}
}

int multiout_attachable_i::serviceFunction()
{
    LOG_DEBUG(multiout_attachable_i, "serviceFunction() example log message");

    bulkio::InFloatPort::dataTransfer *tmp = dataFloat_in->getPacket(bulkio::Const::BLOCKING);
    if (not tmp) { // No data is available
        return NOOP;
    }
    if (tmp->sriChanged) {
        dataSDDS_out->pushSRI(tmp->SRI, bulkio::time::utils::now());
        dataVITA49_out->pushSRI(tmp->SRI, bulkio::time::utils::now());
    }

    this->packets_ingested++;
    return NORMAL;
}

void multiout_attachable_i::vita49StreamDefChanged(Vita49StreamDefs *oldValue,
													   Vita49StreamDefs *newValue)
{
    std::set<VITA49StreamDefinition_struct> removedEntries;
    std::set<VITA49StreamDefinition_struct> addedEntries;

	// Grab all the attachIds
	std::vector<std::string> oldAttachIds;
	std::vector<std::string> newAttachIds;
	Vita49StreamDefs::const_iterator iter, iter2;
	for (iter=oldValue->begin(); iter!=oldValue->end(); iter++) oldAttachIds.push_back(iter->id);
	for (iter=newValue->begin(); iter!=newValue->end(); iter++) newAttachIds.push_back(iter->id);

	// Find which old streams need to be detached
	for (iter=oldValue->begin(); iter!=oldValue->end(); iter++) {
		if (std::find(newAttachIds.begin(), newAttachIds.end(), iter->id) == newAttachIds.end()) {
			this->dataVITA49_out->removeStream(iter->id.c_str());
		}
	}

	// Find which new streams need to be attached
	for (iter=newValue->begin(); iter!=newValue->end(); iter++) {
		// Check if the entry is new
		bool isNew = std::find(oldAttachIds.begin(), oldAttachIds.end(), iter->id) == oldAttachIds.end();

		// Check if the entry was updated
		bool isUpdated = false;
		for (iter2=oldValue->begin(); iter2!=oldValue->end(); iter2++) {
			if (iter2->id == iter->id) {
				isUpdated = (iter->port != iter2->port);
			}
		}

        BULKIO::VITA49StreamDefinition newAttachment;
        newAttachment.id = iter->id.c_str();
        newAttachment.ip_address = iter->ip_address.c_str();
        newAttachment.port = iter->port;
        newAttachment.protocol = BULKIO::VITA49_TCP_TRANSPORT;
        newAttachment.valid_data_format = iter->valid_data_format;
        newAttachment.vlan = iter->vlan;
        newAttachment.data_format.channel_tag_size = iter->channel_tag_size;
        newAttachment.data_format.complexity = BULKIO::VITA49_COMPLEX_CARTESIAN;
        newAttachment.data_format.data_item_format = BULKIO::VITA49_32F;
        newAttachment.data_format.data_item_size = iter->data_item_size;
        newAttachment.data_format.event_tag_size = iter->event_tag_size;
        newAttachment.data_format.item_packing_field_size = iter->item_packing_field_size;
        newAttachment.data_format.packing_method_processing_efficient = iter->packing_method_processing_efficient;
        newAttachment.data_format.repeat_count = iter->repeat_count;
        newAttachment.data_format.repeating = iter->repeating;
        newAttachment.data_format.vector_size = iter->vector_size;
		
        if (isNew) this->dataVITA49_out->addStream(newAttachment);
        if (isUpdated) this->dataVITA49_out->updateStream(newAttachment);
	}

	this->VITA49StreamDefinitions = *newValue;
}

void multiout_attachable_i::sddsStreamDefChanged(SddsStreamDefs *oldValue,
													 SddsStreamDefs *newValue)
{
	    std::set<SDDSStreamDefinition_struct> removedEntries;
	    std::set<SDDSStreamDefinition_struct> addedEntries;

		// Grab all the attachIds
		std::vector<std::string> oldAttachIds;
		std::vector<std::string> newAttachIds;
		SddsStreamDefs::const_iterator iter, iter2;

		for (iter=oldValue->begin(); iter!=oldValue->end(); iter++) oldAttachIds.push_back(iter->id);
		for (iter=newValue->begin(); iter!=newValue->end(); iter++) newAttachIds.push_back(iter->id);

		// Find which old streams need to be detached
		for (iter=oldValue->begin(); iter!=oldValue->end(); iter++) {
			if (std::find(newAttachIds.begin(), newAttachIds.end(), iter->id) == newAttachIds.end()) {
				this->dataSDDS_out->removeStream(iter->id.c_str());
			}
		}

		// Find which new streams need to be attached
		for (iter=newValue->begin(); iter!=newValue->end(); iter++) {
			// Check if the entry is new
			bool isNew = std::find(oldAttachIds.begin(), oldAttachIds.end(), iter->id) == oldAttachIds.end();

			// Check if the entry was updated
			bool isUpdated = false;
			for (iter2=oldValue->begin(); iter2!=oldValue->end(); iter2++) {
				if (iter2->id == iter->id) {
					isUpdated = (iter->port != iter2->port);
				}
			}

            BULKIO::SDDSStreamDefinition newAttachment;
            newAttachment.id = iter->id.c_str();
            newAttachment.port = iter->port;
            newAttachment.vlan = iter->vlan;
            newAttachment.dataFormat = BULKIO::SDDS_CF;
            newAttachment.multicastAddress = iter->multicastAddress.c_str();
            newAttachment.privateInfo = iter->privateInfo.c_str();
            newAttachment.sampleRate = iter->sampleRate;
            newAttachment.timeTagValid = iter->timeTagValid;
            
            if (isNew) this->dataSDDS_out->addStream(newAttachment);
            if (isUpdated) this->dataSDDS_out->updateStream(newAttachment);
		}

		this->SDDSStreamDefinitions = *newValue;
}

void multiout_attachable_i::newSriCallback(const BULKIO::StreamSRI& sri) {
    BULKIO::StreamSRISequence* sriList;
    //std::cout << "NewSRICallback-StreamID: " << sri.streamID << std::endl;
    //std::cout << "NewSRICallback-xdelta: " << sri.xdelta << std::endl;
    
    // Query SRIs to ensure deadlock doesn't occur
    sriList = this->dataSDDS_in->activeSRIs();
    delete sriList;

    // Query SRIs to ensure deadlock doesn't occur
    sriList = this->dataVITA49_in->activeSRIs();
    delete sriList;

	this->callback_stats.num_new_sri_callbacks++;
}

void multiout_attachable_i::sriChangeCallback(const BULKIO::StreamSRI& sri) {
    BULKIO::StreamSRISequence* sriList;

    //std::cout << "SRIChangeCallback-StreamID: " << sri.streamID << std::endl;
    //std::cout << "SRIChangeCallback-xdelta: " << sri.xdelta << std::endl;
    
    // Query SRIs to ensure deadlock doesn't occur
    sriList = this->dataSDDS_in->activeSRIs();
    delete sriList;

    // Query SRIs to ensure deadlock doesn't occur
    sriList = this->dataVITA49_in->activeSRIs();
    delete sriList;

	this->callback_stats.num_sri_change_callbacks++;
}
