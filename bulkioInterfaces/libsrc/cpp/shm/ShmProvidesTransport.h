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

#include "ipcfifo.h"
#include "ShmInputTransport.h"

namespace bulkio {

    template <typename PortType>
    class ShmInputManager : public InputManager<PortType> {
    public:
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

        ShmInputTransport<PortType>* createInputTransport(const std::string& transportId,
                                                          const redhawk::PropertyMap& properties)
        {
            if (!properties.contains("fifo")) {
                throw ExtendedCF::NegotiationError("Invalid properties for shared memory connection");
            }
            const std::string location = properties["fifo"].toString();
            IPCFifo* fifo = new IPCFifoClient(location);
            try {
                fifo->beginConnect();
            } catch (const std::exception& exc) {
                delete fifo;
                std::string message = "Failed to connect to FIFO " + location;
                throw ExtendedCF::NegotiationError(message.c_str());
            }
            return new ShmInputTransport<PortType>(this->_port, transportId, fifo);
        }
    };
}

#endif // __bulkio_inputtransportmanager_h
