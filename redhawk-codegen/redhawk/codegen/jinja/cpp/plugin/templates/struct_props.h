/*#
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
 #*/
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct plugin_registration_struct {
    plugin_registration_struct ()
    {
    }

    static std::string getId() {
        return std::string("plugin::registration");
    }

    static const char* getFormat() {
        return "ssss";
    }

    std::string id;
    std::string name;
    std::string description;
    std::string metric_port;
};

inline bool operator>>= (const CORBA::Any& a, plugin_registration_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("plugin::registration::id")) {
        if (!(props["plugin::registration::id"] >>= s.id)) return false;
    }
    if (props.contains("plugin::registration::name")) {
        if (!(props["plugin::registration::name"] >>= s.name)) return false;
    }
    if (props.contains("plugin::registration::description")) {
        if (!(props["plugin::registration::description"] >>= s.description)) return false;
    }
    if (props.contains("plugin::registration::metric_port")) {
        if (!(props["plugin::registration::metric_port"] >>= s.metric_port)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const plugin_registration_struct& s) {
    redhawk::PropertyMap props;
 
    props["plugin::registration::id"] = s.id;
 
    props["plugin::registration::name"] = s.name;
 
    props["plugin::registration::description"] = s.description;
 
    props["plugin::registration::metric_port"] = s.metric_port;
    a <<= props;
}

inline bool operator== (const plugin_registration_struct& s1, const plugin_registration_struct& s2) {
    if (s1.id!=s2.id)
        return false;
    if (s1.name!=s2.name)
        return false;
    if (s1.description!=s2.description)
        return false;
    if (s1.metric_port!=s2.metric_port)
        return false;
    return true;
}

inline bool operator!= (const plugin_registration_struct& s1, const plugin_registration_struct& s2) {
    return !(s1==s2);
}

struct plugin_heartbeat_struct {
    plugin_heartbeat_struct ()
    {
    }

    static std::string getId() {
        return std::string("plugin::heartbeat");
    }

    static const char* getFormat() {
        return "s";
    }

    std::string plugin_id;
};

inline bool operator>>= (const CORBA::Any& a, plugin_heartbeat_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("plugin::heartbeat::plugin_id")) {
        if (!(props["plugin::heartbeat::plugin_id"] >>= s.plugin_id)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const plugin_heartbeat_struct& s) {
    redhawk::PropertyMap props;
 
    props["plugin::heartbeat::plugin_id"] = s.plugin_id;
    a <<= props;
}

inline bool operator== (const plugin_heartbeat_struct& s1, const plugin_heartbeat_struct& s2) {
    if (s1.plugin_id!=s2.plugin_id)
        return false;
    return true;
}

inline bool operator!= (const plugin_heartbeat_struct& s1, const plugin_heartbeat_struct& s2) {
    return !(s1==s2);
}

struct plugin_message_struct {
    plugin_message_struct ()
    {
    }

    static std::string getId() {
        return std::string("plugin::message");
    }

    static const char* getFormat() {
        return "ssbusss";
    }

    std::string plugin_id;
    std::string metric_name;
    bool busy;
    CF::UTCTime metric_timestamp;
    std::string metric_reason;
    std::string metric_threshold_value;
    std::string metric_recorded_value;
};

inline bool operator>>= (const CORBA::Any& a, plugin_message_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("plugin::message::plugin_id")) {
        if (!(props["plugin::message::plugin_id"] >>= s.plugin_id)) return false;
    }
    if (props.contains("plugin::message::metric_name")) {
        if (!(props["plugin::message::metric_name"] >>= s.metric_name)) return false;
    }
    if (props.contains("plugin::message::busy")) {
        if (!(props["plugin::message::busy"] >>= s.busy)) return false;
    }
    if (props.contains("plugin::message::metric_timestamp")) {
        if (!(props["plugin::message::metric_timestamp"] >>= s.metric_timestamp)) return false;
    }
    if (props.contains("plugin::message::metric_reason")) {
        if (!(props["plugin::message::metric_reason"] >>= s.metric_reason)) return false;
    }
    if (props.contains("plugin::message::metric_threshold_value")) {
        if (!(props["plugin::message::metric_threshold_value"] >>= s.metric_threshold_value)) return false;
    }
    if (props.contains("plugin::message::metric_recorded_value")) {
        if (!(props["plugin::message::metric_recorded_value"] >>= s.metric_recorded_value)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const plugin_message_struct& s) {
    redhawk::PropertyMap props;
 
    props["plugin::message::plugin_id"] = s.plugin_id;
 
    props["plugin::message::metric_name"] = s.metric_name;
 
    props["plugin::message::busy"] = s.busy;
 
    props["plugin::message::metric_timestamp"] = s.metric_timestamp;
 
    props["plugin::message::metric_reason"] = s.metric_reason;
 
    props["plugin::message::metric_threshold_value"] = s.metric_threshold_value;
 
    props["plugin::message::metric_recorded_value"] = s.metric_recorded_value;
    a <<= props;
}

inline bool operator== (const plugin_message_struct& s1, const plugin_message_struct& s2) {
    if (s1.plugin_id!=s2.plugin_id)
        return false;
    if (s1.metric_name!=s2.metric_name)
        return false;
    if (s1.busy!=s2.busy)
        return false;
    if (s1.metric_timestamp!=s2.metric_timestamp)
        return false;
    if (s1.metric_reason!=s2.metric_reason)
        return false;
    if (s1.metric_threshold_value!=s2.metric_threshold_value)
        return false;
    if (s1.metric_recorded_value!=s2.metric_recorded_value)
        return false;
    return true;
}

inline bool operator!= (const plugin_message_struct& s1, const plugin_message_struct& s2) {
    return !(s1==s2);
}

struct plugin_set_threshold_struct {
    plugin_set_threshold_struct ()
    {
    }

    static std::string getId() {
        return std::string("plugin::set_threshold");
    }

    static const char* getFormat() {
        return "sss";
    }

    std::string plugin_id;
    std::string metric_name;
    std::string metric_threshold_value;
};

inline bool operator>>= (const CORBA::Any& a, plugin_set_threshold_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("plugin::set_threshold::plugin_id")) {
        if (!(props["plugin::set_threshold::plugin_id"] >>= s.plugin_id)) return false;
    }
    if (props.contains("plugin::set_threshold::metric_name")) {
        if (!(props["plugin::set_threshold::metric_name"] >>= s.metric_name)) return false;
    }
    if (props.contains("plugin::set_threshold::metric_threshold_value")) {
        if (!(props["plugin::set_threshold::metric_threshold_value"] >>= s.metric_threshold_value)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const plugin_set_threshold_struct& s) {
    redhawk::PropertyMap props;
 
    props["plugin::set_threshold::plugin_id"] = s.plugin_id;
 
    props["plugin::set_threshold::metric_name"] = s.metric_name;
 
    props["plugin::set_threshold::metric_threshold_value"] = s.metric_threshold_value;
    a <<= props;
}

inline bool operator== (const plugin_set_threshold_struct& s1, const plugin_set_threshold_struct& s2) {
    if (s1.plugin_id!=s2.plugin_id)
        return false;
    if (s1.metric_name!=s2.metric_name)
        return false;
    if (s1.metric_threshold_value!=s2.metric_threshold_value)
        return false;
    return true;
}

inline bool operator!= (const plugin_set_threshold_struct& s1, const plugin_set_threshold_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
