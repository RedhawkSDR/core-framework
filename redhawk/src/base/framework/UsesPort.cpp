#include <ossie/UsesPort.h>
#include <ossie/CorbaSequence.h>

namespace redhawk {

    UsesPort::UsesPort(const std::string& name) :
        Port_Uses_base_impl(name)
    {
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
        //if (_disconnectCB) {
        //    (*_disconnectCB)(connectionId);
        //}
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

    void UsesPort::_transportDisconnected(const std::string& connectionId, BasicTransport* transport)
    {
    }
}
