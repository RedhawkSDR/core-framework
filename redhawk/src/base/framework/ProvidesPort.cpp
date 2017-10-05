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

#include <ossie/ProvidesPort.h>
#include <ossie/PropertyMap.h>
#include <ossie/ossieSupport.h>

namespace redhawk {

    NegotiableProvidesPortBase::NegotiableProvidesPortBase(const std::string& name) :
        Port_Provides_base_impl(name),
        _transportMapInitialized(false)
    {
    }
    
    NegotiableProvidesPortBase::~NegotiableProvidesPortBase()
    {
    }
     
    CF::Properties* NegotiableProvidesPortBase::supportedTransports()
    {
        _initializeTransportMap();

        redhawk::PropertyMap transports;
        for (TransportManagerMap::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            transports[manager->first] = manager->second->transportProperties();
        }
        return new CF::Properties(transports);
    }

    ExtendedCF::NegotiationResult* NegotiableProvidesPortBase::negotiateTransport(const char* protocol, const CF::Properties& props)
    {
        _initializeTransportMap();

        InputTransportManager* manager = _getTransportManager(protocol);
        if (!manager) {
            std::string message = "Cannot negotiate protocol '" + std::string(protocol) + "'";
            throw ExtendedCF::NegotiationError(message.c_str());
        }

        InputTransport* transport = manager->createInput(props);
        transport->start();

        std::string negotiationId = ossie::generateUUID();
        _inputs[negotiationId] = transport;

        ExtendedCF::NegotiationResult_var result = new ExtendedCF::NegotiationResult;
        result->negotiationId = negotiationId.c_str();
        return result._retn();
    }

    void NegotiableProvidesPortBase::disconnectTransport(const char* connectionId)
    {
    }

    InputTransportManager* NegotiableProvidesPortBase::_getTransportManager(const std::string& protocol)
    {
        TransportManagerMap::iterator manager = _transportManagers.find(protocol);
        if (manager == _transportManagers.end()) {
            return 0;
        }
        return manager->second;
    }

    void NegotiableProvidesPortBase::_addTransportManager(const std::string& name, InputTransportManager* manager)
    {
        _transportManagers[name] = manager;
    }

    void NegotiableProvidesPortBase::_initializeTransportMap()
    {
        if (!_transportMapInitialized) {
            _initializeTransports();
            _transportMapInitialized = true;
        }
    }
}
