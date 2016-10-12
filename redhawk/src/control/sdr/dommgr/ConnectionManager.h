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

#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include <ossie/CF/cf.h>
#include "connectionSupport.h"

class DomainManager_impl;

class ConnectionManager_impl : public virtual POA_CF::ConnectionManager {
public:
    ConnectionManager_impl(DomainManager_impl* domain);
    virtual ~ConnectionManager_impl();

    virtual char* connect(const CF::ConnectionManager::EndpointRequest& usesEndpoint, const CF::ConnectionManager::EndpointRequest& providesEndpoint, const char* requesterId, const char* connectionId);

    virtual void disconnect(const char* connectionId);

    virtual CF::ConnectionManager::ConnectionStatusSequence* connections();

    virtual void listConnections(CORBA::ULong count, CF::ConnectionManager::ConnectionStatusSequence_out connections, CF::ConnectionStatusIterator_out iter);

private:
    ossie::Endpoint* requestToEndpoint(const CF::ConnectionManager::EndpointRequest& request);

    DomainManager_impl* _domainManager;
};

#endif // CONNECTIONMANAGER_H
