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

#include <BulkioTransport.h>
#include "bulkio_typetraits.h"
#include "bulkio_in_port.h"
#include "bulkio_out_port.h"
#include "bulkio_p.h"

namespace bulkio {

    template <typename PortType>
    OutputTransport<PortType>::OutputTransport(OutPortType* port, PtrType objref) :
        redhawk::UsesTransport(port),
        _port(port),
        _objref(PortType::_duplicate(objref)),
        _stats(port->getName())
    {
        // Manually set the bit size because the statistics ctor only takes a
        // byte count
        _stats.setBitSize(NativeTraits<PortType>::bits);
    }

    template <typename PortType>
    OutputTransport<PortType>::~OutputTransport()
    {
    }

    template <typename PortType>
    void OutputTransport<PortType>::disconnect()
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
    void OutputTransport<PortType>::pushSRI(const std::string& streamID, const BULKIO::StreamSRI& sri, int version)
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
    void OutputTransport<PortType>::pushPacket(const BufferType& data,
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
    BULKIO::PortStatistics OutputTransport<PortType>::getStatistics()
    {
        BULKIO::PortStatistics statistics = _stats.retrieve();

        // Use our own stream tracking to fill in the statistics stream IDs
        statistics.streamIDs.length(0);
        for (VersionMap::iterator stream = _sriVersions.begin(); stream != _sriVersions.end(); ++stream) {
            ossie::corba::push_back(statistics.streamIDs, stream->first.c_str());
        }

        // Add extended statistics from subclasses to the keywords
        ossie::corba::extend(statistics.keywords, _getExtendedStatistics());

        return statistics;
    }

    template <typename PortType>
    void OutputTransport<PortType>::_sendPacket(const BufferType& data,
                                                const BULKIO::PrecisionUTCTime& T,
                                                bool EOS,
                                                const std::string& streamID,
                                                const BULKIO::StreamSRI& sri)
    {
        this->_pushPacket(data, T, EOS, streamID);
        this->_recordPush(streamID, this->_dataLength(data), EOS);
    }

    template <typename PortType>
    void OutputTransport<PortType>::_recordPush(const std::string& streamID, size_t elements, bool endOfStream)
    {
        _stats.update(elements, 0.0, endOfStream, streamID);
    }

    template <typename PortType>
    redhawk::PropertyMap OutputTransport<PortType>::_getExtendedStatistics()
    {
        return redhawk::PropertyMap();
    }

    template <typename PortType>
    size_t OutputTransport<PortType>::_dataLength(const BufferType& data)
    {
        return data.size();
    }

    template <>
    size_t OutputTransport<BULKIO::dataFile>::_dataLength(const std::string& /*unused*/)
    {
        return 1;
    }

    //
    // InputTransport
    //
    template <typename PortType>
    InputTransport<PortType>::InputTransport(InPortType* port, const std::string& transportId) :
        redhawk::ProvidesTransport(port, transportId),
        _port(port)
    {
    }

    //
    // OutputManager
    //
    template <typename PortType>
    OutputManager<PortType>::OutputManager(OutPortType* port) :
        _port(port)
    {
    }

    template <typename PortType>
    redhawk::UsesTransport*
    OutputManager<PortType>::createUsesTransport(CORBA::Object_ptr object,
                                                 const std::string& connectionId,
                                                 const redhawk::PropertyMap& properties)
    {
        typename PortType::_var_type port;
        try {
            port = PortType::_narrow(object);
        } catch (const CORBA::Exception&) {
            // If this narrow fails something has gone horribly wrong, but just
            // let the negotiation layer handle it
            return 0;
        }
        return createOutputTransport(port, connectionId, properties);
    }

    //
    // InputManager
    //
    template <typename PortType>
    InputManager<PortType>::InputManager(InPortType* port) :
        _port(port)
    {
    }

    template <typename PortType>
    redhawk::ProvidesTransport*
    InputManager<PortType>::createProvidesTransport(const std::string& transportId,
                                                    const redhawk::PropertyMap& properties)
    {
        return createInputTransport(transportId, properties);
    }

    //
    // TransportFactory
    //
    template <typename PortType>
    std::string BulkioTransportFactory<PortType>::repoId()
    {
        return PortType::_PD_repoId;
    }

    template <typename PortType>
    redhawk::ProvidesTransportManager*
    BulkioTransportFactory<PortType>::createProvidesManager(redhawk::NegotiableProvidesPortBase* port)
    {
        InPortType* bulkio_port = dynamic_cast<InPortType*>(port);
        if (!bulkio_port) {
            throw std::logic_error("incorrect input port type for BulkIO transport factory " + repoId());
        }
        return this->createInputManager(bulkio_port);
    }

    template <typename PortType>
    redhawk::UsesTransportManager*
    BulkioTransportFactory<PortType>::createUsesManager(redhawk::NegotiableUsesPort* port)
    {
        OutPortType* bulkio_port = dynamic_cast<OutPortType*>(port);
        if (!bulkio_port) {
            throw std::logic_error("incorrect output port type for BulkIO transport factory " + repoId());
        }
        return this->createOutputManager(bulkio_port);
    }

#define INSTANTIATE_TEMPLATE(x)                 \
    template class OutputTransport<x>;          \
    template class OutputManager<x>;            \
    template class InputTransport<x>;           \
    template class InputManager<x>;             \
    template class BulkioTransportFactory<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
}
