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

#include "ShmTransport.h"
#include "ipcfifo.h"
#include "MessageBuffer.h"

#include "bulkio_p.h"

namespace bulkio {

    template <typename PortType>
    class ShmTransport : public PortTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortTransport<PortType>::BufferType BufferType;
        typedef typename CorbaTraits<PortType>::TransportType TransportType;

        ShmTransport(const std::string& connectionId, const std::string& name, IPCFifo* fifo, PtrType port) :
            PortTransport<PortType>(connectionId, name, port),
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

        virtual std::string transportType() const
        {
            return "shmipc";
        }

        virtual CF::Properties transportInfo() const
        {
            return CF::Properties();
        }

        virtual void disconnect()
        {
            PortTransport<PortType>::disconnect();
            _fifo->close();
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
            MessageBuffer msg;
            msg.write(data.size());

            // Temporary buffer to ensure that if a copy is made, it gets
            // released after the transfer
            BufferType copy;

            // If the packet is non-empty, write the additional shared memory
            // information for the remote side to pick up
            if (!data.empty()) {
                const void* base;
                size_t offset;

                // Check that the buffer is already in shared memory (this is
                // hoped to be the common case); if not, copy it into another
                // buffer that is allocated in shared memory
                if (data.is_process_shared()) {
                    base = data.base();
                    offset = reinterpret_cast<size_t>(data.data()) - reinterpret_cast<size_t>(base);
                } else {
                    copy = data.copy(redhawk::shm::Allocator<typename BufferType::value_type>());
                    base = copy.base();
                    offset = 0;
                }

                redhawk::shm::MemoryRef ref = redhawk::shm::Heap::getRef(base);
                msg.write(ref.heap);
                msg.write(ref.superblock);
                msg.write(ref.offset);
                msg.write(offset);
            }

            msg.write(T);
            msg.write(EOS);
            msg.write(streamID);

            try {
                _fifo->write(msg.buffer(), msg.size());
            } catch (const std::exception& exc) {
                throw redhawk::FatalTransportError(exc.what());
            }

            size_t status;
            if (_fifo->read(&status, sizeof(size_t)) != sizeof(size_t)) {
                RH_NL_ERROR("ShmTransport", "Bad response");
            }
        }

    private:
        IPCFifo* _fifo;
    };

    template <typename PortType>
    PortTransport<PortType>*
    ShmTransportFactory<PortType>::Create(const std::string& connectionId,
                                          const std::string& name,
                                          ExtendedCF::NegotiableProvidesPort_ptr negotiablePort,
                                          const redhawk::PropertyMap& transportProps,
                                          PtrType port)
    {
        // For testing, allow disabling
        const char* shm_env = getenv("BULKIO_SHM");
        if (shm_env) {
            if (strcmp(shm_env, "disable") == 0) {
                return 0;
            }
        }

        // If the other end of the connection has a different hostname, it
        // is reasonable to assume that we cannot use shared memory
        char host[HOST_NAME_MAX+1];
        gethostname(host, sizeof(host));
        const std::string hostname(host);
        if (transportProps.get("hostname", "").toString() != hostname) {
            RH_NL_TRACE("ShmTransport", "Connection '" << connectionId << "' is on another host");
            return 0;
        }

        RH_NL_DEBUG("ShmTransport", "Attempting to negotiate shared memory IPC");
        IPCFifo* fifo = new IPCFifoServer(name + "-fifo");
        redhawk::PropertyMap props;
        props["fifo"] = fifo->name();
        fifo->beginConnect();
        ExtendedCF::NegotiationResult_var result;
        try {
            result = negotiablePort->negotiateTransport("shmipc", props);
        } catch (const ExtendedCF::NegotiationError& exc) {
            RH_NL_ERROR("ShmTransport", "Error negotiating shared memory IPC: " << exc.msg);
            delete fifo;
            return 0;
        }
        fifo->finishConnect();

        return new ShmTransport<PortType>(connectionId, name, fifo, port);
    }

#define INSTANTIATE_TEMPLATE(x)                 \
    template class ShmTransport<x>;             \
    template class ShmTransportFactory<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_TEMPLATE);
}
