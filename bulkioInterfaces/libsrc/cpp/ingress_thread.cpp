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
            size_t count;
            void* base;
            void* data;
            msg.read(base);
            msg.read(data);
            msg.read(count);
            msg.read(T);
            msg.read(EOS);
            msg.read(streamID);
            if (msg.offset() < msg.size()) {
                std::cerr << "Message bytes left over" << std::endl;
            }

            this->_queuePacket(redhawk::buffer<NativeType>(count), T, EOS, streamID);

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
