/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

#include "ShmOutputTransport.h"
#include "ShmInputTransport.h"

#include "bulkio_p.h"

namespace bulkio {

    template <typename PortType>
    class ShmTransportFactory : public BulkioTransportFactory<PortType>
    {
    public:
        ShmTransportFactory()
        {
        }

        virtual std::string transportType()
        {
            return "shmipc";
        }

        virtual int defaultPriority()
        {
            return 1;
        }

        virtual InputManager<PortType>* createInputManager(InPort<PortType>* port)
        {
            return new ShmInputManager<PortType>(port);
        }

        virtual OutputManager<PortType>* createOutputManager(OutPort<PortType>* port)
        {
            return new ShmOutputManager<PortType>(port);
        }
    };

    static int initializeModule()
    {
#define REGISTER_FACTORY(x)                                             \
        {                                                               \
            static ShmTransportFactory<x> factory;                      \
            redhawk::TransportRegistry::RegisterTransport(&factory);    \
        }

        FOREACH_NUMERIC_PORT_TYPE(REGISTER_FACTORY);

        return 0;
    }

    static int initialized = initializeModule();
}
