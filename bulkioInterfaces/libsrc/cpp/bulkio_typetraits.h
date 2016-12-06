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

#include "BULKIO_Interfaces.h"

namespace bulkio {

    template <class PortType>
    struct CorbaTraits {
    };

#define DEFINE_CORBA_TRAITS(NAME,TT,ST) \
    template <>                         \
    struct CorbaTraits<NAME> {          \
        typedef POA_##NAME POAType;     \
        typedef TT TransportType;       \
        typedef ST SequenceType;        \
    };

    DEFINE_CORBA_TRAITS(BULKIO::dataChar, CORBA::Char, PortTypes::CharSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataOctet, CORBA::Octet, CF::OctetSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataShort, CORBA::Short, PortTypes::ShortSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataUshort, CORBA::UShort, PortTypes::UshortSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataLong, CORBA::Long, PortTypes::LongSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataUlong, CORBA::ULong, PortTypes::UlongSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataLongLong, CORBA::LongLong, PortTypes::LongLongSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataUlongLong, CORBA::ULongLong, PortTypes::UlongLongSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataFloat, CORBA::Float, PortTypes::FloatSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataDouble, CORBA::Double, PortTypes::DoubleSequence);
    DEFINE_CORBA_TRAITS(BULKIO::dataFile, char, char*);
    DEFINE_CORBA_TRAITS(BULKIO::dataXML, char, char*);

#undef DEFINE_CORBA_TRAITS

    template <typename PortType>
    struct NativeTraits {
        typedef typename CorbaTraits<PortType>::TransportType NativeType;
    };

    template <>
    struct NativeTraits<BULKIO::dataChar> {
        typedef int8_t NativeType;
    };

    template <typename PortType>
    struct BufferTraits {
        typedef typename NativeTraits<PortType>::NativeType NativeType;
        typedef std::vector<NativeType> VectorType;
        typedef redhawk::shared_buffer<NativeType> BufferType;
    };

    template <>
    struct BufferTraits<BULKIO::dataXML> {
        typedef std::string VectorType;
        typedef std::string BufferType;
    };

    template <>
    struct BufferTraits<BULKIO::dataFile> {
        typedef std::string VectorType;
        typedef std::string BufferType;
    };
}

#endif // __bulkio_typetraits_h
