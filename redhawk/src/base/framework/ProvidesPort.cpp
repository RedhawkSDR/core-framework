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

    ProvidesTransport::ProvidesTransport(NegotiableProvidesPortBase* port, const std::string& transportId) :
        _port(port),
        _transportId(transportId)
    {
    }

    const std::string& ProvidesTransport::transportId() const
    {
        return _transportId;
    }


    NegotiableProvidesPortBase::NegotiableProvidesPortBase(const std::string& name) :
        Port_Provides_base_impl(name)
    {
    }
    
    NegotiableProvidesPortBase::~NegotiableProvidesPortBase()
    {
    }
     
    void NegotiableProvidesPortBase::initializePort()
    {
        const std::string repid = getRepid();
        TransportStack* transports = TransportRegistry::GetTransports(repid);
        if (!transports) {
            // No registered transports for this port type
            return;
        }

        for (TransportStack::iterator iter = transports->begin(); iter != transports->end(); ++iter) {
            TransportFactory* transport = *iter;
            RH_NL_INFO("NegotiableProvidesPortBase", "Adding provides transport '" << transport->transportType()
                       << "' for '" << repid << "'");
            _addTransportManager(transport->transportType(), transport->createProvidesManager(this));
        }
    }

    void NegotiableProvidesPortBase::releasePort()
    {
        for (TransportMap::iterator transport = _transports.begin(); transport != _transports.end(); ++transport) {
            transport->second->stopTransport();
        }
    }

    ExtendedCF::TransportInfoSequence* NegotiableProvidesPortBase::supportedTransports()
    {
        ExtendedCF::TransportInfoSequence_var transports = new ExtendedCF::TransportInfoSequence;
        for (TransportManagerMap::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            ExtendedCF::TransportInfo transport;
            transport.transportName = manager->first.c_str();
            transport.transportProperties = manager->second->transportProperties();
            ossie::corba::push_back(transports, transport);
        }
        return transports._retn();
    }

    ExtendedCF::NegotiationResult* NegotiableProvidesPortBase::negotiateTransport(const char* protocol, const CF::Properties& props)
    {
        boost::mutex::scoped_lock lock(_transportMutex);
        ProvidesTransportManager* manager = _getTransportManager(protocol);
        if (!manager) {
            std::string message = "Cannot negotiate protocol '" + std::string(protocol) + "'";
            throw ExtendedCF::NegotiationError(message.c_str());
        }

        std::string transport_id = ossie::generateUUID();
        const redhawk::PropertyMap& transport_props = redhawk::PropertyMap::cast(props);
        ProvidesTransport* transport = manager->createProvidesTransport(transport_id, transport_props);
        transport->startTransport();

        _transports[transport_id] = transport;

        ExtendedCF::NegotiationResult_var result = new ExtendedCF::NegotiationResult;
        result->negotiationId = transport_id.c_str();
        return result._retn();
    }

    void NegotiableProvidesPortBase::disconnectTransport(const char* connectionId)
    {
        boost::mutex::scoped_lock lock(_transportMutex);
        TransportMap::iterator transport = _transports.find(connectionId);
        if (transport == _transports.end()) {
            // TODO: throw exception
            return;
        }
        transport->second->stopTransport();
        delete transport->second;
        _transports.erase(transport);
    }

    ProvidesTransportManager* NegotiableProvidesPortBase::_getTransportManager(const std::string& protocol)
    {
        TransportManagerMap::iterator manager = _transportManagers.find(protocol);
        if (manager == _transportManagers.end()) {
            return 0;
        }
        return manager->second;
    }

    void NegotiableProvidesPortBase::_addTransportManager(const std::string& name, ProvidesTransportManager* manager)
    {
        _transportManagers[name] = manager;
    }
}
