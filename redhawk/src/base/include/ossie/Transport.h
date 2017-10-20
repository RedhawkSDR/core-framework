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

#ifndef OSSIE_TRANSPORT_H
#define OSSIE_TRANSPORT_H

#include <stdexcept>
#include <string>
#include <map>

#include <omniORB4/CORBA.h>

#include "CF/NegotiablePort.h"
#include "PropertyMap.h"

namespace redhawk {

    class NegotiableProvidesPortBase;
    class UsesPort;
    class NegotiableUsesPort;

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

    class Transport {
    public:
        virtual ~Transport()
        {
        }

        virtual std::string transportType() const = 0;

        virtual CF::Properties transportInfo() const
        {
            return CF::Properties();
        }
    };

    class TransportManager {
    public:
        virtual ~TransportManager()
        {
        }

        virtual std::string transportType() = 0;

        virtual CF::Properties transportProperties()
        {
            return CF::Properties();
        }
    };

    class UsesTransport : public Transport
    {
    public:
        UsesTransport(UsesPort* port);
        virtual ~UsesTransport() { }

        bool isAlive() const;
        void setAlive(bool alive);

        virtual void disconnect() { }

    private:
        UsesPort* _port;
        bool _alive;
    };

    class UsesTransportManager : public TransportManager
    {
    public:
        virtual ~UsesTransportManager()
        {
        }

        /**
         * Return null to abort transport negotiation.
         *
         * Must not throw an exception.
         */
        virtual UsesTransport* createUsesTransport(CORBA::Object_ptr object,
                                                   const std::string& connectionId,
                                                   const redhawk::PropertyMap& properties) = 0;

        /**
         * Must not throw an exception.
         */
        virtual redhawk::PropertyMap getNegotiationProperties(UsesTransport*)
        {
            return redhawk::PropertyMap();
        }

        /**
         * May throw a TransportError to abort transport negotiation.
         */         
        virtual void setNegotiationResult(UsesTransport*, const redhawk::PropertyMap&)
        {
        }
    };

    class ProvidesTransport : public Transport
    {
    public:
        ProvidesTransport(NegotiableProvidesPortBase* port, const std::string& transportId);

        virtual ~ProvidesTransport()
        {
        }

        const std::string& transportId() const;

        /**
         * May throw a TransportError to abort transport negotiation.
         */         
        virtual void startTransport()
        {
        }

        virtual void stopTransport()
        {
        }

    protected:
        NegotiableProvidesPortBase* _port;
        const std::string _transportId;
    };

    class ProvidesTransportManager : public TransportManager
    {
    public:
        virtual ~ProvidesTransportManager()
        {
        }

        /**
         * Return null to abort transport negotiation.
         *
         * Must not throw an exception.
         */
        virtual ProvidesTransport* createProvidesTransport(const std::string& transportId,
                                                           const redhawk::PropertyMap& properties) = 0;

        /**
         * Must not throw an exception.
         */
        virtual redhawk::PropertyMap getNegotiationProperties(ProvidesTransport*)
        {
            return redhawk::PropertyMap();
        }
    };

    class TransportFactory
    {
    public:
        ~TransportFactory()
        {
        }

        virtual std::string transportType() = 0;
        virtual std::string repoId() = 0;
        virtual int defaultPriority() = 0;

        virtual ProvidesTransportManager* createProvidesManager(redhawk::NegotiableProvidesPortBase* port) = 0;
        virtual UsesTransportManager* createUsesManager(redhawk::NegotiableUsesPort* port) = 0;
    };

    typedef std::vector<TransportFactory*> TransportStack;

    class TransportRegistry
    {
    public:
        static void RegisterTransport(TransportFactory* transport);
        static TransportStack GetTransports(const std::string& repoId);

    private:
        TransportRegistry();

        class Impl;

        static Impl& Instance();
    };
}

#endif // OSSIE_TRANSPORT_H
