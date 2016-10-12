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

struct test_message_struct {
    test_message_struct () {};
    
    std::string getName() {
        return std::string("test_message");
    };
    
    float item_float;
    std::string item_string;
};

inline bool operator>>= (const CORBA::Any& a, test_message_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    if (props.length() != 2) return false;
    if (!(props[0].value >>= s.item_float)) return false;
    if (!(props[1].value >>= s.item_string)) return false;
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

struct some_struct_struct {
    some_struct_struct ()
    {
    }   
    CORBA::Double some_number;
    std::string some_string;
};

inline bool operator>>= (const CORBA::Any& a, some_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    if (!(props[0].value >>= s.some_number)) return false;
    if (!(props[1].value >>= s.some_string)) return false;
    return true;
};

inline void operator<<= (CORBA::Any& a, const some_struct_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("some_number");
    props[0].value <<= s.some_number;
    props[1].id = CORBA::string_dup("some_string");
    props[1].value <<= s.some_string;
    a <<= props;
};

inline bool operator== (some_struct_struct& s1, const some_struct_struct& s2) {
    if (s1.some_number!=s2.some_number)
        return false;
    if (s1.some_string!=s2.some_string)
        return false;
    return true;
};
inline bool operator!= (some_struct_struct& s1, const some_struct_struct& s2) {
    return !(s1==s2);
};
/*SPECIALIZE_ASSIGNMENT(some_struct_struct, , newval)
SPECIALIZE_FROMANY(some_struct_struct, some_struct_struct, ,tmp, v=tmp;);
SPECIALIZE_TOANY(some_struct_struct, some_struct_struct, tmp = v.value;, tmp)*/

inline bool operator== (std::vector<some_struct_struct>& s1, const std::vector<some_struct_struct>& s2) {
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
inline bool operator!= (std::vector<some_struct_struct>& s1, const std::vector<some_struct_struct>& s2) {
    return !(s1==s2);
};
/*SPECIALIZE_ASSIGNMENT(std::vector<some_struct_struct>, CORBA::AnySeq* anySeqPtr;anySeqPtr = new CORBA::AnySeq();anySeqPtr->length(newval.size());CORBA::AnySeq_var anySeq(anySeqPtr);for (CORBA::ULong ii = 0; ii < anySeq->length(); ++ii) {anySeq[ii] <<= newval[ii];}, anySeq)
template <> inline bool PropertyWrapper<propertyChange<std::vector<some_struct_struct> > >::fromAny (const CORBA::Any& a, propertyChange<std::vector<some_struct_struct> > & v) {
    CORBA::AnySeq* anySeqPtr;
    if (!(a >>= anySeqPtr))
        return false;
    CORBA::AnySeq& anySeq = *anySeqPtr;
    std::vector<some_struct_struct> tmp;
    tmp.resize(anySeq.length());
    for (CORBA::ULong ii = 0; ii < anySeq.length(); ++ii) {
        if (!(anySeq[ii] >>= tmp[ii])) {
            return false;
        }
        v = tmp;
    }
    return true;
};
template <> inline void PropertyWrapper<propertyChange<std::vector<some_struct_struct> > >::toAny (const propertyChange<std::vector<some_struct_struct> > & v, CORBA::Any& a) {
    CORBA::AnySeq_var tmp=new CORBA::AnySeq();
    tmp->length(v.value.size());
    for (CORBA::ULong ii = 0; ii < tmp->length(); ++ii) {
        tmp[ii] <<= v.value[ii];
    }
    a <<= tmp;
};*/

#endif
