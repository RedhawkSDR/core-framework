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
    
    UsesTransport::UsesTransport(UsesPort* port, const std::string& connectionId, CORBA::Object_ptr objref) :
        _port(port),
        _connectionId(connectionId),
        _objref(CORBA::Object::_duplicate(objref)),
        _alive(true)
    {
    }

    const std::string& UsesTransport::connectionId() const
    {
        return _connectionId;
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

    CORBA::Object_ptr UsesTransport::objref() const
    {
        return _objref;
    }


    UsesPort::UsesPort(const std::string& name) :
        Port_Uses_base_impl(name)
    {
    }

    UsesPort::~UsesPort()
    {
        for (TransportList::iterator port = _transports.begin(); port != _transports.end(); ++port) {
            delete *port;
        }
    }

    void UsesPort::connectPort(CORBA::Object_ptr connection, const char* connectionId)
    {
        RH_TRACE_ENTER(logger);

        // Give a specific exception message for nil
        if (CORBA::is_nil(connection)) {
            throw CF::Port::InvalidPort(1, "Nil object reference");
        }

        // Attempt to check the type of the remote object to reject invalid
        // types; note this does not require the lock
        _validatePort(connection);

        const std::string connection_id(connectionId);
        {
            // Acquire the state lock before modifying the container
            boost::mutex::scoped_lock lock(updatingPortsLock);

            TransportList::iterator entry = _findTransportEntry(connection_id);
            if (entry == _transports.end()) {
                UsesTransport* transport = _createTransport(connection, connection_id);
                _addTransportEntry(transport);
                RH_DEBUG(logger, "Using " << transport->getDescription()
                         << " for connection '" << connection_id << "'");
            } else {
                // TODO: Replace the object reference
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

            TransportList::iterator transport = _findTransportEntry(connectionId);
            if (transport == _transports.end()) {
                std::string message = std::string("No connection ") + connectionId;
                throw CF::Port::InvalidPort(2, message.c_str());
            }

            RH_DEBUG(logger, "Disconnecting connection '" << connectionId << "'");
            try {
                (*transport)->disconnect();
            } catch (const std::exception& exc) {
                if ((*transport)->isAlive()) {
                    RH_WARN(logger, "Exception disconnecting '" << connectionId << "': "
                            << exc.what());
                }
            } catch (const CORBA::Exception& exc) {
                if ((*transport)->isAlive()) {
                    RH_WARN(logger, "Exception disconnecting '" << connectionId << "': "
                            << ossie::corba::describeException(exc));
                }
            }

            delete (*transport);
            _transports.erase(transport);

            if (_transports.empty()) {
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
        for (TransportList::iterator port = _transports.begin(); port != _transports.end(); ++port) {
            ExtendedCF::UsesConnection conn;
            conn.connectionId = (*port)->connectionId().c_str();
            if ((*port)->isAlive()) {
                conn.port = CORBA::Object::_duplicate((*port)->objref());
            } else {
                conn.port = CORBA::Object::_nil();
            }
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

    UsesPort::TransportList::iterator UsesPort::_findTransportEntry(const std::string& connectionId)
    {
        TransportList::iterator entry = _transports.begin();
        for (; entry != _transports.end(); ++entry) {
            if ((*entry)->connectionId() == connectionId) {
                return entry;
            }
        }
        return entry;
    }

    void UsesPort::_addTransportEntry(UsesTransport* transport)
    {
        _transports.push_back(transport);
    }

    UsesTransport* UsesPort::_createTransport(CORBA::Object_ptr object, const std::string& connectionId)
    {
        return new UsesTransport(this, connectionId, object);
    }

    NegotiableUsesPort::NegotiableUsesPort(const std::string& name) :
        UsesPort(name)
    {
    }

    NegotiableUsesPort::~NegotiableUsesPort()
    {
    }

    void NegotiableUsesPort::initializePort()
    {
        const std::string repo_id = getRepid();
        TransportStack* transports = TransportRegistry::GetTransports(repo_id);
        if (!transports) {
            // No registered transports for this port type
            return;
        }

        for (TransportStack::iterator iter = transports->begin(); iter != transports->end(); ++iter) {
            TransportFactory* transport = *iter;
            RH_INFO(logger, "Adding uses transport '" << transport->transportType()
                    << "' for '" << repo_id << "'");
            _transportManagers.push_back(transport->createUsesManager(this));
        }
    }

    ExtendedCF::TransportInfoSequence* NegotiableUsesPort::supportedTransports()
    {
        ExtendedCF::TransportInfoSequence_var transports = new ExtendedCF::TransportInfoSequence;
        for (TransportManagerList::iterator manager = _transportManagers.begin(); manager != _transportManagers.end(); ++manager) {
            ExtendedCF::TransportInfo info;
            info.transportName = (*manager)->transportName().c_str();
            info.transportProperties = (*manager)->transportProperties();
            ossie::corba::push_back(transports, info);
        }
        return transports._retn();
    }

    ExtendedCF::ConnectionStatusSequence* NegotiableUsesPort::connectionStatus()
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        ExtendedCF::ConnectionStatusSequence_var retVal = new ExtendedCF::ConnectionStatusSequence();
        for (TransportList::iterator port = _transports.begin(); port != _transports.end(); ++port) {
            ExtendedCF::ConnectionStatus status;
            status.connectionId = (*port)->connectionId().c_str();
            status.port = CORBA::Object::_duplicate((*port)->objref());
            status.alive = (*port)->isAlive();
            status.transportType = (*port)->transportType().c_str();
            status.transportInfo = (*port)->transportInfo();
            ossie::corba::push_back(retVal, status);
        }
        return retVal._retn();
    }

    UsesTransport* NegotiableUsesPort::_createTransport(CORBA::Object_ptr object, const std::string& connectionId)
    {
        PortBase* local_port = ossie::corba::getLocalServant<PortBase>(object);
        if (local_port) {
            UsesTransport* transport = _createLocalTransport(local_port, object, connectionId);
            if (transport) {
                return transport;
            }
        }

        ExtendedCF::NegotiableProvidesPort_var negotiable_port = ossie::corba::_narrowSafe<ExtendedCF::NegotiableProvidesPort>(object);
        if (!CORBA::is_nil(negotiable_port)) {
            UsesTransport* transport = _negotiateTransport(negotiable_port, connectionId);
            if (transport) {
                return transport;
            }
        }

        return _createDefaultTransport(object, connectionId);
    }

    UsesTransport* NegotiableUsesPort::_createLocalTransport(PortBase*, CORBA::Object_ptr, const std::string&)
    {
        return 0;
    }

    UsesTransport* NegotiableUsesPort::_createDefaultTransport(CORBA::Object_ptr object, const std::string& connectionId)
    {
        return UsesPort::_createTransport(object, connectionId);
    }

    UsesTransport* NegotiableUsesPort::_negotiateTransport(ExtendedCF::NegotiableProvidesPort_ptr negotiablePort,
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
            const std::string transport_name = (*manager)->transportName();
            for (CORBA::ULong index = 0; index < supported_transports->length(); ++index) {
                if (transport_name == (const char*) supported_transports[index].transportName) {
                    RH_TRACE(logger, "Trying to negotiate transport '" << transport_name
                             << "' for connection '" << connectionId << "'");
                    const redhawk::PropertyMap& transport_props = redhawk::PropertyMap::cast(supported_transports[index].transportProperties);
                    UsesTransport* transport = (*manager)->createUsesTransport(negotiablePort, connectionId, transport_props);
                    if (transport) {
                        return transport;
                    }
                }
            }
            RH_TRACE(logger, "Provides side for connection '" << connectionId
                     << "' does not support transport '" << transport_name << "'");
        }
        return 0;
    }
}
