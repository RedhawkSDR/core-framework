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

#include <ossie/UsesPort.h>
#include <ossie/CorbaUtils.h>
#include <ossie/CorbaSequence.h>
#include <ossie/PropertyMap.h>

namespace redhawk {
    
    UsesTransport::UsesTransport(UsesPort* port) :
        _port(port),
        _alive(true)
    {
    }

    std::string UsesTransport::getDescription() const
    {
        return "basic transport";
    }

    bool UsesTransport::isAlive() const
    {
        return _alive;
    }

    void UsesTransport::setAlive(bool alive)
    {
        _alive = alive;
    }


    UsesPort::Connection::Connection(const std::string& connectionId, CORBA::Object_ptr objref,
                                     UsesTransport* transport):
        connectionId(connectionId),
        objref(CORBA::Object::_duplicate(objref)),
        transport(transport)
    {
    }

    UsesPort::Connection::~Connection()
    {
        delete transport;
    }

    void UsesPort::Connection::disconnected()
    {
        transport->disconnect();
    }


    UsesPort::UsesPort(const std::string& name) :
        Port_Uses_base_impl(name)
    {
    }

    UsesPort::~UsesPort()
    {
        for (ConnectionList::iterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
            delete *connection;
        }
    }

    void UsesPort::connectPort(CORBA::Object_ptr object, const char* connectionId)
    {
        RH_TRACE_ENTER(logger);

        // Give a specific exception message for nil
        if (CORBA::is_nil(object)) {
            throw CF::Port::InvalidPort(1, "Nil object reference");
        }

        // Attempt to check the type of the remote object to reject invalid
        // types; note this does not require the lock
        _validatePort(object);

        const std::string connection_id(connectionId);
        {
            // Acquire the state lock before modifying the container
            boost::mutex::scoped_lock lock(updatingPortsLock);

            ConnectionList::iterator entry = _findConnection(connection_id);
            if (entry == _connections.end()) {
                Connection* connection = _createConnection(object, connection_id);
                _connections.push_back(connection);

                RH_DEBUG(logger, "Using " << connection->transport->getDescription()
                         << " for connection '" << connection_id << "'");
            } else {
                // TODO: Replace the object reference, or throw an exception?
            }

            active = true;
        }

        _portConnected(connectionId);
        RH_TRACE_EXIT(logger);
    }

    void UsesPort::disconnectPort(const char* connectionId)
    {
        RH_TRACE_ENTER(logger);
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);

            ConnectionList::iterator connection = _findConnection(connectionId);
            if (connection == _connections.end()) {
                std::string message = std::string("No connection ") + connectionId;
                throw CF::Port::InvalidPort(2, message.c_str());
            }

            RH_DEBUG(logger, "Disconnecting connection '" << connectionId << "'");
            UsesTransport* transport = (*connection)->transport;
            try {
                (*connection)->disconnected();
            } catch (const std::exception& exc) {
                if (transport->isAlive()) {
                    RH_WARN(logger, "Exception disconnecting '" << connectionId << "': "
                            << exc.what());
                }
            } catch (const CORBA::Exception& exc) {
                if (transport->isAlive()) {
                    RH_WARN(logger, "Exception disconnecting '" << connectionId << "': "
                            << ossie::corba::describeException(exc));
                }
            }

            delete (*connection);
            _connections.erase(connection);

            if (_connections.empty()) {
                active = false;
            }
        }

        _portDisconnected(connectionId);
        RH_TRACE_EXIT(logger);
    }

    ExtendedCF::UsesConnectionSequence* UsesPort::connections()
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence();
        for (ConnectionList::iterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
            ExtendedCF::UsesConnection conn;
            conn.connectionId = (*connection)->connectionId.c_str();
            conn.port = CORBA::Object::_duplicate((*connection)->objref);
            ossie::corba::push_back(retVal, conn);
        }
        return retVal._retn();
    }

    void UsesPort::setLogger(LOGGER newLogger)
    {
        logger = newLogger;
    }

    void UsesPort::_validatePort(CORBA::Object_ptr object)
    {
        const std::string rep_id = getRepid();
        bool valid;
        try {
            valid = object->_is_a(rep_id.c_str());
        } catch (...) {
            // If _is_a throws an exception, assume the remote object is
            // unreachable (e.g., dead)
            throw CF::Port::InvalidPort(1, "Object unreachable");
        }

        if (!valid) {
            std::string message = "Object does not support " + rep_id;
            throw CF::Port::InvalidPort(1, message.c_str());
        }
    }

    UsesPort::ConnectionList::iterator UsesPort::_findConnection(const std::string& connectionId)
    {
        ConnectionList::iterator entry = _connections.begin();
        for (; entry != _connections.end(); ++entry) {
            if ((*entry)->connectionId == connectionId) {
                return entry;
            }
        }
        return entry;
    }

    UsesPort::Connection* UsesPort::_createConnection(CORBA::Object_ptr object, const std::string& connectionId)
    {
        UsesTransport* transport = _createTransport(object, connectionId);
        return new Connection(connectionId, object, transport);
    }


    NegotiableUsesPort::NegotiableUsesPort(const std::string& name) :
        UsesPort(name)
    {
    }

    NegotiableUsesPort::~NegotiableUsesPort()
    {
        for (TransportManagerList::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            delete *manager;
        }
    }

    void NegotiableUsesPort::initializePort()
    {
        const std::string repo_id = getRepid();
        TransportStack transports = TransportRegistry::GetTransports(repo_id);
        for (TransportStack::iterator iter = transports.begin(); iter != transports.end(); ++iter) {
            TransportFactory* transport = *iter;
            RH_DEBUG(logger, "Adding uses transport '" << transport->transportType()
                     << "' for '" << repo_id << "'");
            _transportManagers.push_back(transport->createUsesManager(this));
        }
    }

    ExtendedCF::TransportInfoSequence* NegotiableUsesPort::supportedTransports()
    {
        ExtendedCF::TransportInfoSequence_var transports = new ExtendedCF::TransportInfoSequence;
        for (TransportManagerList::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            ExtendedCF::TransportInfo info;
            info.transportType = (*manager)->transportType().c_str();
            info.transportProperties = (*manager)->transportProperties();
            ossie::corba::push_back(transports, info);
        }
        return transports._retn();
    }

    ExtendedCF::ConnectionStatusSequence* NegotiableUsesPort::connectionStatus()
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        ExtendedCF::ConnectionStatusSequence_var retVal = new ExtendedCF::ConnectionStatusSequence();
        for (ConnectionList::iterator connection = _connections.begin(); connection != _connections.end(); ++connection) {
            ExtendedCF::ConnectionStatus status;
            status.connectionId = (*connection)->connectionId.c_str();
            status.port = CORBA::Object::_duplicate((*connection)->objref);
            UsesTransport* transport = (*connection)->transport;
            status.alive = transport->isAlive();
            status.transportType = transport->transportType().c_str();
            status.transportInfo = transport->transportInfo();
            ossie::corba::push_back(retVal, status);
        }
        return retVal._retn();
    }

    class NegotiableUsesPort::NegotiatedConnection : public UsesPort::Connection
    {
    public:
        NegotiatedConnection(const std::string& connectionId, ExtendedCF::NegotiableProvidesPort_ptr negotiablePort,
                             const std::string& transportId, UsesTransport* transport) :
            Connection(connectionId, negotiablePort, transport),
            negotiablePort(ExtendedCF::NegotiableProvidesPort::_duplicate(negotiablePort)),
            transportId(transportId)
        {
        }

        virtual void disconnected()
        {
            Connection::disconnected();

            if (!CORBA::is_nil(negotiablePort) && !transportId.empty()) {
                try {
                    negotiablePort->disconnectTransport(transportId.c_str());
                } catch (const CORBA::Exception& exc) {
                    // Ignore 
                } 
            }
        }

        ExtendedCF::NegotiableProvidesPort_var negotiablePort;
        std::string transportId;
    };

    UsesPort::Connection* NegotiableUsesPort::_createConnection(CORBA::Object_ptr object,
                                                                const std::string& connectionId)
    {
        PortBase* local_port = ossie::corba::getLocalServant<PortBase>(object);
        if (local_port) {
            UsesTransport* transport = _createLocalTransport(local_port, object, connectionId);
            if (transport) {
                return new Connection(connectionId, object, transport);
            }
        }

        ExtendedCF::NegotiableProvidesPort_var negotiable_port = ossie::corba::_narrowSafe<ExtendedCF::NegotiableProvidesPort>(object);
        if (!CORBA::is_nil(negotiable_port)) {
            Connection* connection = _negotiateConnection(negotiable_port, connectionId);
            if (connection) {
                return connection;
            }
        }

        UsesTransport* transport = _createTransport(object, connectionId);
        return new Connection(connectionId, object, transport);
    }

    UsesTransport* NegotiableUsesPort::_createLocalTransport(PortBase*, CORBA::Object_ptr, const std::string&)
    {
        return 0;
    }

    static const redhawk::PropertyMap* findTransportProperties(const ExtendedCF::TransportInfoSequence& transports,
                                                               const std::string& transportType)
    {
        for (CORBA::ULong index = 0; index < transports.length(); ++index) {
            if (transportType == static_cast<const char*>(transports[index].transportType)) {
                return &redhawk::PropertyMap::cast(transports[index].transportProperties);
            }
        }
        return 0;
    }

    NegotiableUsesPort::NegotiatedConnection*
    NegotiableUsesPort::_negotiateConnection(ExtendedCF::NegotiableProvidesPort_ptr negotiablePort,
                                             const std::string& connectionId)
    {
        RH_DEBUG(logger, "Trying to negotiate transport for connection '" << connectionId << "'");
        ExtendedCF::TransportInfoSequence_var supported_transports;
        try {
            supported_transports = negotiablePort->supportedTransports();
        } catch (const CORBA::Exception& exc) {
            // Can't negotiate with an inaccessible object
            return 0;
        }

        for (TransportManagerList::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            const std::string transport_type = (*manager)->transportType();
            const redhawk::PropertyMap* transport_props = findTransportProperties(supported_transports, transport_type);
            if (transport_props) {
                RH_DEBUG(logger, "Trying to negotiate transport '" << transport_type
                         << "' for connection '" << connectionId << "'");
                NegotiatedConnection* connection = _negotiateTransport(negotiablePort, connectionId, *manager, *transport_props);
                if (connection) {
                    return connection;
                }
            } else {
                RH_DEBUG(logger, "Provides side for connection '" << connectionId
                         << "' does not support transport '" << transport_type << "'");
            }
        }
        return 0;
    }

    NegotiableUsesPort::NegotiatedConnection*
    NegotiableUsesPort::_negotiateTransport(ExtendedCF::NegotiableProvidesPort_ptr negotiablePort,
                                            const std::string& connectionId,
                                            UsesTransportManager* manager,
                                            const redhawk::PropertyMap& properties)
    {
        UsesTransport* transport = manager->createUsesTransport(negotiablePort, connectionId, properties);
        if (!transport) {
            return 0;
        }

        const std::string transport_type = manager->transportType();
        redhawk::PropertyMap negotiation_props = manager->getNegotiationProperties(transport);
        ExtendedCF::NegotiationResult_var result;
        try {
            result = negotiablePort->negotiateTransport(transport_type.c_str(), negotiation_props);
        } catch (const ExtendedCF::NegotiationError& exc) {
            RH_ERROR(logger, "Error negotiating transport '" << transport_type << ": " << exc.msg);
            delete transport;
            return 0;
        }

        const std::string transport_id(result->transportId);
        NegotiatedConnection* connection = new NegotiatedConnection(connectionId, negotiablePort, transport_id, transport);
        try {
            manager->setNegotiationResult(transport, redhawk::PropertyMap::cast(result->properties));
        } catch (const std::exception& exc) {
            RH_ERROR(logger, "Error completing transport '" << transport_type << "' connection: "
                     << exc.what());
            delete connection;
            return 0;
        }
        return connection;
    }

}
