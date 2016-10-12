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
#include <ossie/CF/cf.h>

struct test_message_struct {
    test_message_struct ()
    {
    };

    static std::string getId() {
        return std::string("DCE:ced3f264-3ac8-4eea-8877-66a97428ded7");
    };

    float item_float;
    std::string item_string;
};

inline bool operator>>= (const CORBA::Any& a, test_message_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("item_float", props[idx].id)) {
            if (!(props[idx].value >>= s.item_float)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("item_string", props[idx].id)) {
            if (!(props[idx].value >>= s.item_string)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const test_message_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("item_float");
    props[0].value <<= s.item_float;
    props[1].id = CORBA::string_dup("item_string");
    props[1].value <<= s.item_string;
    a <<= props;
};

inline bool operator== (const test_message_struct& s1, const test_message_struct& s2) {
    if (s1.item_float!=s2.item_float)
        return false;
    if (s1.item_string!=s2.item_string)
        return false;
    return true;
};

inline bool operator!= (const test_message_struct& s1, const test_message_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
