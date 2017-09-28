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
            size_t offset;
            size_t count;
            msg.read(ref.heap);
            msg.read(ref.superblock);
            msg.read(ref.offset);
            msg.read(offset);
            msg.read(count);
            msg.read(T);
            msg.read(EOS);
            msg.read(streamID);
            if (msg.offset() < msg.size()) {
                std::cerr << "Message bytes left over" << std::endl;
            }

            redhawk::shm::HeapClient* heap = _getHeap(ref.heap);

            NativeType* base = reinterpret_cast<NativeType*>(heap->fetch(ref));
            redhawk::buffer<NativeType> buffer(base, count, &redhawk::shm::HeapClient::deallocate);
            this->_queuePacket(buffer, T, EOS, streamID);

            // Send response back
            size_t status = 0;
            _fifo->write(&status, sizeof(size_t));
        }
    }

    template <class PortType>
    redhawk::shm::HeapClient* ShmIngressThread<PortType>::_getHeap(const std::string& name)
    {
        typename HeapMap::iterator existing = _heaps.find(name);
        if (existing != _heaps.end()) {
            return existing->second;
        }

        redhawk::shm::HeapClient* heap = new redhawk::shm::HeapClient(name);
        _heaps[name] = heap;
        return heap;
    }

#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    template class IngressThread<x>;    \
    template class ShmIngressThread<x>;

    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
}
