#include "ingress_thread.h"
#include "ipcfifo.h"
#include "MessageBuffer.h"
#include "bulkio_p.h"

namespace bulkio {

    template <class PortType>
    ShmIngressThread<PortType>::ShmIngressThread(InPortType* port, IPCFifo* fifo) :
        IngressThread<PortType>(port),
        _fifo(fifo)
    {
    }

    template <class PortType>
    ShmIngressThread<PortType>::~ShmIngressThread()
    {
        _fifo->close();
        delete _fifo;
    }

    template <class PortType>
    void ShmIngressThread<PortType>::_threadStarted()
    {
        _fifo->finishConnect();
    }

    template <class PortType>
    void ShmIngressThread<PortType>::_run()
    {
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

                NativeType* base = reinterpret_cast<NativeType*>(_heapClient.fetch(ref));
                buffer = redhawk::make_buffer(base, count, &redhawk::shm::HeapClient::deallocate);
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
    template class IngressThread<x>;    \
    template class ShmIngressThread<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
