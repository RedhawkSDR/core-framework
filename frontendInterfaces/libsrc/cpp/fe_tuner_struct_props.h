/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK frontendInterfaces.
 *
 * REDHAWK frontendInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK frontendInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef FE_TUNER_STRUCTPROPS_H
#define FE_TUNER_STRUCTPROPS_H

#include <ossie/CorbaUtils.h>
#include <ossie/PropertyInterface.h>
#include "fe_types.h"

inline bool operator>>= (const CORBA::Any& a, frontend::frontend_tuner_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("FRONTEND::tuner_allocation::tuner_type", props[idx].id)) {
            if (!(props[idx].value >>= s.tuner_type)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::allocation_id", props[idx].id)) {
            if (!(props[idx].value >>= s.allocation_id)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::center_frequency", props[idx].id)) {
            if (!(props[idx].value >>= s.center_frequency)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::bandwidth", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::bandwidth_tolerance", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth_tolerance)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::sample_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.sample_rate)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::sample_rate_tolerance", props[idx].id)) {
            if (!(props[idx].value >>= s.sample_rate_tolerance)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::device_control", props[idx].id)) {
            if (!(props[idx].value >>= s.device_control)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::group_id", props[idx].id)) {
            if (!(props[idx].value >>= s.group_id)) return false;
        }
        if (!strcmp("FRONTEND::tuner_allocation::rf_flow_id", props[idx].id)) {
            if (!(props[idx].value >>= s.rf_flow_id)) return false;
        }
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend::frontend_tuner_allocation_struct& s) {
    CF::Properties props;
    props.length(10);
    props[0].id = CORBA::string_dup("FRONTEND::tuner_allocation::tuner_type");
    props[0].value <<= s.tuner_type;
    props[1].id = CORBA::string_dup("FRONTEND::tuner_allocation::allocation_id");
    props[1].value <<= s.allocation_id;
    props[2].id = CORBA::string_dup("FRONTEND::tuner_allocation::center_frequency");
    props[2].value <<= s.center_frequency;
    props[3].id = CORBA::string_dup("FRONTEND::tuner_allocation::bandwidth");
    props[3].value <<= s.bandwidth;
    props[4].id = CORBA::string_dup("FRONTEND::tuner_allocation::bandwidth_tolerance");
    props[4].value <<= s.bandwidth_tolerance;
    props[5].id = CORBA::string_dup("FRONTEND::tuner_allocation::sample_rate");
    props[5].value <<= s.sample_rate;
    props[6].id = CORBA::string_dup("FRONTEND::tuner_allocation::sample_rate_tolerance");
    props[6].value <<= s.sample_rate_tolerance;
    props[7].id = CORBA::string_dup("FRONTEND::tuner_allocation::device_control");
    props[7].value <<= s.device_control;
    props[8].id = CORBA::string_dup("FRONTEND::tuner_allocation::group_id");
    props[8].value <<= s.group_id;
    props[9].id = CORBA::string_dup("FRONTEND::tuner_allocation::rf_flow_id");
    props[9].value <<= s.rf_flow_id;
    a <<= props;
}

inline bool operator== (const frontend::frontend_tuner_allocation_struct& s1, const frontend::frontend_tuner_allocation_struct& s2) {
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

inline bool operator!= (const frontend::frontend_tuner_allocation_struct& s1, const frontend::frontend_tuner_allocation_struct& s2) {
    return !(s1==s2);
}

template<> inline short StructProperty<frontend::frontend_tuner_allocation_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    frontend::frontend_tuner_allocation_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

inline bool operator>>= (const CORBA::Any& a, frontend::frontend_listener_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("FRONTEND::listener_allocation::existing_allocation_id", props[idx].id)) {
            if (!(props[idx].value >>= s.existing_allocation_id)) return false;
        }
        if (!strcmp("FRONTEND::listener_allocation::listener_allocation_id", props[idx].id)) {
            if (!(props[idx].value >>= s.listener_allocation_id)) return false;
        }
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend::frontend_listener_allocation_struct& s) {
    CF::Properties props;
    props.length(2);
    props[0].id = CORBA::string_dup("FRONTEND::listener_allocation::existing_allocation_id");
    props[0].value <<= s.existing_allocation_id;
    props[1].id = CORBA::string_dup("FRONTEND::listener_allocation::listener_allocation_id");
    props[1].value <<= s.listener_allocation_id;
    a <<= props;
}

inline bool operator== (const frontend::frontend_listener_allocation_struct& s1, const frontend::frontend_listener_allocation_struct& s2) {
    if (s1.existing_allocation_id!=s2.existing_allocation_id)
        return false;
    if (s1.listener_allocation_id!=s2.listener_allocation_id)
        return false;
    return true;
}

inline bool operator!= (const frontend::frontend_listener_allocation_struct& s1, const frontend::frontend_listener_allocation_struct& s2) {
    return !(s1==s2);
}

template<> inline short StructProperty<frontend::frontend_listener_allocation_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    frontend::frontend_listener_allocation_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

inline bool operator>>= (const CORBA::Any& a, frontend::default_frontend_tuner_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("FRONTEND::tuner_status::tuner_type", props[idx].id)) {
            if (!(props[idx].value >>= s.tuner_type)) return false;
        }
        if (!strcmp("FRONTEND::tuner_status::allocation_id_csv", props[idx].id)) {
            if (!(props[idx].value >>= s.allocation_id_csv)) return false;
        }
        if (!strcmp("FRONTEND::tuner_status::center_frequency", props[idx].id)) {
            if (!(props[idx].value >>= s.center_frequency)) return false;
        }
        if (!strcmp("FRONTEND::tuner_status::bandwidth", props[idx].id)) {
            if (!(props[idx].value >>= s.bandwidth)) return false;
        }
        if (!strcmp("FRONTEND::tuner_status::sample_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.sample_rate)) return false;
        }
        if (!strcmp("FRONTEND::tuner_status::group_id", props[idx].id)) {
            if (!(props[idx].value >>= s.group_id)) return false;
        }
        if (!strcmp("FRONTEND::tuner_status::rf_flow_id", props[idx].id)) {
            if (!(props[idx].value >>= s.rf_flow_id)) return false;
        }
        if (!strcmp("FRONTEND::tuner_status::enabled", props[idx].id)) {
            if (!(props[idx].value >>= s.enabled)) return false;
        }
    }
    return true;
}

inline void operator<<= (CORBA::Any& a, const frontend::default_frontend_tuner_status_struct_struct& s) {
    CF::Properties props;
    props.length(8);
    props[0].id = CORBA::string_dup("FRONTEND::tuner_status::tuner_type");
    props[0].value <<= s.tuner_type;
    props[1].id = CORBA::string_dup("FRONTEND::tuner_status::allocation_id_csv");
    props[1].value <<= s.allocation_id_csv;
    props[2].id = CORBA::string_dup("FRONTEND::tuner_status::center_frequency");
    props[2].value <<= s.center_frequency;
    props[3].id = CORBA::string_dup("FRONTEND::tuner_status::bandwidth");
    props[3].value <<= s.bandwidth;
    props[4].id = CORBA::string_dup("FRONTEND::tuner_status::sample_rate");
    props[4].value <<= s.sample_rate;
    props[5].id = CORBA::string_dup("FRONTEND::tuner_status::group_id");
    props[5].value <<= s.group_id;
    props[6].id = CORBA::string_dup("FRONTEND::tuner_status::rf_flow_id");
    props[6].value <<= s.rf_flow_id;
    props[7].id = CORBA::string_dup("FRONTEND::tuner_status::enabled");
    props[7].value <<= s.enabled;
    a <<= props;
}

inline bool operator== (const frontend::default_frontend_tuner_status_struct_struct& s1, const frontend::default_frontend_tuner_status_struct_struct& s2) {
    if (s1.tuner_type!=s2.tuner_type)
        return false;
    if (s1.allocation_id_csv!=s2.allocation_id_csv)
        return false;
    if (s1.center_frequency!=s2.center_frequency)
        return false;
    if (s1.bandwidth!=s2.bandwidth)
        return false;
    if (s1.sample_rate!=s2.sample_rate)
        return false;
    if (s1.group_id!=s2.group_id)
        return false;
    if (s1.rf_flow_id!=s2.rf_flow_id)
        return false;
    if (s1.enabled!=s2.enabled)
        return false;
    return true;
}

inline bool operator!= (const frontend::default_frontend_tuner_status_struct_struct& s1, const frontend::default_frontend_tuner_status_struct_struct& s2) {
    return !(s1==s2);
}

template<> inline short StructProperty<frontend::default_frontend_tuner_status_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    frontend::default_frontend_tuner_status_struct_struct tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

inline bool operator== (const std::vector<frontend::default_frontend_tuner_status_struct_struct>& s1, const std::vector<frontend::default_frontend_tuner_status_struct_struct>& s2) {
    if (s1.size() != s2.size()) {
        return false;
    }
    for (unsigned int i=0; i<s1.size(); i++) {
        if (s1[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

inline bool operator!= (const std::vector<frontend::default_frontend_tuner_status_struct_struct>& s1, const std::vector<frontend::default_frontend_tuner_status_struct_struct>& s2) {
    return !(s1==s2);
}

template<> inline short StructSequenceProperty<frontend::default_frontend_tuner_status_struct_struct>::compare (const CORBA::Any& a) {
    if (super::isNil_) {
        CORBA::TypeCode_var aType = a.type();
        if (aType->kind() == (CORBA::tk_null)) {
            return 0;
        }
        return 1;
    }

    std::vector<frontend::default_frontend_tuner_status_struct_struct> tmp;
    if (fromAny(a, tmp)) {
        if (tmp != this->value_) {
            return 1;
        }

        return 0;
    } else {
        return 1;
    }
}

#endif
