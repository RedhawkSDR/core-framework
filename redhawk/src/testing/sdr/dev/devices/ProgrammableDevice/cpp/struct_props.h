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
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>

struct hw_load_status_struct {
    hw_load_status_struct ()
    {
        state = 0;
    };

    static std::string getId() {
        return std::string("hw_load_status");
    };

    std::string request_id;
    std::string requester_id;
    std::string hardware_id;
    std::string load_filepath;
    unsigned short state;
};

inline bool operator>>= (const CORBA::Any& a, hw_load_status_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("hw_load_status::request_id", props[idx].id)) {
            if (!(props[idx].value >>= s.request_id)) return false;
        }
        else if (!strcmp("hw_load_status::requester_id", props[idx].id)) {
            if (!(props[idx].value >>= s.requester_id)) return false;
        }
        else if (!strcmp("hw_load_status::hardware_id", props[idx].id)) {
            if (!(props[idx].value >>= s.hardware_id)) return false;
        }
        else if (!strcmp("hw_load_status::load_filepath", props[idx].id)) {
            if (!(props[idx].value >>= s.load_filepath)) return false;
        }
        else if (!strcmp("hw_load_status::state", props[idx].id)) {
            if (!(props[idx].value >>= s.state)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const hw_load_status_struct& s) {
    CF::Properties props;
    props.length(5);
    props[0].id = CORBA::string_dup("hw_load_status::request_id");
    props[0].value <<= s.request_id;
    props[1].id = CORBA::string_dup("hw_load_status::requester_id");
    props[1].value <<= s.requester_id;
    props[2].id = CORBA::string_dup("hw_load_status::hardware_id");
    props[2].value <<= s.hardware_id;
    props[3].id = CORBA::string_dup("hw_load_status::load_filepath");
    props[3].value <<= s.load_filepath;
    props[4].id = CORBA::string_dup("hw_load_status::state");
    props[4].value <<= s.state;
    a <<= props;
};

inline bool operator== (const hw_load_status_struct& s1, const hw_load_status_struct& s2) {
    if (s1.request_id!=s2.request_id)
        return false;
    if (s1.requester_id!=s2.requester_id)
        return false;
    if (s1.hardware_id!=s2.hardware_id)
        return false;
    if (s1.load_filepath!=s2.load_filepath)
        return false;
    if (s1.state!=s2.state)
        return false;
    return true;
};

inline bool operator!= (const hw_load_status_struct& s1, const hw_load_status_struct& s2) {
    return !(s1==s2);
};

struct hw_load_request_struct {
    hw_load_request_struct ()
    {
    };

    static std::string getId() {
        return std::string("hw_load_request");
    };

    std::string request_id;
    std::string requester_id;
    std::string hardware_id;
    std::string load_filepath;
};

inline bool operator>>= (const CORBA::Any& a, hw_load_request_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("hw_load_request::request_id", props[idx].id)) {
            if (!(props[idx].value >>= s.request_id)) return false;
        }
        else if (!strcmp("hw_load_request::requester_id", props[idx].id)) {
            if (!(props[idx].value >>= s.requester_id)) return false;
        }
        else if (!strcmp("hw_load_request::hardware_id", props[idx].id)) {
            if (!(props[idx].value >>= s.hardware_id)) return false;
        }
        else if (!strcmp("hw_load_request::load_filepath", props[idx].id)) {
            if (!(props[idx].value >>= s.load_filepath)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const hw_load_request_struct& s) {
    CF::Properties props;
    props.length(4);
    props[0].id = CORBA::string_dup("hw_load_request::request_id");
    props[0].value <<= s.request_id;
    props[1].id = CORBA::string_dup("hw_load_request::requester_id");
    props[1].value <<= s.requester_id;
    props[2].id = CORBA::string_dup("hw_load_request::hardware_id");
    props[2].value <<= s.hardware_id;
    props[3].id = CORBA::string_dup("hw_load_request::load_filepath");
    props[3].value <<= s.load_filepath;
    a <<= props;
};

inline bool operator== (const hw_load_request_struct& s1, const hw_load_request_struct& s2) {
    if (s1.request_id!=s2.request_id)
        return false;
    if (s1.requester_id!=s2.requester_id)
        return false;
    if (s1.hardware_id!=s2.hardware_id)
        return false;
    if (s1.load_filepath!=s2.load_filepath)
        return false;
    return true;
};

inline bool operator!= (const hw_load_request_struct& s1, const hw_load_request_struct& s2) {
    return !(s1==s2);
};


#endif
