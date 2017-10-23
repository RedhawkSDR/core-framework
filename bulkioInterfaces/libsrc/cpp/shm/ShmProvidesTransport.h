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
#ifndef __bulkio_inputtransportmanager_h
#define __bulkio_inputtransportmanager_h

#include <ossie/PropertyMap.h>
#include <ossie/ProvidesPort.h>

#include "ShmInputTransport.h"

namespace bulkio {

    template <typename PortType>
    class ShmInputManager : public InputManager<PortType> {
    public:
        typedef ShmInputTransport<PortType> InputTransportType;

        ShmInputManager(InPort<PortType>* port) :
            InputManager<PortType>(port)
        {
        }

        virtual std::string transportType()
        {
            return "shmipc";
        }

        virtual CF::Properties transportProperties()
        {
            CF::Properties properties;

            char host[HOST_NAME_MAX+1];
            gethostname(host, sizeof(host));

            ossie::corba::push_back(properties, redhawk::PropertyType("hostname", std::string(host)));
            return properties;
        }

        InputTransportType* createInputTransport(const std::string& transportId,
                                                       const redhawk::PropertyMap& properties)
        {
            if (!properties.contains("fifo")) {
                throw redhawk::FatalTransportError("invalid properties for shared memory connection");
            }
            const std::string location = properties["fifo"].toString();
            try {
                return new ShmInputTransport<PortType>(this->_port, transportId, location);
            } catch (const std::exception& exc) {
                throw redhawk::FatalTransportError("failed to connect to FIFO " + location);
            }
        }

        virtual redhawk::PropertyMap getNegotiationProperties(redhawk::ProvidesTransport* providesTransport)
        {
            InputTransportType* transport = dynamic_cast<InputTransportType*>(providesTransport);
            if (!transport) {
                throw std::logic_error("invalid provides transport instance");
            }
            redhawk::PropertyMap properties;
            properties["fifo"] = transport->getFifoName();
            return properties;
        }
    };
}

#endif // __bulkio_inputtransportmanager_h
