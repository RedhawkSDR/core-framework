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

#include <ossie/CorbaUtils.h>

struct prop_foo_struct {
    prop_foo_struct ()
    {
    }
    bool item_bool;
    std::string item_string;
};

inline bool operator>>= (const CORBA::Any& a, prop_foo_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    if (props.length() != 2) return false;
    if (!(props[0].value >>= s.item_bool)) return false;
    if (!(props[1].value >>= s.item_string)) return false;
    return true;
};

inline void operator<<= (CORBA::Any& a, const prop_foo_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("item_bool");
    props[0].value <<= s.item_bool;
    props[1].id = CORBA::string_dup("item_string");
    props[1].value <<= s.item_string;
    a <<= props;
};

#endif
