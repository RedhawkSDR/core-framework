#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct s_prop_struct {
    s_prop_struct ()
    {
    }

    static std::string getId() {
        return std::string("s_prop");
    }

    static const char* getFormat() {
        return "sh[d]";
    }

    std::string s_prop__a;
    short s_prop__b;
    std::vector<double> abc;
};

inline bool operator>>= (const CORBA::Any& a, s_prop_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("s_prop::a")) {
        if (!(props["s_prop::a"] >>= s.s_prop__a)) return false;
    }
    if (props.contains("s_prop::b")) {
        if (!(props["s_prop::b"] >>= s.s_prop__b)) return false;
    }
    if (props.contains("abc")) {
        if (!(props["abc"] >>= s.abc)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const s_prop_struct& s) {
    redhawk::PropertyMap props;
 
    props["s_prop::a"] = s.s_prop__a;
 
    props["s_prop::b"] = s.s_prop__b;
 
    props["abc"] = s.abc;
    a <<= props;
}

inline bool operator== (const s_prop_struct& s1, const s_prop_struct& s2) {
    if (s1.s_prop__a!=s2.s_prop__a)
        return false;
    if (s1.s_prop__b!=s2.s_prop__b)
        return false;
    if (s1.abc!=s2.abc)
        return false;
    return true;
}

inline bool operator!= (const s_prop_struct& s1, const s_prop_struct& s2) {
    return !(s1==s2);
}

struct sq_prop_s_struct {
    sq_prop_s_struct ()
    {
    }

    static std::string getId() {
        return std::string("sq_prop_s");
    }

    static const char* getFormat() {
        return "fs";
    }

    float sq_prop__a;
    std::string sq_prop__b;
};

inline bool operator>>= (const CORBA::Any& a, sq_prop_s_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("sq_prop::a")) {
        if (!(props["sq_prop::a"] >>= s.sq_prop__a)) return false;
    }
    if (props.contains("sq_prop::b")) {
        if (!(props["sq_prop::b"] >>= s.sq_prop__b)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const sq_prop_s_struct& s) {
    redhawk::PropertyMap props;
 
    props["sq_prop::a"] = s.sq_prop__a;
 
    props["sq_prop::b"] = s.sq_prop__b;
    a <<= props;
}

inline bool operator== (const sq_prop_s_struct& s1, const sq_prop_s_struct& s2) {
    if (s1.sq_prop__a!=s2.sq_prop__a)
        return false;
    if (s1.sq_prop__b!=s2.sq_prop__b)
        return false;
    return true;
}

inline bool operator!= (const sq_prop_s_struct& s1, const sq_prop_s_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
