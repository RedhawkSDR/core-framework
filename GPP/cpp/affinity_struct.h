#ifndef AFFINITY_STRUCTPROPS_H
#define AFFINITY_STRUCTPROPS_H

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct affinity_struct {
    affinity_struct ()
    {
        force_override = false;
        deploy_per_socket = false;
        disabled = true;
    };

    static std::string getId() {
        return std::string("affinity");
    };

    std::string exec_directive_value;
    std::string exec_directive_class;
    bool force_override;
    std::string blacklist_cpus;
    bool deploy_per_socket;
    bool disabled;
};

inline bool operator>>= (const CORBA::Any& a, affinity_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("affinity::exec_directive_value")) {
        if (!(props["affinity::exec_directive_value"] >>= s.exec_directive_value)) return false;
    }
    if (props.contains("affinity::exec_directive_class")) {
        if (!(props["affinity::exec_directive_class"] >>= s.exec_directive_class)) return false;
    }
    if (props.contains("affinity::force_override")) {
        if (!(props["affinity::force_override"] >>= s.force_override)) return false;
    }
    if (props.contains("affinity::blacklist_cpus")) {
        if (!(props["affinity::blacklist_cpus"] >>= s.blacklist_cpus)) return false;
    }
    if (props.contains("affinity::deploy_per_socket")) {
        if (!(props["affinity::deploy_per_socket"] >>= s.deploy_per_socket)) return false;
    }
    if (props.contains("affinity::disabled")) {
        if (!(props["affinity::disabled"] >>= s.disabled)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const affinity_struct& s) {
    redhawk::PropertyMap props;
 
    props["affinity::exec_directive_value"] = s.exec_directive_value;
 
    props["affinity::exec_directive_class"] = s.exec_directive_class;
 
    props["affinity::force_override"] = s.force_override;
 
    props["affinity::blacklist_cpus"] = s.blacklist_cpus;
 
    props["affinity::deploy_per_socket"] = s.deploy_per_socket;
 
    props["affinity::disabled"] = s.disabled;
    a <<= props;
}

inline bool operator== (const affinity_struct& s1, const affinity_struct& s2) {
    if (s1.exec_directive_value!=s2.exec_directive_value)
        return false;
    if (s1.exec_directive_class!=s2.exec_directive_class)
        return false;
    if (s1.force_override!=s2.force_override)
        return false;
    if (s1.blacklist_cpus!=s2.blacklist_cpus)
        return false;
    if (s1.deploy_per_socket!=s2.deploy_per_socket)
        return false;
    if (s1.disabled!=s2.disabled)
        return false;
    return true;
}

inline bool operator!= (const affinity_struct& s1, const affinity_struct& s2) {
    return !(s1==s2);
}


#endif 
