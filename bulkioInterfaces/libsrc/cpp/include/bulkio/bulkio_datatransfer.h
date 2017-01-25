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

#ifndef __bulkio_datatransfer_h
#define __bulkio_datatransfer_h

#include <vector>

#include <ossie/PropertyMap.h>

#include <BULKIO/bulkioDataTypes.h>

namespace bulkio {

    //
    // DataTransfer
    //
    // This is the packet of information returned from an InPort's getPacket method.  The DataTransferTraits class
    // defines the type context for this structure. 
    //
    // This class tries to implement as efficient as possible data movement from the supplied PortSequenceType object.
    // The supplied PortSequenceType's data buffer is used to set the start/end/length attributes of the dataBuffer object that will
    // be used by the component.  This class takes ownership of the PortSequenceType's memory buffer and assigns it the
    // the dataBuffer's start address. The DataBufferType allows developers to use standard 
    // stl iterators and algorithms against the data in this buffer.
    //
    // All remaining member variables use each type's assignment/copy methods. It is assumed the
    // PrecisionUTCTime and StreamSRI object will perform a "deep" copy.
    // 
    //
    template <typename BufferType>
    struct DataTransfer {
        typedef BufferType DataBufferType;

        //
        // Construct a DataTransfer object to be returned from an InPort's getPacket method
        // 
        template <class Tin>
        DataTransfer(const Tin& data, const BULKIO::PrecisionUTCTime& T, bool EOS, const char* streamID,
                     const BULKIO::StreamSRI& H, bool sriChanged, bool inputQueueFlushed) :
            T(T),
            EOS(EOS),
            streamID(streamID),
            SRI(H),
            sriChanged(sriChanged),
            inputQueueFlushed(inputQueueFlushed)
        {
            assign(dataBuffer, data);
        }

        DataBufferType dataBuffer;
        BULKIO::PrecisionUTCTime T;
        bool EOS;
        std::string streamID;
        BULKIO::StreamSRI SRI;
        bool sriChanged;
        bool inputQueueFlushed;

        redhawk::PropertyMap& getKeywords()
        {
            return redhawk::PropertyMap::cast(SRI.keywords);
        }

        const redhawk::PropertyMap& getKeywords() const
        {
            return redhawk::PropertyMap::cast(SRI.keywords);
        }

    private:
        template <class Tout, class Alloc, class Tin>
        static void assign(std::vector<Tout,Alloc>& dest, const _CORBA_Sequence<Tin>& src)
        {
            if (src.release()) {
                _CORBA_Sequence<Tin>& in = const_cast<_CORBA_Sequence<Tin>&>(src);
                const size_t length = in.length();
                typedef typename std::_Vector_base<Tout, Alloc>::_Vector_impl* VectorPtr;
                VectorPtr vectorPtr = (VectorPtr)(&dest);
                vectorPtr->_M_start = reinterpret_cast<Tout*>(in.get_buffer(1));
                vectorPtr->_M_finish = vectorPtr->_M_start + length;
                vectorPtr->_M_end_of_storage = vectorPtr->_M_finish;
            } else {
                dest.assign(src.get_buffer(), src.get_buffer() + src.length());
            }
        }

    };

    template <>
    struct DataTransfer<std::string> {
        //
        // Construct a DataTransfer object to be returned from an InPort's getPacket method
        // 
        DataTransfer(const char* data, const BULKIO::PrecisionUTCTime& T, bool EOS, const char* streamID,
                     const BULKIO::StreamSRI& H, bool sriChanged, bool inputQueueFlushed) :
            T(T),
            EOS(EOS),
            streamID(streamID),
            SRI(H),
            sriChanged(sriChanged),
            inputQueueFlushed(inputQueueFlushed)
        {
            assign(dataBuffer, data);
        }

        DataTransfer(const char* data, bool EOS, const char* streamID, const BULKIO::StreamSRI& H,
                     bool sriChanged, bool inputQueueFlushed) :
            T(),
            EOS(EOS),
            streamID(streamID),
            SRI(H),
            sriChanged(sriChanged),
            inputQueueFlushed(inputQueueFlushed)
        {
            assign(dataBuffer, data);
        }

        std::string dataBuffer;
        BULKIO::PrecisionUTCTime T;
        bool EOS;
        std::string streamID;
        BULKIO::StreamSRI SRI;
        bool sriChanged;
        bool inputQueueFlushed;

        redhawk::PropertyMap& getKeywords()
        {
            return redhawk::PropertyMap::cast(SRI.keywords);
        }

        const redhawk::PropertyMap& getKeywords() const
        {
            return redhawk::PropertyMap::cast(SRI.keywords);
        }

    private:
        static void assign(std::string& dest, const char* src)
        {
            if (src) {
                dest = src;
            }
        }
    };


    typedef DataTransfer<std::vector<int8_t> > CharDataTransfer;
    typedef DataTransfer<std::vector<CORBA::Octet> > OctetDataTransfer;
    typedef DataTransfer<std::vector<CORBA::Short> > ShortDataTransfer;
    typedef DataTransfer<std::vector<CORBA::UShort> > UShortDataTransfer;
    typedef DataTransfer<std::vector<CORBA::Long> > LongDataTransfer;
    typedef DataTransfer<std::vector<CORBA::ULong> > ULongDataTransfer;
    typedef DataTransfer<std::vector<CORBA::LongLong> > LongLongDataTransfer;
    typedef DataTransfer<std::vector<CORBA::ULongLong> > ULongLongDataTransfer;
    typedef DataTransfer<std::vector<CORBA::Float> > FloatDataTransfer;
    typedef DataTransfer<std::vector<CORBA::Double> > DoubleDataTransfer;
    typedef DataTransfer<std::string> StringDataTransfer;
}

#endif // __bulkio_datatransfer_h
