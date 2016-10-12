#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct p4_struct {
    p4_struct ()
    {
    };

    static std::string getId() {
        return std::string("p4");
    };

    std::string p4sub1;
    float p4sub2;
};

inline bool operator>>= (const CORBA::Any& a, p4_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    redhawk::PropertyMap props(*temp);
    if (props.contains("p4sub1")) {
        if (!(props["p4sub1"] >>= s.p4sub1)) return false;
    }
    if (props.contains("p4sub2")) {
        if (!(props["p4sub2"] >>= s.p4sub2)) return false;
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const p4_struct& s) {
    redhawk::PropertyMap props;
 
    props["p4sub1"] = s.p4sub1;
 
    props["p4sub2"] = s.p4sub2;
    a <<= props;
};

inline bool operator== (const p4_struct& s1, const p4_struct& s2) {
    if (s1.p4sub1!=s2.p4sub1)
        return false;
    if (s1.p4sub2!=s2.p4sub2)
        return false;
    return true;
};

inline bool operator!= (const p4_struct& s1, const p4_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
