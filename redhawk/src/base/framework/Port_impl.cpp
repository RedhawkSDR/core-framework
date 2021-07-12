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

#if __cplusplus >= 201103L
# define NOEXCEPT noexcept
#else
# define NOEXCEPT throw()
#endif

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

UpstreamRegistrar::UpstreamRegistrar() {
}

UpstreamRegistrar::~UpstreamRegistrar() {
}

void UpstreamRegistrar::setUpstream(const CF::UpstreamTuple &src) {
    for (std::vector<CF::UpstreamTuple>::iterator itr=_upstreams.begin(); itr!=_upstreams.end(); ++itr) {
        if ((itr->upstream->_is_equivalent(src.upstream)) and (itr->port->_is_equivalent(src.port))) {
            return;
        }
    }
    _upstreams.push_back(src);
}

void UpstreamRegistrar::removeUpstream(const CF::UpstreamTuple &src) {
    for (std::vector<CF::UpstreamTuple>::iterator itr=_upstreams.begin(); itr!=_upstreams.end(); ++itr) {
        if ((itr->upstream->_is_equivalent(src.upstream)) and (itr->port->_is_equivalent(src.port))) {
            _upstreams.erase(itr);
        }
    }
}

CF::UpstreamSequence* UpstreamRegistrar::upstreams() {
    CF::UpstreamSequence_var retval = new CF::UpstreamSequence();
    retval->length(_upstreams.size());
    for (unsigned int i=0; i<retval->length(); i++) {
        retval[i] = _upstreams[i];
    }
    return retval._retn();
}

namespace redhawk {

    PortCallError::PortCallError( const std::string &msg, const std::vector<std::string> &connectionids ) :
        std::runtime_error(PortCallError::makeMessage(msg, connectionids)) {}

    PortCallError::~PortCallError() NOEXCEPT {}

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
