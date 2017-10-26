#ifndef __bulkio_ShmInputTransport_h
#define __bulkio_ShmInputTransport_h

#include <boost/thread.hpp>

#include <ossie/shm/HeapClient.h>
#include <ossie/ProvidesPort.h>

#include <BulkioTransport.h>
#include <bulkio_in_port.h>

#include "FifoIPC.h"

namespace bulkio {

    class MessageBuffer;

    template <typename PortType>
    class ShmInputTransport : public InputTransport<PortType> {
    public:
        typedef InPort<PortType> InPortType;
        typedef typename NativeTraits<PortType>::NativeType NativeType;
        typedef typename BufferTraits<PortType>::BufferType BufferType;

        ShmInputTransport(InPortType* port, const std::string& transportId, const std::string& writePath);
        ~ShmInputTransport();

        virtual std::string transportType() const;

        virtual void startTransport();
        virtual void stopTransport();

        const std::string& getFifoName() const;

    protected:
        void _run();

        bool _isRunning();
        bool _receiveMessage();

        void _receivePushPacket(MessageBuffer& msg);

        volatile bool _running;
        boost::mutex _mutex;
        boost::thread _thread;
        FifoEndpoint _fifo;
        redhawk::shm::HeapClient _heapClient;
    };
}

#endif // __bulkio_ShmInputTransport_h
