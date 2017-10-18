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

    class UsesTransport
    {
    public:
        UsesTransport(UsesPort* port);
        virtual ~UsesTransport() { }

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
        UsesPort* _port;
        bool _alive;
    };

    class UsesTransportManager
    {
    public:
        virtual ~UsesTransportManager()
        {
        }

        virtual std::string transportName() = 0;

        virtual CF::Properties transportProperties()
        {
            return CF::Properties();
        }

        virtual redhawk::PropertyMap getNegotiationProperties(UsesTransport*)
        {
            return redhawk::PropertyMap();
        }

        virtual void setNegotiationResult(UsesTransport*, const redhawk::PropertyMap&)
        {
        }

        virtual UsesTransport* createUsesTransport(CORBA::Object_ptr object,
                                                   const std::string& connectionId,
                                                   const redhawk::PropertyMap& properties) = 0;
    };

    class ProvidesTransport
    {
    public:
        ProvidesTransport(NegotiableProvidesPortBase* port, const std::string& transportId);

        virtual ~ProvidesTransport()
        {
        }

        const std::string& transportId() const;

        virtual std::string transportType() const
        {
            return "";
        }

        virtual CF::Properties transportInfo() const
        {
            return CF::Properties();
        }

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

    class ProvidesTransportManager
    {
    public:
        virtual ~ProvidesTransportManager()
        {
        }

        virtual ProvidesTransport* createProvidesTransport(const std::string& transportId,
                                                           const redhawk::PropertyMap& properties) = 0;

        virtual CF::Properties transportProperties()
        {
            return CF::Properties();
        }

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
        virtual std::string repid() = 0;

        virtual ProvidesTransportManager* createProvidesManager(redhawk::NegotiableProvidesPortBase* port) = 0;
        virtual UsesTransportManager* createUsesManager(redhawk::NegotiableUsesPort* port) = 0;
    };

    typedef std::vector<TransportFactory*> TransportStack;

    class TransportRegistry
    {
    public:
        static void RegisterTransport(TransportFactory* transport);
        static TransportStack* GetTransports(const std::string& repid);

    private:
        TransportRegistry();

        static TransportRegistry& Instance();
        static TransportRegistry* _instance;

        void _registerTransport(TransportFactory* transport);
        TransportStack* _getTransport(const std::string& repid);

        typedef std::map<std::string,TransportStack> TransportMap;
        TransportMap _registry;
    };
}

#endif // OSSIE_TRANSPORT_H
