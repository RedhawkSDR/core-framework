#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct mystruct_struct {
    mystruct_struct ()
    {
        mystruct__mysimple = "x";
    };

    static std::string getId() {
        return std::string("mystruct");
    };

    std::string mystruct__mysimple;
    std::vector<std::string> mystruct__mysimpleseq;
};

inline bool operator>>= (const CORBA::Any& a, mystruct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("mystruct::mysimple")) {
        if (!(props["mystruct::mysimple"] >>= s.mystruct__mysimple)) return false;
    }
    if (props.contains("mystruct::mysimpleseq")) {
        if (!(props["mystruct::mysimpleseq"] >>= s.mystruct__mysimpleseq)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const mystruct_struct& s) {
    redhawk::PropertyMap props;
 
    props["mystruct::mysimple"] = s.mystruct__mysimple;
 
    props["mystruct::mysimpleseq"] = s.mystruct__mysimpleseq;
    a <<= props;
}

inline bool operator== (const mystruct_struct& s1, const mystruct_struct& s2) {
    if (s1.mystruct__mysimple!=s2.mystruct__mysimple)
        return false;
    if (s1.mystruct__mysimpleseq!=s2.mystruct__mysimpleseq)
        return false;
    return true;
}

inline bool operator!= (const mystruct_struct& s1, const mystruct_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
