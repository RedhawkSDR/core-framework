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

#include <ossie/CorbaIterator.h>

#include "ConnectionManager.h"
#include "DomainManager_impl.h"
#include "Endpoints.h"

typedef ossie::corba::Iterator<CF::ConnectionManager::ConnectionStatusType,
                               CF::ConnectionManager::ConnectionStatusType_out,
                               CF::ConnectionManager::ConnectionStatusSequence,
                               CF::ConnectionManager::ConnectionStatusSequence_out,
                               CF::ConnectionStatusIterator,
                               POA_CF::ConnectionStatusIterator> ConnectionStatusIter;

static CF::ConnectionManager::ConnectionStatusSequence* connectionListToSequence(const ossie::ConnectionList& connections)
{
    CF::ConnectionManager::ConnectionStatusSequence_var result = new CF::ConnectionManager::ConnectionStatusSequence();

    for (ossie::ConnectionList::const_iterator ii = connections.begin(); ii != connections.end(); ++ii) {
        CF::ConnectionManager::ConnectionStatusType status;
        status.usesEndpoint = ii->uses->toEndpointStatusType();
        status.providesEndpoint = ii->provides->toEndpointStatusType();
        status.connectionId = CORBA::string_dup(ii->identifier.c_str());
        status.requesterId = CORBA::string_dup(ii->requesterId.c_str());
        status.connectionRecordId = CORBA::string_dup(ii->connectionRecordId.c_str());
        status.connected = ii->connected;
        ossie::corba::push_back(result, status);
    }

    return result._retn();
}

ConnectionManager_impl::ConnectionManager_impl(DomainManager_impl* domainManager) :
    _domainManager(domainManager)
{
}

ConnectionManager_impl::~ConnectionManager_impl()
{
}

ossie::Endpoint* ConnectionManager_impl::requestToEndpoint(const CF::ConnectionManager::EndpointRequest& request)
{
    ossie::Endpoint* endpoint = 0;
    switch (request.endpoint._d()) {
    case CF::ConnectionManager::ENDPOINT_APPLICATION:
        endpoint = new ossie::ApplicationEndpoint(request.endpoint.applicationId());
        endpoint->setIdentifier(request.endpoint.applicationId());
        break;
    case CF::ConnectionManager::ENDPOINT_DEVICE:
        endpoint = new ossie::ComponentEndpoint(request.endpoint.deviceId());
        endpoint->setIdentifier(request.endpoint.deviceId());
        break;
    case CF::ConnectionManager::ENDPOINT_SERVICE:
        endpoint = new ossie::ServiceEndpoint(request.endpoint.serviceName());
        endpoint->setIdentifier(request.endpoint.serviceName());
        break;
    case CF::ConnectionManager::ENDPOINT_EVENTCHANNEL:
        endpoint = new ossie::EventChannelEndpoint(request.endpoint.channelName());
        endpoint->setIdentifier(request.endpoint.channelName());
        break;
    case CF::ConnectionManager::ENDPOINT_OBJECTREF:
        endpoint = new ossie::ObjectrefEndpoint(request.endpoint.objectRef());
        endpoint->setIdentifier(std::string(""));
        break;
    case CF::ConnectionManager::ENDPOINT_COMPONENT:
        endpoint = new ossie::ComponentEndpoint(request.endpoint.componentId());
        endpoint->setIdentifier(request.endpoint.componentId());
        break;
    case CF::ConnectionManager::ENDPOINT_DOMAINMANAGER:
        endpoint = new ossie::FindByDomainFinderEndpoint("domainmanager","");
        endpoint->setIdentifier(this->_domainManager->_domainName);
        break;
    case CF::ConnectionManager::ENDPOINT_DEVICEMANAGER:
        throw(CF::Port::InvalidPort(1, "Unsupported endpoint. DeviceManager endpoint is not supported"));
    }

    const std::string portName(request.portName);
    if (portName.empty()) {
        return endpoint;
    } else {
        return new ossie::PortEndpoint(endpoint, portName);
    }
}

char* ConnectionManager_impl::connect(const CF::ConnectionManager::EndpointRequest& usesEndpoint, const CF::ConnectionManager::EndpointRequest& providesEndpoint, const char* requesterId, const char* connectionId)
{
    // If no connection ID was given, generate one
    std::string _connectionId = connectionId;
    if (_connectionId.empty()) {
        _connectionId = ossie::generateUUID();
    }

    std::unique_ptr<ossie::Endpoint> uses(requestToEndpoint(usesEndpoint));
    std::unique_ptr<ossie::Endpoint> provides(requestToEndpoint(providesEndpoint));
    ossie::ConnectionNode connection(uses.release(), provides.release(), _connectionId, requesterId, "");
    try {
        connection.connect(_domainManager->_connectionManager);
    } catch (ossie::InvalidConnection &e){
        std::ostringstream err;
        err << "Unable to create a connection: "<<e.what();
        throw (CF::Port::InvalidPort(1, err.str().c_str()));
    }
    std::string connectionRecordId = _domainManager->_connectionManager.restoreConnection(requesterId, connection);
    return CORBA::string_dup(connectionRecordId.c_str());
}

void ConnectionManager_impl::disconnect(const char* connectionRecordId)
{
    try {
        _domainManager->_connectionManager.breakConnection(connectionRecordId);
    } catch ( ossie::InvalidConnection &e) {
        std::ostringstream err;
        err << "Unable to remove a connection: "<<e.what();
        throw (CF::Port::InvalidPort(1, err.str().c_str()));
    }
}

CF::ConnectionManager::ConnectionStatusSequence* ConnectionManager_impl::connections()
{
    CF::ConnectionManager::ConnectionStatusSequence_var result = new CF::ConnectionManager::ConnectionStatusSequence();

    const ossie::ConnectionTable connection_table = _domainManager->_connectionManager.getConnections();
    for (ossie::ConnectionTable::const_iterator ii = connection_table.begin(); ii != connection_table.end(); ++ii) {
        CF::ConnectionManager::ConnectionStatusSequence_var items = connectionListToSequence(ii->second);
        ossie::corba::extend(result, items);
    }
    
    return result._retn();
}

void ConnectionManager_impl::listConnections(CORBA::ULong count, CF::ConnectionManager::ConnectionStatusSequence_out conns, CF::ConnectionStatusIterator_out iter)
{
    iter = ConnectionStatusIter::list(count, conns, connections());
}
