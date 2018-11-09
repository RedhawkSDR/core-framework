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
#include <redhawk/FRONTEND/TunerControl.h>

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

    FRONTEND::RFInfoPkt getRFInfoPkt(const RFInfoPkt &val);
    FRONTEND::RFInfoPkt* returnRFInfoPkt(const RFInfoPkt &val);
    RFInfoPkt returnRFInfoPkt(const FRONTEND::RFInfoPkt &tmpVal);
    FRONTEND::GPSInfo* returnGPSInfo(const frontend::GPSInfo &val);
    frontend::GPSInfo returnGPSInfo(const FRONTEND::GPSInfo &tmpVal);
    FRONTEND::GpsTimePos* returnGpsTimePos(const frontend::GpsTimePos &val);
    frontend::GpsTimePos returnGpsTimePos(const FRONTEND::GpsTimePos &tmpVal);
    FRONTEND::NavigationPacket* returnNavigationPacket(const frontend::NavigationPacket &val);
    frontend::NavigationPacket returnNavigationPacket(const FRONTEND::NavigationPacket &tmpVal);
    void copyRFInfoPktSequence(const RFInfoPktSequence &src, FRONTEND::RFInfoPktSequence &dest);
    void copyRFInfoPktSequence(const FRONTEND::RFInfoPktSequence &src, RFInfoPktSequence &dest );
    
    FRONTEND::ScanningTuner::ScanStatus* returnScanStatus(const frontend::ScanStatus &val);
    frontend::ScanStatus returnScanStatus(const FRONTEND::ScanningTuner::ScanStatus &tmpVal);
    FRONTEND::ScanningTuner::ScanStrategy* returnScanStrategy(const frontend::ScanStrategy &val);
    frontend::ScanStrategy* returnScanStrategy(const FRONTEND::ScanningTuner::ScanStrategy &tmpVal);

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

            std::vector<std::string> getConnectionIds()
            {
                std::vector<std::string> retval;
                for (unsigned int i = 0; i < outConnections.size(); i++) {
                    retval.push_back(outConnections[i].second);
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
        
            std::string getRepid() const {
                return PortType::_PD_repoId;
            };

        protected:
            std::vector < std::pair<PortType_var, std::string> > outConnections;
            ExtendedCF::UsesConnectionSequence recConnections;
            bool recConnectionsRefresh;
    };

} // end of frontend namespace


#endif
