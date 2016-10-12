#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
//#include <CF/cf.h>

struct my_struct_name_struct {
    my_struct_name_struct ()
    {
        struct_octet_name = 1;
        struct_short_name = 2;
        struct_ushort_name = 3;
        struct_long_name = 4;
        struct_ulong_name = 5;
        struct_longlong_name = 6LL;
        struct_ulonglong_name = 7LL;
        struct_seq_octet_name.push_back(1);
        struct_seq_octet_name.push_back(2);
        struct_seq_short_name.push_back(1);
        struct_seq_short_name.push_back(2);
        struct_seq_ushort_name.push_back(1);
        struct_seq_ushort_name.push_back(2);
        struct_seq_long_name.push_back(1);
        struct_seq_long_name.push_back(2);
        struct_seq_ulong_name.push_back(1);
        struct_seq_ulong_name.push_back(2);
        struct_seq_longlong_name.push_back(1LL);
        struct_seq_longlong_name.push_back(2LL);
        struct_seq_ulonglong_name.push_back(1LL);
        struct_seq_ulonglong_name.push_back(2LL);
    };

    static std::string getId() {
        return std::string("my_struct");
    };

    unsigned char struct_octet_name;
    short struct_short_name;
    unsigned short struct_ushort_name;
    CORBA::Long struct_long_name;
    CORBA::ULong struct_ulong_name;
    CORBA::LongLong struct_longlong_name;
    CORBA::ULongLong struct_ulonglong_name;
    std::vector<unsigned char> struct_seq_octet_name;
    std::vector<short> struct_seq_short_name;
    std::vector<unsigned short> struct_seq_ushort_name;
    std::vector<CORBA::Long> struct_seq_long_name;
    std::vector<CORBA::ULong> struct_seq_ulong_name;
    std::vector<CORBA::LongLong> struct_seq_longlong_name;
    std::vector<CORBA::ULongLong> struct_seq_ulonglong_name;
};

inline bool operator>>= (const CORBA::Any& a, my_struct_name_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("struct_octet", props[idx].id)) {
            if (!(props[idx].value >>= CORBA::Any::to_octet(s.struct_octet_name))) return false;
        }
        else if (!strcmp("struct_short", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_short_name)) return false;
        }
        else if (!strcmp("struct_ushort", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_ushort_name)) return false;
        }
        else if (!strcmp("struct_long", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_long_name)) return false;
        }
        else if (!strcmp("struct_ulong", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_ulong_name)) return false;
        }
        else if (!strcmp("struct_longlong", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_longlong_name)) return false;
        }
        else if (!strcmp("struct_ulonglong", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_ulonglong_name)) return false;
        }
        else if (!strcmp("struct_seq_octet", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_seq_octet_name)) return false;
        }
        else if (!strcmp("struct_seq_short", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_seq_short_name)) return false;
        }
        else if (!strcmp("struct_seq_ushort", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_seq_ushort_name)) return false;
        }
        else if (!strcmp("struct_seq_long", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_seq_long_name)) return false;
        }
        else if (!strcmp("struct_seq_ulong", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_seq_ulong_name)) return false;
        }
        else if (!strcmp("struct_seq_longlong", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_seq_longlong_name)) return false;
        }
        else if (!strcmp("struct_seq_ulonglong", props[idx].id)) {
            if (!(props[idx].value >>= s.struct_seq_ulonglong_name)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const my_struct_name_struct& s) {
    CF::Properties props;
    props.length(14);
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
    props[7].id = CORBA::string_dup("struct_seq_octet");
    props[7].value <<= s.struct_seq_octet_name;
    props[8].id = CORBA::string_dup("struct_seq_short");
    props[8].value <<= s.struct_seq_short_name;
    props[9].id = CORBA::string_dup("struct_seq_ushort");
    props[9].value <<= s.struct_seq_ushort_name;
    props[10].id = CORBA::string_dup("struct_seq_long");
    props[10].value <<= s.struct_seq_long_name;
    props[11].id = CORBA::string_dup("struct_seq_ulong");
    props[11].value <<= s.struct_seq_ulong_name;
    props[12].id = CORBA::string_dup("struct_seq_longlong");
    props[12].value <<= s.struct_seq_longlong_name;
    props[13].id = CORBA::string_dup("struct_seq_ulonglong");
    props[13].value <<= s.struct_seq_ulonglong_name;
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
    if (s1.struct_seq_octet_name!=s2.struct_seq_octet_name)
        return false;
    if (s1.struct_seq_short_name!=s2.struct_seq_short_name)
        return false;
    if (s1.struct_seq_ushort_name!=s2.struct_seq_ushort_name)
        return false;
    if (s1.struct_seq_long_name!=s2.struct_seq_long_name)
        return false;
    if (s1.struct_seq_ulong_name!=s2.struct_seq_ulong_name)
        return false;
    if (s1.struct_seq_longlong_name!=s2.struct_seq_longlong_name)
        return false;
    if (s1.struct_seq_ulonglong_name!=s2.struct_seq_ulonglong_name)
        return false;
    return true;
};

inline bool operator!= (const my_struct_name_struct& s1, const my_struct_name_struct& s2) {
    return !(s1==s2);
};

struct ss_struct_name_struct {
    ss_struct_name_struct ()
    {
    };

    static std::string getId() {
        return std::string("ss_struct");
    };

    unsigned char ss_octet_name;
    short ss_short_name;
    unsigned short ss_ushort_name;
    CORBA::Long ss_long_name;
    CORBA::ULong ss_ulong_name;
    CORBA::LongLong ss_longlong_name;
    CORBA::ULongLong ss_ulonglong_name;
    std::vector<unsigned char> ss_seq_octet_name;
    std::vector<short> ss_seq_short_name;
    std::vector<unsigned short> ss_seq_ushort_name;
    std::vector<CORBA::Long> ss_seq_long_name;
    std::vector<CORBA::ULong> ss_seq_ulong_name;
    std::vector<CORBA::LongLong> ss_seq_longlong_name;
    std::vector<CORBA::ULongLong> ss_seq_ulonglong_name;
};

inline bool operator>>= (const CORBA::Any& a, ss_struct_name_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("ss_octet", props[idx].id)) {
            if (!(props[idx].value >>= CORBA::Any::to_octet(s.ss_octet_name))) return false;
        }
        else if (!strcmp("ss_short", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_short_name)) return false;
        }
        else if (!strcmp("ss_ushort", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_ushort_name)) return false;
        }
        else if (!strcmp("ss_long", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_long_name)) return false;
        }
        else if (!strcmp("ss_ulong", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_ulong_name)) return false;
        }
        else if (!strcmp("ss_longlong", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_longlong_name)) return false;
        }
        else if (!strcmp("ss_ulonglong", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_ulonglong_name)) return false;
        }
        else if (!strcmp("ss_seq_octet", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_seq_octet_name)) return false;
        }
        else if (!strcmp("ss_seq_short", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_seq_short_name)) return false;
        }
        else if (!strcmp("ss_seq_ushort", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_seq_ushort_name)) return false;
        }
        else if (!strcmp("ss_seq_long", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_seq_long_name)) return false;
        }
        else if (!strcmp("ss_seq_ulong", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_seq_ulong_name)) return false;
        }
        else if (!strcmp("ss_seq_longlong", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_seq_longlong_name)) return false;
        }
        else if (!strcmp("ss_seq_ulonglong", props[idx].id)) {
            if (!(props[idx].value >>= s.ss_seq_ulonglong_name)) return false;
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const ss_struct_name_struct& s) {
    CF::Properties props;
    props.length(14);
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
    props[7].id = CORBA::string_dup("ss_seq_octet");
    props[7].value <<= s.ss_seq_octet_name;
    props[8].id = CORBA::string_dup("ss_seq_short");
    props[8].value <<= s.ss_seq_short_name;
    props[9].id = CORBA::string_dup("ss_seq_ushort");
    props[9].value <<= s.ss_seq_ushort_name;
    props[10].id = CORBA::string_dup("ss_seq_long");
    props[10].value <<= s.ss_seq_long_name;
    props[11].id = CORBA::string_dup("ss_seq_ulong");
    props[11].value <<= s.ss_seq_ulong_name;
    props[12].id = CORBA::string_dup("ss_seq_longlong");
    props[12].value <<= s.ss_seq_longlong_name;
    props[13].id = CORBA::string_dup("ss_seq_ulonglong");
    props[13].value <<= s.ss_seq_ulonglong_name;
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
    if (s1.ss_seq_octet_name!=s2.ss_seq_octet_name)
        return false;
    if (s1.ss_seq_short_name!=s2.ss_seq_short_name)
        return false;
    if (s1.ss_seq_ushort_name!=s2.ss_seq_ushort_name)
        return false;
    if (s1.ss_seq_long_name!=s2.ss_seq_long_name)
        return false;
    if (s1.ss_seq_ulong_name!=s2.ss_seq_ulong_name)
        return false;
    if (s1.ss_seq_longlong_name!=s2.ss_seq_longlong_name)
        return false;
    if (s1.ss_seq_ulonglong_name!=s2.ss_seq_ulonglong_name)
        return false;
    return true;
};

inline bool operator!= (const ss_struct_name_struct& s1, const ss_struct_name_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
