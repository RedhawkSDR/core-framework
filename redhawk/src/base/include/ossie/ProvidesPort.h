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
#include "Transport.h"

namespace redhawk {

    class NegotiableProvidesPortBase : public Port_Provides_base_impl
#ifdef BEGIN_AUTOCOMPLETE_IGNORE
                                     , public virtual POA_ExtendedCF::NegotiableProvidesPort
#endif
    {
    public:
        NegotiableProvidesPortBase(const std::string& name);
        ~NegotiableProvidesPortBase();

        virtual void initializePort();
        virtual void releasePort();

        virtual ExtendedCF::TransportInfoSequence* supportedTransports();
        virtual ExtendedCF::NegotiationResult* negotiateTransport(const char* transportType,
                                                                  const CF::Properties& transportProperties);
        virtual void disconnectTransport(const char* transportId);

    protected:
        ProvidesTransportManager* _getTransportManager(const std::string& transportType);

        ProvidesTransport* _getTransport(const std::string identifier);

        boost::mutex _transportMutex;

        typedef std::vector<ProvidesTransportManager*> TransportManagerList;
        TransportManagerList _transportManagers;

        typedef std::map<std::string,ProvidesTransport*> TransportMap;
        TransportMap _transports;
    };
}

#endif // OSSIE_PROVIDESPORT_H
