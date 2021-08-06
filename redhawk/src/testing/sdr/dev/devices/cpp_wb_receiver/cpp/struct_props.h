#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include <ossie/PropertyMap.h>

struct frontend_listener_allocation_struct {
    frontend_listener_allocation_struct ()
    {
    }

    static std::string getId() {
        return std::string("FRONTEND::listener_allocation");
    }

    static const char* getFormat() {
        return "ss";
    }

    std::string existing_allocation_id;
    std::string listener_allocation_id;
};

inline bool operator>>= (const CORBA::Any& a, frontend_listener_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("FRONTEND::listener_allocation::existing_allocation_id")) {
        if (!(props["FRONTEND::listener_allocation::existing_allocation_id"] >>= s.existing_allocation_id)) return false;
    }
    if (props.contains("FRONTEND::listener_allocation::listener_allocation_id")) {
        if (!(props["FRONTEND::listener_allocation::listener_allocation_id"] >>= s.listener_allocation_id)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend_listener_allocation_struct& s) {
    redhawk::PropertyMap props;
 
    props["FRONTEND::listener_allocation::existing_allocation_id"] = s.existing_allocation_id;
 
    props["FRONTEND::listener_allocation::listener_allocation_id"] = s.listener_allocation_id;
    a <<= props;
}

inline bool operator== (const frontend_listener_allocation_struct& s1, const frontend_listener_allocation_struct& s2) {
    if (s1.existing_allocation_id!=s2.existing_allocation_id)
        return false;
    if (s1.listener_allocation_id!=s2.listener_allocation_id)
        return false;
    return true;
}

inline bool operator!= (const frontend_listener_allocation_struct& s1, const frontend_listener_allocation_struct& s2) {
    return !(s1==s2);
}

struct frontend_tuner_allocation_struct {
    frontend_tuner_allocation_struct ()
    {
    }

    static std::string getId() {
        return std::string("FRONTEND::tuner_allocation");
    }

    static const char* getFormat() {
        return "ssdddddbss";
    }

    std::string tuner_type;
    std::string allocation_id;
    double center_frequency;
    double bandwidth;
    double bandwidth_tolerance;
    double sample_rate;
    double sample_rate_tolerance;
    bool device_control;
    std::string group_id;
    std::string rf_flow_id;
};

inline bool operator>>= (const CORBA::Any& a, frontend_tuner_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("FRONTEND::tuner_allocation::tuner_type")) {
        if (!(props["FRONTEND::tuner_allocation::tuner_type"] >>= s.tuner_type)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::allocation_id")) {
        if (!(props["FRONTEND::tuner_allocation::allocation_id"] >>= s.allocation_id)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::center_frequency")) {
        if (!(props["FRONTEND::tuner_allocation::center_frequency"] >>= s.center_frequency)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::bandwidth")) {
        if (!(props["FRONTEND::tuner_allocation::bandwidth"] >>= s.bandwidth)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::bandwidth_tolerance")) {
        if (!(props["FRONTEND::tuner_allocation::bandwidth_tolerance"] >>= s.bandwidth_tolerance)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::sample_rate")) {
        if (!(props["FRONTEND::tuner_allocation::sample_rate"] >>= s.sample_rate)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::sample_rate_tolerance")) {
        if (!(props["FRONTEND::tuner_allocation::sample_rate_tolerance"] >>= s.sample_rate_tolerance)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::device_control")) {
        if (!(props["FRONTEND::tuner_allocation::device_control"] >>= s.device_control)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::group_id")) {
        if (!(props["FRONTEND::tuner_allocation::group_id"] >>= s.group_id)) return false;
    }
    if (props.contains("FRONTEND::tuner_allocation::rf_flow_id")) {
        if (!(props["FRONTEND::tuner_allocation::rf_flow_id"] >>= s.rf_flow_id)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend_tuner_allocation_struct& s) {
    redhawk::PropertyMap props;
 
    props["FRONTEND::tuner_allocation::tuner_type"] = s.tuner_type;
 
    props["FRONTEND::tuner_allocation::allocation_id"] = s.allocation_id;
 
    props["FRONTEND::tuner_allocation::center_frequency"] = s.center_frequency;
 
    props["FRONTEND::tuner_allocation::bandwidth"] = s.bandwidth;
 
    props["FRONTEND::tuner_allocation::bandwidth_tolerance"] = s.bandwidth_tolerance;
 
    props["FRONTEND::tuner_allocation::sample_rate"] = s.sample_rate;
 
    props["FRONTEND::tuner_allocation::sample_rate_tolerance"] = s.sample_rate_tolerance;
 
    props["FRONTEND::tuner_allocation::device_control"] = s.device_control;
 
    props["FRONTEND::tuner_allocation::group_id"] = s.group_id;
 
    props["FRONTEND::tuner_allocation::rf_flow_id"] = s.rf_flow_id;
    a <<= props;
}

inline bool operator== (const frontend_tuner_allocation_struct& s1, const frontend_tuner_allocation_struct& s2) {
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.allocation_id!=s2.allocation_id)
        return false;
    if (s1.center_frequency!=s2.center_frequency)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
        return false;
    if (s1.bandwidth_tolerance!=s2.bandwidth_tolerance)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.sample_rate_tolerance!=s2.sample_rate_tolerance)
        return false;
    if (s1.device_control!=s2.device_control)
        return false;
    if (s1.group_id!=s2.group_id)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    return true;
}

inline bool operator!= (const frontend_tuner_allocation_struct& s1, const frontend_tuner_allocation_struct& s2) {
    return !(s1==s2);
}

namespace enums {
    // Enumerated values for FRONTEND::scanner_allocation
    namespace frontend_scanner_allocation {
        // Enumerated values for FRONTEND::scanner_allocation::mode
        namespace mode {
            static const std::string SPAN_SCAN = "SPAN_SCAN";
            static const std::string DISCRETE_SCAN = "DISCRETE_SCAN";
        }
        // Enumerated values for FRONTEND::scanner_allocation::control_mode
        namespace control_mode {
            static const std::string TIME_BASED = "TIME_BASED";
            static const std::string SAMPLE_BASED = "SAMPLE_BASED";
        }
    }
}

struct frontend_scanner_allocation_struct {
    frontend_scanner_allocation_struct ()
    {
    }

    static std::string getId() {
        return std::string("FRONTEND::scanner_allocation");
    }

    static const char* getFormat() {
        return "ddssd";
    }

    double min_freq;
    double max_freq;
    std::string mode;
    std::string control_mode;
    double control_limit;
};

inline bool operator>>= (const CORBA::Any& a, frontend_scanner_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("FRONTEND::scanner_allocation::min_freq")) {
        if (!(props["FRONTEND::scanner_allocation::min_freq"] >>= s.min_freq)) return false;
    }
    if (props.contains("FRONTEND::scanner_allocation::max_freq")) {
        if (!(props["FRONTEND::scanner_allocation::max_freq"] >>= s.max_freq)) return false;
    }
    if (props.contains("FRONTEND::scanner_allocation::mode")) {
        if (!(props["FRONTEND::scanner_allocation::mode"] >>= s.mode)) return false;
    }
    if (props.contains("FRONTEND::scanner_allocation::control_mode")) {
        if (!(props["FRONTEND::scanner_allocation::control_mode"] >>= s.control_mode)) return false;
    }
    if (props.contains("FRONTEND::scanner_allocation::control_limit")) {
        if (!(props["FRONTEND::scanner_allocation::control_limit"] >>= s.control_limit)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend_scanner_allocation_struct& s) {
    redhawk::PropertyMap props;
 
    props["FRONTEND::scanner_allocation::min_freq"] = s.min_freq;
 
    props["FRONTEND::scanner_allocation::max_freq"] = s.max_freq;
 
    props["FRONTEND::scanner_allocation::mode"] = s.mode;
 
    props["FRONTEND::scanner_allocation::control_mode"] = s.control_mode;
 
    props["FRONTEND::scanner_allocation::control_limit"] = s.control_limit;
    a <<= props;
}

inline bool operator== (const frontend_scanner_allocation_struct& s1, const frontend_scanner_allocation_struct& s2) {
    if (s1.min_freq!=s2.min_freq)
        return false;
    if (s1.max_freq!=s2.max_freq)
        return false;
    if (s1.mode!=s2.mode)
        return false;
    if (s1.control_mode!=s2.control_mode)
        return false;
    if (s1.control_limit!=s2.control_limit)
        return false;
    return true;
}

inline bool operator!= (const frontend_scanner_allocation_struct& s1, const frontend_scanner_allocation_struct& s2) {
    return !(s1==s2);
}

struct frontend_tuner_status_struct_struct {
    frontend_tuner_status_struct_struct ()
    {
    }

    static std::string getId() {
        return std::string("FRONTEND::tuner_status_struct");
    }

    static const char* getFormat() {
        return "sddbssdbbs";
    }

    std::string allocation_id_csv;
    double bandwidth;
    double center_frequency;
    bool enabled;
    std::string group_id;
    std::string rf_flow_id;
    double sample_rate;
    bool scan_mode_enabled;
    bool supports_scan;
    std::string tuner_type;
};

inline bool operator>>= (const CORBA::Any& a, frontend_tuner_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    const redhawk::PropertyMap& props = redhawk::PropertyMap::cast(*temp);
    if (props.contains("FRONTEND::tuner_status::allocation_id_csv")) {
        if (!(props["FRONTEND::tuner_status::allocation_id_csv"] >>= s.allocation_id_csv)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::bandwidth")) {
        if (!(props["FRONTEND::tuner_status::bandwidth"] >>= s.bandwidth)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::center_frequency")) {
        if (!(props["FRONTEND::tuner_status::center_frequency"] >>= s.center_frequency)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::enabled")) {
        if (!(props["FRONTEND::tuner_status::enabled"] >>= s.enabled)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::group_id")) {
        if (!(props["FRONTEND::tuner_status::group_id"] >>= s.group_id)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::rf_flow_id")) {
        if (!(props["FRONTEND::tuner_status::rf_flow_id"] >>= s.rf_flow_id)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::sample_rate")) {
        if (!(props["FRONTEND::tuner_status::sample_rate"] >>= s.sample_rate)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::scan_mode_enabled")) {
        if (!(props["FRONTEND::tuner_status::scan_mode_enabled"] >>= s.scan_mode_enabled)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::supports_scan")) {
        if (!(props["FRONTEND::tuner_status::supports_scan"] >>= s.supports_scan)) return false;
    }
    if (props.contains("FRONTEND::tuner_status::tuner_type")) {
        if (!(props["FRONTEND::tuner_status::tuner_type"] >>= s.tuner_type)) return false;
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend_tuner_status_struct_struct& s) {
    redhawk::PropertyMap props;
 
    props["FRONTEND::tuner_status::allocation_id_csv"] = s.allocation_id_csv;
 
    props["FRONTEND::tuner_status::bandwidth"] = s.bandwidth;
 
    props["FRONTEND::tuner_status::center_frequency"] = s.center_frequency;
 
    props["FRONTEND::tuner_status::enabled"] = s.enabled;
 
    props["FRONTEND::tuner_status::group_id"] = s.group_id;
 
    props["FRONTEND::tuner_status::rf_flow_id"] = s.rf_flow_id;
 
    props["FRONTEND::tuner_status::sample_rate"] = s.sample_rate;
 
    props["FRONTEND::tuner_status::scan_mode_enabled"] = s.scan_mode_enabled;
 
    props["FRONTEND::tuner_status::supports_scan"] = s.supports_scan;
 
    props["FRONTEND::tuner_status::tuner_type"] = s.tuner_type;
    a <<= props;
}

inline bool operator== (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    if (s1.allocation_id_csv!=s2.allocation_id_csv)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
        return false;
    if (s1.center_frequency!=s2.center_frequency)
        return false;
    if (s1.enabled!=s2.enabled)
        return false;
    if (s1.group_id!=s2.group_id)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.scan_mode_enabled!=s2.scan_mode_enabled)
        return false;
    if (s1.supports_scan!=s2.supports_scan)
        return false;
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    return true;
}

inline bool operator!= (const frontend_tuner_status_struct_struct& s1, const frontend_tuner_status_struct_struct& s2) {
    return !(s1==s2);
}

#endif // STRUCTPROPS_H
