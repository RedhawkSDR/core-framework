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
#ifndef FE_RFSOURCE_PORT_H
#define FE_RFSOURCE_PORT_H

#include "fe_port_impl.h"

#include <redhawk/FRONTEND/RFInfo.h>
#include "fe_types.h"

namespace frontend {
    
    class rfsource_delegation {
        public:
            virtual std::vector<RFInfoPkt> get_available_rf_inputs(const std::string& port_name) {
                return std::vector<RFInfoPkt>();
            }
            virtual void set_available_rf_inputs(const std::string& port_name, std::vector<RFInfoPkt> &inputs) {
            }
            virtual RFInfoPkt get_current_rf_input(const std::string& port_name) {
                return RFInfoPkt();
            }
            virtual void set_current_rf_input(const std::string& port_name, const RFInfoPkt &input) {
            }
    };
    
    class InRFSourcePort : public POA_FRONTEND::RFSource, public Port_Provides_base_impl
    {
        public:
            InRFSourcePort(std::string port_name, rfsource_delegation *_parent) : 
            Port_Provides_base_impl(port_name)
            {
                parent = _parent;
            };
            ~InRFSourcePort() {};
            
            FRONTEND::RFInfoPktSequence* available_rf_inputs() {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::vector<frontend::RFInfoPkt> retval = this->parent->get_available_rf_inputs(this->name);
                FRONTEND::RFInfoPktSequence* tmpVal = new FRONTEND::RFInfoPktSequence();
                std::vector<frontend::RFInfoPkt>::iterator itr = retval.begin();
                while (itr != retval.end()) {
                    FRONTEND::RFInfoPkt_var tmp = frontend::returnRFInfoPkt((*itr));
                    tmpVal->length(tmpVal->length()+1);
                    (*tmpVal)[tmpVal->length()-1] = tmp;
                    itr++;
                }
                return tmpVal;
            };
            void available_rf_inputs( const FRONTEND::RFInfoPktSequence& data) {
                boost::mutex::scoped_lock lock(this->portAccess);
                std::vector<frontend::RFInfoPkt> inputs;
                inputs.resize(data.length());
                for (unsigned int i=0; i<inputs.size(); i++) {
                    inputs[i] = frontend::returnRFInfoPkt(data[i]);
                }
                this->parent->set_available_rf_inputs(this->name, inputs);
            };
            FRONTEND::RFInfoPkt* current_rf_input() {
                boost::mutex::scoped_lock lock(this->portAccess);
                frontend::RFInfoPkt retval = this->parent->get_current_rf_input(this->name);
                FRONTEND::RFInfoPkt* tmpVal = frontend::returnRFInfoPkt(retval);
                return tmpVal;
            };
            void current_rf_input( const FRONTEND::RFInfoPkt& data) {
                boost::mutex::scoped_lock lock(this->portAccess);
                frontend::RFInfoPkt input = frontend::returnRFInfoPkt(data);
                this->parent->set_current_rf_input(this->name, input);
            };
            std::string getRepid() const
            {
                return "IDL:FRONTEND/RFSource:1.0";
            };
            
        protected:
            rfsource_delegation *parent;
            boost::mutex portAccess;
    };
    // ----------------------------------------------------------------------------------------
    // OutRFSourcePort declaration
    // ----------------------------------------------------------------------------------------
    class OutRFSourcePort : public OutFrontendPort<FRONTEND::RFSource_var,FRONTEND::RFSource>
    {
        public:
            OutRFSourcePort(std::string port_name);
            ~OutRFSourcePort();

            FRONTEND::RFInfoPktSequence* available_rf_inputs();
            void available_rf_inputs(const FRONTEND::RFInfoPktSequence& data);
            FRONTEND::RFInfoPkt* current_rf_input();
            void current_rf_input(const FRONTEND::RFInfoPkt& data);;
    };

} // end of frontend namespace

#endif
