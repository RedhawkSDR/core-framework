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

#include "CorbaTransport.h"

#include <ossie/CorbaUtils.h>

#include "bulkio_time_operators.h"
#include "bulkio_p.h"

namespace bulkio {

    template <typename PortType>
    class CorbaTransport : public OutputTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename PortType::_var_type VarType;
        typedef typename OutputTransport<PortType>::BufferType BufferType;
        typedef typename CorbaTraits<PortType>::SequenceType SequenceType;
        typedef typename CorbaTraits<PortType>::TransportType TransportType;

        CorbaTransport(OutPort<PortType>* parent, PtrType port) :
            OutputTransport<PortType>(parent, port)
        {
        }

        virtual std::string transportType() const
        {
            return "CORBA";
        }

        virtual CF::Properties transportInfo() const
        {
            return CF::Properties();
        }

        void disconnect()
        {
            // Set a timeout for the duration of this call, in case the remote
            // side is in a bad state.
            omniORB::setClientCallTimeout(this->_objref, 1000);
            OutputTransport<PortType>::disconnect();
            omniORB::setClientCallTimeout(this->_objref, 0);
        }

    protected:
        virtual void _pushSRI(const BULKIO::StreamSRI& sri)
        {
            try {
                this->_objref->pushSRI(sri);
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::FatalTransportError(ossie::corba::describeException(exc));
            }
        }

        virtual void _pushPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID)
        {
            try {
                _pushPacketImpl(data, T, EOS, streamID.c_str());
            } catch (const CORBA::TIMEOUT& exc) {
                throw redhawk::TransportTimeoutError("Push timed out");
            } catch (const CORBA::SystemException& exc) {
                throw redhawk::FatalTransportError(ossie::corba::describeException(exc));
            }
        }

    private:
        void _pushPacketImpl(const BufferType& data,
                             const BULKIO::PrecisionUTCTime& T,
                             bool EOS,
                             const char* streamID)
        {
            const TransportType* ptr = reinterpret_cast<const TransportType*>(data.data());
            const SequenceType buffer(data.size(), data.size(), const_cast<TransportType*>(ptr), false);
            this->_objref->pushPacket(buffer, T, EOS, streamID);
        }
    };

    template <>
    void CorbaTransport<BULKIO::dataBit>::_pushPacketImpl(const redhawk::shared_bitbuffer& data,
                                                          const BULKIO::PrecisionUTCTime& T,
                                                          bool EOS,
                                                          const char* streamID)
    {
        BULKIO::BitSequence buffer;
        size_t bytes = (data.size() + 7) / 8;
        const CORBA::Octet* ptr;
        redhawk::bitbuffer temp;
        if (data.offset() == 0) {
            // Bit buffer is byte-aligned, so it can be directly wrapped with a
            // non-owning CORBA sequence
            ptr = reinterpret_cast<const CORBA::Octet*>(data.data());
        } else {
            // Not byte-aligned, copy bits into a temporary buffer and use that
            // as the data for the CORBA sequence
            temp = data.copy();
            ptr = reinterpret_cast<const CORBA::Octet*>(temp.data());
        }
        buffer.data.replace(bytes, bytes, const_cast<CORBA::Octet*>(ptr), false);
        buffer.bits = data.size();
        _objref->pushPacket(buffer, T, EOS, streamID);
    }

    template <>
    void CorbaTransport<BULKIO::dataFile>::_pushPacketImpl(const std::string& data,
                                                            const BULKIO::PrecisionUTCTime& T,
                                                            bool EOS,
                                                            const char* streamID)
    {
        _objref->pushPacket(data.c_str(), T, EOS, streamID);
    }

    template <>
    void CorbaTransport<BULKIO::dataXML>::_pushPacketImpl(const std::string& data,
                                                           const BULKIO::PrecisionUTCTime& /* unused */,
                                                           bool EOS,
                                                           const char* streamID)
    {
        _objref->pushPacket(data.c_str(), EOS, streamID);
    }

    template <typename PortType>
    class ChunkingTransport : public CorbaTransport<PortType>
    {
    public:
        typedef typename PortType::_ptr_type PtrType;
        typedef typename OutputTransport<PortType>::BufferType BufferType;
        typedef typename CorbaTraits<PortType>::TransportType TransportType;

        ChunkingTransport(OutPort<PortType>* parent, PtrType port) :
            CorbaTransport<PortType>(parent, port),
            maxSamplesPerPush(_getMaxSamplesPerPush())
        {
        }

        /*
         * Push a packet whose payload may not fit within the CORBA limit. The
         * packet is broken down into sub-packets and sent via multiple pushPacket
         * calls.  The EOS is set to false for all of the sub-packets, except for
         * the last sub-packet, which uses the input EOS argument.
         */
        virtual void _sendPacket(const BufferType& data,
                                 const BULKIO::PrecisionUTCTime& T,
                                 bool EOS,
                                 const std::string& streamID,
                                 const BULKIO::StreamSRI& sri)
        {
            double xdelta = sri.xdelta;
            size_t itemSize = sri.mode?2:1;
            size_t frameSize = itemSize;
            if (sri.subsize > 0) {
                frameSize *= sri.subsize;
            }
            // Quantize the push size (in terms of scalars) to the nearest frame,
            // which takes both the complex mode and subsize into account
            const size_t maxPushSize = (maxSamplesPerPush/frameSize) * frameSize;

            // Always do at least one push (may be empty), ensuring that all samples
            // are pushed
            size_t first = 0;
            size_t samplesRemaining = data.size();

            // Initialize time of first subpacket
            BULKIO::PrecisionUTCTime packetTime = T;
      
            do {
                // Don't send more samples than are remaining
                const size_t pushSize = std::min(samplesRemaining, maxPushSize);
                samplesRemaining -= pushSize;

                // Send end-of-stream as false for all sub-packets except for the
                // last one (when there are no samples remaining after this push),
                // which gets the input EOS.
                bool packetEOS = false;
                if (samplesRemaining == 0) {
                    packetEOS = EOS;
                }

                // Take the next slice of the input buffer.
                BufferType subPacket = data.slice(first, first + pushSize);
                CorbaTransport<PortType>::_sendPacket(subPacket, packetTime, packetEOS, streamID, sri);

                // Synthesize the next packet timestamp
                if (packetTime.tcstatus == BULKIO::TCS_VALID) {
                    packetTime += (pushSize/itemSize)* xdelta;
                }

                // Advance buffer to next sub-packet boundary
                first += pushSize;
            } while (samplesRemaining > 0);
        }

    private:
        static inline size_t _getMaxSamplesPerPush()
        {
            // Take the maximum transfer size in bytes, multiply by some number
            // < 1 to leave some margin for the CORBA header, then determine
            // the maximum number of elements via bits, to support numeric data
            // data types (e.g., float) and packed bits.
            const size_t max_bits = (size_t) (bulkio::Const::MaxTransferBytes() * .9) * 8;
            return max_bits / NativeTraits<PortType>::bits;
        }

        const size_t maxSamplesPerPush;
    };

    template <typename PortType>
    OutputTransport<PortType>* CorbaTransportFactory<PortType>::Create(OutPort<PortType>* parent,
                                                                       PtrType port)
    {
        return new ChunkingTransport<PortType>(parent, port);
    }

    template <>
    OutputTransport<BULKIO::dataFile>*
    CorbaTransportFactory<BULKIO::dataFile>::Create(OutPort<BULKIO::dataFile>* parent,
                                                    PtrType port)
    {
        return new CorbaTransport<BULKIO::dataFile>(parent, port);
    }

    template <>
    OutputTransport<BULKIO::dataXML>*
    CorbaTransportFactory<BULKIO::dataXML>::Create(OutPort<BULKIO::dataXML>* parent,
                                                   PtrType port)
    {
        return new CorbaTransport<BULKIO::dataXML>(parent, port);
    }

#define INSTANTIATE_TEMPLATE(x)                \
    template class CorbaTransport<x>;          \
    template class CorbaTransportFactory<x>;

#define INSTANTIATE_NUMERIC_TEMPLATE(x)         \
    template class ChunkingTransport<x>;

    FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
    FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
    INSTANTIATE_NUMERIC_TEMPLATE(BULKIO::dataBit);
}
