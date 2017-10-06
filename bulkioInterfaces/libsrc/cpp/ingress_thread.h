#ifndef __bulkio_ingressthread_h
#define __bulkio_ingressthread_h

#include <boost/thread.hpp>

#include <ossie/shm/HeapClient.h>
#include <ossie/ProvidesPort.h>

#include <bulkio_in_port.h>

#include "ipcfifo.h"

namespace bulkio {

    template <typename PortType>
    class IngressThread : public redhawk::ProvidesTransport {
    public:
        typedef InPort<PortType> InPortType;
        
        IngressThread(InPortType* port) :
            _inPort(port)
        {
        }

        virtual ~IngressThread()
        {
        }

        void start()
        {
            _thread = boost::thread(&IngressThread::run, this);
        }

        void stop()
        {
            // TODO
        }

    protected:
        void run() {
            _threadStarted();

            _run();
        }

        typedef typename InPortType::BufferType BufferType;

        inline void _queuePacket(const BufferType& data, const BULKIO::PrecisionUTCTime& T, bool eos, const std::string& streamID)
        {
          _inPort->queuePacket(data, T, eos, streamID);
        }

        virtual void _threadStarted() { }
        virtual void _run() = 0;

        InPortType* _inPort;
        boost::thread _thread;
    };

    template <typename PortType>
    class ShmIngressThread : public IngressThread<PortType> {
    public:
        typedef InPort<PortType> InPortType;
        typedef IngressThread<PortType> Base;
        typedef typename NativeTraits<PortType>::NativeType NativeType;

        ShmIngressThread(InPortType* port, IPCFifo* fifo);
        ~ShmIngressThread();

    protected:
        virtual void _threadStarted();
        virtual void _run();

        IPCFifo* _fifo;
        redhawk::shm::HeapClient _heapClient;
    };
}

#endif // __bulkio_ingressthread_h
