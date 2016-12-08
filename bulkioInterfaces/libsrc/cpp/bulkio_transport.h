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
#ifndef __bulkio_transport_h
#define __bulkio_transport_h

#include <ossie/UsesPort.h>

#include "bulkio_base.h"
#include "bulkio_typetraits.h"

namespace bulkio {

    template <typename PortType>
    class PortTransport : public redhawk::BasicTransport
    {
    public:
        typedef typename PortType::_var_type VarType;
        typedef typename PortType::_ptr_type PtrType;
        typedef typename NativeTraits<PortType>::NativeType NativeType;
        typedef typename BufferTraits<PortType>::BufferType BufferType;

        static PortTransport* Factory(const std::string& connectionId, const std::string& name, PtrType port);

        PortTransport(const std::string& connectionId, const std::string& name, PtrType objref);

        virtual ~PortTransport();

        virtual void disconnect();

        void pushSRI(const std::string& streamID, const BULKIO::StreamSRI& sri, int version);

        void pushPacket(const BufferType& data,
                        const BULKIO::PrecisionUTCTime& T,
                        bool EOS,
                        const std::string& streamID,
                        const BULKIO::StreamSRI& sri);

        PtrType port();

        linkStatistics stats;

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri) = 0;

        virtual void _sendPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID,
                                 const BULKIO::StreamSRI& sri);

        virtual void _pushPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID) = 0;

        //
        // Returns the total number of elements of data in a pushPacket call, for
        // statistical tracking; enables XML and File specialization, which have
        // different notions of size
        //
        size_t _dataLength(const BufferType& data);

        VarType _port;
        typedef std::map<std::string,int> VersionMap;
        VersionMap _sriVersions;
    };

}

#endif // __bulkio_transport_h
