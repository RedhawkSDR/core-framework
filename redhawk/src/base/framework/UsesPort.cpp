#include <ossie/UsesPort.h>
#include <ossie/CorbaSequence.h>

namespace redhawk {

    UsesPort::UsesPort(const std::string& name) :
        Port_Uses_base_impl(name)
    {
    }

    void UsesPort::connectPort(CORBA::Object_ptr connection, const char* connectionId)
    {
        // Give a specific exception message for nil
        if (CORBA::is_nil(connection)) {
            throw CF::Port::InvalidPort(1, "Nil object reference");
        }

        // Attempt to check the type of the remote object to reject invalid
        // types; note this does not require the lock
        const std::string rep_id = getRepid();
        bool valid;
        try {
            valid = connection->_is_a(rep_id.c_str());
        } catch (...) {
            // If _is_a throws an exception, assume the remote object is
            // unreachable (e.g., dead)
            throw CF::Port::InvalidPort(1, "Object unreachable");
        }

        if (!valid) {
            std::string message = "Object does not support " + rep_id;
            throw CF::Port::InvalidPort(1, message.c_str());
        }

        const std::string connection_id(connectionId);
        {
            // Acquire the state lock before modifying the container
            boost::mutex::scoped_lock lock(updatingPortsLock);

            transport_list::iterator entry = _findTransportEntry(connection_id);
            if (entry == _transports.end()) {
                BasicTransport* transport = _createTransport(connection, connection_id);
                _addTransportEntry(connection_id, transport);
            } else {
                // TODO: Replace the object reference
            }

            active = true;
        }

        _portConnected(connectionId);
    }

    void UsesPort::disconnectPort(const char* connectionId)
    {
        //TRACE_ENTER(logger, "OutPort::disconnectPort" );
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);

            transport_list::iterator iter = _findTransportEntry(connectionId);
            if (iter != _transports.end()) {
                _transportDisconnected(iter->first, iter->second);
            }

            //LOG_DEBUG( logger, "DISCONNECT, PORT/CONNECTION: "  << name << "/" << connectionId );
            delete iter->second;
            _transports.erase(iter);

            if (_transports.empty()) {
                active = false;
            }
        }

        _portDisconnected(connectionId);
        //TRACE_EXIT(logger, "OutPort::disconnectPort" );
    }

    ExtendedCF::UsesConnectionSequence* UsesPort::connections()
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        ExtendedCF::UsesConnectionSequence_var retVal = new ExtendedCF::UsesConnectionSequence();
        for (transport_list::iterator port = _transports.begin(); port != _transports.end(); ++port) {
            ExtendedCF::UsesConnection conn;
            conn.connectionId = port->first.c_str();
            conn.port = port->second->objref();
            ossie::corba::push_back(retVal, conn);
        }
        return retVal._retn();
    }

    UsesPort::transport_list::iterator UsesPort::_findTransportEntry(const std::string& connectionId)
    {
        transport_list::iterator entry = _transports.begin();
        for (; entry != _transports.end(); ++entry) {
            if (entry->first == connectionId) {
                return entry;
            }
        }
        return entry;
    }

    void UsesPort::_addTransportEntry(const std::string& connectionId, BasicTransport* transport)
    {
        _transports.push_back(transport_entry(connectionId, transport));
    }

    BasicTransport* UsesPort::_createTransport(CORBA::Object_ptr object, const std::string& connectionId)
    {
        return new BasicTransport(object);
    }

    void UsesPort::_transportDisconnected(const std::string& connectionId, BasicTransport* transport)
    {
    }
}
