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
#ifndef BURSTIO_INPORTIMPL_H
#define BURSTIO_INPORTIMPL_H

#include <stdexcept>

#include <burstio/BurstPacket.h>
#include <burstio/utils.h>

#include "debug_impl.h"

namespace burstio {
    template <class Traits>
    InPort<Traits>::InPort(std::string port_name) : 
        Port_Provides_base_impl(port_name),
        __logger(__classlogger),
        queueOffset_(0),
        queuedBursts_(0),
        queueThreshold_(DEFAULT_QUEUE_THRESHOLD),
        blockOccurred_(false),
        started_(false),
        statistics_(port_name, sizeof(ElementType)*8)
    {
    }

    template <class Traits>
    InPort<Traits>::~InPort()
    {
    }

    template <class Traits>
    void InPort<Traits>::setLogger (LoggerPtr logger)
    {
        __logger = logger;
    }


    template <class Traits>
    size_t InPort<Traits>::getQueueThreshold () const
    {
        return queueThreshold_;
    }

    template <class Traits>
    void InPort<Traits>::setQueueThreshold (size_t count)
    {
        if (count == 0) {
            throw std::invalid_argument("Queue threshold must be at least 1");
        }
        boost::mutex::scoped_lock lock(queueMutex_);
        if (count > queueThreshold_) {
            queueNotFull_.notify_all();
        }
        queueThreshold_ = count;
    }

    template <class Traits>
    void InPort<Traits>::start ()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        LOG_INSTANCE_TRACE("Port started");
        started_ = true;
    }

    template <class Traits>
    void InPort<Traits>::stop ()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        LOG_INSTANCE_TRACE("Port stopped");
        started_ = false;
        queueNotEmpty_.notify_all();
        queueNotFull_.notify_all();
    }

    template <class Traits>
    void InPort<Traits>::startPort ()
    {
        start();
    }

    template <class Traits>
    void InPort<Traits>::stopPort ()
    {
        stop();
    }

    template <class Traits>
    void InPort<Traits>::pushBursts(const InPort<Traits>::BurstSequenceType& bursts)
    {
        BULKIO::PrecisionUTCTime begin = burstio::utils::now();

        boost::mutex::scoped_lock lock(queueMutex_);

        // Calculate queue depth based on state at invocation; this makes it
        // easy to tell if a consumer is keeping up (average = 0, or at least
        // doesn't grow) or blocking (average >= 100).
        float queue_depth = queuedBursts_ / (float)queueThreshold_;

        // Only set the block flag once to avoid multiple notifications
        bool block_reported = false;

        // Wait until the queue is below the blocking threshold
        while (started_ && (queuedBursts_ >= queueThreshold_)) {
            // Report that this call blocked
            if (!block_reported) {
                block_reported = true;
                blockOccurred_ = true;
            }
            queueNotFull_.wait(lock);
        }

        // Discard bursts if processing is not started
        if (!started_) {
            LOG_INSTANCE_TRACE("Port is stopped, discarding " << bursts.length() << " bursts");
            return;
        }

        // Count total elements and active stream IDs
        const size_t total_bursts = bursts.length();
        size_t total_elements = 0;
        for (CORBA::ULong index = 0; index < total_bursts; ++index) {
            const BurstType& burst = bursts[index];
            total_elements += burst.data.length();
            if (!burst.EOS) {
                streamIDs_.insert(static_cast<const char*>(burst.SRI.streamID));
            }
        }

        // Add bursts to queue, if there are any
        if (total_bursts > 0) {
            LOG_INSTANCE_TRACE("Queueing " << total_bursts << " bursts");
            queue_.push_back(new BurstSequenceType());
            ossie::corba::move(queue_.back(), const_cast<BurstSequenceType&>(bursts));
            queuedBursts_ += total_bursts;
            queueNotEmpty_.notify_all();
        } else {
            LOG_INSTANCE_DEBUG("Push contained no bursts");
        }

        // Record total time spent in pushBursts for latency measurement
        double elapsed = burstio::utils::elapsed(begin);
        statistics_.record(total_bursts, total_elements, queue_depth, elapsed);
    }

    template <class Traits>
    typename InPort<Traits>::PacketType* InPort<Traits>::getBurst (float timeout)
    {
        boost::mutex::scoped_lock lock(queueMutex_);

        if (!waitBurst(timeout, lock)) {
            return 0;
        }
        LOG_INSTANCE_TRACE("Returning burst from queue");

        BurstSequenceType& front = queue_.front();
        BurstType& queue_burst = front[queueOffset_];

        PacketType* burst = new PacketType();
        burst->sri_ = queue_burst.SRI;
        burst->eos_ = queue_burst.EOS;
        burst->time_ = queue_burst.T;
        ossie::corba::move(burst->data_, queue_burst.data);
        burst->blockOccurred_ = blockOccurred_;
        // If a block had occurred, it has been reported now, so clear the flag
        blockOccurred_ = false;

        if (burst->getEOS()) {
            LOG_INSTANCE_TRACE("Received EOS for stream \"" << burst->getStreamID() << "\"");
            streamIDs_.erase(burst->getStreamID());
        }

        queueOffset_++;
        if (queueOffset_ == front.length()) {
            queue_.pop_front();
            queueOffset_ = 0;
        }

        queuedBursts_--;
        queueNotFull_.notify_all();

        return burst;
    }

    template <class Traits>
    typename InPort<Traits>::BurstSequenceType* InPort<Traits>::getBursts (float timeout)
    {
        boost::mutex::scoped_lock lock(queueMutex_);

        if (!waitBurst(timeout, lock)) {
            // Return an empty sequence instead of null; sequence var types do
            // not have any way to check whether they are null, so this allows
            // the user to handle the timeout (or stopped) case gracefully.
            return new BurstSequenceType();
        }

        BurstSequenceVar bursts; 
        if (queueOffset_ == 0) {
            LOG_INSTANCE_TRACE("Returning burst sequence from front of queue");
            bursts = queue_.front()._retn();
        } else {
            LOG_INSTANCE_TRACE("Copying remaining burst sequence from front of queue");
            BurstSequenceType& queue_bursts = queue_.front();
            const CORBA::ULong length = queue_bursts.length() - queueOffset_;
            bursts = new BurstSequenceType();
            bursts->length(length);
            BurstType* dest = &bursts[0];
            BurstType* const end = dest + length;
            for (BurstType* source = &queue_bursts[queueOffset_]; dest != end; ++dest, ++source) {
                dest->SRI = source->SRI;
                dest->T = source->T;
                dest->EOS = source->EOS;
                ossie::corba::move(dest->data, source->data);
            }
        }

        queue_.pop_front();
        queueOffset_ = 0;
        queuedBursts_ -= bursts->length();
        queueNotFull_.notify_all();

        return bursts._retn();
    }

    template <class Traits>
    bool InPort<Traits>::blockOccurred ()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        bool retval = blockOccurred_;
        blockOccurred_ = false;
        return retval;
    }

    template <class Traits>
    bool InPort<Traits>::waitBurst (float timeout, boost::mutex::scoped_lock& lock)
    {
        if (timeout > 0.0) {
            boost::posix_time::time_duration offset = boost::posix_time::microseconds(timeout*1e6);
            LOG_INSTANCE_TRACE("Waiting " << offset.total_microseconds() << " us for burst");
            boost::system_time to_time = boost::get_system_time() + offset;
            while (started_ && queuedBursts_ == 0) {
                if (!queueNotEmpty_.timed_wait(lock, to_time)) {
                    return false;
                }
            }
        } else if (timeout < 0.0) {
            LOG_INSTANCE_TRACE("Waiting indefinitely for burst");
            while (started_ && (queuedBursts_ == 0)) {
                queueNotEmpty_.wait(lock);
            }
        }
        return started_ && (queuedBursts_ > 0);
    }

    template <class Traits>
    BULKIO::PortUsageType InPort<Traits>::state()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        if (queuedBursts_ == 0) {
            return BULKIO::IDLE;
        } else if (queuedBursts_ >= queueThreshold_) {
            return BULKIO::BUSY;
        } else {
            return BULKIO::ACTIVE;
        }
    }

    template <class Traits>
    BULKIO::PortStatistics* InPort<Traits>::statistics()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        BULKIO::PortStatistics_var stats = statistics_.retrieve();
        for (std::set<std::string>::iterator ii = streamIDs_.begin(); ii != streamIDs_.end(); ++ii) {
            burstio::utils::push_back(stats->streamIDs, ii->c_str());
        }
        return stats._retn();
    }

    template <class Traits>
    size_t InPort<Traits>::getQueueDepth ()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        return queuedBursts_;
    }

    template <class Traits>
    void InPort<Traits>::flush ()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        if (!queue_.empty()) {
            statistics_.flushOccurred(queuedBursts_);
            queue_.clear();
            queueOffset_ = 0;
            queuedBursts_ = 0;
            queueNotFull_.notify_all();
        }
    }
}

#define INSTANTIATE_TEMPLATE(traits, name) \
    PREPARE_CLASS_LOGGING(name);           \
    template class InPort<traits>;

#endif // BURSTIO_INPORTIMPL_H
