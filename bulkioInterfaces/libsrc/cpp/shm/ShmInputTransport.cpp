#include "ShmInputTransport.h"
#include "ipcfifo.h"
#include "MessageBuffer.h"
#include "bulkio_p.h"

namespace bulkio {

    template <class PortType>
    ShmInputTransport<PortType>::ShmInputTransport(InPortType* port, const std::string& transportId, IPCFifo* fifo) :
        InputTransport<PortType>(port, transportId),
        _running(false),
        _fifo(fifo)
    {
    }

    template <class PortType>
    ShmInputTransport<PortType>::~ShmInputTransport()
    {
        _fifo->close();
        delete _fifo;

        // The HeapClient is automatically detached by its destructor, ensuring
        // that any heap(s) it was attached to can be cleaned up
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
    bool ShmInputTransport<PortType>::_isRunning()
    {
        boost::mutex::scoped_lock lock(_mutex);
        return _running;
    }

    template <class PortType>
    void ShmInputTransport<PortType>::_run()
    {
        _fifo->finishConnect();

        while (_isRunning()) {
            if (!_receiveMessage()) {
                return;
            }
        }
    }

    template <class PortType>
    bool ShmInputTransport<PortType>::_receiveMessage()
    {
        BULKIO::PrecisionUTCTime T;
        bool EOS;
        std::string streamID;

        MessageBuffer msg(512);
        size_t msg_length = _fifo->read(msg.buffer(), msg.size());
        if (msg_length == 0) {
            return false;
        }
        msg.resize(msg_length);

        redhawk::buffer<NativeType> buffer;
        size_t count;
        msg.read(count);
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
                redhawk::buffer<char> temp(ptr, bytes, &redhawk::shm::HeapClient::deallocate);
                temp.trim(offset);

                buffer = redhawk::buffer<NativeType>::recast(temp);
            } else {
                // Normal alignment, include the start offset (if any) in the
                // initial buffer size, then trim to the desired start.
                NativeType* ptr = reinterpret_cast<NativeType*>(base);
                count += start;
                buffer = redhawk::make_buffer(ptr, count, &redhawk::shm::HeapClient::deallocate);
                if (start > 0) {
                    buffer.trim(start);
                }
            }
        }
        msg.read(T);
        msg.read(EOS);
        msg.read(streamID);
        if (msg.offset() < msg.size()) {
            std::cerr << "Message bytes left over" << std::endl;
        }

        this->_queuePacket(buffer, T, EOS, streamID);

        // Send response back
        size_t status = 0;
        _fifo->write(&status, sizeof(size_t));

        return true;
    }


#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    template class ShmInputTransport<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
