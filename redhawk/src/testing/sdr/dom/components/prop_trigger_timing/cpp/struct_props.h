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
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct prop_3_struct {
    prop_3_struct ()
    {
    };

    static std::string getId() {
        return std::string("prop_3");
    };

    std::string prop_3_a;
};

inline bool operator>>= (const CORBA::Any& a, prop_3_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("prop_3_a")) {
        if (!(props["prop_3_a"] >>= s.prop_3_a)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const prop_3_struct& s) {
    redhawk::PropertyMap props;
 
    props["prop_3_a"] = s.prop_3_a;
    a <<= props;
}

inline bool operator== (const prop_3_struct& s1, const prop_3_struct& s2) {
    if (s1.prop_3_a!=s2.prop_3_a)
        return false;
    return true;
}

inline bool operator!= (const prop_3_struct& s1, const prop_3_struct& s2) {
    return !(s1==s2);
}

struct prop_4_a_struct {
    prop_4_a_struct ()
    {
    };

    static std::string getId() {
        return std::string("prop_4_a");
    };

    std::string prop_4_b;
};

inline bool operator>>= (const CORBA::Any& a, prop_4_a_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("prop_4_b")) {
        if (!(props["prop_4_b"] >>= s.prop_4_b)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const prop_4_a_struct& s) {
    redhawk::PropertyMap props;
 
    props["prop_4_b"] = s.prop_4_b;
    a <<= props;
}

inline bool operator== (const prop_4_a_struct& s1, const prop_4_a_struct& s2) {
    if (s1.prop_4_b!=s2.prop_4_b)
        return false;
    return true;
}

inline bool operator!= (const prop_4_a_struct& s1, const prop_4_a_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
