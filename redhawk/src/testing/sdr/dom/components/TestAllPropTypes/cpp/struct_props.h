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

struct struct_vars_struct {
	struct_vars_struct ()
	{
	};

    std::string getId() {
        return std::string("struct_vars");
    };
	
	std::string struct_string;
	bool struct_boolean;
	CORBA::ULong struct_ulong;
	std::string struct_objref;
	short struct_short;
	float struct_float;
	unsigned char struct_octet;
	char struct_char;
	unsigned short struct_ushort;
	double struct_double;
	CORBA::Long struct_long;
	CORBA::LongLong struct_longlong;
	CORBA::ULongLong struct_ulonglong;
};

inline bool operator>>= (const CORBA::Any& a, struct_vars_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("struct_string", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_string)) return false;
		}
		if (!strcmp("struct_boolean", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_boolean)) return false;
		}
		if (!strcmp("struct_ulong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_ulong)) return false;
		}
		if (!strcmp("struct_objref", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_objref)) return false;
		}
		if (!strcmp("struct_short", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_short)) return false;
		}
		if (!strcmp("struct_float", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_float)) return false;
		}
		if (!strcmp("struct_octet", props[idx].id)) {
			if (!(props[idx].value >>= CORBA::Any::to_octet(s.struct_octet))) return false;
		}
		if (!strcmp("struct_char", props[idx].id)) {
		CORBA::Char temp_struct_char;
			if (!(props[idx].value >>= CORBA::Any::to_char(temp_struct_char))) return false;
			s.struct_char = temp_struct_char;	
		}
		if (!strcmp("struct_ushort", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_ushort)) return false;
		}
		if (!strcmp("struct_double", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_double)) return false;
		}
		if (!strcmp("struct_long", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_long)) return false;
		}
		if (!strcmp("struct_longlong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_longlong)) return false;
		}
		if (!strcmp("struct_ulonglong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_ulonglong)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const struct_vars_struct& s) {
	CF::Properties props;
	props.length(13);
	props[0].id = CORBA::string_dup("struct_string");
	props[0].value <<= s.struct_string;
	props[1].id = CORBA::string_dup("struct_boolean");
	props[1].value <<= s.struct_boolean;
	props[2].id = CORBA::string_dup("struct_ulong");
	props[2].value <<= s.struct_ulong;
	props[3].id = CORBA::string_dup("struct_objref");
	props[3].value <<= s.struct_objref;
	props[4].id = CORBA::string_dup("struct_short");
	props[4].value <<= s.struct_short;
	props[5].id = CORBA::string_dup("struct_float");
	props[5].value <<= s.struct_float;
	props[6].id = CORBA::string_dup("struct_octet");
	props[6].value <<= CORBA::Any::from_octet(s.struct_octet);
	props[7].id = CORBA::string_dup("struct_char");
	props[7].value <<= CORBA::Any::from_char(s.struct_char);
	props[8].id = CORBA::string_dup("struct_ushort");
	props[8].value <<= s.struct_ushort;
	props[9].id = CORBA::string_dup("struct_double");
	props[9].value <<= s.struct_double;
	props[10].id = CORBA::string_dup("struct_long");
	props[10].value <<= s.struct_long;
	props[11].id = CORBA::string_dup("struct_longlong");
	props[11].value <<= s.struct_longlong;
	props[12].id = CORBA::string_dup("struct_ulonglong");
	props[12].value <<= s.struct_ulonglong;
	a <<= props;
};

inline bool operator== (const struct_vars_struct& s1, const struct_vars_struct& s2) {
    if (s1.struct_string!=s2.struct_string)
        return false;
    if (s1.struct_boolean!=s2.struct_boolean)
        return false;
    if (s1.struct_ulong!=s2.struct_ulong)
        return false;
    if (s1.struct_objref!=s2.struct_objref)
        return false;
    if (s1.struct_short!=s2.struct_short)
        return false;
    if (s1.struct_float!=s2.struct_float)
        return false;
    if (s1.struct_octet!=s2.struct_octet)
        return false;
    if (s1.struct_char!=s2.struct_char)
        return false;
    if (s1.struct_ushort!=s2.struct_ushort)
        return false;
    if (s1.struct_double!=s2.struct_double)
        return false;
    if (s1.struct_long!=s2.struct_long)
        return false;
    if (s1.struct_longlong!=s2.struct_longlong)
        return false;
    if (s1.struct_ulonglong!=s2.struct_ulonglong)
        return false;
    return true;
};

inline bool operator!= (const struct_vars_struct& s1, const struct_vars_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<struct_vars_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    struct_vars_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

struct struct_seq_vars_struct {
	struct_seq_vars_struct ()
	{
	};

    std::string getId() {
        return std::string("struct_seq_vars");
    };
	
	std::string struct_seq_string;
	bool struct_seq_boolean;
	CORBA::ULong struct_seq_ulong;
	std::string struct_seq_objref;
	short struct_seq_short;
	float struct_seq_float;
	unsigned char struct_seq_octet;
	char struct_seq_char;
	unsigned short struct_seq_ushort;
	double struct_seq_double;
	CORBA::Long struct_seq_long;
	CORBA::LongLong struct_seq_longlong;
	CORBA::ULongLong struct_seq_ulonglong;
};

inline bool operator>>= (const CORBA::Any& a, struct_seq_vars_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("struct_seq_string", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_string)) return false;
		}
		if (!strcmp("struct_seq_boolean", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_boolean)) return false;
		}
		if (!strcmp("struct_seq_ulong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_ulong)) return false;
		}
		if (!strcmp("struct_seq_objref", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_objref)) return false;
		}
		if (!strcmp("struct_seq_short", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_short)) return false;
		}
		if (!strcmp("struct_seq_float", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_float)) return false;
		}
		if (!strcmp("struct_seq_octet", props[idx].id)) {
			if (!(props[idx].value >>= CORBA::Any::to_octet(s.struct_seq_octet))) return false;
		}
		if (!strcmp("struct_seq_char", props[idx].id)) {
		CORBA::Char temp_struct_seq_char;
			if (!(props[idx].value >>= CORBA::Any::to_char(temp_struct_seq_char))) return false;
			s.struct_seq_char = temp_struct_seq_char;	
		}
		if (!strcmp("struct_seq_ushort", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_ushort)) return false;
		}
		if (!strcmp("struct_seq_double", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_double)) return false;
		}
		if (!strcmp("struct_seq_long", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_long)) return false;
		}
		if (!strcmp("struct_seq_longlong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_longlong)) return false;
		}
		if (!strcmp("struct_seq_ulonglong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_seq_ulonglong)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const struct_seq_vars_struct& s) {
	CF::Properties props;
	props.length(13);
	props[0].id = CORBA::string_dup("struct_seq_string");
	props[0].value <<= s.struct_seq_string;
	props[1].id = CORBA::string_dup("struct_seq_boolean");
	props[1].value <<= s.struct_seq_boolean;
	props[2].id = CORBA::string_dup("struct_seq_ulong");
	props[2].value <<= s.struct_seq_ulong;
	props[3].id = CORBA::string_dup("struct_seq_objref");
	props[3].value <<= s.struct_seq_objref;
	props[4].id = CORBA::string_dup("struct_seq_short");
	props[4].value <<= s.struct_seq_short;
	props[5].id = CORBA::string_dup("struct_seq_float");
	props[5].value <<= s.struct_seq_float;
	props[6].id = CORBA::string_dup("struct_seq_octet");
	props[6].value <<= CORBA::Any::from_octet(s.struct_seq_octet);
	props[7].id = CORBA::string_dup("struct_seq_char");
	props[7].value <<= CORBA::Any::from_char(s.struct_seq_char);
	props[8].id = CORBA::string_dup("struct_seq_ushort");
	props[8].value <<= s.struct_seq_ushort;
	props[9].id = CORBA::string_dup("struct_seq_double");
	props[9].value <<= s.struct_seq_double;
	props[10].id = CORBA::string_dup("struct_seq_long");
	props[10].value <<= s.struct_seq_long;
	props[11].id = CORBA::string_dup("struct_seq_longlong");
	props[11].value <<= s.struct_seq_longlong;
	props[12].id = CORBA::string_dup("struct_seq_ulonglong");
	props[12].value <<= s.struct_seq_ulonglong;
	a <<= props;
};

inline bool operator== (const struct_seq_vars_struct& s1, const struct_seq_vars_struct& s2) {
    if (s1.struct_seq_string!=s2.struct_seq_string)
        return false;
    if (s1.struct_seq_boolean!=s2.struct_seq_boolean)
        return false;
    if (s1.struct_seq_ulong!=s2.struct_seq_ulong)
        return false;
    if (s1.struct_seq_objref!=s2.struct_seq_objref)
        return false;
    if (s1.struct_seq_short!=s2.struct_seq_short)
        return false;
    if (s1.struct_seq_float!=s2.struct_seq_float)
        return false;
    if (s1.struct_seq_octet!=s2.struct_seq_octet)
        return false;
    if (s1.struct_seq_char!=s2.struct_seq_char)
        return false;
    if (s1.struct_seq_ushort!=s2.struct_seq_ushort)
        return false;
    if (s1.struct_seq_double!=s2.struct_seq_double)
        return false;
    if (s1.struct_seq_long!=s2.struct_seq_long)
        return false;
    if (s1.struct_seq_longlong!=s2.struct_seq_longlong)
        return false;
    if (s1.struct_seq_ulonglong!=s2.struct_seq_ulonglong)
        return false;
    return true;
};

inline bool operator!= (const struct_seq_vars_struct& s1, const struct_seq_vars_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<struct_seq_vars_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    struct_seq_vars_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

inline bool operator== (const std::vector<struct_seq_vars_struct>& s1, const std::vector<struct_seq_vars_struct>& s2) {
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

inline bool operator!= (const std::vector<struct_seq_vars_struct>& s1, const std::vector<struct_seq_vars_struct>& s2) {
    return !(s1==s2);
};

template<> inline short StructSequenceProperty<struct_seq_vars_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<struct_seq_vars_struct> tmp;
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
