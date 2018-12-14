/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef BURSTIO_USESPORT_DECL_H
#define BURSTIO_USESPORT_DECL_H

#include <map>
#include <string>

#include <boost/bind.hpp>
#include <boost/thread/mutex.hpp>

#include <ossie/Port_impl.h>
#include <ossie/callback.h>

#include "utils.h"

namespace burstio {

    template <class PortType>
    class BasicTransport {
    public:
        typedef PortType port_type;
        typedef typename port_type::_ptr_type ptr_type;
        typedef typename port_type::_var_type var_type;

        BasicTransport(ptr_type port, const std::string& connectionId) :
            port_(port_type::_duplicate(port)),
            connectionId_(connectionId)
        {
        }

        virtual ~BasicTransport() { }

        const std::string& getConnectionId() const
        {
            return connectionId_;
        }

        ptr_type objref() const
        {
            return port_type::_duplicate(port_);
        }

    protected:
        var_type port_;
        const std::string connectionId_;
    };

    template <class PortType, class TransportType=BasicTransport<PortType> >
    class UsesPort : public Port_Uses_base_impl, public virtual POA_ExtendedCF::QueryablePort
    {
    public:
        typedef PortType port_type;
        typedef typename port_type::_ptr_type ptr_type;
        typedef typename port_type::_var_type var_type;
        typedef TransportType transport_type;

        typedef std::pair<std::string,var_type> connection_type;
        typedef std::vector<connection_type> connection_list;

        // Register the member function 'func' to be called on class instance
        // 'target' when a new connection is made. The function receives one
        // argument, the connection ID:
        //
        //   void Target::func(const std::string&);
        //
        template <class Target, class Func>
        void addConnectListener (Target target, Func func)
        {
            connectListeners_.add(target, func);
        }

        // Unregister the member function 'func' on class instance 'target'
        // from further connection notifications. If the pair has not been
        // registered previously, it is ignored.
        template <class Target, class Func>
        void removeConnectListener (Target target, Func func)
        {
            connectListeners_.remove(target, func);
        }

        // Register the member function 'func' to be called on class instance
        // 'target' when an existing connection is broken. The function
        // receives one argument, the connection ID:
        //
        //   void Target::func(const std::string&);
        //
        template <class Target, class Func>
        void addDisconnectListener (Target target, Func func)
        {
            disconnectListeners_.add(target, func);
        }

        // Unregister the member function 'func' on class instance 'target'
        // from further disconnection notifications. If the pair has not been
        // registered previously, it is ignored.
        template <class Target, class Func>
        void removeDisconnectListener (Target target, Func func)
        {
            disconnectListeners_.remove(target, func);
        }

        virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId)
        {
            // Give a specific exception message for nil
            if (CORBA::is_nil(connection)) {
                throw CF::Port::InvalidPort(1, "Nil object reference");
            }

            // Attempt to narrow the remote object to the template parameter
            // type; note this does not require the lock
            var_type port;
            try {
                port = port_type::_narrow(connection);
            } catch (...) {
                // If the object type is not obviously the desired type,
                // _narrow will invoke _is_a, which may throw a CORBA exception
                // if the remote object is unreachable (e.g., dead)
                throw CF::Port::InvalidPort(1, "Object unreachable");
            }

            // If the narrow returned nil without throwing an exception, it's
            // safe to assume the object is the wrong type
            if (CORBA::is_nil(port)) {
                std::string message = "Object does not support " + std::string(port_type::_PD_repoId);
                throw CF::Port::InvalidPort(1, message.c_str());
            }

            {
                // Acquire the state lock before modifying the container
                boost::mutex::scoped_lock lock(updatingPortsLock);

                typename ConnectionMap::iterator entry = connections_.find(connectionId);
                if (entry == connections_.end()) {
                    // Store the new connection and pass the new entry along to
                    // connectionAdded
                    connections_[connectionId] = _createConnection(port, connectionId);
                } else {
                    // TODO: Replace the object reference
                    //entry->second.port = port._retn();
                }
            }

            connectListeners_(connectionId);
        }

        virtual void disconnectPort(const char* connectionId)
        {
            {
                boost::mutex::scoped_lock lock(updatingPortsLock);
                typename ConnectionMap::iterator existing = connections_.find(connectionId);
                if (existing == connections_.end()) {
                    std::string message = std::string("No connection ") + connectionId;
                    throw CF::Port::InvalidPort(2, message.c_str());
                }

                delete existing->second;
                connections_.erase(existing);
            }

            disconnectListeners_(connectionId);
        }

        virtual ExtendedCF::UsesConnectionSequence* connections()
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);
            ExtendedCF::UsesConnectionSequence_var retval = new ExtendedCF::UsesConnectionSequence();
            retval->length(connections_.size());
            CORBA::ULong index = 0;
            for (typename ConnectionMap::iterator ii = connections_.begin(); ii != connections_.end(); ++ii, ++index) {
                retval[index].connectionId = ii->first.c_str();
                retval[index].port = CORBA::Object::_duplicate(ii->second->objref());
            }
            return retval._retn();
        }

        var_type getConnection (const std::string& connectionId)
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);
            typename ConnectionMap::iterator existing = connections_.find(connectionId);
            if (existing == connections_.end()) {
                throw std::invalid_argument("No connection " + connectionId);
            }
            return existing->second->objref();
        }

        connection_list getConnections()
        {
            boost::mutex::scoped_lock lock(updatingPortsLock);
            connection_list result;
            for (typename ConnectionMap::iterator ii = connections_.begin(); ii != connections_.end(); ++ii) {
                result.push_back(std::make_pair(ii->first, ii->second->objref()));
            }
            return result;
        }

    protected:
        UsesPort (std::string port_name) :
            Port_Uses_base_impl(port_name)
        {
        }

        virtual transport_type* _createConnection(ptr_type port, const std::string& connectionId) = 0;

        typedef std::map<std::string, transport_type*> ConnectionMap;
        ConnectionMap connections_;

    private:
        ossie::notification<void (const std::string&)> connectListeners_;
        ossie::notification<void (const std::string&)> disconnectListeners_;
    };

}

#endif // BURSTIO_USESPORT_DECL_H
