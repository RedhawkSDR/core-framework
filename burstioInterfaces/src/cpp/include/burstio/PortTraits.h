/*
 * This file is protected by Copyright. Please refer to the COPYRIGHT file
 * distributed with this source distribution.
 *
 * This file is part of REDHAWK burstioInterfaces.
 *
 * REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */
#ifndef BURSTIO_PORTTRAITS_H
#define BURSTIO_PORTTRAITS_H

#include <BURSTIO/burstio_burstByte.h>
#include <BURSTIO/burstio_burstDouble.h>
#include <BURSTIO/burstio_burstFloat.h>
#include <BURSTIO/burstio_burstLongLong.h>
#include <BURSTIO/burstio_burstLong.h>
#include <BURSTIO/burstio_burstShort.h>
#include <BURSTIO/burstio_burstUbyte.h>
#include <BURSTIO/burstio_burstUlongLong.h>
#include <BURSTIO/burstio_burstUlong.h>
#include <BURSTIO/burstio_burstUshort.h>

namespace burstio {
    template <class Port, class POA, class Burst, class BurstSequence, class Element, class Sequence, class Native>
    struct PortTraits {
        typedef Port PortType;
        typedef POA POAType;
        typedef Burst BurstType;
        typedef BurstSequence BurstSequenceType;
        typedef Element ElementType;
        typedef Sequence SequenceType;
        typedef Native NativeType;
    };

#define DEFINE_PORTTRAITS(T, CT, ST, NT) \
    struct T##Traits : public PortTraits<BURSTIO::burst##T, POA_BURSTIO::burst##T, BURSTIO::T##Burst, BURSTIO::T##BurstSequence, CORBA::CT, ST, NT> { \
    };

    DEFINE_PORTTRAITS(Byte, Octet, CF::OctetSequence, signed char);
    DEFINE_PORTTRAITS(Double, Double, PortTypes::DoubleSequence, CORBA::Double);
    DEFINE_PORTTRAITS(Float, Float, PortTypes::FloatSequence, CORBA::Float);
    DEFINE_PORTTRAITS(Long, Long, PortTypes::LongSequence, CORBA::Long);
    DEFINE_PORTTRAITS(LongLong, LongLong, PortTypes::LongLongSequence, CORBA::LongLong);
    DEFINE_PORTTRAITS(Short, Short, PortTypes::ShortSequence, CORBA::Short);
    DEFINE_PORTTRAITS(Ubyte, Octet, CF::OctetSequence, unsigned char);
    DEFINE_PORTTRAITS(Ulong, ULong, PortTypes::UlongSequence, CORBA::ULong);
    DEFINE_PORTTRAITS(UlongLong, ULongLong, PortTypes::UlongLongSequence, CORBA::ULongLong);
    DEFINE_PORTTRAITS(Ushort, UShort, PortTypes::UshortSequence, CORBA::UShort);
#undef DEFINE_PORTTRAITS
}

#endif // BURSTIO_PORTTRAITS_H
