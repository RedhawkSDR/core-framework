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
#ifndef OSSIE_PROVIDESPORT_H
#define OSSIE_PROVIDESPORT_H

#include <string>
#include <map>

#include "CF/DataType.h"
#include "CF/NegotiablePort.h"
#include "Port_impl.h"
#include "Autocomplete.h"

namespace redhawk {

    class InputTransport
    {
    public:
        InputTransport()
        {
        }

        virtual ~InputTransport()
        {
        }

        virtual void start()
        {
        }

        virtual void stop()
        {
        }
    };

    class InputTransportManager
    {
    public:
        InputTransportManager()
        {
        }

        virtual InputTransport* createInput(const CF::Properties& properties) = 0;

        virtual CF::Properties transportProperties()
        {
            return CF::Properties();
        }
    };

    class NegotiableProvidesPortBase : public Port_Provides_base_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
                                     , public virtual POA_ExtendedCF::NegotiableProvidesPort {
#endif
    public:
        NegotiableProvidesPortBase(const std::string& name);
        ~NegotiableProvidesPortBase();

        virtual CF::Properties* supportedTransports();
        virtual ExtendedCF::NegotiationResult* negotiateTransport(const char* protocol, const CF::Properties& props);
        virtual void disconnectTransport(const char* connectionId);

    protected:
        virtual void _initializeTransports()
        {
        }

        void _addTransportManager(const std::string& transport, InputTransportManager* manager);

        InputTransportManager* _getTransportManager(const std::string& protocol);

        InputTransport* _getInput(const std::string identifier);

        typedef std::map<std::string,InputTransportManager*> TransportManagerMap;
        TransportManagerMap _transportManagers;

        typedef std::map<std::string,InputTransport*> InputMap;
        InputMap _inputs;

    private:
        void _initializeTransportMap();
        bool _transportMapInitialized;
    };
}

#endif // OSSIE_PROVIDESPORT_H
