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
#include <ossie/PropertyInterface.h>

struct station_struct {
    station_struct ()
    {
        name = "WBJC";
        frequency = 91.7;
    };

    static std::string getId() {
        return std::string("station");
    };

    std::string name;
    float frequency;
};

inline bool operator>>= (const CORBA::Any& a, station_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("name", props[idx].id)) {
            if (!(props[idx].value >>= s.name)) return false;
        }
        if (!strcmp("frequency", props[idx].id)) {
            if (!(props[idx].value >>= s.frequency)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const station_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("name");
    props[0].value <<= s.name;
    props[1].id = CORBA::string_dup("frequency");
    props[1].value <<= s.frequency;
    a <<= props;
};

inline bool operator== (const station_struct& s1, const station_struct& s2) {
    if (s1.name!=s2.name)
        return false;
    if (s1.frequency!=s2.frequency)
        return false;
    return true;
};

inline bool operator!= (const station_struct& s1, const station_struct& s2) {
    return !(s1==s2);
};

struct endpoint_struct {
    endpoint_struct ()
    {
    };

    static std::string getId() {
        return std::string("endpoint");
    };

    std::string host;
    short port;
};

inline bool operator>>= (const CORBA::Any& a, endpoint_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("host", props[idx].id)) {
            if (!(props[idx].value >>= s.host)) return false;
        }
        if (!strcmp("port", props[idx].id)) {
            if (!(props[idx].value >>= s.port)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const endpoint_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("host");
    props[0].value <<= s.host;
    props[1].id = CORBA::string_dup("port");
    props[1].value <<= s.port;
    a <<= props;
};

inline bool operator== (const endpoint_struct& s1, const endpoint_struct& s2) {
    if (s1.host!=s2.host)
        return false;
    if (s1.port!=s2.port)
        return false;
    return true;
};

inline bool operator!= (const endpoint_struct& s1, const endpoint_struct& s2) {
    return !(s1==s2);
};


#endif
