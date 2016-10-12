/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef STRUCTPROPS_H
#define STRUCTPROPS_H

/*******************************************************************************************

    AUTO-GENERATED CODE. DO NOT MODIFY

*******************************************************************************************/

#include <ossie/CorbaUtils.h>
#include <CF/cf.h>
#include "affinity_struct.h"

struct nic_allocation_struct {
    nic_allocation_struct ()
    {
        data_rate = 0.0;
        data_size = 1;
        multicast_support = "False";
    };

    static std::string getId() {
        return std::string("nic_allocation");
    };

    std::string identifier;
    float data_rate;
    short data_size;
    std::string multicast_support;
    std::string ip_addressable;
    std::string interface;
};

inline bool operator>>= (const CORBA::Any& a, nic_allocation_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("nic_allocation::identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.identifier)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation::data_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.data_rate)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation::data_size", props[idx].id)) {
            if (!(props[idx].value >>= s.data_size)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation::multicast_support", props[idx].id)) {
            if (!(props[idx].value >>= s.multicast_support)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation::ip_addressable", props[idx].id)) {
            if (!(props[idx].value >>= s.ip_addressable)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation::interface", props[idx].id)) {
            if (!(props[idx].value >>= s.interface)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const nic_allocation_struct& s) {
    CF::Properties props;
    props.length(6);
    props[0].id = CORBA::string_dup("nic_allocation::identifier");
    props[0].value <<= s.identifier;
    props[1].id = CORBA::string_dup("nic_allocation::data_rate");
    props[1].value <<= s.data_rate;
    props[2].id = CORBA::string_dup("nic_allocation::data_size");
    props[2].value <<= s.data_size;
    props[3].id = CORBA::string_dup("nic_allocation::multicast_support");
    props[3].value <<= s.multicast_support;
    props[4].id = CORBA::string_dup("nic_allocation::ip_addressable");
    props[4].value <<= s.ip_addressable;
    props[5].id = CORBA::string_dup("nic_allocation::interface");
    props[5].value <<= s.interface;
    a <<= props;
};

inline bool operator== (const nic_allocation_struct& s1, const nic_allocation_struct& s2) {
    if (s1.identifier!=s2.identifier)
        return false;
    if (s1.data_rate!=s2.data_rate)
        return false;
    if (s1.data_size!=s2.data_size)
        return false;
    if (s1.multicast_support!=s2.multicast_support)
        return false;
    if (s1.ip_addressable!=s2.ip_addressable)
        return false;
    if (s1.interface!=s2.interface)
        return false;
    return true;
};

inline bool operator!= (const nic_allocation_struct& s1, const nic_allocation_struct& s2) {
    return !(s1==s2);
};

struct advanced_struct {
    advanced_struct ()
    {
        maximum_throughput_percentage = 80.0;
    };

    static std::string getId() {
        return std::string("advanced");
    };

    double maximum_throughput_percentage;
};

inline bool operator>>= (const CORBA::Any& a, advanced_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("maximum_throughput_percentage", props[idx].id)) {
            if (!(props[idx].value >>= s.maximum_throughput_percentage)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const advanced_struct& s) {
    CF::Properties props;
    props.length(1);
    props[0].id = CORBA::string_dup("maximum_throughput_percentage");
    props[0].value <<= s.maximum_throughput_percentage;
    a <<= props;
};

inline bool operator== (const advanced_struct& s1, const advanced_struct& s2) {
    if (s1.maximum_throughput_percentage!=s2.maximum_throughput_percentage)
        return false;
    return true;
};

inline bool operator!= (const advanced_struct& s1, const advanced_struct& s2) {
    return !(s1==s2);
};

struct threshold_event_struct {
    threshold_event_struct ()
    {
    };

    static std::string getId() {
        return std::string("threshold_event");
    };

    std::string source_id;
    std::string resource_id;
    std::string threshold_class;
    std::string type;
    std::string threshold_value;
    std::string measured_value;
    std::string message;
    double timestamp;
};

inline bool operator>>= (const CORBA::Any& a, threshold_event_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("threshold_event::source_id", props[idx].id)) {
            if (!(props[idx].value >>= s.source_id)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("threshold_event::resource_id", props[idx].id)) {
            if (!(props[idx].value >>= s.resource_id)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("threshold_event::threshold_class", props[idx].id)) {
            if (!(props[idx].value >>= s.threshold_class)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("threshold_event::type", props[idx].id)) {
            if (!(props[idx].value >>= s.type)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("threshold_event::threshold_value", props[idx].id)) {
            if (!(props[idx].value >>= s.threshold_value)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("threshold_event::measured_value", props[idx].id)) {
            if (!(props[idx].value >>= s.measured_value)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("threshold_event::message", props[idx].id)) {
            if (!(props[idx].value >>= s.message)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("threshold_event::timestamp", props[idx].id)) {
            if (!(props[idx].value >>= s.timestamp)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const threshold_event_struct& s) {
    CF::Properties props;
    props.length(8);
    props[0].id = CORBA::string_dup("threshold_event::source_id");
    props[0].value <<= s.source_id;
    props[1].id = CORBA::string_dup("threshold_event::resource_id");
    props[1].value <<= s.resource_id;
    props[2].id = CORBA::string_dup("threshold_event::threshold_class");
    props[2].value <<= s.threshold_class;
    props[3].id = CORBA::string_dup("threshold_event::type");
    props[3].value <<= s.type;
    props[4].id = CORBA::string_dup("threshold_event::threshold_value");
    props[4].value <<= s.threshold_value;
    props[5].id = CORBA::string_dup("threshold_event::measured_value");
    props[5].value <<= s.measured_value;
    props[6].id = CORBA::string_dup("threshold_event::message");
    props[6].value <<= s.message;
    props[7].id = CORBA::string_dup("threshold_event::timestamp");
    props[7].value <<= s.timestamp;
    a <<= props;
};

inline bool operator== (const threshold_event_struct& s1, const threshold_event_struct& s2) {
    if (s1.source_id!=s2.source_id)
        return false;
    if (s1.resource_id!=s2.resource_id)
        return false;
    if (s1.threshold_class!=s2.threshold_class)
        return false;
    if (s1.type!=s2.type)
        return false;
    if (s1.threshold_value!=s2.threshold_value)
        return false;
    if (s1.measured_value!=s2.measured_value)
        return false;
    if (s1.message!=s2.message)
        return false;
    if (s1.timestamp!=s2.timestamp)
        return false;
    return true;
};

inline bool operator!= (const threshold_event_struct& s1, const threshold_event_struct& s2) {
    return !(s1==s2);
};

struct thresholds_struct {
    thresholds_struct ()
    {
        cpu_idle = 10;
        mem_free = 100;
        nic_usage = 900;
    };

    static std::string getId() {
        return std::string("thresholds");
    };

    float cpu_idle;
    CORBA::LongLong mem_free;
    CORBA::Long nic_usage;
};

inline bool operator>>= (const CORBA::Any& a, thresholds_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("cpu_idle", props[idx].id)) {
            if (!(props[idx].value >>= s.cpu_idle)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("mem_free", props[idx].id)) {
            if (!(props[idx].value >>= s.mem_free)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_usage", props[idx].id)) {
            if (!(props[idx].value >>= s.nic_usage)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const thresholds_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("cpu_idle");
    props[0].value <<= s.cpu_idle;
    props[1].id = CORBA::string_dup("mem_free");
    props[1].value <<= s.mem_free;
    props[2].id = CORBA::string_dup("nic_usage");
    props[2].value <<= s.nic_usage;
    a <<= props;
};

inline bool operator== (const thresholds_struct& s1, const thresholds_struct& s2) {
    if (s1.cpu_idle!=s2.cpu_idle)
        return false;
    if (s1.mem_free!=s2.mem_free)
        return false;
    if (s1.nic_usage!=s2.nic_usage)
        return false;
    return true;
};

inline bool operator!= (const thresholds_struct& s1, const thresholds_struct& s2) {
    return !(s1==s2);
};

struct nic_allocation_status_struct_struct {
    nic_allocation_status_struct_struct ()
    {
    };

    static std::string getId() {
        return std::string("nic_allocation_status_struct");
    };

    std::string identifier;
    float data_rate;
    short data_size;
    std::string multicast_support;
    std::string ip_addressable;
    std::string interface;
};

inline bool operator>>= (const CORBA::Any& a, nic_allocation_status_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("nic_allocation_status::identifier", props[idx].id)) {
            if (!(props[idx].value >>= s.identifier)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation_status::data_rate", props[idx].id)) {
            if (!(props[idx].value >>= s.data_rate)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation_status::data_size", props[idx].id)) {
            if (!(props[idx].value >>= s.data_size)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation_status::multicast_support", props[idx].id)) {
            if (!(props[idx].value >>= s.multicast_support)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation_status::ip_addressable", props[idx].id)) {
            if (!(props[idx].value >>= s.ip_addressable)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_allocation_status::interface", props[idx].id)) {
            if (!(props[idx].value >>= s.interface)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const nic_allocation_status_struct_struct& s) {
    CF::Properties props;
    props.length(6);
    props[0].id = CORBA::string_dup("nic_allocation_status::identifier");
    props[0].value <<= s.identifier;
    props[1].id = CORBA::string_dup("nic_allocation_status::data_rate");
    props[1].value <<= s.data_rate;
    props[2].id = CORBA::string_dup("nic_allocation_status::data_size");
    props[2].value <<= s.data_size;
    props[3].id = CORBA::string_dup("nic_allocation_status::multicast_support");
    props[3].value <<= s.multicast_support;
    props[4].id = CORBA::string_dup("nic_allocation_status::ip_addressable");
    props[4].value <<= s.ip_addressable;
    props[5].id = CORBA::string_dup("nic_allocation_status::interface");
    props[5].value <<= s.interface;
    a <<= props;
};

inline bool operator== (const nic_allocation_status_struct_struct& s1, const nic_allocation_status_struct_struct& s2) {
    if (s1.identifier!=s2.identifier)
        return false;
    if (s1.data_rate!=s2.data_rate)
        return false;
    if (s1.data_size!=s2.data_size)
        return false;
    if (s1.multicast_support!=s2.multicast_support)
        return false;
    if (s1.ip_addressable!=s2.ip_addressable)
        return false;
    if (s1.interface!=s2.interface)
        return false;
    return true;
};

inline bool operator!= (const nic_allocation_status_struct_struct& s1, const nic_allocation_status_struct_struct& s2) {
    return !(s1==s2);
};

struct nic_metrics_struct_struct {
    nic_metrics_struct_struct ()
    {
        rate = 0.0;
        multicast_support = false;
        rate_allocated = 0;
        time = 0;
        current_throughput = 0;
    };

    static std::string getId() {
        return std::string("nic_metrics_struct");
    };

    std::string interface;
    std::string mac_address;
    double rate;
    std::string ipv4_address;
    std::string ipv4_netmask;
    std::string ipv4_broadcast;
    std::string ipv6_address;
    std::string ipv6_netmask;
    std::string ipv6_scope;
    std::string flags;
    std::string module;
    std::string mtu;
    std::string state;
    std::string rx_bytes;
    std::string rx_compressed;
    std::string rx_crc_errors;
    std::string rx_dropped;
    std::string rx_errors;
    std::string rx_packets;
    std::string tx_bytes;
    std::string tx_compressed;
    std::string tx_dropped;
    std::string tx_errors;
    std::string tx_packets;
    std::string tx_queue_len;
    std::string vlans;
    bool multicast_support;
    double rate_allocated;
    std::string time_string_utc;
    double time;
    double current_throughput;
};

inline bool operator>>= (const CORBA::Any& a, nic_metrics_struct_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("nic_metrics::interface", props[idx].id)) {
            if (!(props[idx].value >>= s.interface)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::mac_address", props[idx].id)) {
            if (!(props[idx].value >>= s.mac_address)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rate", props[idx].id)) {
            if (!(props[idx].value >>= s.rate)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::ipv4_address", props[idx].id)) {
            if (!(props[idx].value >>= s.ipv4_address)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::ipv4_netmask", props[idx].id)) {
            if (!(props[idx].value >>= s.ipv4_netmask)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::ipv4_broadcast", props[idx].id)) {
            if (!(props[idx].value >>= s.ipv4_broadcast)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::ipv6_address", props[idx].id)) {
            if (!(props[idx].value >>= s.ipv6_address)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::ipv6_netmask", props[idx].id)) {
            if (!(props[idx].value >>= s.ipv6_netmask)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::ipv6_scope", props[idx].id)) {
            if (!(props[idx].value >>= s.ipv6_scope)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::flags", props[idx].id)) {
            if (!(props[idx].value >>= s.flags)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::module", props[idx].id)) {
            if (!(props[idx].value >>= s.module)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::mtu", props[idx].id)) {
            if (!(props[idx].value >>= s.mtu)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::state", props[idx].id)) {
            if (!(props[idx].value >>= s.state)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rx_bytes", props[idx].id)) {
            if (!(props[idx].value >>= s.rx_bytes)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rx_compressed", props[idx].id)) {
            if (!(props[idx].value >>= s.rx_compressed)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rx_crc_errors", props[idx].id)) {
            if (!(props[idx].value >>= s.rx_crc_errors)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rx_dropped", props[idx].id)) {
            if (!(props[idx].value >>= s.rx_dropped)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rx_errors", props[idx].id)) {
            if (!(props[idx].value >>= s.rx_errors)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rx_packets", props[idx].id)) {
            if (!(props[idx].value >>= s.rx_packets)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::tx_bytes", props[idx].id)) {
            if (!(props[idx].value >>= s.tx_bytes)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::tx_compressed", props[idx].id)) {
            if (!(props[idx].value >>= s.tx_compressed)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::tx_dropped", props[idx].id)) {
            if (!(props[idx].value >>= s.tx_dropped)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::tx_errors", props[idx].id)) {
            if (!(props[idx].value >>= s.tx_errors)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::tx_packets", props[idx].id)) {
            if (!(props[idx].value >>= s.tx_packets)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::tx_queue_len", props[idx].id)) {
            if (!(props[idx].value >>= s.tx_queue_len)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::vlans", props[idx].id)) {
            if (!(props[idx].value >>= s.vlans)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::multicast_support", props[idx].id)) {
            if (!(props[idx].value >>= s.multicast_support)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::rate_allocated", props[idx].id)) {
            if (!(props[idx].value >>= s.rate_allocated)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::time_string_utc", props[idx].id)) {
            if (!(props[idx].value >>= s.time_string_utc)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::time", props[idx].id)) {
            if (!(props[idx].value >>= s.time)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("nic_metrics::current_throughput", props[idx].id)) {
            if (!(props[idx].value >>= s.current_throughput)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const nic_metrics_struct_struct& s) {
    CF::Properties props;
    props.length(31);
    props[0].id = CORBA::string_dup("nic_metrics::interface");
    props[0].value <<= s.interface;
    props[1].id = CORBA::string_dup("nic_metrics::mac_address");
    props[1].value <<= s.mac_address;
    props[2].id = CORBA::string_dup("nic_metrics::rate");
    props[2].value <<= s.rate;
    props[3].id = CORBA::string_dup("nic_metrics::ipv4_address");
    props[3].value <<= s.ipv4_address;
    props[4].id = CORBA::string_dup("nic_metrics::ipv4_netmask");
    props[4].value <<= s.ipv4_netmask;
    props[5].id = CORBA::string_dup("nic_metrics::ipv4_broadcast");
    props[5].value <<= s.ipv4_broadcast;
    props[6].id = CORBA::string_dup("nic_metrics::ipv6_address");
    props[6].value <<= s.ipv6_address;
    props[7].id = CORBA::string_dup("nic_metrics::ipv6_netmask");
    props[7].value <<= s.ipv6_netmask;
    props[8].id = CORBA::string_dup("nic_metrics::ipv6_scope");
    props[8].value <<= s.ipv6_scope;
    props[9].id = CORBA::string_dup("nic_metrics::flags");
    props[9].value <<= s.flags;
    props[10].id = CORBA::string_dup("nic_metrics::module");
    props[10].value <<= s.module;
    props[11].id = CORBA::string_dup("nic_metrics::mtu");
    props[11].value <<= s.mtu;
    props[12].id = CORBA::string_dup("nic_metrics::state");
    props[12].value <<= s.state;
    props[13].id = CORBA::string_dup("nic_metrics::rx_bytes");
    props[13].value <<= s.rx_bytes;
    props[14].id = CORBA::string_dup("nic_metrics::rx_compressed");
    props[14].value <<= s.rx_compressed;
    props[15].id = CORBA::string_dup("nic_metrics::rx_crc_errors");
    props[15].value <<= s.rx_crc_errors;
    props[16].id = CORBA::string_dup("nic_metrics::rx_dropped");
    props[16].value <<= s.rx_dropped;
    props[17].id = CORBA::string_dup("nic_metrics::rx_errors");
    props[17].value <<= s.rx_errors;
    props[18].id = CORBA::string_dup("nic_metrics::rx_packets");
    props[18].value <<= s.rx_packets;
    props[19].id = CORBA::string_dup("nic_metrics::tx_bytes");
    props[19].value <<= s.tx_bytes;
    props[20].id = CORBA::string_dup("nic_metrics::tx_compressed");
    props[20].value <<= s.tx_compressed;
    props[21].id = CORBA::string_dup("nic_metrics::tx_dropped");
    props[21].value <<= s.tx_dropped;
    props[22].id = CORBA::string_dup("nic_metrics::tx_errors");
    props[22].value <<= s.tx_errors;
    props[23].id = CORBA::string_dup("nic_metrics::tx_packets");
    props[23].value <<= s.tx_packets;
    props[24].id = CORBA::string_dup("nic_metrics::tx_queue_len");
    props[24].value <<= s.tx_queue_len;
    props[25].id = CORBA::string_dup("nic_metrics::vlans");
    props[25].value <<= s.vlans;
    props[26].id = CORBA::string_dup("nic_metrics::multicast_support");
    props[26].value <<= s.multicast_support;
    props[27].id = CORBA::string_dup("nic_metrics::rate_allocated");
    props[27].value <<= s.rate_allocated;
    props[28].id = CORBA::string_dup("nic_metrics::time_string_utc");
    props[28].value <<= s.time_string_utc;
    props[29].id = CORBA::string_dup("nic_metrics::time");
    props[29].value <<= s.time;
    props[30].id = CORBA::string_dup("nic_metrics::current_throughput");
    props[30].value <<= s.current_throughput;
    a <<= props;
};

inline bool operator== (const nic_metrics_struct_struct& s1, const nic_metrics_struct_struct& s2) {
    if (s1.interface!=s2.interface)
        return false;
    if (s1.mac_address!=s2.mac_address)
        return false;
    if (s1.rate!=s2.rate)
        return false;
    if (s1.ipv4_address!=s2.ipv4_address)
        return false;
    if (s1.ipv4_netmask!=s2.ipv4_netmask)
        return false;
    if (s1.ipv4_broadcast!=s2.ipv4_broadcast)
        return false;
    if (s1.ipv6_address!=s2.ipv6_address)
        return false;
    if (s1.ipv6_netmask!=s2.ipv6_netmask)
        return false;
    if (s1.ipv6_scope!=s2.ipv6_scope)
        return false;
    if (s1.flags!=s2.flags)
        return false;
    if (s1.module!=s2.module)
        return false;
    if (s1.mtu!=s2.mtu)
        return false;
    if (s1.state!=s2.state)
        return false;
    if (s1.rx_bytes!=s2.rx_bytes)
        return false;
    if (s1.rx_compressed!=s2.rx_compressed)
        return false;
    if (s1.rx_crc_errors!=s2.rx_crc_errors)
        return false;
    if (s1.rx_dropped!=s2.rx_dropped)
        return false;
    if (s1.rx_errors!=s2.rx_errors)
        return false;
    if (s1.rx_packets!=s2.rx_packets)
        return false;
    if (s1.tx_bytes!=s2.tx_bytes)
        return false;
    if (s1.tx_compressed!=s2.tx_compressed)
        return false;
    if (s1.tx_dropped!=s2.tx_dropped)
        return false;
    if (s1.tx_errors!=s2.tx_errors)
        return false;
    if (s1.tx_packets!=s2.tx_packets)
        return false;
    if (s1.tx_queue_len!=s2.tx_queue_len)
        return false;
    if (s1.vlans!=s2.vlans)
        return false;
    if (s1.multicast_support!=s2.multicast_support)
        return false;
    if (s1.rate_allocated!=s2.rate_allocated)
        return false;
    if (s1.time_string_utc!=s2.time_string_utc)
        return false;
    if (s1.time!=s2.time)
        return false;
    if (s1.current_throughput!=s2.current_throughput)
        return false;
    return true;
};

inline bool operator!= (const nic_metrics_struct_struct& s1, const nic_metrics_struct_struct& s2) {
    return !(s1==s2);
};

struct interfaces_struct {
    interfaces_struct ()
    {
    };

    static std::string getId() {
        return std::string("interfaces");
    };

    std::string interface;
    float throughput;
    std::string vlans;
};

inline bool operator>>= (const CORBA::Any& a, interfaces_struct& s) {
    CF::Properties* temp;
    if (!(a >>= temp)) return false;
    CF::Properties& props = *temp;
    for (unsigned int idx = 0; idx < props.length(); idx++) {
        if (!strcmp("interface", props[idx].id)) {
            if (!(props[idx].value >>= s.interface)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("throughput", props[idx].id)) {
            if (!(props[idx].value >>= s.throughput)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
        else if (!strcmp("vlans", props[idx].id)) {
            if (!(props[idx].value >>= s.vlans)) {
                CORBA::TypeCode_var typecode = props[idx].value.type();
                if (typecode->kind() != CORBA::tk_null) {
                    return false;
                }
            }
        }
    }
    return true;
};

inline void operator<<= (CORBA::Any& a, const interfaces_struct& s) {
    CF::Properties props;
    props.length(3);
    props[0].id = CORBA::string_dup("interface");
    props[0].value <<= s.interface;
    props[1].id = CORBA::string_dup("throughput");
    props[1].value <<= s.throughput;
    props[2].id = CORBA::string_dup("vlans");
    props[2].value <<= s.vlans;
    a <<= props;
};

inline bool operator== (const interfaces_struct& s1, const interfaces_struct& s2) {
    if (s1.interface!=s2.interface)
        return false;
    if (s1.throughput!=s2.throughput)
        return false;
    if (s1.vlans!=s2.vlans)
        return false;
    return true;
};

inline bool operator!= (const interfaces_struct& s1, const interfaces_struct& s2) {
    return !(s1==s2);
};

#endif // STRUCTPROPS_H
