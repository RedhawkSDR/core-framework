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
#ifndef BURSTIO_BURSTPACKET_H
#define BURSTIO_BURSTPACKET_H

#include <complex>

#include <boost/make_shared.hpp>

#include "PortTraits.h"

namespace burstio {
    template <class Traits>
    class BurstPacket {
    public:
        typedef typename Traits::BurstType BurstType;
        typedef typename Traits::NativeType NativeType;
        typedef typename Traits::SequenceType SequenceType;

        typedef std::complex<NativeType> ComplexType;

        // Returns the stream ID of this burst.
        inline std::string getStreamID() const {
            return std::string(sri_.streamID);
        }

        // Returns the number of scalar elements in the burst data. If the
        // burst data is complex (i.e., isComplex() is true), the number of
        // complex pairs is half of this value.
        inline size_t getSize() const {
            return data_.length();
        }

        // Returns a view of the burst data as a pointer to the native C++
        // type (e.g., short*).
        inline NativeType* getData() {
            return reinterpret_cast<NativeType*>(data_.get_buffer());
        }

        // Returns true if the burst data is complex, false otherwise.
        inline bool isComplex() const {
            return sri_.mode != 0;
        }

        // Returns a view of the burst data as a pointer to complex pairs of
        // the native C++ type (e.g., std::complex<short>).
        inline ComplexType* getComplexData() {
            return reinterpret_cast<ComplexType*>(data_.get_buffer());
        }

        // Returns true if this was the last burst in the stream.
        inline bool getEOS() const {
            return eos_;
        }

        // Get the timestamp for this burst.
        inline BULKIO::PrecisionUTCTime& getTime() {
            return time_;
        }

        // Get the SRI for this burst.
        inline BURSTIO::BurstSRI& getSRI() {
            return sri_;
        }

        // Returns true if a pushBursts call blocked since the last burst was
        // retrieved.
        inline bool blockOccurred() const {
            return blockOccurred_;
        }

        // Get the CORBA sequence containing the burst data.
        SequenceType& getSequence() {
            return data_;
        }

    private:
        BurstPacket()
        {
        }

        friend class InPort<Traits>;

        bool eos_;
        BURSTIO::BurstSRI sri_;
        BULKIO::PrecisionUTCTime time_;
        SequenceType data_;
        bool blockOccurred_;
    };

    typedef BurstPacket<ByteTraits>      BytePacket;
    typedef BurstPacket<DoubleTraits>    DoublePacket;
    typedef BurstPacket<FloatTraits>     FloatPacket;
    typedef BurstPacket<LongTraits>      LongPacket;
    typedef BurstPacket<LongLongTraits>  LongLongPacket;
    typedef BurstPacket<ShortTraits>     ShortPacket;
    typedef BurstPacket<UbyteTraits>     UbytePacket;
    typedef BurstPacket<UshortTraits>    UshortPacket;
    typedef BurstPacket<UlongTraits>     UlongPacket;
    typedef BurstPacket<UlongLongTraits> UlongLongPacket;
}

#endif // BURSTIO_BURSTPACKET_H
