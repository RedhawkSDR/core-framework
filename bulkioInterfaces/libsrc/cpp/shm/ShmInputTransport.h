#ifndef __bulkio_ShmInputTransport_h
#define __bulkio_ShmInputTransport_h

#include <boost/thread.hpp>

#include <ossie/shm/HeapClient.h>
#include <ossie/ProvidesPort.h>

#include <BulkioTransport.h>
#include <bulkio_in_port.h>

#include "ipcfifo.h"

namespace bulkio {

    template <typename PortType>
    class ShmInputTransport : public InputTransport<PortType> {
    public:
        typedef InPort<PortType> InPortType;
        typedef typename NativeTraits<PortType>::NativeType NativeType;

        ShmInputTransport(InPortType* port, const std::string& transportId, IPCFifo* fifo);
        ~ShmInputTransport();

        void startTransport();
        void stopTransport();

    protected:
        void _run();

        bool _isRunning();
        bool _receiveMessage();

        volatile bool _running;
        boost::mutex _mutex;
        boost::thread _thread;
        IPCFifo* _fifo;
        redhawk::shm::HeapClient _heapClient;
    };
}

#endif // __bulkio_ShmInputTransport_h
