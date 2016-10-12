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
#ifndef BURSTIO_BURSTSTATISTICS_H
#define BURSTIO_BURSTSTATISTICS_H

#include <string>
#include <list>

#include <BULKIO/bio_runtimeStats.h>

namespace burstio {

    struct StatPoint
    {
        StatPoint();

        BULKIO::PrecisionUTCTime timestamp;

        size_t bursts;
        size_t elements;
        float queueDepth;
        float delay;

        StatPoint& operator+= (const StatPoint&);
    };

    template <class StatType>
    class BurstStatistics
    {
    public:
        BurstStatistics(const std::string& name, size_t bitsPerElement);
        ~BurstStatistics();

        void record (size_t bursts, size_t elements, float queueDepth, float delay);

        BULKIO::PortStatistics* retrieve() const;

    protected:
        typedef StatType stat_type;

        virtual void addKeywords_ (const stat_type& totals, size_t count,
                                   _CORBA_Sequence<CF::DataType>& keywords) const;

        const std::string name_;
        std::list<stat_type> statistics_;
        size_t bitsPerElement_;
        size_t windowSize_;
    };

    class SenderStatistics : public BurstStatistics<StatPoint>
    {
    public:
        SenderStatistics(const std::string& name, size_t bitsPerElement);

    private:
        typedef BurstStatistics<StatPoint> super;
    };

    struct ReceiverStatPoint : public StatPoint
    {
        ReceiverStatPoint();

        size_t flushes;
        size_t dropped;

        ReceiverStatPoint& operator+= (const ReceiverStatPoint&);
    };

    class ReceiverStatistics : public BurstStatistics<ReceiverStatPoint>
    {
    public:
        ReceiverStatistics(const std::string& name, size_t bitsPerElement);

        void flushOccurred(size_t bursts);

    private:
        typedef BurstStatistics<ReceiverStatPoint> super;

        virtual void addKeywords_ (const ReceiverStatPoint& totals, size_t count,
                                   _CORBA_Sequence<CF::DataType>& keywords) const;

        CORBA::ULong flushCount_;
        CORBA::ULong burstsDropped_;
    };
}

#endif // BURSTIO_BURSTSTATISTICS_H
