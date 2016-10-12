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

struct some_struct_struct {
    some_struct_struct ()
    {
    };

    std::string getName() {
        return std::string("some_struct");
    };
    
    float some_float;
};

inline bool operator>>= (const CORBA::Any& a, some_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    if (!(props[0].value >>= s.some_float)) return false;
    return true;
};

inline void operator<<= (CORBA::Any& a, const some_struct_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("some_float");
    props[0].value <<= s.some_float;
    a <<= props;
};

inline bool operator== (some_struct_struct& s1, const some_struct_struct& s2) {
    if (s1.some_float!=s2.some_float)
        return false;
    return true;
};
inline bool operator!= (some_struct_struct& s1, const some_struct_struct& s2) {
    return !(s1==s2);
};
struct interim_id_struct {
    interim_id_struct ()
    {
    };

    std::string getName() {
        return std::string("interim_id");
    };
    
    short some_short;
};

inline bool operator>>= (const CORBA::Any& a, interim_id_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    if (!(props[0].value >>= s.some_short)) return false;
    return true;
};

inline void operator<<= (CORBA::Any& a, const interim_id_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("some_short");
    props[0].value <<= s.some_short;
    a <<= props;
};

inline bool operator== (interim_id_struct& s1, const interim_id_struct& s2) {
    if (s1.some_short!=s2.some_short)
        return false;
    return true;
};
inline bool operator!= (interim_id_struct& s1, const interim_id_struct& s2) {
    return !(s1==s2);
};



inline bool operator== (std::vector<interim_id_struct>& s1, const std::vector<interim_id_struct>& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }
    for (unsigned int i=0; i<s1.size(); i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
};
inline bool operator!= (std::vector<interim_id_struct>& s1, const std::vector<interim_id_struct>& s2) {
    return !(s1==s2);
};

#endif
