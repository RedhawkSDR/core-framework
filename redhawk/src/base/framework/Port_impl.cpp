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


#include "ossie/Port_impl.h"

Port_impl::Port_impl()
{
}

Port_impl::~Port_impl()
{
}

void Port_impl::connectPort(CORBA::Object_ptr connection, const char* connectionId)
{
}

void Port_impl::disconnectPort(const char* connectionId)
{
}

LOGGER PortBase::getLogger()
{
    return _portLog;
}

void PortBase::setLogger(LOGGER newLogger)
{
    _portLog = newLogger;
}

namespace redhawk {

    PortCallError::PortCallError( const std::string &msg, const std::vector<std::string> &connectionids ) :
        std::runtime_error(PortCallError::makeMessage(msg, connectionids)) {}

    PortCallError::~PortCallError() throw () {}

    std::string PortCallError::makeMessage(const std::string& msg, const std::vector<std::string>& connectionids) {
        std::ostringstream cnvt;
        cnvt.str("");
        cnvt << msg;
        if (not connectionids.empty()) {
            cnvt << "Connections available: ";
            for (std::vector<std::string>::const_iterator connectionid=connectionids.begin(); connectionid!=connectionids.end(); connectionid++) {
                cnvt << *connectionid;
                if (connectionid!=connectionids.end()-1)
                    cnvt << ", ";
            }
        }
        return cnvt.str();
    }
}
