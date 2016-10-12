#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
//#include <CF/cf.h>
#include <ossie/OptionalProperty.h>
#include <ossie/AnyUtils.h>

struct my_struct_name_struct {
    my_struct_name_struct ()
    {
    };

    static std::string getId() {
        return std::string("my_struct");
    };

    optional_property<unsigned char> struct_octet_name;
    optional_property<short> struct_short_name;
    optional_property<unsigned short> struct_ushort_name;
    optional_property<CORBA::Long> struct_long_name;
    optional_property<CORBA::ULong> struct_ulong_name;
    optional_property<CORBA::LongLong> struct_longlong_name;
    optional_property<CORBA::ULongLong> struct_ulonglong_name;
    optional_property<std::string> struct_string_name;
    optional_property<std::vector<unsigned char> > struct_seq_octet_name;
    optional_property<std::vector<short> > struct_seq_short_name;
    optional_property<std::vector<unsigned short> > struct_seq_ushort_name;
    optional_property<std::vector<CORBA::Long> > struct_seq_long_name;
    optional_property<std::vector<CORBA::ULong> > struct_seq_ulong_name;
    optional_property<std::vector<CORBA::LongLong> > struct_seq_longlong_name;
    optional_property<std::vector<CORBA::ULongLong> > struct_seq_ulonglong_name;
};

inline bool operator>>= (const CORBA::Any& a, my_struct_name_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    redhawk::PropertyMap props(*temp);
    if (props.contains("struct_octet")) {
        if (!(ossie::any::isNull(props["struct_octet"]))) {
            unsigned char tmp;
            if (!(props["struct_octet"] >>= CORBA::Any::to_octet(tmp))) return false;
            s.struct_octet_name = tmp;
        } else {
            s.struct_octet_name.reset();
        }
    }
    if (props.contains("struct_short")) {
        if (!(ossie::any::isNull(props["struct_short"]))) {
            short tmp;
            if (!(props["struct_short"] >>= tmp)) return false;
            s.struct_short_name = tmp;
        } else {
            s.struct_short_name.reset();
        }
    }
    if (props.contains("struct_ushort")) {
        if (!(ossie::any::isNull(props["struct_ushort"]))) {
            unsigned short tmp;
            if (!(props["struct_ushort"] >>= tmp)) return false;
            s.struct_ushort_name = tmp;
        } else {
            s.struct_ushort_name.reset();
        }
    }
    if (props.contains("struct_long")) {
        if (!(ossie::any::isNull(props["struct_long"]))) {
            CORBA::Long tmp;
            if (!(props["struct_long"] >>= tmp)) return false;
            s.struct_long_name = tmp;
        } else {
            s.struct_long_name.reset();
        }
    }
    if (props.contains("struct_ulong")) {
        if (!(ossie::any::isNull(props["struct_ulong"]))) {
            CORBA::ULong tmp;
            if (!(props["struct_ulong"] >>= tmp)) return false;
            s.struct_ulong_name = tmp;
        } else {
            s.struct_ulong_name.reset();
        }
    }
    if (props.contains("struct_longlong")) {
        if (!(ossie::any::isNull(props["struct_longlong"]))) {
            CORBA::LongLong tmp;
            if (!(props["struct_longlong"] >>= tmp)) return false;
            s.struct_longlong_name = tmp;
        } else {
            s.struct_longlong_name.reset();
        }
    }
    if (props.contains("struct_ulonglong")) {
        if (!(ossie::any::isNull(props["struct_ulonglong"]))) {
            CORBA::ULongLong tmp;
            if (!(props["struct_ulonglong"] >>= tmp)) return false;
            s.struct_ulonglong_name = tmp;
        } else {
            s.struct_ulonglong_name.reset();
        }
    }
    if (props.contains("struct_string")) {
        if (!(ossie::any::isNull(props["struct_string"]))) {
            std::string tmp;
            if (!(props["struct_string"] >>= tmp)) return false;
            s.struct_string_name = tmp;
        } else {
            s.struct_string_name.reset();
        }
    }
    if (props.contains("struct_seq_octet")) {
        if (!(ossie::any::isNull(props["struct_seq_octet"]))) {
            std::vector<unsigned char> tmp;
            if (!(props["struct_seq_octet"] >>= tmp)) return false;
            s.struct_seq_octet_name = tmp;
        } else {
            s.struct_seq_octet_name.reset();
        }
    }
    if (props.contains("struct_seq_short")) {
        if (!(ossie::any::isNull(props["struct_seq_short"]))) {
            std::vector<short> tmp;
            if (!(props["struct_seq_short"] >>= tmp)) return false;
            s.struct_seq_short_name = tmp;
        } else {
            s.struct_seq_short_name.reset();
        }
    }
    if (props.contains("struct_seq_ushort")) {
        if (!(ossie::any::isNull(props["struct_seq_ushort"]))) {
            std::vector<unsigned short> tmp;
            if (!(props["struct_seq_ushort"] >>= tmp)) return false;
            s.struct_seq_ushort_name = tmp;
        } else {
            s.struct_seq_ushort_name.reset();
        }
    }
    if (props.contains("struct_seq_long")) {
        if (!(ossie::any::isNull(props["struct_seq_long"]))) {
            std::vector<CORBA::Long> tmp;
            if (!(props["struct_seq_long"] >>= tmp)) return false;
            s.struct_seq_long_name = tmp;
        } else {
            s.struct_seq_long_name.reset();
        }
    }
    if (props.contains("struct_seq_ulong")) {
        if (!(ossie::any::isNull(props["struct_seq_ulong"]))) {
            std::vector<CORBA::ULong> tmp;
            if (!(props["struct_seq_ulong"] >>= tmp)) return false;
            s.struct_seq_ulong_name = tmp;
        } else {
            s.struct_seq_ulong_name.reset();
        }
    }
    if (props.contains("struct_seq_longlong")) {
        if (!(ossie::any::isNull(props["struct_seq_longlong"]))) {
            std::vector<CORBA::LongLong> tmp;
            if (!(props["struct_seq_longlong"] >>= tmp)) return false;
            s.struct_seq_longlong_name = tmp;
        } else {
            s.struct_seq_longlong_name.reset();
        }
    }
    if (props.contains("struct_seq_ulonglong")) {
        if (!(ossie::any::isNull(props["struct_seq_ulonglong"]))) {
            std::vector<CORBA::ULongLong> tmp;
            if (!(props["struct_seq_ulonglong"] >>= tmp)) return false;
            s.struct_seq_ulonglong_name = tmp;
        } else {
            s.struct_seq_ulonglong_name.reset();
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const my_struct_name_struct& s) {
    redhawk::PropertyMap props;
    if (s.struct_octet_name.isSet()) {
        props["struct_octet"] = CORBA::Any::from_octet(*(s.struct_octet_name));
    }
    if (s.struct_short_name.isSet()) {
        props["struct_short"] = *(s.struct_short_name);
    }
    if (s.struct_ushort_name.isSet()) {
        props["struct_ushort"] = *(s.struct_ushort_name);
    }
    if (s.struct_long_name.isSet()) {
        props["struct_long"] = *(s.struct_long_name);
    }
    if (s.struct_ulong_name.isSet()) {
        props["struct_ulong"] = *(s.struct_ulong_name);
    }
    if (s.struct_longlong_name.isSet()) {
        props["struct_longlong"] = *(s.struct_longlong_name);
    }
    if (s.struct_ulonglong_name.isSet()) {
        props["struct_ulonglong"] = *(s.struct_ulonglong_name);
    }
    if (s.struct_string_name.isSet()) {
        props["struct_string"] = *(s.struct_string_name);
    }
    if (s.struct_seq_octet_name.isSet()) {
        props["struct_seq_octet"] = *(s.struct_seq_octet_name);
    }
    if (s.struct_seq_short_name.isSet()) {
        props["struct_seq_short"] = *(s.struct_seq_short_name);
    }
    if (s.struct_seq_ushort_name.isSet()) {
        props["struct_seq_ushort"] = *(s.struct_seq_ushort_name);
    }
    if (s.struct_seq_long_name.isSet()) {
        props["struct_seq_long"] = *(s.struct_seq_long_name);
    }
    if (s.struct_seq_ulong_name.isSet()) {
        props["struct_seq_ulong"] = *(s.struct_seq_ulong_name);
    }
    if (s.struct_seq_longlong_name.isSet()) {
        props["struct_seq_longlong"] = *(s.struct_seq_longlong_name);
    }
    if (s.struct_seq_ulonglong_name.isSet()) {
        props["struct_seq_ulonglong"] = *(s.struct_seq_ulonglong_name);
    }
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
    if (s1.struct_string_name!=s2.struct_string_name)
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

#endif // STRUCTPROPS_H
