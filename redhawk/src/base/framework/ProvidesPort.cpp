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
        for (TransportMap::iterator transport = _transports.begin(); transport != _transports.end(); ++transport) {
            delete transport->second;
        }

        for (TransportManagerList::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            delete (*manager);
        }
    }

    void NegotiableProvidesPortBase::initializePort()
    {
        const std::string repo_id = getRepid();
        TransportStack transports = TransportRegistry::GetTransports(repo_id);
        for (TransportStack::iterator iter = transports.begin(); iter != transports.end(); ++iter) {
            TransportFactory* transport = *iter;
            RH_DEBUG(_portLog, "Adding provides transport '" << transport->transportType()
                     << "' for '" << repo_id << "'");
            _transportManagers.push_back(transport->createProvidesManager(this));
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
        for (TransportManagerList::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            ExtendedCF::TransportInfo transport;
            transport.transportType = (*manager)->transportType().c_str();
            transport.transportProperties = (*manager)->transportProperties();
            ossie::corba::push_back(transports, transport);
        }
        return transports._retn();
    }

    ExtendedCF::NegotiationResult*
    NegotiableProvidesPortBase::negotiateTransport(const char* transportType,
                                                   const CF::Properties& transportProperties)
    {
        boost::mutex::scoped_lock lock(_transportMutex);

        // Find the appropriate transport manager; the caller should have
        // checked this port's supported transport types already, but it's
        // still technically possible to give us a bad type.
        ProvidesTransportManager* manager = _getTransportManager(transportType);
        if (!manager) {
            std::string message = "cannot negotiate transport type '" + std::string(transportType) + "'";
            throw ExtendedCF::NegotiationError(message.c_str());
        }

        // Create a unique identifier for this transport instance, that should
        // be used later for disconnect
        std::string transport_id = ossie::generateUUID();
        const redhawk::PropertyMap& transport_props = redhawk::PropertyMap::cast(transportProperties);

        // Ask the manager to create a transport instance; if for any reason it
        // doesn't want to (e.g., bad properties), it should throw an exception
        // (preferably some form of TransportError).
        ProvidesTransport* transport;
        try {
            transport = manager->createProvidesTransport(transport_id, transport_props);
        } catch (const std::exception& exc) {
            throw ExtendedCF::NegotiationError(exc.what());
        }

        // An exception is preferred, but handle null as well.
        if (!transport) {
            std::string message = "cannot create provides transport type '" + std::string(transportType) + "'";
            throw ExtendedCF::NegotiationError(message.c_str());
        }

        // Attempt to start the transport instance. This should be unlikely to
        // fail, but the transport still has the option to throw an exception;
        // if it does, make sure to delete the transport.
        try {
            transport->startTransport();
        } catch (const std::exception& exc) {
            delete transport;
            throw ExtendedCF::NegotiationError(exc.what());
        } catch (...) {
            RH_ERROR(_portLog, "Unexpected error starting transport type '" << transportType << "'");
            delete transport;
            throw ExtendedCF::NegotiationError("failed to start transport");
        }

        // Take ownership of the transport instance, and return the results of
        // negotiation back to the caller. If the uses side rejects the results
        // (again, should be unlikely at this point), it's responsible for
        // breaking the connection by calling disconnectTransport().
        _transports[transport_id] = transport;
        ExtendedCF::NegotiationResult_var result = new ExtendedCF::NegotiationResult;
        result->transportId = transport_id.c_str();
        result->properties = manager->getNegotiationProperties(transport);
        return result._retn();
    }

    void NegotiableProvidesPortBase::disconnectTransport(const char* transportId)
    {
        boost::mutex::scoped_lock lock(_transportMutex);

        // Make sure it's a valid transport ID
        TransportMap::iterator transport = _transports.find(transportId);
        if (transport == _transports.end()) {
            throw ExtendedCF::NegotiationError("invalid transport identifier");
        }

        // Stop the transport, logging exceptions but continuing on
        try {
            transport->second->stopTransport();
        } catch (const std::exception& exc) {
            RH_ERROR(_portLog, "Error stopping transport '" << transportId << "': " << exc.what());
        } catch (...) {
            RH_ERROR(_portLog, "Unknown error stopping transport '" << transportId << "'");
        }

        // Finish cleaning up
        delete transport->second;
        _transports.erase(transport);
    }

    ProvidesTransportManager* NegotiableProvidesPortBase::_getTransportManager(const std::string& transportType)
    {
        for (TransportManagerList::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            if ((*manager)->transportType() == transportType) {
                return (*manager);
            }
        }
        return 0;
    }
}
