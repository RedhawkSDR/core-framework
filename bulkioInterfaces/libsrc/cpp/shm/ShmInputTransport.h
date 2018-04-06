/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK bulkioInterfaces.
 *
 * REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

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

        void _receiveSharedBuffer(MessageBuffer& msg, BufferType& buffer, size_t size);

        volatile bool _running;
        boost::mutex _mutex;
        boost::thread _thread;
        FifoEndpoint _fifo;
        redhawk::shm::HeapClient _heapClient;
    };
}

#endif // __bulkio_ShmInputTransport_h
