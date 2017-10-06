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

namespace redhawk {
    class TransportError : public std::runtime_error
    {
    public:
        TransportError(const std::string& message) :
            std::runtime_error(message)
        {
        }
    };

    class TransportTimeoutError : public TransportError
    {
    public:
        TransportTimeoutError(const std::string& message) :
            TransportError(message)
        {
        }
    };

    class FatalTransportError : public TransportError
    {
    public:
        FatalTransportError(const std::string& message) :
            TransportError(message)
        {
        }
    };

    class UsesTransport
    {
    public:
        UsesTransport(const std::string& connectionId, CORBA::Object_ptr objref);
        virtual ~UsesTransport() { }

        const std::string& connectionId() const;
        CORBA::Object_ptr objref() const;

        virtual std::string transportType() const
        {
            return "";
        }

        virtual CF::Properties transportInfo() const
        {
            return CF::Properties();
        }

        virtual std::string getDescription() const;

        bool isAlive() const;
        void setAlive(bool alive);

        virtual void disconnect() { }

    private:
        const std::string _connectionId;
        CORBA::Object_var _objref;
        bool _alive;
    };

    class UsesPort : public Port_Uses_base_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
                   , public virtual POA_ExtendedCF::NegotiableUsesPort
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

        virtual ExtendedCF::ConnectionStatusSequence* connectionStatus();

        void setLogger(LOGGER newLogger);

    protected:
        typedef std::vector<UsesTransport*> TransportList;

        template <class TransportType>
        class TransportIteratorAdapter {
        public:
            typedef TransportList::iterator IteratorType;

            TransportIteratorAdapter()
            {
            }

            TransportIteratorAdapter(IteratorType iter) :
                _iterator(iter)
            {
            }

            inline TransportType* operator*()
            {
                return static_cast<TransportType*>(*_iterator);
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

        TransportList::iterator _findTransportEntry(const std::string& connectionId);
        void _addTransportEntry(UsesTransport* transport);

        virtual UsesTransport* _createTransport(CORBA::Object_ptr object, const std::string& connectionId);

        LOGGER logger;

        TransportList _transports;

    private:
        ossie::notification<void (const std::string&)> _portConnected;
        ossie::notification<void (const std::string&)> _portDisconnected;
    };
}

#endif // OSSIE_USESPORT_H
