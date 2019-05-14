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
#include "FifoIPC.h"
#include "MessageBuffer.h"

#include <numeric>

#include <ossie/shm/Heap.h>

#include <bulkio_in_port.h>
#include <bulkio_out_port.h>

#include "bulkio_p.h"

namespace bulkio {

    struct ShmStatPoint {
        ShmStatPoint() :
            shmTransfer(0),
            copied(0)
        {
        }

        ShmStatPoint(bool shmTransfer, bool copied) :
            shmTransfer(shmTransfer),
            copied(copied)
        {
        }

        ShmStatPoint operator+ (const ShmStatPoint& other) const
        {
            ShmStatPoint result(*this);
            result += other;
            return result;
        }

        ShmStatPoint& operator+= (const ShmStatPoint& other)
        {
            shmTransfer += other.shmTransfer;
            copied += other.copied;
            return *this;
        }

        int shmTransfer;
        int copied;
    };

    template <typename PortType>
    class ShmOutputTransport : public OutputTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename OutputTransport<PortType>::BufferType BufferType;
        typedef typename BufferType::value_type ElementType;
        typedef typename CorbaTraits<PortType>::TransportType TransportType;

        ShmOutputTransport(OutPort<PortType>* parent, PtrType port) :
            OutputTransport<PortType>(parent, port),
            _fifo()
        {
        }

        ~ShmOutputTransport()
        {
        }

        virtual std::string transportType() const
        {
            return "shmipc";
        }

        virtual CF::Properties transportInfo() const
        {
            return CF::Properties();
        }

        const std::string& getFifoName()
        {
            return _fifo.name();
        }

        void finishConnect(const std::string& filename)
        {
            _fifo.connect(filename);

            // The provides side should have already opened its write end, so
            // if the FIFO doesn't sync immediately, something is wrong.
            _fifo.sync(0);
        }

        virtual void disconnect()
        {
            OutputTransport<PortType>::disconnect();
            _fifo.disconnect();
        }

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri)
        {
            try {
                this->_objref->pushSRI(sri);
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::FatalTransportError(ossie::corba::describeException(exc));
            }
        }

        virtual void _pushPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID)
        {
            MessageBuffer header;
            header.write("pushPacket");

            header.write(data.size());
            header.write(T);
            header.write(EOS);
            header.write(streamID);

            // Temporary buffer to ensure that if a copy is made, it gets
            // released after the transfer
            BufferType copy;

            // Data may be sent over the FIFO if shared memory is unavailable;
            // this is slower but provides a more graceful failure mode
            const void* body = 0;
            size_t body_size = 0;

            // If the packet is non-empty, write the additional shared memory
            // information for the remote side to pick up
            if (!data.empty()) {
                // Track whether the buffer was able to be transferred via a
                // shared memory object, or if it has to be copied
                bool shm_transfer = false;

                // Check that the buffer is already in shared memory (this is
                // hoped to be the common case); if not, copy it into another
                if (data.get_memory().is_process_shared()) {
                    // Include the offset from the start of allocated memory to
                    // the first element of the buffer
                    const void* base = data.get_memory().address();
                    size_t offset = reinterpret_cast<size_t>(data.data()) - reinterpret_cast<size_t>(base);
                    shm_transfer = _transferBuffer(header, base, offset);
                } else {
                    // Try to explicitly allocate from shared memory via the
                    // global function (which will return a null pointer on
                    // failure, as opposed to throwing an exception)
                    size_t count = data.size();
                    size_t bytes = count * sizeof(ElementType);
                    ElementType* ptr = static_cast<ElementType*>(redhawk::shm::allocate(bytes));
                    if (ptr) {
                        // Make a copy of the data into the new shared memory,
                        // ensuring it gets cleaned up appropriately
                        std::memcpy(ptr, data.data(), bytes);
                        copy = BufferType(ptr, count, redhawk::shm::deallocate);
                        shm_transfer = _transferBuffer(header, ptr, 0);
                    } else {
                        // Shared memory must be exhausted, fall back to using
                        // in-band transfer
                        shm_transfer = false;
                    }
                }

                // If we weren't able to transfer the buffer using shared
                // memory, set up to copy it via the FIFO
                if (!shm_transfer) {
                    header.write(true);
                    body = data.data();
                    body_size = data.size() * sizeof(data[0]);
                }
            }

            _sendMessage(header.buffer(), header.size(), body, body_size);

            ShmStatPoint stat(body_size == 0, !copy.empty());
            _recordExtendedStatistics(stat);
        }

        virtual redhawk::PropertyMap _getExtendedStatistics()
        {
            ShmStatPoint stats = std::accumulate(_extendedStats.begin(), _extendedStats.end(), ShmStatPoint());
            double copy_rate = 0.0;
            double shm_rate = 0.0;
            if (!_extendedStats.empty()) {
                copy_rate = stats.copied * 100.0 / _extendedStats.size();
                shm_rate = stats.shmTransfer * 100.0 / _extendedStats.size();
            }

            redhawk::PropertyMap statistics;
            statistics["shm::copy_rate"] = copy_rate;
            statistics["shm::shm_rate"] = shm_rate;
            return statistics;
        }

    private:
        void _recordExtendedStatistics(const ShmStatPoint& stat)
        {
            _extendedStats.push_back(stat);
            if (_extendedStats.size() > 10) {
                _extendedStats.pop_front();
            }
        }

        bool _transferBuffer(MessageBuffer& header, const void* base, size_t offset)
        {
            redhawk::shm::MemoryRef ref = redhawk::shm::Heap::getRef(base);
            if (!ref) {
                // The allocator was unable to use shared memory
                return false;
            }

            header.write(false);
            header.write(ref.heap);
            header.write(ref.superblock);
            header.write(ref.offset);
            header.write(offset);

            return true;
        }

        void _sendMessage(const void* header, size_t hsize, const void* body, size_t bsize)
        {
            // NOTE: This method needs to complete atomically (in that all
            // reads and writes must be completed) to avoid corruption in the
            // message stream. The FIFO class no longer supports thread
            // interruption for this reason, but thread interruption can be
            // disabled for the duration of this method if necessary.
            try {
                _fifo.write(&hsize, sizeof(hsize));
                _fifo.write(header, hsize);
                if (bsize > 0) {
                    _fifo.write(body, bsize);
                }
            } catch (const std::exception& exc) {
                throw redhawk::FatalTransportError(exc.what());
            }

            size_t status = 0;
            size_t count = 0;
            try {
                count = _fifo.read(&status, sizeof(size_t));
            } catch (const std::exception& exc) {
                throw redhawk::FatalTransportError(exc.what());
            }

            if (count != sizeof(size_t)) {
                throw redhawk::FatalTransportError("failed to read response");
            } else if (status != 0) {
                throw redhawk::TransportError("call failed");
            }
        }

        FifoEndpoint _fifo;

        std::deque<ShmStatPoint> _extendedStats;
    };

    template <typename PortType>
    ShmOutputManager<PortType>::ShmOutputManager(OutPort<PortType>* port) :
        OutputManager<PortType>(port)
    {
        char host[HOST_NAME_MAX+1];
        gethostname(host, sizeof(host));
        _hostname = host;
    }

    template <typename PortType>
    std::string ShmOutputManager<PortType>::transportType()
    {
        return "shmipc";
    }

    template <typename PortType>
    CF::Properties ShmOutputManager<PortType>::transportProperties()
    {
        CF::Properties properties;
        ossie::corba::push_back(properties, redhawk::PropertyType("hostname", _hostname));
        return properties;
    }

    template <typename PortType>
    OutputTransport<PortType>*
    ShmOutputManager<PortType>::createOutputTransport(PtrType object,
                                                      const std::string& connectionId,
                                                      const redhawk::PropertyMap& properties)
    {
        // For testing, allow disabling
        const char* shm_env = getenv("BULKIO_SHM");
        if (shm_env && (strcmp(shm_env, "disable") == 0)) {
            return 0;
        }

        // If the other end of the connection has a different hostname, it
        // is reasonable to assume that we cannot use shared memory
        if (properties.get("hostname", "").toString() != _hostname) {
            RH_NL_TRACE("ShmTransport", "Connection '" << connectionId << "' is on another host");
            return 0;
        }

        // Check whether shared memory is enabled--there may not be enough free
        // space to create the heap. The degraded send-via-FIFO mode is usually
        // slower CORBA.
        if (!redhawk::shm::isEnabled()) {
            RH_NL_DEBUG("ShmTransport", "Cannot create SHM transport, shared memory is not available");
            return 0;
        }

        return new ShmOutputTransport<PortType>(this->_port, object);
    }

    template <typename PortType>
    redhawk::PropertyMap ShmOutputManager<PortType>::getNegotiationProperties(redhawk::UsesTransport* transport)
    {
        TransportType* shm_transport = dynamic_cast<TransportType*>(transport);
        if (!shm_transport) {
            throw std::logic_error("invalid transport type");
        }

        redhawk::PropertyMap properties;
        properties["fifo"] = shm_transport->getFifoName();
        return properties;
    }

    template <typename PortType>
    void ShmOutputManager<PortType>::setNegotiationResult(redhawk::UsesTransport* transport,
                                                          const redhawk::PropertyMap& properties)
    {
        TransportType* shm_transport = dynamic_cast<TransportType*>(transport);
        if (!shm_transport) {
            throw std::logic_error("invalid transport type");
        }

        if (!properties.contains("fifo")) {
            throw redhawk::FatalTransportError("invalid properties for shared memory connection");
        }

        std::string fifo_name = properties["fifo"].toString();
        RH_NL_DEBUG("ShmTransport", "Connecting to provides port FIFO: " << fifo_name);
        shm_transport->finishConnect(fifo_name);
    }

#define INSTANTIATE_TEMPLATE(x)                 \
    template class ShmOutputTransport<x>;       \
    template class ShmOutputManager<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_TEMPLATE);
}
