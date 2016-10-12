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
#ifndef FE_RFINFO_PORT_H
#define FE_RFINFO_PORT_H

#include "fe_port_impl.h"

#include <redhawk/FRONTEND/RFInfo.h>
#include "fe_types.h"

namespace frontend {
    
    class rfinfo_delegation {
        public:
            virtual std::string get_rf_flow_id(const std::string& port_name) {
                return std::string("none");
            }
            virtual void set_rf_flow_id(const std::string& port_name, const std::string& id) {
            }
            virtual RFInfoPkt get_rfinfo_pkt(const std::string& port_name) {
                return RFInfoPkt();
            }
            virtual void set_rfinfo_pkt(const std::string& port_name, const RFInfoPkt &pkt) {
            }
    };
    
    
    class InRFInfoPort : public POA_FRONTEND::RFInfo, public Port_Provides_base_impl
    {
        public:
            InRFInfoPort(std::string port_name, rfinfo_delegation *_parent) : 
            Port_Provides_base_impl(port_name)
            {
                parent = _parent;
            };
            ~InRFInfoPort() {};
            
            char* rf_flow_id() {
                boost::mutex::scoped_lock lock(portAccess);
                return (CORBA::string_dup(this->parent->get_rf_flow_id(this->name).c_str()));
            };
            void rf_flow_id(const char *id) {
                boost::mutex::scoped_lock lock(portAccess);
                std::string _id(id);
                this->parent->set_rf_flow_id(this->name, _id);
                return;
            };
            FRONTEND::RFInfoPkt* rfinfo_pkt() {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::RFInfoPkt retval = this->parent->get_rfinfo_pkt(this->name);
                FRONTEND::RFInfoPkt* tmpVal = frontend::returnRFInfoPkt(retval);
                return tmpVal;
            };
            void rfinfo_pkt(const FRONTEND::RFInfoPkt& data) {
                boost::mutex::scoped_lock lock(portAccess);
                frontend::RFInfoPkt input = frontend::returnRFInfoPkt(data);
                this->parent->set_rfinfo_pkt(this->name, input);
                return;
            };
            std::string getRepid() const {
                return "IDL:FRONTEND/RFInfo:1.0";
            };
            
        protected:
            rfinfo_delegation *parent;
            boost::mutex portAccess;
    };

    // ----------------------------------------------------------------------------------------
    // OutRFInfoPort declaration
    // ----------------------------------------------------------------------------------------
    template<typename PortType_var, typename PortType>
    class OutRFInfoPortT : public OutFrontendPort<PortType_var,PortType>
    {
        public:
            OutRFInfoPortT(std::string port_name) : OutFrontendPort<PortType_var, PortType>(port_name)
            {};
            ~OutRFInfoPortT(){};
            
            std::string rf_flow_id() {
                CORBA::String_var retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        retval = ((*i).first)->rf_flow_id();
                    }
                }
                std::string str_retval = ossie::corba::returnString(retval);
                return str_retval;
            };
            void rf_flow_id(std::string &data) {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        ((*i).first)->rf_flow_id(data.c_str());
                    }
                }
                return;
            };
            frontend::RFInfoPkt rfinfo_pkt() {
                frontend::RFInfoPkt retval;
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        const FRONTEND::RFInfoPkt_var tmp = ((*i).first)->rfinfo_pkt();
                        retval = frontend::returnRFInfoPkt(tmp);
                    }
                }
                return retval;
            };
            void rfinfo_pkt(frontend::RFInfoPkt data) {
                typename std::vector < std::pair < PortType_var, std::string > >::iterator i;
                boost::mutex::scoped_lock lock(this->updatingPortsLock);   // don't want to process while command information is coming in
                if (this->active) {
                    for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        const FRONTEND::RFInfoPkt_var tmp = frontend::returnRFInfoPkt(data);
                        ((*i).first)->rfinfo_pkt(tmp);
                    }
                }
                return;
            };
    };
    class OutRFInfoPort : public OutRFInfoPortT<FRONTEND::RFInfo_var,FRONTEND::RFInfo> {
        public:
            OutRFInfoPort(std::string port_name) : OutRFInfoPortT<FRONTEND::RFInfo_var,FRONTEND::RFInfo>(port_name)
            {};
    };
    
} // end of frontend namespace


#endif
