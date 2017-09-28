#include "ingress_thread.h"
#include "ipcfifo.h"
#include "MessageBuffer.h"
#include "bulkio_p.h"

namespace bulkio {

    template <class PortType>
    ShmIngressThread<PortType>::ShmIngressThread(InPortType* port, IPCFifo* fifo) :
        IngressThread<PortType>(port),
        _fifo(fifo),
        _heap(0)
    {
    }

    template <class PortType>
    ShmIngressThread<PortType>::~ShmIngressThread()
    {
        _fifo->close();
        delete _fifo;

        delete _heap;
    }

    template <class PortType>
    void ShmIngressThread<PortType>::_threadStarted()
    {
        _fifo->finishConnect();

        MessageBuffer msg(512);
        size_t msg_length = _fifo->read(msg.buffer(), msg.size());
        msg.resize(msg_length);

        std::string heap_name;
        msg.read(heap_name);

        _heap = new redhawk::shm::HeapClient(heap_name);
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

            redhawk::shm::Heap::ID id;
            size_t offset;
            size_t count;
            msg.read(id);
            msg.read(offset);
            msg.read(count);
            msg.read(T);
            msg.read(EOS);
            msg.read(streamID);
            if (msg.offset() < msg.size()) {
                std::cerr << "Message bytes left over" << std::endl;
            }

            NativeType* base = reinterpret_cast<NativeType*>(_heap->fetch(id));
            redhawk::buffer<NativeType> buffer(base, count, &redhawk::shm::HeapClient::deallocate);
            this->_queuePacket(buffer, T, EOS, streamID);

            // Send response back
            size_t status = 0;
            _fifo->write(&status, sizeof(size_t));
        }
    }

#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    template class IngressThread<x>;    \
    template class ShmIngressThread<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
