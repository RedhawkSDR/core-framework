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

struct memory_allocation_struct {
    memory_allocation_struct ()
    {
        contiguous = true;
        memory_type = 0;
    };

    static std::string getId() {
        return std::string("memory_allocation");
    };

    CORBA::Long capacity;
    bool contiguous;
    CORBA::Long memory_type;
};

inline bool operator>>= (const CORBA::Any& a, memory_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("capacity", props[idx].id)) {
            if (!(props[idx].value >>= s.capacity)) return false;
        }
        else if (!strcmp("contiguous", props[idx].id)) {
            if (!(props[idx].value >>= s.contiguous)) return false;
        }
        else if (!strcmp("memory_type", props[idx].id)) {
            if (!(props[idx].value >>= s.memory_type)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const memory_allocation_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("capacity");
    props[0].value <<= s.capacity;
    props[1].id = CORBA::string_dup("contiguous");
    props[1].value <<= s.contiguous;
    props[2].id = CORBA::string_dup("memory_type");
    props[2].value <<= s.memory_type;
    a <<= props;
};

inline bool operator== (const memory_allocation_struct& s1, const memory_allocation_struct& s2) {
    if (s1.capacity!=s2.capacity)
        return false;
    if (s1.contiguous!=s2.contiguous)
        return false;
    if (s1.memory_type!=s2.memory_type)
        return false;
    return true;
};

inline bool operator!= (const memory_allocation_struct& s1, const memory_allocation_struct& s2) {
    return !(s1==s2);
};


#endif
