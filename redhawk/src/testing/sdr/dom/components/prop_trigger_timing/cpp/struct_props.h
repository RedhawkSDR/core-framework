#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct prop_3_struct {
    prop_3_struct ()
    {
    };

    static std::string getId() {
        return std::string("prop_3");
    };

    std::string prop_3_a;
};

inline bool operator>>= (const CORBA::Any& a, prop_3_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("prop_3_a")) {
        if (!(props["prop_3_a"] >>= s.prop_3_a)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const prop_3_struct& s) {
    redhawk::PropertyMap props;
 
    props["prop_3_a"] = s.prop_3_a;
    a <<= props;
}

inline bool operator== (const prop_3_struct& s1, const prop_3_struct& s2) {
    if (s1.prop_3_a!=s2.prop_3_a)
        return false;
    return true;
}

inline bool operator!= (const prop_3_struct& s1, const prop_3_struct& s2) {
    return !(s1==s2);
}

struct prop_4_a_struct {
    prop_4_a_struct ()
    {
    };

    static std::string getId() {
        return std::string("prop_4_a");
    };

    std::string prop_4_b;
};

inline bool operator>>= (const CORBA::Any& a, prop_4_a_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("prop_4_b")) {
        if (!(props["prop_4_b"] >>= s.prop_4_b)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const prop_4_a_struct& s) {
    redhawk::PropertyMap props;
 
    props["prop_4_b"] = s.prop_4_b;
    a <<= props;
}

inline bool operator== (const prop_4_a_struct& s1, const prop_4_a_struct& s2) {
    if (s1.prop_4_b!=s2.prop_4_b)
        return false;
    return true;
}

inline bool operator!= (const prop_4_a_struct& s1, const prop_4_a_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
