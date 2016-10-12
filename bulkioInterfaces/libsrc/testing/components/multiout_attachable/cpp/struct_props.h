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
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
//Need to comment this out to build locally 
//#include <bulkio/bulkio.h>
#include "bulkio.h"
typedef bulkio::connection_descriptor_struct connection_descriptor_struct;

struct callback_stats_struct {
    callback_stats_struct ()
    {
        num_sdds_attaches = 0;
        num_sdds_detaches = 0;
        num_vita49_attaches = 0;
        num_vita49_detaches = 0;
        num_new_sri_callbacks = 0;
        num_sri_change_callbacks = 0;
    };

    static std::string getId() {
        return std::string("callback_stats");
    };

    unsigned short num_sdds_attaches;
    unsigned short num_sdds_detaches;
    unsigned short num_vita49_attaches;
    unsigned short num_vita49_detaches;
    unsigned short num_new_sri_callbacks;
    unsigned short num_sri_change_callbacks;
};

inline bool operator>>= (const CORBA::Any& a, callback_stats_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("num_sdds_attaches", props[idx].id)) {
            if (!(props[idx].value >>= s.num_sdds_attaches)) return false;
        }
        else if (!strcmp("num_sdds_detaches", props[idx].id)) {
            if (!(props[idx].value >>= s.num_sdds_detaches)) return false;
        }
        else if (!strcmp("num_vita49_attaches", props[idx].id)) {
            if (!(props[idx].value >>= s.num_vita49_attaches)) return false;
        }
        else if (!strcmp("num_vita49_detaches", props[idx].id)) {
            if (!(props[idx].value >>= s.num_vita49_detaches)) return false;
        }
        else if (!strcmp("num_new_sri_callbacks", props[idx].id)) {
            if (!(props[idx].value >>= s.num_new_sri_callbacks)) return false;
        }
        else if (!strcmp("num_sri_change_callbacks", props[idx].id)) {
            if (!(props[idx].value >>= s.num_sri_change_callbacks)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const callback_stats_struct& s) {
    CF::Properties props;
    props.length(6);
    props[0].id = CORBA::string_dup("num_sdds_attaches");
    props[0].value <<= s.num_sdds_attaches;
    props[1].id = CORBA::string_dup("num_sdds_detaches");
    props[1].value <<= s.num_sdds_detaches;
    props[2].id = CORBA::string_dup("num_vita49_attaches");
    props[2].value <<= s.num_vita49_attaches;
    props[3].id = CORBA::string_dup("num_vita49_detaches");
    props[3].value <<= s.num_vita49_detaches;
    props[4].id = CORBA::string_dup("num_new_sri_callbacks");
    props[4].value <<= s.num_new_sri_callbacks;
    props[5].id = CORBA::string_dup("num_sri_change_callbacks");
    props[5].value <<= s.num_sri_change_callbacks;
    a <<= props;
};

inline bool operator== (const callback_stats_struct& s1, const callback_stats_struct& s2) {
    if (s1.num_sdds_attaches!=s2.num_sdds_attaches)
        return false;
    if (s1.num_sdds_detaches!=s2.num_sdds_detaches)
        return false;
    if (s1.num_vita49_attaches!=s2.num_vita49_attaches)
        return false;
    if (s1.num_vita49_detaches!=s2.num_vita49_detaches)
        return false;
    if (s1.num_new_sri_callbacks!=s2.num_new_sri_callbacks)
        return false;
    if (s1.num_sri_change_callbacks!=s2.num_sri_change_callbacks)
        return false;
    return true;
};

inline bool operator!= (const callback_stats_struct& s1, const callback_stats_struct& s2) {
    return !(s1==s2);
};

struct SDDSStreamDefinition_struct {
    SDDSStreamDefinition_struct ()
    {
        multicastAddress = "0.0.0.0";
        vlan = 0;
        port = 0;
        timeTagValid = false;
    };

    static std::string getId() {
        return std::string("SDDSStreamDefinition");
    };

    std::string id;
    std::string multicastAddress;
    CORBA::ULong vlan;
    CORBA::ULong port;
    CORBA::ULong sampleRate;
    bool timeTagValid;
    std::string privateInfo;
};

inline bool operator>>= (const CORBA::Any& a, SDDSStreamDefinition_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("sdds::id", props[idx].id)) {
            if (!(props[idx].value >>= s.id)) return false;
        }
        else if (!strcmp("sdds::multicastAddress", props[idx].id)) {
            if (!(props[idx].value >>= s.multicastAddress)) return false;
        }
        else if (!strcmp("sdds::vlan", props[idx].id)) {
            if (!(props[idx].value >>= s.vlan)) return false;
        }
        else if (!strcmp("sdds::port", props[idx].id)) {
            if (!(props[idx].value >>= s.port)) return false;
        }
        else if (!strcmp("sdds::sampleRate", props[idx].id)) {
            if (!(props[idx].value >>= s.sampleRate)) return false;
        }
        else if (!strcmp("sdds::timeTagValid", props[idx].id)) {
            if (!(props[idx].value >>= s.timeTagValid)) return false;
        }
        else if (!strcmp("sdds::privateInfo", props[idx].id)) {
            if (!(props[idx].value >>= s.privateInfo)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const SDDSStreamDefinition_struct& s) {
    CF::Properties props;
    props.length(7);
    props[0].id = CORBA::string_dup("sdds::id");
    props[0].value <<= s.id;
    props[1].id = CORBA::string_dup("sdds::multicastAddress");
    props[1].value <<= s.multicastAddress;
    props[2].id = CORBA::string_dup("sdds::vlan");
    props[2].value <<= s.vlan;
    props[3].id = CORBA::string_dup("sdds::port");
    props[3].value <<= s.port;
    props[4].id = CORBA::string_dup("sdds::sampleRate");
    props[4].value <<= s.sampleRate;
    props[5].id = CORBA::string_dup("sdds::timeTagValid");
    props[5].value <<= s.timeTagValid;
    props[6].id = CORBA::string_dup("sdds::privateInfo");
    props[6].value <<= s.privateInfo;
    a <<= props;
};

inline bool operator== (const SDDSStreamDefinition_struct& s1, const SDDSStreamDefinition_struct& s2) {
    if (s1.id!=s2.id)
        return false;
    if (s1.multicastAddress!=s2.multicastAddress)
        return false;
    if (s1.vlan!=s2.vlan)
        return false;
    if (s1.port!=s2.port)
        return false;
    if (s1.sampleRate!=s2.sampleRate)
        return false;
    if (s1.timeTagValid!=s2.timeTagValid)
        return false;
    if (s1.privateInfo!=s2.privateInfo)
        return false;
    return true;
};

inline bool operator!= (const SDDSStreamDefinition_struct& s1, const SDDSStreamDefinition_struct& s2) {
    return !(s1==s2);
};

struct VITA49StreamDefinition_struct {
    VITA49StreamDefinition_struct ()
    {
        ip_address = "0.0.0.0";
        vlan = 0;
        port = 0;
        valid_data_format = false;
        packing_method_processing_efficient = false;
        repeating = false;
        channel_tag_size = 0;
        item_packing_field_size = 0;
        data_item_size = 0;
        repeat_count = 0;
        vector_size = 0;
    };

    static std::string getId() {
        return std::string("VITA49StreamDefinition");
    };

    std::string id;
    std::string ip_address;
    CORBA::ULong vlan;
    CORBA::ULong port;
    bool valid_data_format;
    bool packing_method_processing_efficient;
    bool repeating;
    CORBA::Long event_tag_size;
    CORBA::Long channel_tag_size;
    CORBA::Long item_packing_field_size;
    CORBA::Long data_item_size;
    CORBA::Long repeat_count;
    CORBA::Long vector_size;
};

inline bool operator>>= (const CORBA::Any& a, VITA49StreamDefinition_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("vita49::id", props[idx].id)) {
            if (!(props[idx].value >>= s.id)) return false;
        }
        else if (!strcmp("vita49::ip_address", props[idx].id)) {
            if (!(props[idx].value >>= s.ip_address)) return false;
        }
        else if (!strcmp("vita49::vlan", props[idx].id)) {
            if (!(props[idx].value >>= s.vlan)) return false;
        }
        else if (!strcmp("vita49::port", props[idx].id)) {
            if (!(props[idx].value >>= s.port)) return false;
        }
        else if (!strcmp("vita49::valid_data_format", props[idx].id)) {
            if (!(props[idx].value >>= s.valid_data_format)) return false;
        }
        else if (!strcmp("vita49::packing_method_processing_efficient", props[idx].id)) {
            if (!(props[idx].value >>= s.packing_method_processing_efficient)) return false;
        }
        else if (!strcmp("vita49::repeating", props[idx].id)) {
            if (!(props[idx].value >>= s.repeating)) return false;
        }
        else if (!strcmp("vita49::event_tag_size", props[idx].id)) {
            if (!(props[idx].value >>= s.event_tag_size)) return false;
        }
        else if (!strcmp("vita49::channel_tag_size", props[idx].id)) {
            if (!(props[idx].value >>= s.channel_tag_size)) return false;
        }
        else if (!strcmp("vita49::item_packing_field_size", props[idx].id)) {
            if (!(props[idx].value >>= s.item_packing_field_size)) return false;
        }
        else if (!strcmp("vita49::data_item_size", props[idx].id)) {
            if (!(props[idx].value >>= s.data_item_size)) return false;
        }
        else if (!strcmp("vita49::repeat_count", props[idx].id)) {
            if (!(props[idx].value >>= s.repeat_count)) return false;
        }
        else if (!strcmp("vita49::vector_size", props[idx].id)) {
            if (!(props[idx].value >>= s.vector_size)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const VITA49StreamDefinition_struct& s) {
    CF::Properties props;
    props.length(13);
    props[0].id = CORBA::string_dup("vita49::id");
    props[0].value <<= s.id;
    props[1].id = CORBA::string_dup("vita49::ip_address");
    props[1].value <<= s.ip_address;
    props[2].id = CORBA::string_dup("vita49::vlan");
    props[2].value <<= s.vlan;
    props[3].id = CORBA::string_dup("vita49::port");
    props[3].value <<= s.port;
    props[4].id = CORBA::string_dup("vita49::valid_data_format");
    props[4].value <<= s.valid_data_format;
    props[5].id = CORBA::string_dup("vita49::packing_method_processing_efficient");
    props[5].value <<= s.packing_method_processing_efficient;
    props[6].id = CORBA::string_dup("vita49::repeating");
    props[6].value <<= s.repeating;
    props[7].id = CORBA::string_dup("vita49::event_tag_size");
    props[7].value <<= s.event_tag_size;
    props[8].id = CORBA::string_dup("vita49::channel_tag_size");
    props[8].value <<= s.channel_tag_size;
    props[9].id = CORBA::string_dup("vita49::item_packing_field_size");
    props[9].value <<= s.item_packing_field_size;
    props[10].id = CORBA::string_dup("vita49::data_item_size");
    props[10].value <<= s.data_item_size;
    props[11].id = CORBA::string_dup("vita49::repeat_count");
    props[11].value <<= s.repeat_count;
    props[12].id = CORBA::string_dup("vita49::vector_size");
    props[12].value <<= s.vector_size;
    a <<= props;
};

inline bool operator== (const VITA49StreamDefinition_struct& s1, const VITA49StreamDefinition_struct& s2) {
    if (s1.id!=s2.id)
        return false;
    if (s1.ip_address!=s2.ip_address)
        return false;
    if (s1.vlan!=s2.vlan)
        return false;
    if (s1.port!=s2.port)
        return false;
    if (s1.valid_data_format!=s2.valid_data_format)
        return false;
    if (s1.packing_method_processing_efficient!=s2.packing_method_processing_efficient)
        return false;
    if (s1.repeating!=s2.repeating)
        return false;
    if (s1.event_tag_size!=s2.event_tag_size)
        return false;
    if (s1.channel_tag_size!=s2.channel_tag_size)
        return false;
    if (s1.item_packing_field_size!=s2.item_packing_field_size)
        return false;
    if (s1.data_item_size!=s2.data_item_size)
        return false;
    if (s1.repeat_count!=s2.repeat_count)
        return false;
    if (s1.vector_size!=s2.vector_size)
        return false;
    return true;
};

inline bool operator!= (const VITA49StreamDefinition_struct& s1, const VITA49StreamDefinition_struct& s2) {
    return !(s1==s2);
};

struct sdds_attachment_struct {
    sdds_attachment_struct ()
    {
        port = 0;
    };

    static std::string getId() {
        return std::string("sdds_attachment");
    };

    std::string streamId;
    std::string attachId;
    CORBA::ULong port;
};

inline bool operator>>= (const CORBA::Any& a, sdds_attachment_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("sdds::streamId", props[idx].id)) {
            if (!(props[idx].value >>= s.streamId)) return false;
        }
        else if (!strcmp("sdds::attachId", props[idx].id)) {
            if (!(props[idx].value >>= s.attachId)) return false;
        }
        else if (!strcmp("sdds::rec_port", props[idx].id)) {
            if (!(props[idx].value >>= s.port)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const sdds_attachment_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("sdds::streamId");
    props[0].value <<= s.streamId;
    props[1].id = CORBA::string_dup("sdds::attachId");
    props[1].value <<= s.attachId;
    props[2].id = CORBA::string_dup("sdds::rec_port");
    props[2].value <<= s.port;
    a <<= props;
};

inline bool operator== (const sdds_attachment_struct& s1, const sdds_attachment_struct& s2) {
    if (s1.streamId!=s2.streamId)
        return false;
    if (s1.attachId!=s2.attachId)
        return false;
    if (s1.port!=s2.port)
        return false;
    return true;
};

inline bool operator!= (const sdds_attachment_struct& s1, const sdds_attachment_struct& s2) {
    return !(s1==s2);
};

struct vita49_attachment_struct {
    vita49_attachment_struct ()
    {
        port = 0;
    };

    static std::string getId() {
        return std::string("vita49_attachment");
    };

    std::string streamId;
    std::string attachId;
    CORBA::ULong port;
};

inline bool operator>>= (const CORBA::Any& a, vita49_attachment_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("vita49::streamId", props[idx].id)) {
            if (!(props[idx].value >>= s.streamId)) return false;
        }
        else if (!strcmp("vita49::attachId", props[idx].id)) {
            if (!(props[idx].value >>= s.attachId)) return false;
        }
        else if (!strcmp("vita49::rec_port", props[idx].id)) {
            if (!(props[idx].value >>= s.port)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const vita49_attachment_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("vita49::streamId");
    props[0].value <<= s.streamId;
    props[1].id = CORBA::string_dup("vita49::attachId");
    props[1].value <<= s.attachId;
    props[2].id = CORBA::string_dup("vita49::rec_port");
    props[2].value <<= s.port;
    a <<= props;
};

inline bool operator== (const vita49_attachment_struct& s1, const vita49_attachment_struct& s2) {
    if (s1.streamId!=s2.streamId)
        return false;
    if (s1.attachId!=s2.attachId)
        return false;
    if (s1.port!=s2.port)
        return false;
    return true;
};

inline bool operator!= (const vita49_attachment_struct& s1, const vita49_attachment_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
