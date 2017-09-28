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

#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <string>
#include <vector>
#include <omniORB4/CORBA.h>

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

    class BasicTransport
    {
    public:
        BasicTransport(const std::string& connectionId, CORBA::Object_ptr objref);
        virtual ~BasicTransport() { }

        const std::string& connectionId() const;
        CORBA::Object_ptr objref() const;

        virtual std::string getDescription() const;

        bool isAlive() const;
        void setAlive(bool alive);

        virtual void connect() { }
        virtual void disconnect() { }

    private:
        const std::string _connectionId;
        CORBA::Object_var _objref;
        bool _alive;
    };


    typedef std::vector<BasicTransport*> TransportList;

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
}

#endif // TRANSPORT_H
