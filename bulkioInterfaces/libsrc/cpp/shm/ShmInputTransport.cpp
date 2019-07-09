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

#include "ShmInputTransport.h"
#include "FifoIPC.h"
#include "MessageBuffer.h"

#include <boost/thread.hpp>

#include <ossie/shm/Heap.h>

#include <BulkioTransport.h>
#include <bulkio_in_port.h>

#include "bulkio_p.h"

namespace bulkio {

    template <class PortType>
    class ShmInputTransport : public InputTransport<PortType>
    {
    public:
        typedef ShmInputManager<PortType> ManagerType;
        typedef typename NativeTraits<PortType>::NativeType NativeType;
        typedef typename BufferTraits<PortType>::BufferType BufferType;

        ShmInputTransport(InPort<PortType>* port, const std::string& transportId,
                          ManagerType* manager, const std::string& writePath) :
            InputTransport<PortType>(port, transportId),
            _manager(manager),
            _running(false),
            _fifo()
        {
            _fifo.connect(writePath);
        }

        ~ShmInputTransport()
        {
            _fifo.disconnect();
        }

        std::string transportType() const
        {
            return "shmipc";
        }

        void startTransport()
        {
            _running = true;
            _thread = boost::thread(&ShmInputTransport::_run, this);
        }

        void stopTransport()
        {
            {
                boost::mutex::scoped_lock lock(_mutex);
                if (!_running) {
                    return;
                }

                _running = false;
                _thread.interrupt();
            }
            _thread.join();
        }

        const std::string& getFifoName() const
        {
            return _fifo.name();
        }

    protected:
        bool _isRunning()
        {
            boost::mutex::scoped_lock lock(_mutex);
            return _running;
        }

        void _run()
        {
            // Give the FIFO up to a second to sychronize with the other
            // side. This method is being run on a thread that gets started
            // when the transport is negotiated, so the uses side may take a
            // moment to receive the result and connect on its end.
            try {
                _fifo.sync(1000);
            } catch (const std::exception& exc) {
                RH_NL_ERROR("ShmTransport", "Synchronization failed on BulkIO input transport: " << exc.what());
                return;
            }

            while (_isRunning()) {
                if (!_receiveMessage()) {
                    return;
                }
            }
        }

        bool _receiveMessage()
        {
            size_t msg_length;
            if (_fifo.read(&msg_length, sizeof(msg_length)) != sizeof(msg_length)) {
                return false;
            }

            MessageBuffer msg(msg_length);
            if (_fifo.read(msg.buffer(), msg.size()) != msg_length){
                return false;
            }

            std::string message_name;
            try {
                msg.read(message_name);
                if (message_name == "pushPacket") {
                    _receivePushPacket(msg);
                } else {
                    // Unknown message type, send error response back
                    throw std::logic_error("invalid message type");
                }
            } catch (const std::exception& exc) {
                RH_NL_ERROR("ShmTransport", "Error handling message '" << message_name << "': " << exc.what());
                size_t status = 1;
                _fifo.write(&status, sizeof(size_t));
            }

            return true;
        }

        void _receivePushPacket(MessageBuffer& msg)
        {
            size_t count;
            msg.read(count);

            BULKIO::PrecisionUTCTime T;
            msg.read(T);

            bool EOS;
            msg.read(EOS);

            std::string streamID;
            msg.read(streamID);

            BufferType buffer;
            if (count > 0) {
                bool inband_data;
                msg.read(inband_data);

                if (inband_data) {
                    redhawk::buffer<NativeType> temp(count);
                    _fifo.read(temp.data(), temp.size() * sizeof(NativeType));
                    buffer = temp;
                } else {
                    _receiveSharedBuffer(msg, buffer, count);
                }
            }

            if (msg.offset() < msg.size()) {
                std::cerr << "Message bytes left over" << std::endl;
            }

            this->_queuePacket(buffer, T, EOS, streamID);

            // Send response back
            size_t status = 0;
            _fifo.write(&status, sizeof(size_t));
        }

        void _receiveSharedBuffer(MessageBuffer& msg, BufferType& buffer, size_t size)
        {
            redhawk::shm::MemoryRef ref;
            msg.read(ref.heap);
            msg.read(ref.superblock);
            msg.read(ref.offset);

            size_t offset;
            msg.read(offset);

            void* base = _manager->fetchShmRef(ref);

            // Find the first element, which may be offset from the base
            // pointer.  If so, start with a larger buffer and then trim the
            // elements off of the front.
            size_t start = offset / sizeof(NativeType);
            if ((start * sizeof(NativeType)) != offset) {
                // The starting element is not aligned from the base, which
                // will require some additional care to adjust. Start with a
                // char buffer, trim that to the correct starting point, and
                // then recast to the desired type.
                char* ptr = reinterpret_cast<char*>(base);
                size_t bytes = offset + size * sizeof(NativeType);
                redhawk::shared_buffer<char> temp(ptr, bytes, &redhawk::shm::HeapClient::deallocate, redhawk::detail::process_shared_tag());
                temp.trim(offset);

                buffer = BufferType::recast(temp);
            } else {
                // Normal alignment, include the start offset (if any) in the
                // initial buffer size, then trim to the desired start.
                NativeType* ptr = reinterpret_cast<NativeType*>(base);
                size += start;
                buffer = BufferType(ptr, size, &redhawk::shm::HeapClient::deallocate, redhawk::detail::process_shared_tag());
                if (start > 0) {
                    buffer.trim(start);
                }
            }
        }

        ManagerType* _manager;
        volatile bool _running;
        boost::mutex _mutex;
        boost::thread _thread;
        FifoEndpoint _fifo;
    };

    template <class PortType>
    ShmInputManager<PortType>::ShmInputManager(InPort<PortType>* port) :
        InputManager<PortType>(port)
    {
    }

    template <class PortType>
    std::string ShmInputManager<PortType>::transportType()
    {
        return "shmipc";
    }

    template <class PortType>
    CF::Properties ShmInputManager<PortType>::transportProperties()
    {
        CF::Properties properties;

        char host[HOST_NAME_MAX+1];
        gethostname(host, sizeof(host));

        ossie::corba::push_back(properties, redhawk::PropertyType("hostname", std::string(host)));
        return properties;
    }

    template <class PortType>
    InputTransport<PortType>* ShmInputManager<PortType>::createInputTransport(const std::string& transportId,
                                                                              const redhawk::PropertyMap& properties)
    {
        if (!properties.contains("fifo")) {
            throw redhawk::FatalTransportError("invalid properties for shared memory connection");
        }
        const std::string location = properties["fifo"].toString();
        try {
            return new ShmInputTransport<PortType>(this->_port, transportId, this, location);
        } catch (const std::exception& exc) {
            throw redhawk::FatalTransportError("failed to connect to FIFO " + location);
        }
    }

    template <class PortType>
    redhawk::PropertyMap ShmInputManager<PortType>::getNegotiationProperties(redhawk::ProvidesTransport* providesTransport)
    {
        InputTransportType* transport = dynamic_cast<InputTransportType*>(providesTransport);
        if (!transport) {
            throw std::logic_error("invalid provides transport instance");
        }
        redhawk::PropertyMap properties;
        properties["fifo"] = transport->getFifoName();
        return properties;
    }

    template <class PortType>
    void* ShmInputManager<PortType>::fetchShmRef(const redhawk::shm::MemoryRef& ref)
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _heapClient.fetch(ref);
    }

#define INSTANTIATE_NUMERIC_TEMPLATE(x)         \
    template class ShmInputTransport<x>;        \
    template class ShmInputManager<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
