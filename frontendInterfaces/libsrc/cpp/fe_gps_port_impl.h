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
#ifndef FE_GPS_PORT_H
#define FE_GPS_PORT_H

#include "fe_port_impl.h"

#include <redhawk/FRONTEND/GPS.h>

namespace frontend {

    class gps_delegation {
        public:
            virtual frontend::GPSInfo get_gps_info(const std::string& port_name) {
                return frontend::GPSInfo();
            }
            virtual void set_gps_info(const std::string& port_name, const frontend::GPSInfo &gps_info) {
            }
            virtual frontend::GpsTimePos get_gps_time_pos(const std::string& port_name) {
                return frontend::GpsTimePos();
            }
            virtual void set_gps_time_pos(const std::string& port_name, const frontend::GpsTimePos &gps_time_pos) {
            }
    };
    // ----------------------------------------------------------------------------------------
    // InGPSPort declaration
    // ----------------------------------------------------------------------------------------
    class InGPSPort : public POA_FRONTEND::GPS, public Port_Provides_base_impl
    {
        public:
            InGPSPort(std::string port_name, gps_delegation *_parent) : 
            Port_Provides_base_impl(port_name)
            {
                parent = _parent;
            };
            ~InGPSPort() {};
            
            FRONTEND::GPSInfo* gps_info() {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::GPSInfo retval = this->parent->get_gps_info(this->name);
                FRONTEND::GPSInfo* tmpVal = frontend::returnGPSInfo(retval);
                return tmpVal;
            };
            void gps_info(const FRONTEND::GPSInfo &gps) {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::GPSInfo input = frontend::returnGPSInfo(gps);
                this->parent->set_gps_info(this->name, input);
                return;
            };
            FRONTEND::GpsTimePos* gps_time_pos() {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::GpsTimePos retval = this->parent->get_gps_time_pos(this->name);
                FRONTEND::GpsTimePos* tmpVal = frontend::returnGpsTimePos(retval);
                return tmpVal;
            };
            void gps_time_pos(const FRONTEND::GpsTimePos& gps_time_pos) {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::GpsTimePos input = frontend::returnGpsTimePos(gps_time_pos);
                this->parent->set_gps_time_pos(this->name, input);
                return;
            };
            
        protected:
            gps_delegation *parent;
            boost::mutex portAccess;
    };

    // ----------------------------------------------------------------------------------------
    // OutGPSPort declaration
    // ----------------------------------------------------------------------------------------
    template<typename PortType_var, typename PortType>
    class OutGPSPortT : public OutFrontendPort<PortType_var,PortType>
    {
        public:
            OutGPSPortT(std::string port_name) : OutFrontendPort<PortType_var, PortType>(port_name)
            {};
            ~OutGPSPortT(){};
            
            frontend::GPSInfo gps_info() {
                frontend::GPSInfo retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        try {
                            const FRONTEND::GPSInfo_var tmp = ((*i).first)->gps_info();
                            retval = frontend::returnGPSInfo(tmp);
                        } catch(...) {
                        }
                    }
                }
                return retval;
            };
            void gps_info(const frontend::GPSInfo &gps) {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        try {
                            const FRONTEND::GPSInfo_var tmp = frontend::returnGPSInfo(gps);
                            ((*i).first)->gps_info(tmp);
                        } catch(...) {
                        }
                    }
                }
                return;
            };
            frontend::GpsTimePos gps_time_pos() {
                frontend::RFInfoPkt retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        try {
                            const FRONTEND::GpsTimePos_var tmp = ((*i).first)->gps_time_pos();
                            retval = frontend::returnGpsTimePos(tmp);
                        } catch(...) {
                        }
                    }
                }
                return retval;
            };
            void gps_time_pos(frontend::GpsTimePos gps_time_pos) {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        try {
                            const FRONTEND::GpsTimePos_var tmp = frontend::returnGpsTimePos(gps_time_pos);
                            ((*i).first)->gps_time_pos(tmp);
                        } catch(...) {
                        }
                    }
                }
                return;
            };
            
    };
    class OutGPSPort : public OutGPSPortT<FRONTEND::GPS_var,FRONTEND::GPS> {
        public:
            OutGPSPort(std::string port_name) : OutGPSPortT<FRONTEND::GPS_var,FRONTEND::GPS>(port_name)
            {};
    };
    
} // end of frontend namespace


#endif
