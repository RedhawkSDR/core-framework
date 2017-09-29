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
#include "bulkio_time_operators.h"
#include "bulkio_in_port.h"
#include "bulkio_p.h"

#include "ipcfifo.h"
#include "MessageBuffer.h"

namespace bulkio {
    template <typename PortType>
    class LocalTransport;

    template <typename PortType>
    class RemoteTransport;

    template <typename PortType>
    class ChunkingTransport;

    template <typename PortType>
    class NegotiableTransportFactory;

    template <typename PortType>
    PortTransport<PortType>* PortTransport<PortType>::Factory(const std::string& connectionId,
                                                              const std::string& name,
                                                              PtrType port)
    {
        typedef InPort<PortType> LocalPortType;
        LocalPortType* local_port = ossie::corba::getLocalServant<LocalPortType>(port);
        if (local_port) {
            return LocalTransport<PortType>::Factory(connectionId, name, local_port, port);
        }

        PortTransport* transport = NegotiableTransportFactory<PortType>::Create(connectionId, name, port);
        if (transport) {
            return transport;
        }

        return RemoteTransport<PortType>::Factory(connectionId, name, port);
    }

    template <typename PortType>
    PortTransport<PortType>::PortTransport(const std::string& connectionId, const std::string& name, PtrType objref) :
        redhawk::BasicTransport(connectionId, objref),
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
        omniORB::setClientCallTimeout(1000);
        for (VersionMap::iterator stream = _sriVersions.begin(); stream != _sriVersions.end(); ++stream) {
            try {
                this->_pushPacket(BufferType(), bulkio::time::utils::notSet(), true, stream->first);
            } catch (redhawk::TransportTimeoutError& e) {
                // ignore the timeout. The destination is in a bad state
            }
        }
        omniORB::setClientCallTimeout(0);
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

    template <typename PortType>
    class RemoteTransport : public PortTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTransport<PortType>::BufferType BufferType;
        typedef typename CorbaTraits<PortType>::SequenceType SequenceType;
        typedef typename CorbaTraits<PortType>::TransportType TransportType;

        static RemoteTransport* Factory(const std::string& connectionId, const std::string& name, PtrType port)
        {
            return new ChunkingTransport<PortType>(connectionId, name, port);
        }

        RemoteTransport(const std::string& connectionId, const std::string& name, PtrType port) :
            PortTransport<PortType>(connectionId, name, port)
        {
        }

        virtual std::string getDescription() const
        {
            return "CORBA BulkIO transport";
        }

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri)
        {
            try {
                this->_port->pushSRI(sri);
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::FatalTransportError(ossie::corba::describeException(exc));
            }
        }

        virtual void _pushPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID)
        {
            try {
                _pushPacketImpl(data, T, EOS, streamID.c_str());
            } catch (const CORBA::TIMEOUT& exc) {
                throw redhawk::TransportTimeoutError("Push timed out");
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::FatalTransportError(ossie::corba::describeException(exc));
            }
        }

    private:
        void _pushPacketImpl(const BufferType& data,
                             const BULKIO::PrecisionUTCTime& T,
                             bool EOS,
                             const char* streamID)
        {
            const TransportType* ptr = reinterpret_cast<const TransportType*>(data.data());
            const SequenceType buffer(data.size(), data.size(), const_cast<TransportType*>(ptr), false);
            this->_port->pushPacket(buffer, T, EOS, streamID);
        }
    };

    template <>
    RemoteTransport<BULKIO::dataFile>* RemoteTransport<BULKIO::dataFile>::Factory(const std::string& connectionId,
                                                                                  const std::string& name,
                                                                                  PtrType port)
    {
        return new RemoteTransport<BULKIO::dataFile>(connectionId, name, port);
    }

    template <>
    void RemoteTransport<BULKIO::dataFile>::_pushPacketImpl(const std::string& data,
                                                            const BULKIO::PrecisionUTCTime& T,
                                                            bool EOS,
                                                            const char* streamID)
    {
        _port->pushPacket(data.c_str(), T, EOS, streamID);
    }

    template <>
    RemoteTransport<BULKIO::dataXML>* RemoteTransport<BULKIO::dataXML>::Factory(const std::string& connectionId,
                                                                                const std::string& name,
                                                                                PtrType port)
    {
        return new RemoteTransport<BULKIO::dataXML>(connectionId, name, port);
    }

    template <>
    void RemoteTransport<BULKIO::dataXML>::_pushPacketImpl(const std::string& data,
                                                           const BULKIO::PrecisionUTCTime& /* unused */,
                                                           bool EOS,
                                                           const char* streamID)
    {
        _port->pushPacket(data.c_str(), EOS, streamID);
    }

    template <typename PortType>
    class ChunkingTransport : public RemoteTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTransport<PortType>::BufferType BufferType;
        typedef typename CorbaTraits<PortType>::TransportType TransportType;

        ChunkingTransport(const std::string& connectionId, const std::string& name, PtrType port) :
            RemoteTransport<PortType>(connectionId, name, port)      
        {
            // Multiply by some number < 1 to leave some margin for the CORBA header
            const size_t maxPayloadSize = (size_t) (bulkio::Const::MaxTransferBytes() * .9);
            maxSamplesPerPush = maxPayloadSize/sizeof(TransportType);
        }

        /*
         * Push a packet whose payload may not fit within the CORBA limit. The
         * packet is broken down into sub-packets and sent via multiple pushPacket
         * calls.  The EOS is set to false for all of the sub-packets, except for
         * the last sub-packet, which uses the input EOS argument.
         */
        virtual void _sendPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID,
                                 const BULKIO::StreamSRI& sri)
        {
            double xdelta = sri.xdelta;
            size_t itemSize = sri.mode?2:1;
            size_t frameSize = itemSize;
            if (sri.subsize > 0) {
                frameSize *= sri.subsize;
            }
            // Quantize the push size (in terms of scalars) to the nearest frame,
            // which takes both the complex mode and subsize into account
            const size_t maxPushSize = (maxSamplesPerPush/frameSize) * frameSize;

            // Always do at least one push (may be empty), ensuring that all samples
            // are pushed
            size_t first = 0;
            size_t samplesRemaining = data.size();

            // Initialize time of first subpacket
            BULKIO::PrecisionUTCTime packetTime = T;
      
            do {
                // Don't send more samples than are remaining
                const size_t pushSize = std::min(samplesRemaining, maxPushSize);
                samplesRemaining -= pushSize;

                // Send end-of-stream as false for all sub-packets except for the
                // last one (when there are no samples remaining after this push),
                // which gets the input EOS.
                bool packetEOS = false;
                if (samplesRemaining == 0) {
                    packetEOS = EOS;
                }

                // Take the next slice of the input buffer.
                BufferType subPacket = data.slice(first, first + pushSize);
                RemoteTransport<PortType>::_sendPacket(subPacket, packetTime, packetEOS, streamID, sri);

                // Synthesize the next packet timestamp
                if (packetTime.tcstatus == BULKIO::TCS_VALID) {
                    packetTime += (pushSize/itemSize)* xdelta;
                }

                // Advance buffer to next sub-packet boundary
                first += pushSize;
            } while (samplesRemaining > 0);
        }

    private:
        size_t maxSamplesPerPush;
    };


    template <typename PortType>
    class LocalTransport : public PortTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTransport<PortType>::BufferType BufferType;
        typedef InPort<PortType> LocalPortType;

        static LocalTransport* Factory(const std::string& connectionId, const std::string& name,
                                       LocalPortType* localPort, PtrType port)
        {
            return new LocalTransport(connectionId, name, localPort, port);
        }

        LocalTransport(const std::string& connectionId, const std::string& name,
                       LocalPortType* localPort, PtrType port) :
            PortTransport<PortType>(connectionId, name, port),
            _localPort(localPort)
        {
            _localPort->_add_ref();
        }

        ~LocalTransport()
        {
            _localPort->_remove_ref();
        }

        virtual std::string getDescription() const
        {
            return "local BulkIO connection to " + _localPort->getName();
        }

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri)
        {
            _localPort->pushSRI(sri);
        }

        virtual void _pushPacket(const BufferType& data,
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

        LocalPortType* _localPort;
    };

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

    template <typename PortType>
    class ShmTransport : public ChunkingTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTransport<PortType>::BufferType BufferType;
        typedef typename CorbaTraits<PortType>::TransportType TransportType;

        static ShmTransport* Factory(const std::string& connectionId,
                                     const std::string& name,
                                     BULKIO::NegotiableProvidesPort_ptr negotiablePort,
                                     PtrType port)
        {
            // For testing, allow disabling
            const char* shm_env = getenv("BULKIO_SHM");
            if (shm_env) {
                if (strcmp(shm_env, "disable") == 0) {
                    return 0;
                }
            }

            RH_NL_DEBUG("ShmTransport", "Attempting to negotiate shared memory IPC");
            IPCFifo* fifo = new IPCFifoServer(name + "-fifo");
            redhawk::PropertyMap props;
            props["fifo"] = fifo->name();
            fifo->beginConnect();
            try {
                negotiablePort->negotiate("shmipc", props);
            } catch (const BULKIO::NegotiationError& exc) {
                RH_NL_ERROR("ShmTransport", "Error negotiating shared memory IPC: " << exc.msg);
                delete fifo;
                return 0;
            }
            fifo->finishConnect();

            return new ShmTransport(connectionId, name, fifo, port);
        }

        ShmTransport(const std::string& connectionId, const std::string& name, IPCFifo* fifo, PtrType port) :
            ChunkingTransport<PortType>(connectionId, name, port),
            _fifo(fifo)
        {
        }

        ~ShmTransport()
        {
            delete _fifo;
        }

        virtual std::string getDescription() const
        {
            return "shared memory BulkIO connection";
        }

    protected:

        virtual void _sendPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID,
                                 const BULKIO::StreamSRI&)
        {
            MessageBuffer msg;

            const void* base = data.base();
            redhawk::shm::MemoryRef ref = redhawk::shm::Heap::getRef(base);
            size_t offset = reinterpret_cast<size_t>(data.data()) - reinterpret_cast<size_t>(base);

            msg.write(ref.heap);
            msg.write(ref.superblock);
            msg.write(ref.offset);
            msg.write(offset);
            msg.write(data.size());
            msg.write(T);
            msg.write(EOS);
            msg.write(streamID);

            _fifo->write(msg.buffer(), msg.size());

            size_t status;
            if (_fifo->read(&status, sizeof(size_t)) != sizeof(size_t)) {
                RH_NL_ERROR("ShmTransport", "Bad response");
            }
            this->stats.update(this->_dataLength(data), 0, EOS, streamID);
        }

        virtual void disconnect()
        {
            ChunkingTransport<PortType>::disconnect();
            _fifo->close();
        }

    private:
        IPCFifo* _fifo;
    };

    template <class PortType>
    class NegotiableTransportFactory {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef PortTransport<PortType> TransportType;

        static TransportType* Create(const std::string& connectionId, const std::string& name, PtrType port)
        {
            BULKIO::NegotiableProvidesPort_var negotiable_port;
            try {
                negotiable_port = BULKIO::NegotiableProvidesPort::_narrow(port);
            } catch (...) {
                // Not capable of negotiation
                return 0;
            }
            if (CORBA::is_nil(negotiable_port)) {
                return 0;
            }

            TransportType* transport = ShmTransport<PortType>::Factory(connectionId, name, negotiable_port, port);
            if (transport) {
                return transport;
            }

            return 0;
        };
    };

    template <>
    class NegotiableTransportFactory<BULKIO::dataFile>
    {
    public:
        static PortTransport<BULKIO::dataFile>* Create(const std::string&, const std::string&, BULKIO::dataFile_ptr)
        {
            return 0;
        }
    };

    template <>
    class NegotiableTransportFactory<BULKIO::dataXML>
    {
    public:
        static PortTransport<BULKIO::dataXML>* Create(const std::string&, const std::string&, BULKIO::dataXML_ptr)
        {
            return 0;
        }
    };

#define INSTANTIATE_TEMPLATE(x)                 \
    template class PortTransport<x>;            \
    template class LocalTransport<x>;           \
    template class RemoteTransport<x>;

#define INSTANTIATE_NUMERIC_TEMPLATE(x)         \
    template class ChunkingTransport<x>;        \
    template class ShmTransport<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
