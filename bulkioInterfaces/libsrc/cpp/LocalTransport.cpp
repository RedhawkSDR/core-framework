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
                                                                const std::string& connectionId,
                                                                PortBase* port)
    {
        LocalPortType* local_port = dynamic_cast<LocalPortType*>(port);
        if (local_port) {
            typename PortType::_var_type corba_port = local_port->_this();
            return new LocalTransport(parent, connectionId, local_port, corba_port);
        }
        return 0;
    }

    template <typename PortType>
    LocalTransport<PortType>::LocalTransport(OutPort<PortType>* parent, const std::string& connectionId,
                                             LocalPortType* localPort, PtrType port) :
        OutputTransport<PortType>(parent, connectionId, port),
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
    std::string LocalTransport<PortType>::getDescription() const
    {
        return "local BulkIO connection to " + _localPort->getName();
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
        if (data.transient()) {
            // The data comes from a non-shared source (a vector or raw pointer),
            // so we need to make a copy. This could be optimized for the fanout
            // case by making the copy at a higher level, but only if there's at
            // least one local connection.
            _localPort->queuePacket(data.copy(), T, EOS, streamID);
        } else {
            _localPort->queuePacket(data, T, EOS, streamID);
        }
    }

    template <>
    void LocalTransport<BULKIO::dataFile>::_pushPacket(const std::string& data,
                                                       const BULKIO::PrecisionUTCTime& T,
                                                       bool EOS,
                                                       const std::string& streamID)
    {
        _localPort->queuePacket(data, T, EOS, streamID);
    }

    template <>
    void LocalTransport<BULKIO::dataXML>::_pushPacket(const std::string& data,
                                                      const BULKIO::PrecisionUTCTime& T,
                                                      bool EOS,
                                                      const std::string& streamID)
    {
        _localPort->queuePacket(data, T, EOS, streamID);
    }

#define INSTANTIATE_TEMPLATE(x) template class LocalTransport<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);

}
