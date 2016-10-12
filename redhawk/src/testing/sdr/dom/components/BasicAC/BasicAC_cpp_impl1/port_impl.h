/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file 
 * distributed with this source distribution.
 * 
 * This file is part of REDHAWK core.
 * 
 * REDHAWK core is free software: you can redistribute it and/or modify it 
 * under the terms of the GNU Lesser General Public License as published by the 
 * Free Software Foundation, either version 3 of the License, or (at your 
 * option) any later version.
 * 
 * REDHAWK core is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License 
 * for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License 
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

 
#ifndef PORT_H
#define PORT_H

#include "ossie/Port_impl.h"
#include <queue>
#include <list>

class BasicAC_cpp_impl1_base;
class BasicAC_cpp_impl1_i;

#define CORBA_MAX_TRANSFER_BYTES omniORB::giopMaxMsgSize()

#include "ossie/CF/QueryablePort.h"
 
#include "ossie/CF/cf.h"
 

// ----------------------------------------------------------------------------------------
// CF_Resource_Out_i declaration
// ----------------------------------------------------------------------------------------
class CF_Resource_Out_i : public Port_Uses_base_impl, public POA_ExtendedCF::QueryablePort
{
    public:
        CF_Resource_Out_i(std::string port_name, BasicAC_cpp_impl1_base *_parent);
        ~CF_Resource_Out_i();

        void initialize();

        void releaseObject();

        void runTest(unsigned long testid, CF::Properties& testValues);

        void configure(const CF::Properties& configProperties);

        void query(CF::Properties& configProperties);

        CORBA::Object_ptr getPort(const char* name);

        void start();

        void stop();
        std::string identifier();

        CORBA::Boolean started();


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
            CF::Resource_var port = CF::Resource::_narrow(connection);
            outConnections.push_back(std::make_pair(port, connectionId));
            active = true;
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
        };

        std::vector< std::pair<CF::Resource_var, std::string> > _getConnections()
        {
            return outConnections;
        };

    private:
        BasicAC_cpp_impl1_i *parent;
        std::vector < std::pair<CF::Resource_var, std::string> > outConnections;
        ExtendedCF::UsesConnectionSequence recConnections;
        bool recConnectionsRefresh;
};
#endif
