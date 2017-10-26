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

#include "LocalTransport.h"

#include <bulkio_in_port.h>

#include "bulkio_p.h"

namespace bulkio {

    template <typename PortType>
    LocalTransport<PortType>* LocalTransport<PortType>::Factory(OutPort<PortType>* parent,
                                                                PortBase* port)
    {
        LocalPortType* local_port = dynamic_cast<LocalPortType*>(port);
        if (local_port) {
            typename PortType::_var_type corba_port = local_port->_this();
            return new LocalTransport(parent, local_port, corba_port);
        }
        return 0;
    }

    template <typename PortType>
    LocalTransport<PortType>::LocalTransport(OutPort<PortType>* parent, LocalPortType* localPort, PtrType port) :
        OutputTransport<PortType>(parent, port),
        _localPort(localPort)
    {
        _localPort->_add_ref();
    }

    template <typename PortType>
    LocalTransport<PortType>::~LocalTransport()
    {
        _localPort->_remove_ref();
    }

    template <typename PortType>
    std::string LocalTransport<PortType>::transportType() const
    {
        return "local";
    }

    template <typename PortType>
    CF::Properties LocalTransport<PortType>::transportInfo() const
    {
        return CF::Properties();
    }

    template <typename PortType>
    void LocalTransport<PortType>::_pushSRI(const BULKIO::StreamSRI& sri)
    {
        _localPort->pushSRI(sri);
    }

    template <typename PortType>
    void LocalTransport<PortType>::_pushPacket(const BufferType& data,
                             const BULKIO::PrecisionUTCTime& T,
                             bool EOS,
                             const std::string& streamID)
    {
        _localPort->queuePacket(data, T, EOS, streamID);
    }

#define INSTANTIATE_TEMPLATE(x) template class LocalTransport<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);

}
