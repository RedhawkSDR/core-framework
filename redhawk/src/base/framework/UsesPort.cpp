/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK GPP.
 *
 * REDHAWK GPP is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK GPP is distributed in the hope that it will be useful, but WITHOUT
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

namespace redhawk {
    
    BasicTransport::BasicTransport(const std::string& connectionId, CORBA::Object_ptr objref) :
        _connectionId(connectionId),
        _objref(CORBA::Object::_duplicate(objref)),
        _alive(true)
    {
    }

    const std::string& BasicTransport::connectionId() const
    {
        return _connectionId;
    }

    std::string BasicTransport::getDescription() const
    {
        return "basic transport";
    }

    bool BasicTransport::isAlive() const
    {
        return _alive;
    }

    void BasicTransport::setAlive(bool alive)
    {
        _alive = alive;
    }

    CORBA::Object_ptr BasicTransport::objref() const
    {
        return _objref;
    }


    UsesPort::UsesPort(const std::string& name) :
        Port_Uses_base_impl(name)
    {
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
                BasicTransport* transport = _createTransport(connection, connection_id);
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

    void UsesPort::_addTransportEntry(BasicTransport* transport)
    {
        _transports.push_back(transport);
    }

    BasicTransport* UsesPort::_createTransport(CORBA::Object_ptr object, const std::string& connectionId)
    {
        return new BasicTransport(connectionId, object);
    }
}
