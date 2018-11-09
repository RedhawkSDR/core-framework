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
#ifndef FE_NAVDATA_PORT_H
#define FE_NAVDATA_PORT_H

#include "fe_port_impl.h"

#include <redhawk/FRONTEND/NavigationData.h>

namespace frontend {

    class nav_delegation {
        public:
            virtual frontend::NavigationPacket get_nav_packet(const std::string& port_name) {
                return frontend::NavigationPacket();
            }
            virtual void set_nav_packet(const std::string& port_name, const frontend::NavigationPacket &nav_info) {
            }
    };
    // ----------------------------------------------------------------------------------------
    // InNavDataPort declaration
    // ----------------------------------------------------------------------------------------
    class InNavDataPort : public POA_FRONTEND::NavData, public Port_Provides_base_impl
    {
        public:
            InNavDataPort(std::string port_name, nav_delegation *_parent) : 
            Port_Provides_base_impl(port_name)
            {
                parent = _parent;
            };
            ~InNavDataPort() {};
            
            FRONTEND::NavigationPacket* nav_packet() {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::NavigationPacket retval = this->parent->get_nav_packet(this->name);
                FRONTEND::NavigationPacket* tmpVal = frontend::returnNavigationPacket(retval);
                return tmpVal;
            };
            void nav_packet(const FRONTEND::NavigationPacket &gps) {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::NavigationPacket input = frontend::returnNavigationPacket(gps);
                this->parent->set_nav_packet(this->name, input);
                return;
            };
            std::string getRepid() const {
                return "IDL:FRONTEND/NavData:1.0";
            };
            
        protected:
            nav_delegation *parent;
            boost::mutex portAccess;
    };
    
    // ----------------------------------------------------------------------------------------
    // OutNavDataPort declaration
    // ----------------------------------------------------------------------------------------
    template<typename PortType_var, typename PortType>
    class OutNavDataPortT : public OutFrontendPort<PortType_var,PortType>
    {
        public:
            OutNavDataPortT(std::string port_name) : OutFrontendPort<PortType_var, PortType>(port_name)
            {};
            ~OutNavDataPortT(){};

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
            };
            frontend::NavigationPacket nav_packet() {
                return _get_nav_packet("");
            };
            frontend::NavigationPacket _get_nav_packet(const std::string __connection_id__) {
                frontend::NavigationPacket retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        const FRONTEND::NavigationPacket_var tmp = ((*i).first)->nav_packet();
                        retval = frontend::returnNavigationPacket(tmp);
                    }
                }
                return retval;
            };
            void nav_packet(const frontend::NavigationPacket &nav, const std::string __connection_id__ = "") {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                __evaluateRequestBasedOnConnections(__connection_id__, false, false, false);
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        if (not __connection_id__.empty() and __connection_id__ != i->second)
                            continue;
                        const FRONTEND::NavigationPacket_var tmp = frontend::returnNavigationPacket(nav);
                        ((*i).first)->nav_packet(tmp);
                    }
                }
                return;
            };
    };
    class OutNavDataPort : public OutNavDataPortT<FRONTEND::NavData_var,FRONTEND::NavData> {
        public:
            OutNavDataPort(std::string port_name) : OutNavDataPortT<FRONTEND::NavData_var,FRONTEND::NavData>(port_name)
            {};
    };
    
} // end of frontend namespace


#endif
