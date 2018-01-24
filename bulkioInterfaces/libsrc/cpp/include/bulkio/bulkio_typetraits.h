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

#ifndef __bulkio_typetraits_h
#define __bulkio_typetraits_h

#include <string>
#include <vector>

#include <ossie/shared_buffer.h>
#include <ossie/bitbuffer.h>

#include "BULKIO_Interfaces.h"
#include <ossie/BULKIO/internal/bio_dataExt.h>
#include "bulkio_base.h"

namespace bulkio {

    template <class PortType>
    struct CorbaTraits {
    };
    
#define DEFINE_CORBA_TRAITS(NAME,TT,ST)                         \
    template <>                                                 \
    struct CorbaTraits<BULKIO::NAME> {                          \
        typedef POA_BULKIO::NAME POAType;                       \
        typedef POA_BULKIO::internal::NAME##Ext POATypeExt;     \
        typedef TT TransportType;                               \
        typedef ST SequenceType;                                \
        static const char* name() { return #NAME; }             \
    };

    DEFINE_CORBA_TRAITS(dataChar, CORBA::Char, PortTypes::CharSequence);
    DEFINE_CORBA_TRAITS(dataOctet, CORBA::Octet, CF::OctetSequence);
    DEFINE_CORBA_TRAITS(dataShort, CORBA::Short, PortTypes::ShortSequence);
    DEFINE_CORBA_TRAITS(dataUshort, CORBA::UShort, PortTypes::UshortSequence);
    DEFINE_CORBA_TRAITS(dataLong, CORBA::Long, PortTypes::LongSequence);
    DEFINE_CORBA_TRAITS(dataUlong, CORBA::ULong, PortTypes::UlongSequence);
    DEFINE_CORBA_TRAITS(dataLongLong, CORBA::LongLong, PortTypes::LongLongSequence);
    DEFINE_CORBA_TRAITS(dataUlongLong, CORBA::ULongLong, PortTypes::UlongLongSequence);
    DEFINE_CORBA_TRAITS(dataFloat, CORBA::Float, PortTypes::FloatSequence);
    DEFINE_CORBA_TRAITS(dataDouble, CORBA::Double, PortTypes::DoubleSequence);
    DEFINE_CORBA_TRAITS(dataBit, CORBA::Octet, BULKIO::BitSequence);
    DEFINE_CORBA_TRAITS(dataFile, char, char*);
    DEFINE_CORBA_TRAITS(dataXML, char, char*);

#undef DEFINE_CORBA_TRAITS

    template <typename PortType>
    struct NativeTraits {
        typedef typename CorbaTraits<PortType>::TransportType NativeType;
        static const size_t bits = sizeof(NativeType) * 8;
    };

    template <>
    struct NativeTraits<BULKIO::dataChar> {
        typedef int8_t NativeType;
        static const size_t bits = 8;
    };

    template <>
    struct NativeTraits<BULKIO::dataBit> {
        typedef redhawk::shared_bitbuffer::data_type NativeType;
        static const size_t bits = 1;
    };

    template <typename PortType>
    struct BufferTraits {
        typedef typename NativeTraits<PortType>::NativeType NativeType;
        typedef std::vector<NativeType> VectorType;
        typedef redhawk::shared_buffer<NativeType> BufferType;
        typedef redhawk::buffer<NativeType> MutableBufferType;
    };

    template <>
    struct BufferTraits<BULKIO::dataBit> {
        typedef redhawk::shared_bitbuffer VectorType;
        typedef redhawk::shared_bitbuffer BufferType;
        typedef redhawk::bitbuffer MutableBufferType;
    };

    struct StringBufferTraits {
        typedef std::string VectorType;
        typedef std::string BufferType;
        typedef std::string MutableBufferType;
    };

    template <>
    struct BufferTraits<BULKIO::dataXML> : public StringBufferTraits {
    };

    template <>
    struct BufferTraits<BULKIO::dataFile> : public StringBufferTraits {
    };
}

#endif // __bulkio_typetraits_h
