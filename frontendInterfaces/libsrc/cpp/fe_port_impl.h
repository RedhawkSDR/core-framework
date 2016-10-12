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
#ifndef FE_PORT_H
#define FE_PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/make_shared.hpp>
#include <boost/ref.hpp>

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#include <ossie/CF/QueryablePort.h>

#include <redhawk/FRONTEND/RFInfo.h>
#include <redhawk/FRONTEND/GPS.h>
#include <redhawk/FRONTEND/NavigationData.h>

#include <ossie/CorbaUtils.h>
#include "fe_types.h"


namespace frontend {

    //
    // BEGIN FROM bulkio_p.h
    // used for boost shared pointer instantiation when user
    // supplied callback is provided
    //
    struct null_deleter
    {
        void operator()(void const *) const
        {
        }
    };
    // END FROM bulkio_p.h

    inline FRONTEND::RFInfoPkt* returnRFInfoPkt(const RFInfoPkt &val) {
        FRONTEND::RFInfoPkt* tmpVal = new FRONTEND::RFInfoPkt();
        tmpVal->rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal->rf_center_freq = val.rf_center_freq;
        tmpVal->rf_bandwidth = val.rf_bandwidth;
        tmpVal->if_center_freq = val.if_center_freq;
        tmpVal->spectrum_inverted = val.spectrum_inverted;
        tmpVal->sensor.collector = CORBA::string_dup(val.sensor.collector.c_str());
        tmpVal->sensor.mission = CORBA::string_dup(val.sensor.mission.c_str());
        tmpVal->sensor.rx = CORBA::string_dup(val.sensor.rx.c_str());
        tmpVal->sensor.antenna.description = CORBA::string_dup(val.sensor.antenna.description.c_str());
        tmpVal->sensor.antenna.name = CORBA::string_dup(val.sensor.antenna.name.c_str());
        tmpVal->sensor.antenna.size = CORBA::string_dup(val.sensor.antenna.size.c_str());
        tmpVal->sensor.antenna.type = CORBA::string_dup(val.sensor.antenna.type.c_str());
        tmpVal->sensor.feed.name = CORBA::string_dup(val.sensor.feed.name.c_str());
        tmpVal->sensor.feed.polarization = CORBA::string_dup(val.sensor.feed.polarization.c_str());
        tmpVal->sensor.feed.freq_range.max_val = val.sensor.feed.freq_range.max_val;
        tmpVal->sensor.feed.freq_range.min_val = val.sensor.feed.freq_range.min_val;
        tmpVal->sensor.feed.freq_range.values.length(val.sensor.feed.freq_range.values.size());
        for (unsigned int i=0; i<val.sensor.feed.freq_range.values.size(); i++) {
            tmpVal->sensor.feed.freq_range.values[i] = val.sensor.feed.freq_range.values[i];
        }
        return tmpVal;
    };
    inline RFInfoPkt returnRFInfoPkt(const FRONTEND::RFInfoPkt &tmpVal) {
        RFInfoPkt val;
        val.rf_flow_id = ossie::corba::returnString(tmpVal.rf_flow_id);
        val.rf_center_freq = tmpVal.rf_center_freq;
        val.rf_bandwidth = tmpVal.rf_bandwidth;
        val.if_center_freq = tmpVal.if_center_freq;
        val.spectrum_inverted = tmpVal.spectrum_inverted;
        val.sensor.collector = ossie::corba::returnString(tmpVal.sensor.collector);
        val.sensor.mission = ossie::corba::returnString(tmpVal.sensor.mission);
        val.sensor.rx = ossie::corba::returnString(tmpVal.sensor.rx);
        val.sensor.antenna.description = ossie::corba::returnString(tmpVal.sensor.antenna.description);
        val.sensor.antenna.name = ossie::corba::returnString(tmpVal.sensor.antenna.name);
        val.sensor.antenna.size = ossie::corba::returnString(tmpVal.sensor.antenna.size);
        val.sensor.antenna.type = ossie::corba::returnString(tmpVal.sensor.antenna.type);
        val.sensor.feed.name = ossie::corba::returnString(tmpVal.sensor.feed.name);
        val.sensor.feed.polarization = ossie::corba::returnString(tmpVal.sensor.feed.polarization);
        val.sensor.feed.freq_range.max_val = tmpVal.sensor.feed.freq_range.max_val;
        val.sensor.feed.freq_range.min_val = tmpVal.sensor.feed.freq_range.min_val;
        val.sensor.feed.freq_range.values.resize(tmpVal.sensor.feed.freq_range.values.length());
        for (unsigned int i=0; i<val.sensor.feed.freq_range.values.size(); i++) {
            val.sensor.feed.freq_range.values[i] = tmpVal.sensor.feed.freq_range.values[i];
        }
        return val;
    };
    
    inline FRONTEND::GPSInfo* returnGPSInfo(const frontend::GPSInfo &val) {
        FRONTEND::GPSInfo* tmpVal = new FRONTEND::GPSInfo();
        tmpVal->source_id = CORBA::string_dup(val.source_id.c_str());
        tmpVal->rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal->mode = CORBA::string_dup(val.mode.c_str());
        tmpVal->fom = val.fom;
        tmpVal->tfom = val.tfom;
        tmpVal->datumID = val.datumID;
        tmpVal->time_offset = val.time_offset;
        tmpVal->freq_offset = val.freq_offset;
        tmpVal->time_variance = val.time_variance;
        tmpVal->freq_variance = val.freq_variance;
        tmpVal->satellite_count = val.satellite_count;
        tmpVal->snr = val.snr;
        tmpVal->status_message = CORBA::string_dup(val.status_message.c_str());
        tmpVal->timestamp = val.timestamp;
        tmpVal->additional_info = val.additional_info;
        return tmpVal;
    };
    inline frontend::GPSInfo returnGPSInfo(const FRONTEND::GPSInfo &tmpVal) {
        frontend::GPSInfo val;
        val.source_id = ossie::corba::returnString(tmpVal.source_id);
        val.rf_flow_id = ossie::corba::returnString(tmpVal.rf_flow_id);
        val.mode = ossie::corba::returnString(tmpVal.mode);
        val.fom = tmpVal.fom;
        val.tfom = tmpVal.tfom;
        val.datumID = tmpVal.datumID;
        val.time_offset = tmpVal.time_offset;
        val.freq_offset = tmpVal.freq_offset;
        val.time_variance = tmpVal.time_variance;
        val.freq_variance = tmpVal.freq_variance;
        val.satellite_count = tmpVal.satellite_count;
        val.snr = tmpVal.snr;
        val.status_message = ossie::corba::returnString(tmpVal.status_message);
        val.timestamp = tmpVal.timestamp;
        val.additional_info = tmpVal.additional_info;
        return val;
    };
    
    inline FRONTEND::GpsTimePos* returnGpsTimePos(const frontend::GpsTimePos &val) {
        FRONTEND::GpsTimePos* tmpVal = new FRONTEND::GpsTimePos();
        tmpVal->position.valid = val.position.valid;
        tmpVal->position.datum = CORBA::string_dup(val.position.datum.c_str());
        tmpVal->position.lat = val.position.lat;
        tmpVal->position.lon = val.position.lon;
        tmpVal->position.alt = val.position.alt;
        tmpVal->timestamp = val.timestamp;
        return tmpVal;
    };
    inline frontend::GpsTimePos returnGpsTimePos(const FRONTEND::GpsTimePos &tmpVal) {
        frontend::GpsTimePos val;
        val.position.valid = tmpVal.position.valid;
        val.position.datum = ossie::corba::returnString(tmpVal.position.datum);
        val.position.lat = tmpVal.position.lat;
        val.position.lon = tmpVal.position.lon;
        val.position.alt = tmpVal.position.alt;
        val.timestamp = tmpVal.timestamp;
        return val;
    };
    inline FRONTEND::NavigationPacket* returnNavigationPacket(const frontend::NavigationPacket &val) {
        FRONTEND::NavigationPacket* tmpVal = new FRONTEND::NavigationPacket();
        tmpVal->source_id = CORBA::string_dup(val.source_id.c_str());
        tmpVal->rf_flow_id = CORBA::string_dup(val.rf_flow_id.c_str());
        tmpVal->position.valid = val.position.valid;
        tmpVal->position.datum = CORBA::string_dup(val.position.datum.c_str());
        tmpVal->position.lat = val.position.lat;
        tmpVal->position.lon = val.position.lon;
        tmpVal->position.alt = val.position.alt;
        tmpVal->cposition.valid = val.cposition.valid;
        tmpVal->cposition.datum = CORBA::string_dup(val.cposition.datum.c_str());
        tmpVal->cposition.x = val.cposition.x;
        tmpVal->cposition.y = val.cposition.y;
        tmpVal->cposition.z = val.cposition.z;
        tmpVal->velocity.valid = val.velocity.valid;
        tmpVal->velocity.datum = CORBA::string_dup(val.velocity.datum.c_str());
        tmpVal->velocity.coordinate_system = CORBA::string_dup(val.velocity.coordinate_system.c_str());
        tmpVal->velocity.x = val.velocity.x;
        tmpVal->velocity.y = val.velocity.y;
        tmpVal->velocity.z = val.velocity.z;
        tmpVal->acceleration.valid = val.acceleration.valid;
        tmpVal->acceleration.datum = CORBA::string_dup(val.acceleration.datum.c_str());
        tmpVal->acceleration.coordinate_system = CORBA::string_dup(val.acceleration.coordinate_system.c_str());
        tmpVal->acceleration.x = val.acceleration.x;
        tmpVal->acceleration.y = val.acceleration.y;
        tmpVal->acceleration.z = val.acceleration.z;
        tmpVal->attitude.valid = val.attitude.valid;
        tmpVal->attitude.pitch = val.attitude.pitch;
        tmpVal->attitude.yaw = val.attitude.yaw;
        tmpVal->attitude.roll = val.attitude.roll;
        tmpVal->timestamp = val.timestamp;
        tmpVal->additional_info = val.additional_info;
        return tmpVal;
    };
    inline frontend::NavigationPacket returnNavigationPacket(const FRONTEND::NavigationPacket &tmpVal) {
        frontend::NavigationPacket val;
        val.source_id = ossie::corba::returnString(tmpVal.source_id);
        val.rf_flow_id = ossie::corba::returnString(tmpVal.rf_flow_id);
        val.position.valid = tmpVal.position.valid;
        val.position.datum = ossie::corba::returnString(tmpVal.position.datum);
        val.position.lat = tmpVal.position.lat;
        val.position.lon = tmpVal.position.lon;
        val.position.alt = tmpVal.position.alt;
        val.cposition.valid = tmpVal.cposition.valid;
        val.cposition.datum = ossie::corba::returnString(tmpVal.cposition.datum);
        val.cposition.x = tmpVal.cposition.x;
        val.cposition.y = tmpVal.cposition.y;
        val.cposition.z = tmpVal.cposition.z;
        val.velocity.valid = tmpVal.velocity.valid;
        val.velocity.datum = ossie::corba::returnString(tmpVal.velocity.datum);
        val.velocity.coordinate_system = ossie::corba::returnString(tmpVal.velocity.coordinate_system);
        val.velocity.x = tmpVal.velocity.x;
        val.velocity.y = tmpVal.velocity.y;
        val.velocity.z = tmpVal.velocity.z;
        val.acceleration.valid = tmpVal.acceleration.valid;
        val.acceleration.datum = ossie::corba::returnString(tmpVal.acceleration.datum);
        val.acceleration.coordinate_system = ossie::corba::returnString(tmpVal.acceleration.coordinate_system);
        val.acceleration.x = tmpVal.acceleration.x;
        val.acceleration.y = tmpVal.acceleration.y;
        val.acceleration.z = tmpVal.acceleration.z;
        val.attitude.valid = tmpVal.attitude.valid;
        val.attitude.pitch = tmpVal.attitude.pitch;
        val.attitude.yaw = tmpVal.attitude.yaw;
        val.attitude.roll = tmpVal.attitude.roll;
        val.timestamp = tmpVal.timestamp;
        val.additional_info = tmpVal.additional_info;
        return val;
    };
    // ----------------------------------------------------------------------------------------
    // OutFrontendPort declaration
    // ----------------------------------------------------------------------------------------
    template <typename PortType_var, typename PortType>
    class OutFrontendPort : public Port_Uses_base_impl, public POA_ExtendedCF::QueryablePort
    {
        public:
            OutFrontendPort(std::string port_name) :
                Port_Uses_base_impl(port_name)
            {
                recConnectionsRefresh = false;
                recConnections.length(0);
            }
            ~OutFrontendPort(){
            }

            ExtendedCF::UsesConnectionSequence * connections()
            {
                boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
                if (recConnectionsRefresh) {
                    recConnections.length(outConnections.size());
                    for (unsigned int i = 0; i < outConnections.size(); i++) {
                        recConnections[i].connectionId = CORBA::string_dup(outConnections[i].second.c_str());
                        recConnections[i].port = CORBA::Object::_duplicate(outConnections[i].first);
                    }
                    recConnectionsRefresh = false;
                }
                ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence(recConnections);
                // NOTE: You must delete the object that this function returns!
                return retVal._retn();
            };

            void connectPort(CORBA::Object_ptr connection, const char* connectionId)
            {
                boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
                PortType_var port = PortType::_narrow(connection);
                outConnections.push_back(std::make_pair(port, connectionId));
                active = true;
                recConnectionsRefresh = true;
            };

            void disconnectPort(const char* connectionId)
            {
                boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
                for (unsigned int i = 0; i < outConnections.size(); i++) {
                    if (outConnections[i].second == connectionId) {
                        outConnections.erase(outConnections.begin() + i);
                        break;
                    }
                }

                if (outConnections.size() == 0) {
                    active = false;
                }
                recConnectionsRefresh = true;
            };

            std::vector< std::pair<PortType_var, std::string> > _getConnections()
            {
                return outConnections;
            };

        protected:
            std::vector < std::pair<PortType_var, std::string> > outConnections;
            ExtendedCF::UsesConnectionSequence recConnections;
            bool recConnectionsRefresh;
    };

} // end of frontend namespace


#endif
