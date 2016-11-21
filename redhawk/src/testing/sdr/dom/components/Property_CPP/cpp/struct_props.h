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

struct p4_struct {
    p4_struct ()
    {
    };

    static std::string getId() {
        return std::string("p4");
    };

    std::string p4sub1;
    float p4sub2;
};

inline bool operator>>= (const CORBA::Any& a, p4_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    redhawk::PropertyMap props(*temp);
    if (props.contains("p4sub1")) {
        if (!(props["p4sub1"] >>= s.p4sub1)) return false;
    }
    if (props.contains("p4sub2")) {
        if (!(props["p4sub2"] >>= s.p4sub2)) return false;
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const p4_struct& s) {
    redhawk::PropertyMap props;
 
    props["p4sub1"] = s.p4sub1;
 
    props["p4sub2"] = s.p4sub2;
    a <<= props;
};

inline bool operator== (const p4_struct& s1, const p4_struct& s2) {
    if (s1.p4sub1!=s2.p4sub1)
        return false;
    if (s1.p4sub2!=s2.p4sub2)
        return false;
    return true;
};

inline bool operator!= (const p4_struct& s1, const p4_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
