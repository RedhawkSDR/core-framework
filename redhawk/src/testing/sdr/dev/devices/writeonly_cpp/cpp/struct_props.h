#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct foo_struct_struct {
    foo_struct_struct ()
    {
        abc = "def";
    }

    static std::string getId() {
        return std::string("foo_struct");
    }

    static const char* getFormat() {
        return "s";
    }

    std::string abc;
};

inline bool operator>>= (const CORBA::Any& a, foo_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("abc")) {
        if (!(props["abc"] >>= s.abc)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const foo_struct_struct& s) {
    redhawk::PropertyMap props;
 
    props["abc"] = s.abc;
    a <<= props;
}

inline bool operator== (const foo_struct_struct& s1, const foo_struct_struct& s2) {
    if (s1.abc!=s2.abc)
        return false;
    return true;
}

inline bool operator!= (const foo_struct_struct& s1, const foo_struct_struct& s2) {
    return !(s1==s2);
}

struct ghi_struct {
    ghi_struct ()
    {
    }

    static std::string getId() {
        return std::string("ghi");
    }

    static const char* getFormat() {
        return "s";
    }

    std::string jkl;
};

inline bool operator>>= (const CORBA::Any& a, ghi_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("jkl")) {
        if (!(props["jkl"] >>= s.jkl)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const ghi_struct& s) {
    redhawk::PropertyMap props;
 
    props["jkl"] = s.jkl;
    a <<= props;
}

inline bool operator== (const ghi_struct& s1, const ghi_struct& s2) {
    if (s1.jkl!=s2.jkl)
        return false;
    return true;
}

inline bool operator!= (const ghi_struct& s1, const ghi_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
