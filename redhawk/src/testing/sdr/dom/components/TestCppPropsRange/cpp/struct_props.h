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

struct my_struct_name_struct {
	my_struct_name_struct ()
	{
		struct_octet_name = 1;
		struct_short_name = 2;
		struct_ushort_name = 3;
		struct_long_name = 4;
		struct_ulong_name = 5;
		struct_longlong_name = 6;
		struct_ulonglong_name = 7;
	};

    std::string getId() {
        return std::string("my_struct");
    };
	
	unsigned char struct_octet_name;
	short struct_short_name;
	unsigned short struct_ushort_name;
	CORBA::Long struct_long_name;
	CORBA::ULong struct_ulong_name;
	CORBA::LongLong struct_longlong_name;
	CORBA::ULongLong struct_ulonglong_name;
};

inline bool operator>>= (const CORBA::Any& a, my_struct_name_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("struct_octet", props[idx].id)) {
			if (!(props[idx].value >>= CORBA::Any::to_octet(s.struct_octet_name))) return false;
		}
		if (!strcmp("struct_short", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_short_name)) return false;
		}
		if (!strcmp("struct_ushort", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_ushort_name)) return false;
		}
		if (!strcmp("struct_long", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_long_name)) return false;
		}
		if (!strcmp("struct_ulong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_ulong_name)) return false;
		}
		if (!strcmp("struct_longlong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_longlong_name)) return false;
		}
		if (!strcmp("struct_ulonglong", props[idx].id)) {
			if (!(props[idx].value >>= s.struct_ulonglong_name)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const my_struct_name_struct& s) {
	CF::Properties props;
	props.length(7);
	props[0].id = CORBA::string_dup("struct_octet");
	props[0].value <<= CORBA::Any::from_octet(s.struct_octet_name);
	props[1].id = CORBA::string_dup("struct_short");
	props[1].value <<= s.struct_short_name;
	props[2].id = CORBA::string_dup("struct_ushort");
	props[2].value <<= s.struct_ushort_name;
	props[3].id = CORBA::string_dup("struct_long");
	props[3].value <<= s.struct_long_name;
	props[4].id = CORBA::string_dup("struct_ulong");
	props[4].value <<= s.struct_ulong_name;
	props[5].id = CORBA::string_dup("struct_longlong");
	props[5].value <<= s.struct_longlong_name;
	props[6].id = CORBA::string_dup("struct_ulonglong");
	props[6].value <<= s.struct_ulonglong_name;
	a <<= props;
};

inline bool operator== (const my_struct_name_struct& s1, const my_struct_name_struct& s2) {
    if (s1.struct_octet_name!=s2.struct_octet_name)
        return false;
    if (s1.struct_short_name!=s2.struct_short_name)
        return false;
    if (s1.struct_ushort_name!=s2.struct_ushort_name)
        return false;
    if (s1.struct_long_name!=s2.struct_long_name)
        return false;
    if (s1.struct_ulong_name!=s2.struct_ulong_name)
        return false;
    if (s1.struct_longlong_name!=s2.struct_longlong_name)
        return false;
    if (s1.struct_ulonglong_name!=s2.struct_ulonglong_name)
        return false;
    return true;
};

inline bool operator!= (const my_struct_name_struct& s1, const my_struct_name_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<my_struct_name_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    my_struct_name_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

struct ss_struct_name_struct {
	ss_struct_name_struct ()
	{
	};

    std::string getId() {
        return std::string("ss_struct");
    };
	
	unsigned char ss_octet_name;
	short ss_short_name;
	unsigned short ss_ushort_name;
	CORBA::Long ss_long_name;
	CORBA::ULong ss_ulong_name;
	CORBA::LongLong ss_longlong_name;
	CORBA::ULongLong ss_ulonglong_name;
};

inline bool operator>>= (const CORBA::Any& a, ss_struct_name_struct& s) {
	CF::Properties* temp;
	if (!(a >>= temp)) return false;
	CF::Properties& props = *temp;
	for (unsigned int idx = 0; idx < props.length(); idx++) {
		if (!strcmp("ss_octet", props[idx].id)) {
			if (!(props[idx].value >>= CORBA::Any::to_octet(s.ss_octet_name))) return false;
		}
		if (!strcmp("ss_short", props[idx].id)) {
			if (!(props[idx].value >>= s.ss_short_name)) return false;
		}
		if (!strcmp("ss_ushort", props[idx].id)) {
			if (!(props[idx].value >>= s.ss_ushort_name)) return false;
		}
		if (!strcmp("ss_long", props[idx].id)) {
			if (!(props[idx].value >>= s.ss_long_name)) return false;
		}
		if (!strcmp("ss_ulong", props[idx].id)) {
			if (!(props[idx].value >>= s.ss_ulong_name)) return false;
		}
		if (!strcmp("ss_longlong", props[idx].id)) {
			if (!(props[idx].value >>= s.ss_longlong_name)) return false;
		}
		if (!strcmp("ss_ulonglong", props[idx].id)) {
			if (!(props[idx].value >>= s.ss_ulonglong_name)) return false;
		}
	}
	return true;
};

inline void operator<<= (CORBA::Any& a, const ss_struct_name_struct& s) {
	CF::Properties props;
	props.length(7);
	props[0].id = CORBA::string_dup("ss_octet");
	props[0].value <<= CORBA::Any::from_octet(s.ss_octet_name);
	props[1].id = CORBA::string_dup("ss_short");
	props[1].value <<= s.ss_short_name;
	props[2].id = CORBA::string_dup("ss_ushort");
	props[2].value <<= s.ss_ushort_name;
	props[3].id = CORBA::string_dup("ss_long");
	props[3].value <<= s.ss_long_name;
	props[4].id = CORBA::string_dup("ss_ulong");
	props[4].value <<= s.ss_ulong_name;
	props[5].id = CORBA::string_dup("ss_longlong");
	props[5].value <<= s.ss_longlong_name;
	props[6].id = CORBA::string_dup("ss_ulonglong");
	props[6].value <<= s.ss_ulonglong_name;
	a <<= props;
};

inline bool operator== (const ss_struct_name_struct& s1, const ss_struct_name_struct& s2) {
    if (s1.ss_octet_name!=s2.ss_octet_name)
        return false;
    if (s1.ss_short_name!=s2.ss_short_name)
        return false;
    if (s1.ss_ushort_name!=s2.ss_ushort_name)
        return false;
    if (s1.ss_long_name!=s2.ss_long_name)
        return false;
    if (s1.ss_ulong_name!=s2.ss_ulong_name)
        return false;
    if (s1.ss_longlong_name!=s2.ss_longlong_name)
        return false;
    if (s1.ss_ulonglong_name!=s2.ss_ulonglong_name)
        return false;
    return true;
};

inline bool operator!= (const ss_struct_name_struct& s1, const ss_struct_name_struct& s2) {
    return !(s1==s2);
};

template<> inline short StructProperty<ss_struct_name_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    ss_struct_name_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

inline bool operator== (const std::vector<ss_struct_name_struct>& s1, const std::vector<ss_struct_name_struct>& s2) {
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

inline bool operator!= (const std::vector<ss_struct_name_struct>& s1, const std::vector<ss_struct_name_struct>& s2) {
    return !(s1==s2);
};

template<> inline short StructSequenceProperty<ss_struct_name_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        if (a.type()->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<ss_struct_name_struct> tmp;
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
