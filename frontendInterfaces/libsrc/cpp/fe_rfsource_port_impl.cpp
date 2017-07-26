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

#include "fe_rfsource_port_impl.h"

namespace frontend {

    // ----------------------------------------------------------------------------------------
    // OutRFSourcePort definition
    // ----------------------------------------------------------------------------------------
    OutRFSourcePort::OutRFSourcePort(std::string port_name) :
        OutFrontendPort<FRONTEND::RFSource_var,FRONTEND::RFSource>::OutFrontendPort(port_name)
    {
    }

    OutRFSourcePort::~OutRFSourcePort()
    {
    }

    RFInfoPktSequence OutRFSourcePort::available_rf_inputs()
    {
        FRONTEND::RFInfoPktSequence_var _retval;
        RFInfoPktSequence retval;
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                try {
                    _retval = ((*i).first)->available_rf_inputs();
                }catch(...){
                    // should handle errors
                }
            }
            frontend::copyRFInfoPktSequence(_retval.in(), retval);
        }

        return retval;
    }

    void OutRFSourcePort::available_rf_inputs(const RFInfoPktSequence& data)
    {
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            FRONTEND::RFInfoPktSequence_var _data = new FRONTEND::RFInfoPktSequence();
            frontend::copyRFInfoPktSequence(data, _data.out());
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                ((*i).first)->available_rf_inputs(_data);
            }
        }

        return;
    }

    RFInfoPkt *OutRFSourcePort::current_rf_input()
    {
        FRONTEND::RFInfoPkt_var _retval;
        RFInfoPkt *retval = 0;
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                _retval = ((*i).first)->current_rf_input();
            }
            
            RFInfoPkt __retval;
            __retval = frontend::returnRFInfoPkt(_retval);
            retval= new RFInfoPkt(__retval);
        }

        return retval;
    }

    void OutRFSourcePort::current_rf_input(const RFInfoPkt& data)
    {
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            FRONTEND::RFInfoPkt_var _data = frontend::returnRFInfoPkt(data);
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                ((*i).first)->current_rf_input(_data);
            }
        }

        return;
    }

} // end of frontend namespace
