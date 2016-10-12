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
            
            frontend::NavigationPacket nav_packet() {
                frontend::NavigationPacket retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        try {
                            const FRONTEND::NavigationPacket_var tmp = ((*i).first)->nav_packet();
                            retval = frontend::returnNavigationPacket(tmp);
                        } catch(...) {
                        }
                    }
                }
                return retval;
            };
            void nav_packet(const frontend::NavigationPacket &nav) {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        try {
                            const FRONTEND::NavigationPacket tmp = frontend::returnNavigationPacket(nav);
                            ((*i).first)->nav_packet(tmp);
                        } catch(...) {
                        }
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
