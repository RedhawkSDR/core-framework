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

    FRONTEND::RFInfoPktSequence* OutRFSourcePort::available_rf_inputs()
    {
        FRONTEND::RFInfoPktSequence* retval;
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                try {
                    retval = ((*i).first)->available_rf_inputs();
                } catch(...) {
                }
            }
        }

        return retval;
    }

    void OutRFSourcePort::available_rf_inputs(const FRONTEND::RFInfoPktSequence& data)
    {
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                try {
                    ((*i).first)->available_rf_inputs(data);
                } catch(...) {
                }
            }
        }

        return;
    }

    FRONTEND::RFInfoPkt* OutRFSourcePort::current_rf_input()
    {
        FRONTEND::RFInfoPkt* retval;
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                try {
                    retval = ((*i).first)->current_rf_input();
                } catch(...) {
                }
            }
        }

        return retval;
    }

    void OutRFSourcePort::current_rf_input(const FRONTEND::RFInfoPkt& data)
    {
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in

        if (active) {
            for (i = outConnections.begin(); i != outConnections.end(); ++i) {
                try {
                    ((*i).first)->current_rf_input(data);
                } catch(...) {
                }
            }
        }

        return;
    }
} // end of frontend namespace
