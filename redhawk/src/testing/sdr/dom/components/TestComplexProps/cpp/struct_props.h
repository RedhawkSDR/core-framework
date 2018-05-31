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

struct FloatStruct_struct {
    FloatStruct_struct ()
    {
        FloatStructMember = 6;
    }

    static std::string getId() {
        return std::string("FloatStruct");
    }

    static const char* getFormat() {
        return "f";
    }

    float FloatStructMember;
};

inline bool operator>>= (const CORBA::Any& a, FloatStruct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("FloatStructMember")) {
        if (!(props["FloatStructMember"] >>= s.FloatStructMember)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const FloatStruct_struct& s) {
    redhawk::PropertyMap props;
 
    props["FloatStructMember"] = s.FloatStructMember;
    a <<= props;
}

inline bool operator== (const FloatStruct_struct& s1, const FloatStruct_struct& s2) {
    if (s1.FloatStructMember!=s2.FloatStructMember)
        return false;
    return true;
}

inline bool operator!= (const FloatStruct_struct& s1, const FloatStruct_struct& s2) {
    return !(s1==s2);
}

struct complexFloatStruct_struct {
    complexFloatStruct_struct ()
    {
        complexFloatStructMember = std::complex<float>(6.0,7.0);
        complex_float_seq.push_back(std::complex<float>(3.0,2.0));
    }

    static std::string getId() {
        return std::string("complexFloatStruct");
    }

    static const char* getFormat() {
        return "2f[2f]";
    }

    std::complex<float> complexFloatStructMember;
    std::vector<std::complex<float> > complex_float_seq;
};

inline bool operator>>= (const CORBA::Any& a, complexFloatStruct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("complexFloatStructMember")) {
        if (!(props["complexFloatStructMember"] >>= s.complexFloatStructMember)) return false;
    }
    if (props.contains("complexFloatStruct::complex_float_seq")) {
        if (!(props["complexFloatStruct::complex_float_seq"] >>= s.complex_float_seq)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const complexFloatStruct_struct& s) {
    redhawk::PropertyMap props;
 
    props["complexFloatStructMember"] = s.complexFloatStructMember;
 
    props["complexFloatStruct::complex_float_seq"] = s.complex_float_seq;
    a <<= props;
}

inline bool operator== (const complexFloatStruct_struct& s1, const complexFloatStruct_struct& s2) {
    if (s1.complexFloatStructMember!=s2.complexFloatStructMember)
        return false;
    if (s1.complex_float_seq!=s2.complex_float_seq)
        return false;
    return true;
}

inline bool operator!= (const complexFloatStruct_struct& s1, const complexFloatStruct_struct& s2) {
    return !(s1==s2);
}

struct FloatStructSequenceMember_struct {
    FloatStructSequenceMember_struct ()
    {
        FloatStructSequenceMemberMemember = 6;
        float_seq.push_back(3);
    }

    static std::string getId() {
        return std::string("FloatStructSequenceMember");
    }

    static const char* getFormat() {
        return "f[f]";
    }

    float FloatStructSequenceMemberMemember;
    std::vector<float> float_seq;
};

inline bool operator>>= (const CORBA::Any& a, FloatStructSequenceMember_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("FloatStructSequenceMemberMemember")) {
        if (!(props["FloatStructSequenceMemberMemember"] >>= s.FloatStructSequenceMemberMemember)) return false;
    }
    if (props.contains("FloatStructSequence::float_seq")) {
        if (!(props["FloatStructSequence::float_seq"] >>= s.float_seq)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const FloatStructSequenceMember_struct& s) {
    redhawk::PropertyMap props;
 
    props["FloatStructSequenceMemberMemember"] = s.FloatStructSequenceMemberMemember;
 
    props["FloatStructSequence::float_seq"] = s.float_seq;
    a <<= props;
}

inline bool operator== (const FloatStructSequenceMember_struct& s1, const FloatStructSequenceMember_struct& s2) {
    if (s1.FloatStructSequenceMemberMemember!=s2.FloatStructSequenceMemberMemember)
        return false;
    if (s1.float_seq!=s2.float_seq)
        return false;
    return true;
}

inline bool operator!= (const FloatStructSequenceMember_struct& s1, const FloatStructSequenceMember_struct& s2) {
    return !(s1==s2);
}

struct complexFloatStructSequenceMember_struct {
    complexFloatStructSequenceMember_struct ()
    {
    }

    static std::string getId() {
        return std::string("complexFloatStructSequenceMember");
    }

    static const char* getFormat() {
        return "2f[2f]";
    }

    std::complex<float> complexFloatStructSequenceMemberMemember;
    std::vector<std::complex<float> > complex_float_seq;
};

inline bool operator>>= (const CORBA::Any& a, complexFloatStructSequenceMember_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("complexFloatStructSequenceMemberMemember")) {
        if (!(props["complexFloatStructSequenceMemberMemember"] >>= s.complexFloatStructSequenceMemberMemember)) return false;
    }
    if (props.contains("complexFloatStructSequence::complex_float_seq")) {
        if (!(props["complexFloatStructSequence::complex_float_seq"] >>= s.complex_float_seq)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const complexFloatStructSequenceMember_struct& s) {
    redhawk::PropertyMap props;
 
    props["complexFloatStructSequenceMemberMemember"] = s.complexFloatStructSequenceMemberMemember;
 
    props["complexFloatStructSequence::complex_float_seq"] = s.complex_float_seq;
    a <<= props;
}

inline bool operator== (const complexFloatStructSequenceMember_struct& s1, const complexFloatStructSequenceMember_struct& s2) {
    if (s1.complexFloatStructSequenceMemberMemember!=s2.complexFloatStructSequenceMemberMemember)
        return false;
    if (s1.complex_float_seq!=s2.complex_float_seq)
        return false;
    return true;
}

inline bool operator!= (const complexFloatStructSequenceMember_struct& s1, const complexFloatStructSequenceMember_struct& s2) {
    return !(s1==s2);
}

template<> inline short StructSequenceProperty<complexFloatStructSequenceMember_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<complexFloatStructSequenceMember_struct> tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

template<> inline short StructProperty<FloatStruct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    FloatStruct_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

template<> inline short StructProperty<complexFloatStruct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    complexFloatStruct_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

template<> inline short StructProperty<FloatStructSequenceMember_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    FloatStructSequenceMember_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

template<> inline short StructProperty<complexFloatStructSequenceMember_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    complexFloatStructSequenceMember_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

template<> inline short StructSequenceProperty<FloatStructSequenceMember_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<FloatStructSequenceMember_struct> tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

#endif
