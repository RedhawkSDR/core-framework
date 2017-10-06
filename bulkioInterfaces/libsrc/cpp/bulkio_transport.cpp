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

#include "bulkio_transport.h"
#include "bulkio_typetraits.h"
#include "bulkio_in_port.h"
#include "bulkio_p.h"

namespace bulkio {

    template <typename PortType>
    PortTransport<PortType>::PortTransport(const std::string& connectionId, const std::string& name, PtrType objref) :
        redhawk::UsesTransport(connectionId, objref),
        stats(name, sizeof(NativeType)),
        _port(PortType::_duplicate(objref))
    {
    }

    template <typename PortType>
    PortTransport<PortType>::~PortTransport()
    {
    }

    template <typename PortType>
    void PortTransport<PortType>::disconnect()
    {
        // Send an end-of-stream for all active streams
        for (VersionMap::iterator stream = _sriVersions.begin(); stream != _sriVersions.end(); ++stream) {
            try {
                this->_pushPacket(BufferType(), bulkio::time::utils::notSet(), true, stream->first);
            } catch (redhawk::TransportTimeoutError& e) {
                // Ignore the timeout. The destination is in a bad state
            }
        }
        _sriVersions.clear();
    }

    template <typename PortType>
    void PortTransport<PortType>::pushSRI(const std::string& streamID, const BULKIO::StreamSRI& sri, int version)
    {
        VersionMap::iterator existing = _sriVersions.find(streamID);
        if (existing != _sriVersions.end()) {
            if (version == existing->second) {
                return;
            }
            existing->second = version;
        } else {
            _sriVersions[streamID] = version;
        }
        this->_pushSRI(sri);
    }

    template <typename PortType>
    void PortTransport<PortType>::pushPacket(const BufferType& data,
                                             const BULKIO::PrecisionUTCTime& T,
                                             bool EOS,
                                             const std::string& streamID,
                                             const BULKIO::StreamSRI& sri)
    {
        this->_sendPacket(data, T, EOS, streamID, sri);
        if (EOS) {
            _sriVersions.erase(streamID);
        }
    }

    template <typename PortType>
    typename PortTransport<PortType>::PtrType PortTransport<PortType>::port()
    {
        return _port;
    }

    template <typename PortType>
    void PortTransport<PortType>::_sendPacket(const BufferType& data,
                                              const BULKIO::PrecisionUTCTime& T,
                                              bool EOS,
                                              const std::string& streamID,
                                              const BULKIO::StreamSRI& sri)
    {
        this->_pushPacket(data, T, EOS, streamID);
        stats.update(this->_dataLength(data), 0, EOS, streamID);
    }

    template <typename PortType>
    size_t PortTransport<PortType>::_dataLength(const BufferType& data)
    {
        return data.size();
    }

    template <>
    size_t PortTransport<BULKIO::dataFile>::_dataLength(const std::string& /*unused*/)
    {
        return 1;
    }

#define INSTANTIATE_TEMPLATE(x)                 \
    template class PortTransport<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
}
