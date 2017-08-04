#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct my_msg_struct {
    my_msg_struct ()
    {
    };

    static std::string getId() {
        return std::string("my_msg");
    };

    std::string string_payload;
};

inline bool operator>>= (const CORBA::Any& a, my_msg_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("string_payload")) {
        if (!(props["string_payload"] >>= s.string_payload)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const my_msg_struct& s) {
    redhawk::PropertyMap props;
 
    props["string_payload"] = s.string_payload;
    a <<= props;
}

inline bool operator== (const my_msg_struct& s1, const my_msg_struct& s2) {
    if (s1.string_payload!=s2.string_payload)
        return false;
    return true;
}

inline bool operator!= (const my_msg_struct& s1, const my_msg_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
