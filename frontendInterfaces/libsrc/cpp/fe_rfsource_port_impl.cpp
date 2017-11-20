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
        return _get_available_rf_inputs("");
    }
    RFInfoPktSequence OutRFSourcePort::_get_available_rf_inputs(const std::string __connection_id__)
    {
        FRONTEND::RFInfoPktSequence_var _retval;
        RFInfoPktSequence retval;
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);

        if (active) {
            if (not __connection_id__.empty()) {
                bool found_connection = false;
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    if ((*i).second == __connection_id__) {
                        found_connection = true;
                        _retval = ((*i).first)->available_rf_inputs();
                    }
                }
                if (not found_connection) {
                    throw redhawk::PortCallError("Connection id "+__connection_id__+" not found.", this->getConnectionIds());
                }
            } else {
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                        _retval = ((*i).first)->available_rf_inputs();
                }
            }
            frontend::copyRFInfoPktSequence(_retval.in(), retval);
        }

        return retval;
    }

    void OutRFSourcePort::available_rf_inputs(const RFInfoPktSequence& data, const std::string __connection_id__)
    {
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        __evaluateRequestBasedOnConnections(__connection_id__, false, false, false);

        if (active) {
            FRONTEND::RFInfoPktSequence_var _data = new FRONTEND::RFInfoPktSequence();
            frontend::copyRFInfoPktSequence(data, _data);
            if (not __connection_id__.empty()) {
                bool found_connection = false;
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    if ((*i).second == __connection_id__) {
                        found_connection = true;
                        ((*i).first)->available_rf_inputs(_data);
                    }
                }
                if (not found_connection) {
                    throw redhawk::PortCallError("Connection id "+__connection_id__+" not found.", this->getConnectionIds());
                }
            } else {
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    ((*i).first)->available_rf_inputs(_data);
                }
            }
        }

        return;
    }

    RFInfoPkt *OutRFSourcePort::current_rf_input()
    {
        return _get_current_rf_input("");
    }
    RFInfoPkt *OutRFSourcePort::_get_current_rf_input(const std::string __connection_id__)
    {
        FRONTEND::RFInfoPkt_var _retval;
        RFInfoPkt *retval = 0;
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        __evaluateRequestBasedOnConnections(__connection_id__, true, false, false);

        if (active) {
            if (not __connection_id__.empty()) {
                bool found_connection = false;
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    if ((*i).second == __connection_id__) {
                        found_connection = true;
                        _retval = ((*i).first)->current_rf_input();
                    }
                }
                if (not found_connection) {
                    throw redhawk::PortCallError("Connection id "+__connection_id__+" not found.", this->getConnectionIds());
                }
            } else {
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    _retval = ((*i).first)->current_rf_input();
                }
            }

            RFInfoPkt __retval;
            __retval = frontend::returnRFInfoPkt(_retval);
            retval= new RFInfoPkt(__retval);
        }

        return retval;
    }

    void OutRFSourcePort::current_rf_input(const RFInfoPkt& data, const std::string __connection_id__)
    {
        std::vector < std::pair < FRONTEND::RFSource_var, std::string > >::iterator i;

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        __evaluateRequestBasedOnConnections(__connection_id__, false, false, false);

        if (active) {
            FRONTEND::RFInfoPkt_var _data = frontend::returnRFInfoPkt(data);
            if (not __connection_id__.empty()) {
                bool found_connection = false;
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    if ((*i).second == __connection_id__) {
                        found_connection = true;
                        ((*i).first)->current_rf_input(_data);
                    }
                }
                if (not found_connection) {
                    throw redhawk::PortCallError("Connection id "+__connection_id__+" not found.", this->getConnectionIds());
                }
            } else {
                for (i = this->outConnections.begin(); i != this->outConnections.end(); ++i) {
                    ((*i).first)->current_rf_input(_data);
                }
            }
        }

        return;
    }

} // end of frontend namespace
