#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct foo_struct {
    foo_struct ()
    {
    };

    static std::string getId() {
        return std::string("foo");
    };

    std::string a;
    std::string b;
    std::string c;
};

inline bool operator>>= (const CORBA::Any& a, foo_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("a")) {
        if (!(props["a"] >>= s.a)) return false;
    }
    if (props.contains("b")) {
        if (!(props["b"] >>= s.b)) return false;
    }
    if (props.contains("c")) {
        if (!(props["c"] >>= s.c)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const foo_struct& s) {
    redhawk::PropertyMap props;
 
    props["a"] = s.a;
 
    props["b"] = s.b;
 
    props["c"] = s.c;
    a <<= props;
}

inline bool operator== (const foo_struct& s1, const foo_struct& s2) {
    if (s1.a!=s2.a)
        return false;
    if (s1.b!=s2.b)
        return false;
    if (s1.c!=s2.c)
        return false;
    return true;
}

inline bool operator!= (const foo_struct& s1, const foo_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
