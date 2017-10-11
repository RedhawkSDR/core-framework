#include "ShmInputTransport.h"
#include "ipcfifo.h"
#include "MessageBuffer.h"
#include "bulkio_p.h"

namespace bulkio {

    template <class PortType>
    ShmInputTransport<PortType>::ShmInputTransport(InPortType* port, IPCFifo* fifo) :
        InputTransport<PortType>(port),
        _fifo(fifo)
    {
    }

    template <class PortType>
    ShmInputTransport<PortType>::~ShmInputTransport()
    {
        _fifo->close();
        delete _fifo;
    }

    template <class PortType>
    void ShmInputTransport<PortType>::start()
    {
        _thread = boost::thread(&ShmInputTransport::_run, this);
    }

    template <class PortType>
    void ShmInputTransport<PortType>::stop()
    {
        // TODO: stop input thread
    }

    template <class PortType>
    void ShmInputTransport<PortType>::_run()
    {
        _fifo->finishConnect();

        while (true) {
            BULKIO::PrecisionUTCTime T;
            bool EOS;
            std::string streamID;

            MessageBuffer msg(512);
            size_t msg_length = _fifo->read(msg.buffer(), msg.size());
            if (msg_length == 0) {
                break;
            }
            msg.resize(msg_length);

            redhawk::shm::MemoryRef ref;
            redhawk::buffer<NativeType> buffer;
            size_t offset;
            size_t count;
            msg.read(count);
            if (count > 0) {
                msg.read(ref.heap);
                msg.read(ref.superblock);
                msg.read(ref.offset);
                msg.read(offset);

                void* base = _heapClient.fetch(ref);

                // Find the first element, which may be offset from the base
                // pointer. If so, start with a larger buffer and then trim the
                // elements off of the front.
                size_t start = offset / sizeof(NativeType);
                if ((start * sizeof(NativeType)) != offset) {
                    // The starting element is not aligned from the base, which
                    // will require some additional care to adjust. Start with
                    // a char buffer, trim that to the correct starting point,
                    // and then recast to the desired type.
                    char* ptr = reinterpret_cast<char*>(base);
                    size_t bytes = offset + count * sizeof(NativeType);
                    redhawk::buffer<char> temp(ptr, bytes, &redhawk::shm::HeapClient::deallocate);
                    temp.trim(offset);

                    buffer = redhawk::buffer<NativeType>::recast(temp);
                } else {
                    // Normal alignment, include the start offset (if any) in
                    // the initial buffer size, then trim to the desired start.
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
        }

        // TODO: The ingress thread object doesn't get deleted; detaching the
        //       HeapClient ensures the heap(s) can be cleaned up (for now)
        _heapClient.detach();
    }

#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    template class ShmInputTransport<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
