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
            std::string getRepid() const {
                return "IDL:FRONTEND/GPS:1.0";
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

            std::vector<std::string> getConnectionIds()
            {
                std::vector<std::string> retval;
                for (unsigned int i = 0; i < this->outConnections.size(); i++) {
                    retval.push_back(this->outConnections[i].second);
                }
                return retval;
            };
            void __evaluateRequestBasedOnConnections(const std::string &__connection_id__, bool returnValue, bool inOut, bool out) {
                if (__connection_id__.empty() and (this->outConnections.size() > 1)) {
                    if (out or inOut or returnValue) {
                        throw redhawk::PortCallError("Returned parameters require either a single connection or a populated __connection_id__ to disambiguate the call.",
                                getConnectionIds());
                    }
                }
                if (this->outConnections.empty()) {
                    if (out or inOut or returnValue) {
                        throw redhawk::PortCallError("No connections available.", std::vector<std::string>());
                    } else {
                        if (not __connection_id__.empty()) {
                            std::ostringstream eout;
                            eout<<"The requested connection id ("<<__connection_id__<<") does not exist.";
                            throw redhawk::PortCallError(eout.str(), getConnectionIds());
                        }
                    }
                }
                if ((not __connection_id__.empty()) and (not this->outConnections.empty())) {
                    bool foundConnection = false;
                    typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (i->second == __connection_id__) {
                            foundConnection = true;
                            break;
                        }
                    }
                    if (not foundConnection) {
                        std::ostringstream eout;
                        eout<<"The requested connection id ("<<__connection_id__<<") does not exist.";
                        throw redhawk::PortCallError(eout.str(), getConnectionIds());
                    }
                }
            }
            frontend::GPSInfo gps_info() {
                return _get_gps_info("");
            };
            frontend::GPSInfo _get_gps_info(const std::string __connection_id__) {
                frontend::GPSInfo retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        const FRONTEND::GPSInfo_var tmp = ((*i).first)->gps_info();
                        retval = frontend::returnGPSInfo(tmp);
                    }
                }
                return retval;
            };
            void gps_info(const frontend::GPSInfo &gps, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        const FRONTEND::GPSInfo_var tmp = frontend::returnGPSInfo(gps);
                        ((*i).first)->gps_info(tmp);
                    }
                }
                return;
            };
            frontend::GpsTimePos gps_time_pos() {
                return _get_gps_time_pos("");
            };
            frontend::GpsTimePos _get_gps_time_pos(const std::string __connection_id__) {
                frontend::GpsTimePos retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        const FRONTEND::GpsTimePos_var tmp = ((*i).first)->gps_time_pos();
                        retval = frontend::returnGpsTimePos(tmp);
                    }
                }
                return retval;
            };
            void gps_time_pos(frontend::GpsTimePos gps_time_pos, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        const FRONTEND::GpsTimePos_var tmp = frontend::returnGpsTimePos(gps_time_pos);
                        ((*i).first)->gps_time_pos(tmp);
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
