/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

#include <ossie/CorbaUtils.h>
#include <ossie/PropertyMap.h>
#include <ossie/CF/cf.h>


struct client_wait_times_struct {
    client_wait_times_struct ()
    {
        devices = 10000;
        services = 10000;
        managers = 10000;
    };

    static std::string getId() {
        return std::string("client_wait_times");
    };

    CORBA::ULong devices;
    CORBA::ULong services;
    CORBA::ULong managers;
};

inline bool operator>>= (const CORBA::Any& a, client_wait_times_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("client_wait_times::devices")) {
        if (!(props["client_wait_times::devices"] >>= s.devices)) return false;
    }
    if (props.contains("client_wait_times::services")) {
        if (!(props["client_wait_times::services"] >>= s.services)) return false;
    }
    if (props.contains("client_wait_times::managers")) {
        if (!(props["client_wait_times::managers"] >>= s.managers)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const client_wait_times_struct& s) {
    redhawk::PropertyMap props;
 
    props["client_wait_times::devices"] = s.devices;
 
    props["client_wait_times::services"] = s.services;
 
    props["client_wait_times::managers"] = s.managers;
    a <<= props;
}

inline bool operator== (const client_wait_times_struct& s1, const client_wait_times_struct& s2) {
    if (s1.devices!=s2.devices)
        return false;
    if (s1.services!=s2.services)
        return false;
    if (s1.managers!=s2.managers)
        return false;
    return true;
}

inline bool operator!= (const client_wait_times_struct& s1, const client_wait_times_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
