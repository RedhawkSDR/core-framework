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

#ifndef OSSIE_USESPORT_H
#define OSSIE_USESPORT_H

#include <string>
#include <vector>

#include <omniORB4/CORBA.h>

#include "CF/NegotiablePort.h"

#include "Autocomplete.h"
#include "Port_impl.h"
#include "callback.h"
#include "debug.h"
#include "Transport.h"

namespace redhawk {

    class UsesPort : public Port_Uses_base_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
                   , public virtual POA_ExtendedCF::QueryablePort
#endif
    {
    public:
        UsesPort(const std::string& name);
        virtual ~UsesPort();

        // Register the member function 'func' to be called on class instance
        // 'target' when a new connection is made. The function receives one
        // argument, the connection ID:
        //
        //   void Target::func(const std::string&);
        //
        template <class Target, class Func>
        void addConnectListener (Target target, Func func)
        {
            _portConnected.add(target, func);
        }

        // Unregister the member function 'func' on class instance 'target'
        // from further connection notifications. If the pair has not been
        // registered previously, it is ignored.
        template <class Target, class Func>
        void removeConnectListener (Target target, Func func)
        {
            _portConnected.remove(target, func);
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
            _portDisconnected.add(target, func);
        }

        // Unregister the member function 'func' on class instance 'target'
        // from further disconnection notifications. If the pair has not been
        // registered previously, it is ignored.
        template <class Target, class Func>
        void removeDisconnectListener (Target target, Func func)
        {
            _portDisconnected.remove(target, func);
        }

        virtual void connectPort(CORBA::Object_ptr connection, const char* connectionId);

        virtual void disconnectPort(const char* connectionId);

        virtual ExtendedCF::UsesConnectionSequence* connections();

    protected:
        class Connection {
        public:
            Connection(const std::string& connectionId, CORBA::Object_ptr objref, UsesTransport* transport);
            virtual ~Connection();

            virtual void disconnected();

            std::string connectionId;
            CORBA::Object_var objref;
            UsesTransport* transport;
        };

        typedef std::vector<Connection*> ConnectionList;

        typedef std::vector<UsesTransport*> TransportList;

        template <class TransportType>
        class TransportIteratorAdapter {
        public:
            typedef ConnectionList::iterator IteratorType;

            TransportIteratorAdapter()
            {
            }

            TransportIteratorAdapter(IteratorType iter) :
                _iterator(iter)
            {
            }

            inline const std::string& connectionId()
            {
                return (*_iterator)->connectionId;
            }

            inline TransportType* transport()
            {
                return static_cast<TransportType*>((*_iterator)->transport);
            }

            inline TransportIteratorAdapter& operator++()
            {
                ++_iterator;
                return *this;
            }

            inline TransportIteratorAdapter operator++(int)
            {
                TransportIteratorAdapter result(*this);
                ++(*this);
                return result;
            }

            inline bool operator==(const TransportIteratorAdapter& other) const
            {
                return (_iterator == other._iterator);
            }

            inline bool operator!=(const TransportIteratorAdapter& other) const
            {
                return (_iterator != other._iterator);
            }

        private:
            IteratorType _iterator;
        };

        virtual void _validatePort(CORBA::Object_ptr object);

        ConnectionList::iterator _findConnection(const std::string& connectionId);

        virtual Connection* _createConnection(CORBA::Object_ptr object, const std::string& connectionId);

        virtual UsesTransport* _createTransport(CORBA::Object_ptr object, const std::string& connectionId) = 0;

        bool _hasConnection(const std::string& connectionId);

        ConnectionList _connections;

    private:
        ossie::notification<void (const std::string&)> _portConnected;
        ossie::notification<void (const std::string&)> _portDisconnected;
    };

    class NegotiableUsesPort : public UsesPort
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
                             , public virtual POA_ExtendedCF::NegotiableUsesPort
#endif
    {
    public:
        NegotiableUsesPort(const std::string& name);
        virtual ~NegotiableUsesPort();

        virtual void initializePort();

        virtual ExtendedCF::TransportInfoSequence* supportedTransports();

        virtual ExtendedCF::ConnectionStatusSequence* connectionStatus();

    protected:
        class NegotiatedConnection;

        virtual Connection* _createConnection(CORBA::Object_ptr object, const std::string& connectionId);

        virtual UsesTransport* _createLocalTransport(PortBase* port, CORBA::Object_ptr object, const std::string& connectionId);

        NegotiatedConnection* _negotiateConnection(ExtendedCF::NegotiableProvidesPort_ptr negotiablePort,
                                                   const std::string& connectionId);

        NegotiatedConnection* _negotiateTransport(ExtendedCF::NegotiableProvidesPort_ptr negotiablePort,
                                                  const std::string& connectionId,
                                                  UsesTransportManager* manager,
                                                  const redhawk::PropertyMap& properties);

        typedef std::vector<UsesTransportManager*> TransportManagerList;
        TransportManagerList _transportManagers;
    };
}

#endif // OSSIE_USESPORT_H
