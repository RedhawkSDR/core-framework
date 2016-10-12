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
#ifndef FE_TYPES_H
#define FE_TYPES_H

#include <ossie/CF/cf.h>
#include <vector>
#include <string>
#include <ossie/BULKIO/bulkioDataTypes.h>

namespace frontend {
    
    // Time Type Definition
    enum timeTypes {
        J1950 = 1,
        J1970 = 2,
        JCY = 3
    };
    
    struct FreqRange {
        double min_val;
        double max_val;
        std::vector<double> values;
    };
    struct AntennaInfo {
        std::string name;
        std::string type;
        std::string size;
        std::string description;
    };
    struct FeedInfo {
        std::string name;
        std::string polarization;
        FreqRange freq_range;
    };
    struct SensorInfo {
        std::string mission;
        std::string collector;
        std::string rx;
        AntennaInfo antenna;
        FeedInfo feed;
    };
    struct PathDelay {
        double freq;
        double delay_ns;
    };
    typedef std::vector<PathDelay> PathDelays;
    struct RFCapabilities {
        FreqRange freq_range;
        FreqRange bw_range;
    };
    struct RFInfoPkt {
        std::string rf_flow_id;
        double rf_center_freq;
        double rf_bandwidth;
        double if_center_freq;
        bool spectrum_inverted;
        SensorInfo sensor;
        PathDelays ext_path_delays;
        RFCapabilities capabilities;
        CF::Properties additional_info;
    };
    typedef std::vector<RFInfoPkt> RFInfoPktSequence;
    
    struct PositionInfo {
        bool valid;
        std::string datum;
        double lat;
        double lon;
        double alt;
    };
    struct GPSInfo {
        std::string source_id;
        std::string rf_flow_id;
        std::string mode;
        long fom;
        long tfom;
        long datumID;
        double time_offset;
        double freq_offset;
        double time_variance;
        double freq_variance;
        short satellite_count;
        float snr;
        std::string status_message;
        BULKIO::PrecisionUTCTime timestamp;
        CF::Properties additional_info;
    };
    struct GpsTimePos {
        PositionInfo position;
        BULKIO::PrecisionUTCTime timestamp;
    };
    struct CartesianPositionInfo {
        bool valid;
        std::string datum;
        double x;
        double y;
        double z;
    };
    struct AttitudeInfo {
        bool valid;
        double pitch;
        double yaw;
        double roll;
    };
    struct VelocityInfo {
        bool valid;
        std::string datum;
        std::string coordinate_system;
        double x;
        double y;
        double z;
    };
    struct AccelerationInfo {
        bool valid;
        std::string datum;
        std::string coordinate_system;
        double x;
        double y;
        double z;
    };
    struct NavigationPacket {
        std::string source_id;
        std::string rf_flow_id;
        PositionInfo  position;
        CartesianPositionInfo cposition;
        VelocityInfo  velocity;
        AccelerationInfo  acceleration;
        AttitudeInfo  attitude;
        BULKIO::PrecisionUTCTime timestamp;
        CF::Properties additional_info;
    };
    
    struct frontend_tuner_allocation_struct {
        frontend_tuner_allocation_struct ()
        {
            center_frequency = 0.0;
            bandwidth = 0.0;
            bandwidth_tolerance = 10.0;
            sample_rate = 0.0;
            sample_rate_tolerance = 10.0;
            device_control = true;
        }
        
        static std::string getId() {
            return std::string("FRONTEND::tuner_allocation");
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
    
    struct frontend_listener_allocation_struct {
        frontend_listener_allocation_struct ()
        {
        }
        
        static std::string getId() {
            return std::string("FRONTEND::listener_allocation");
        }
        
        std::string existing_allocation_id;
        std::string listener_allocation_id;
    };
    
    struct default_frontend_tuner_status_struct_struct {
        default_frontend_tuner_status_struct_struct ()
        {
            center_frequency = 0.0;
            bandwidth = 0.0;
            sample_rate = 0.0;
            enabled = false;
        }
        
        static std::string getId() {
            return std::string("frontend_tuner_status_struct");
        }
        
        std::string tuner_type;
        std::string allocation_id_csv;
        double center_frequency;
        double bandwidth;
        double sample_rate;
        std::string group_id;
        std::string rf_flow_id;
        bool enabled;
    };
}

#endif
