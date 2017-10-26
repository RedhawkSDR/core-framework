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
#ifndef __bulkio_localtransport_h
#define __bulkio_localtransport_h

#include <BulkioTransport.h>

namespace bulkio {

    template <typename PortType>
    class LocalTransport : public OutputTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename OutputTransport<PortType>::BufferType BufferType;
        typedef InPort<PortType> LocalPortType;

        static LocalTransport* Factory(OutPort<PortType>* parent, PortBase* port);

        LocalTransport(OutPort<PortType>* parent, LocalPortType* localPort, PtrType port);
        ~LocalTransport();

        virtual std::string transportType() const;
        virtual CF::Properties transportInfo() const;

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri);
        virtual void _pushPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID);

        LocalPortType* _localPort;
    };
}

#endif // __bulkio_localtransport_h
