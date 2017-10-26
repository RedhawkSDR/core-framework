#include "ShmInputTransport.h"
#include "MessageBuffer.h"
#include "bulkio_p.h"

namespace bulkio {

    template <class PortType>
    ShmInputTransport<PortType>::ShmInputTransport(InPortType* port, const std::string& transportId,
                                                   const std::string& writePath) :
        InputTransport<PortType>(port, transportId),
        _running(false),
        _fifo()
    {
        _fifo.connect(writePath);
    }

    template <class PortType>
    ShmInputTransport<PortType>::~ShmInputTransport()
    {
        _fifo.disconnect();

        // The HeapClient is automatically detached by its destructor, ensuring
        // that any heap(s) it was attached to can be cleaned up
    }

    template <class PortType>
    std::string ShmInputTransport<PortType>::transportType() const
    {
        return "shmipc";
    }

    template <class PortType>
    void ShmInputTransport<PortType>::startTransport()
    {
        _running = true;
        _thread = boost::thread(&ShmInputTransport::_run, this);
    }

    template <class PortType>
    void ShmInputTransport<PortType>::stopTransport()
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

    template <class PortType>
    const std::string& ShmInputTransport<PortType>::getFifoName() const
    {
        return _fifo.name();
    }

    template <class PortType>
    bool ShmInputTransport<PortType>::_isRunning()
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _running;
    }

    template <class PortType>
    void ShmInputTransport<PortType>::_run()
    {
        // Give the FIFO up to a second to sychronize with the other side. This
        // method is being run on a thread that gets started when the transport
        // is negotiated, so the uses side may take a moment to receive the
        // result and connect on its end.
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

    template <class PortType>
    bool ShmInputTransport<PortType>::_receiveMessage()
    {
        // FIFOs allow up to 16K in a single write, though BulkIO messages are
        // not that large at present.
        MessageBuffer msg(16384);
        size_t msg_length = _fifo.read(msg.buffer(), msg.size());
        if (msg_length == 0) {
            return false;
        }
        msg.resize(msg_length);

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

    template <class PortType>
    void ShmInputTransport<PortType>::_receivePushPacket(MessageBuffer& msg)
    {
        size_t count;
        msg.read(count);

        BufferType buffer;
        if (count > 0) {
            redhawk::shm::MemoryRef ref;
            msg.read(ref.heap);
            msg.read(ref.superblock);
            msg.read(ref.offset);

            size_t offset;
            msg.read(offset);

            void* base = _heapClient.fetch(ref);

            // Find the first element, which may be offset from the base
            // pointer. If so, start with a larger buffer and then trim the
            // elements off of the front.
            size_t start = offset / sizeof(NativeType);
            if ((start * sizeof(NativeType)) != offset) {
                // The starting element is not aligned from the base, which
                // will require some additional care to adjust. Start with a
                // char buffer, trim that to the correct starting point, and
                // then recast to the desired type.
                char* ptr = reinterpret_cast<char*>(base);
                size_t bytes = offset + count * sizeof(NativeType);
                redhawk::shared_buffer<char> temp(ptr, bytes, &redhawk::shm::HeapClient::deallocate, redhawk::detail::process_shared_tag());
                temp.trim(offset);

                buffer = BufferType::recast(temp);
            } else {
                // Normal alignment, include the start offset (if any) in the
                // initial buffer size, then trim to the desired start.
                NativeType* ptr = reinterpret_cast<NativeType*>(base);
                count += start;
                buffer = BufferType(ptr, count, &redhawk::shm::HeapClient::deallocate, redhawk::detail::process_shared_tag());
                if (start > 0) {
                    buffer.trim(start);
                }
            }
        }

        BULKIO::PrecisionUTCTime T;
        msg.read(T);

        bool EOS;
        msg.read(EOS);

        std::string streamID;
        msg.read(streamID);
        if (msg.offset() < msg.size()) {
            std::cerr << "Message bytes left over" << std::endl;
        }

        this->_queuePacket(buffer, T, EOS, streamID);

        // Send response back
        size_t status = 0;
        _fifo.write(&status, sizeof(size_t));
    }


#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    template class ShmInputTransport<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
