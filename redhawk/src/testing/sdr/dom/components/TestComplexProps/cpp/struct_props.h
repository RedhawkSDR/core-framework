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
        FloatStructMember = 4;
    };

    std::string getId() {
        return std::string("FloatStruct");
    };

    float FloatStructMember;
};

inline bool operator>>= (const CORBA::Any& a, FloatStruct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("FloatStructMember", props[idx].id)) {
            if (!(props[idx].value >>= s.FloatStructMember)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const FloatStruct_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("FloatStructMember");
    props[0].value <<= s.FloatStructMember;
    a <<= props;
};

inline bool operator== (const FloatStruct_struct& s1, const FloatStruct_struct& s2) {
    if (s1.FloatStructMember!=s2.FloatStructMember)
        return false;
    return true;
};

inline bool operator!= (const FloatStruct_struct& s1, const FloatStruct_struct& s2) {
    return !(s1==s2);
};

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

struct complexFloatStruct_struct {
    complexFloatStruct_struct ()
    {
        complexFloatStructMember = std::complex<float> (4.0,5.0);
    };

    std::string getId() {
        return std::string("complexFloatStruct");
    };

    std::complex<float>  complexFloatStructMember;
};

inline bool operator>>= (const CORBA::Any& a, complexFloatStruct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("complexFloatStructMember", props[idx].id)) {
            if (!(props[idx].value >>= s.complexFloatStructMember)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const complexFloatStruct_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("complexFloatStructMember");
    props[0].value <<= s.complexFloatStructMember;
    a <<= props;
};

inline bool operator== (const complexFloatStruct_struct& s1, const complexFloatStruct_struct& s2) {
    if (s1.complexFloatStructMember!=s2.complexFloatStructMember)
        return false;
    return true;
};

inline bool operator!= (const complexFloatStruct_struct& s1, const complexFloatStruct_struct& s2) {
    return !(s1==s2);
};

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

struct FloatStructSequenceMember_struct {
    FloatStructSequenceMember_struct ()
    {
        FloatStructSequenceMemberMemember = 4;
    };

    std::string getId() {
        return std::string("FloatStructSequenceMember");
    };

    float FloatStructSequenceMemberMemember;
};

inline bool operator>>= (const CORBA::Any& a, FloatStructSequenceMember_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("FloatStructSequenceMemberMemember", props[idx].id)) {
            if (!(props[idx].value >>= s.FloatStructSequenceMemberMemember)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const FloatStructSequenceMember_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("FloatStructSequenceMemberMemember");
    props[0].value <<= s.FloatStructSequenceMemberMemember;
    a <<= props;
};

inline bool operator== (const FloatStructSequenceMember_struct& s1, const FloatStructSequenceMember_struct& s2) {
    if (s1.FloatStructSequenceMemberMemember!=s2.FloatStructSequenceMemberMemember)
        return false;
    return true;
};

inline bool operator!= (const FloatStructSequenceMember_struct& s1, const FloatStructSequenceMember_struct& s2) {
    return !(s1==s2);
};

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

struct complexFloatStructSequenceMember_struct {
    complexFloatStructSequenceMember_struct ()
    {
        complexFloatStructSequenceMemberMemember = std::complex<float> (4.0,5.0);
    };

    std::string getId() {
        return std::string("complexFloatStructSequenceMember");
    };

    std::complex<float>  complexFloatStructSequenceMemberMemember;
};

inline bool operator>>= (const CORBA::Any& a, complexFloatStructSequenceMember_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("complexFloatStructSequenceMemberMemember", props[idx].id)) {
            if (!(props[idx].value >>= s.complexFloatStructSequenceMemberMemember)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const complexFloatStructSequenceMember_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("complexFloatStructSequenceMemberMemember");
    props[0].value <<= s.complexFloatStructSequenceMemberMemember;
    a <<= props;
};

inline bool operator== (const complexFloatStructSequenceMember_struct& s1, const complexFloatStructSequenceMember_struct& s2) {
    if (s1.complexFloatStructSequenceMemberMemember!=s2.complexFloatStructSequenceMemberMemember)
        return false;
    return true;
};

inline bool operator!= (const complexFloatStructSequenceMember_struct& s1, const complexFloatStructSequenceMember_struct& s2) {
    return !(s1==s2);
};

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

inline bool operator== (const std::vector<FloatStructSequenceMember_struct>& s1, const std::vector<FloatStructSequenceMember_struct>& s2) {
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

inline bool operator!= (const std::vector<FloatStructSequenceMember_struct>& s1, const std::vector<FloatStructSequenceMember_struct>& s2) {
    return !(s1==s2);
};

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
inline bool operator== (const std::vector<complexFloatStructSequenceMember_struct>& s1, const std::vector<complexFloatStructSequenceMember_struct>& s2) {
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

inline bool operator!= (const std::vector<complexFloatStructSequenceMember_struct>& s1, const std::vector<complexFloatStructSequenceMember_struct>& s2) {
    return !(s1==s2);
};

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

#endif
