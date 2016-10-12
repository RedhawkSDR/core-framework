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
#include <numeric>

#include <burstio/BurstStatistics.h>
#include <burstio/utils.h>

using namespace burstio;

StatPoint::StatPoint() :
    timestamp(burstio::utils::now()),
    bursts(0),
    elements(0),
    queueDepth(0.0f),
    delay(0.0f)
{
}

StatPoint& StatPoint::operator+= (const StatPoint& rhs)
{
    bursts += rhs.bursts;
    elements += rhs.elements;
    queueDepth += rhs.queueDepth;
    delay += rhs.delay;
    return *this;
}

ReceiverStatPoint::ReceiverStatPoint() :
    StatPoint(),
    flushes(0),
    dropped(0)
{
}

ReceiverStatPoint& ReceiverStatPoint::operator+= (const ReceiverStatPoint& rhs)
{
    StatPoint::operator+=(rhs);
    flushes += rhs.flushes;
    dropped += rhs.dropped;
    return *this;
}

namespace burstio {
    const StatPoint operator+ (const StatPoint& lhs, const StatPoint& rhs)
    {
        return StatPoint(lhs)+=rhs;
    }

    const ReceiverStatPoint operator+ (const ReceiverStatPoint& lhs, const ReceiverStatPoint& rhs)
    {
        return ReceiverStatPoint(lhs)+=rhs;
    }
}

template <class StatType>
BurstStatistics<StatType>::BurstStatistics(const std::string& name, size_t bitsPerElement) :
    name_(name),
    bitsPerElement_(bitsPerElement),
    windowSize_(10)
{
}

template <class StatType>
BurstStatistics<StatType>::~BurstStatistics()
{
}

template <class StatType>
void BurstStatistics<StatType>::record (size_t bursts, size_t elements, float queueDepth, float delay)
{
    stat_type point;
    point.bursts = bursts;
    point.elements = elements;
    point.queueDepth = queueDepth;
    point.delay = delay;

    statistics_.push_back(point);
    if (statistics_.size() > windowSize_) {
        statistics_.pop_front();
    }
}

template <class StatType>
BULKIO::PortStatistics* BurstStatistics<StatType>::retrieve () const
{
    BULKIO::PortStatistics_var stats = new BULKIO::PortStatistics();

    size_t count = statistics_.size();
    stat_type totals = std::accumulate(statistics_.begin(), statistics_.end(), stat_type());

    stats->portName = name_.c_str();
    stats->streamIDs.length(0); // To be filled in by caller

    double bursts_per_second = 0.0;
    double bursts_per_push = 0.0;
    double elements_per_burst = 0.0;
    double average_latency = 0.0;

    if (count > 0) {
        const stat_type& first = statistics_.front();
        double elapsed = burstio::utils::elapsed(first.timestamp);

        stats->elementsPerSecond = totals.elements / elapsed;
        stats->bitsPerSecond = stats->elementsPerSecond * bitsPerElement_;
        stats->callsPerSecond = count / elapsed;
        stats->averageQueueDepth = totals.queueDepth / count;

        const stat_type& last = statistics_.back();
        stats->timeSinceLastCall = burstio::utils::elapsed(last.timestamp);

        bursts_per_second = totals.bursts / elapsed;
        bursts_per_push = totals.bursts / (double)count;
        elements_per_burst = totals.elements / (double)totals.bursts;
        average_latency = totals.delay / count;
    } else {
        stats->elementsPerSecond = 0.0;
        stats->bitsPerSecond = 0.0;
        stats->callsPerSecond = 0.0;
        stats->averageQueueDepth = 0.0;
        stats->timeSinceLastCall = 0.0;
    }

    // Add burst-specific stats to keywords
    burstio::utils::addKeyword(stats->keywords, "BURSTS_PER_SECOND", bursts_per_second);
    burstio::utils::addKeyword(stats->keywords, "BURSTS_PER_PUSH", bursts_per_push);
    burstio::utils::addKeyword(stats->keywords, "ELEMENTS_PER_BURST", elements_per_burst);
    burstio::utils::addKeyword(stats->keywords, "AVERAGE_LATENCY", average_latency);
    addKeywords_(totals, count, stats->keywords);

    return stats._retn();
}

template <class StatType>
void BurstStatistics<StatType>::addKeywords_ (const StatType&, size_t, _CORBA_Sequence<CF::DataType>&) const
{
}

template class BurstStatistics<StatPoint>;
template class BurstStatistics<ReceiverStatPoint>;

SenderStatistics::SenderStatistics(const std::string& name, size_t bitsPerElement) :
    super(name, bitsPerElement)
{
}

ReceiverStatistics::ReceiverStatistics(const std::string& name, size_t bitsPerElement) :
    super(name, bitsPerElement),
    flushCount_(0),
    burstsDropped_(0)
{
}

void ReceiverStatistics::flushOccurred (size_t bursts)
{
    ReceiverStatPoint& back = statistics_.back();
    back.flushes++;
    back.dropped += bursts;

    flushCount_++;
    burstsDropped_ += bursts;
}

void ReceiverStatistics::addKeywords_ (const ReceiverStatPoint& totals, size_t, _CORBA_Sequence<CF::DataType>& keywords) const
{
    burstio::utils::addKeyword(keywords, "QUEUE_FLUSHES", (CORBA::ULongLong)totals.flushes);
    double drop_rate = 0.0;
    if (totals.bursts > 0) {
        drop_rate = totals.dropped / (double)totals.bursts;
    }
    burstio::utils::addKeyword(keywords, "DROPPED_RATIO", drop_rate);
    if (flushCount_ > 0) {
        burstio::utils::addKeyword(keywords, "FLUSH_COUNT", flushCount_);
        burstio::utils::addKeyword(keywords, "BURSTS_DROPPED", burstsDropped_);
    }
}
