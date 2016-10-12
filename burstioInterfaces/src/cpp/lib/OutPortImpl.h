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
#ifndef BURSTIO_OUTPORTIMPL_H
#define BURSTIO_OUTPORTIMPL_H

#include <burstio/utils.h>

#include "debug_impl.h"

namespace burstio {

    template <class Traits>
    OutPort<Traits>::Queue::Queue(OutPort<Traits>* port, const std::string& streamID, size_t maxBursts, size_t thresholdBytes, long thresholdLatency) :
        port_(port),
        __logger(port->__logger),
        maxBursts_(maxBursts),
        thresholdBytes_(thresholdBytes),
        thresholdLatency_(boost::posix_time::microseconds(thresholdLatency)),
        bytes_(0),
        streamID_(streamID)
    {
    }

    template <class Traits>
    size_t OutPort<Traits>::Queue::getMaxBursts () const
    {
        boost::mutex::scoped_lock lock(mutex_);
        return maxBursts_;
    }

    template <class Traits>
    void OutPort<Traits>::Queue::setMaxBursts (size_t count)
    {
        boost::mutex::scoped_lock lock(mutex_);
        maxBursts_ = count;
        if (bursts_.length() >= maxBursts_) {
            LOG_INSTANCE_DEBUG("New max bursts " << maxBursts_ << " triggering push");
            executeThreadedFlush();
        }
    }

    template <class Traits>
    size_t OutPort<Traits>::Queue::getByteThreshold () const
    {
        boost::mutex::scoped_lock lock(mutex_);
        return thresholdBytes_;
    }

    template <class Traits>
    void OutPort<Traits>::Queue::setByteThreshold (size_t bytes)
    {
        boost::mutex::scoped_lock lock(mutex_);
        thresholdBytes_ = bytes;
        if (bytes_ >= thresholdBytes_) {
            LOG_INSTANCE_DEBUG("New byte threshold " << thresholdBytes_ << " triggering push");
            executeThreadedFlush();
        }
    }

    template <class Traits>
    long OutPort<Traits>::Queue::getLatencyThreshold () const
    {
        boost::mutex::scoped_lock lock(mutex_);
        return thresholdLatency_.total_microseconds();
    }

    template <class Traits>
    void OutPort<Traits>::Queue::setLatencyThreshold (long usec)
    {
        boost::mutex::scoped_lock lock(mutex_);
        thresholdLatency_ = boost::posix_time::microseconds(usec);
        if (bursts_.length() > 0) {
            port_->scheduleCheck(startTime_ + thresholdLatency_);
        }
    }

    template <class Traits>
    void OutPort<Traits>::Queue::queueBurst (SequenceType& data, const BURSTIO::BurstSRI& sri,
                                         const BULKIO::PrecisionUTCTime& timestamp, bool eos,
                                         bool isComplex)
    {
        boost::mutex::scoped_lock lock(mutex_);
        // If this is the first burst, mark the time for latency guarantees
        if (bursts_.length() == 0) {
            startTime_ = boost::get_system_time();
            LOG_INSTANCE_TRACE("Scheduling latency check on monitor thread after " << thresholdLatency_.total_microseconds() << " usec");
            port_->scheduleCheck(startTime_ + thresholdLatency_);
        }

        const CORBA::ULong index = bursts_.length();
        bursts_.length(index+1);
        BurstType& burst = bursts_[index];
        burst.SRI = sri;
        burst.SRI.mode = isComplex?1:0;
        burst.T = timestamp;
        ossie::corba::move(burst.data, data);
        burst.EOS = eos;

        bytes_ += burst.data.length() * sizeof(ElementType);
        LOG_INSTANCE_TRACE("Queue size: " << bursts_.length() << " bursts / " << bytes_ << " bytes");

        if (shouldFlush()) {
            LOG_INSTANCE_DEBUG("Queued burst exceeded threshold, flushing queue");
            sendBursts_();
        }
    }


    template <class Traits>
    void OutPort<Traits>::Queue::flush ()
    {
        boost::mutex::scoped_lock lock(mutex_);
        sendBursts_();
    }

    template <class Traits>
    bool OutPort<Traits>::Queue::shouldFlush ()
    {
        if (bursts_.length() >= maxBursts_) {
            return true;
        } else if (bytes_ >= thresholdBytes_) {
            return true;
        } else if (boost::get_system_time() >= (startTime_ + thresholdLatency_)) {
            return true;
        }
        return false;
    }

    template <class Traits>
    void OutPort<Traits>::Queue::executeThreadedFlush ()
    {
        port_->monitor_.execute(&OutPort<Traits>::Queue::flush, this);
    }

    template <class Traits>
    void OutPort<Traits>::Queue::checkFlush ()
    {
        boost::mutex::scoped_lock lock(mutex_);
        if (shouldFlush()) {
            sendBursts_();
        }
    }

    template <class Traits>
    void OutPort<Traits>::Queue::sendBursts_ ()
    {
        if (bursts_.length() > 0) {
            port_->sendBursts(bursts_, startTime_, bursts_.length()/(float)maxBursts_, streamID_);
            bursts_.length(0);
            bytes_ = 0;
            startTime_ = boost::posix_time::ptime();
        }
    }

    template <class Traits>
    OutPort<Traits>::OutPort(std::string port_name) :
        super(port_name),
        __logger(__classlogger),
        defaultQueue_(this, "(default)", DEFAULT_MAX_BURSTS, omniORB::giopMaxMsgSize() * 0.9, DEFAULT_LATENCY_THRESHOLD),
        streamQueues_(),
        routingMode_(ROUTE_ALL_INTERLEAVED)
    {
    }

    template <class Traits>
    OutPort<Traits>::~OutPort()
    {
        if (ROUTE_ALL_INTERLEAVED != routingMode_) {
            // Only delete unique stream queues
            for (typename QueueMap::iterator queue = streamQueues_.begin(); queue != streamQueues_.end(); ++queue) {
                delete queue->second;
            }
        }
    }

    template <class Traits>
    void OutPort<Traits>::setLogger (LoggerPtr logger)
    {
        __logger = logger;
    }

    template <class Traits>
    size_t OutPort<Traits>::getMaxBursts () const
    {
        return getDefaultPolicy()->getMaxBursts();
    }

    template <class Traits>
    void OutPort<Traits>::setMaxBursts (size_t count)
    {
        getDefaultPolicy()->setMaxBursts(count);
    }

    template <class Traits>
    size_t OutPort<Traits>::getByteThreshold () const
    {
        return getDefaultPolicy()->getByteThreshold();
    }

    template <class Traits>
    void OutPort<Traits>::setByteThreshold (size_t bytes)
    {
        getDefaultPolicy()->setByteThreshold(bytes);
    }

    template <class Traits>
    long OutPort<Traits>::getLatencyThreshold () const
    {
        return getDefaultPolicy()->getLatencyThreshold();
    }

    template <class Traits>
    void OutPort<Traits>::setLatencyThreshold (long usec)
    {
        getDefaultPolicy()->setLatencyThreshold(usec);
    }

    template <class Traits>
    void OutPort<Traits>::setRoutingMode (RoutingModeType mode)
    {
        routingMode_ = mode;
    }

    template <class Traits>
    OutputPolicy* OutPort<Traits>::getDefaultPolicy ()
    {
        return &defaultQueue_;
    }

    template <class Traits>
    const OutputPolicy* OutPort<Traits>::getDefaultPolicy () const
    {
        return &defaultQueue_;
    }

    template <class Traits>
    OutputPolicy* OutPort<Traits>::getStreamPolicy (const std::string& streamID)
    {
        return &getQueueForStream(streamID);
    }

    template <class Traits>
    const OutputPolicy* OutPort<Traits>::getStreamPolicy (const std::string& streamID) const
    {
        return &getQueueForStream(streamID);
    }

    template <class Traits>
    void OutPort<Traits>::addConnectionFilter (const std::string& streamID, const std::string& connectionID)
    {
        LOG_INSTANCE_DEBUG("Routing stream " << streamID << " to connection " << connectionID);
        boost::mutex::scoped_lock lock(updatingPortsLock);
        routes_[streamID].insert(connectionID);
    }

    template <class Traits>
    void OutPort<Traits>::removeConnectionFilter (const std::string& streamID, const std::string& connectionID)
    {
        LOG_INSTANCE_DEBUG("Unrouting stream " << streamID << " from connection " << connectionID);
        boost::mutex::scoped_lock lock(updatingPortsLock);
        RouteTable::iterator route = routes_.find(streamID);
        if (route != routes_.end()) {
            route->second.erase(connectionID);
        }
    }

    template <class Traits>
    void OutPort<Traits>::start ()
    {
        monitor_.start();
    }

    template <class Traits>
    void OutPort<Traits>::stop ()
    {
        // Stop the monitor thread and remove any queued checks, as the queue(s)
        // will be flushed anyway
        monitor_.stop();
        monitor_.clear();

        flush();
    }

    template <class Traits>
    void OutPort<Traits>::startPort ()
    {
        start();
    }

    template <class Traits>
    void OutPort<Traits>::stopPort ()
    {
        stop();
    }

	template <class Traits>
    std::string OutPort<Traits>::getRepid () const
    {
        return PortType::_PD_repoId;;
    }

    template <class Traits>
    BULKIO::PortUsageType OutPort<Traits>::state ()
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);
        if (connections_.empty()) {
            return BULKIO::IDLE;
        } else {
            return BULKIO::ACTIVE;
        }
    }

    template <class Traits>
    void OutPort<Traits>::pushBurst (SequenceType& data, const BURSTIO::BurstSRI& sri,
                                     const BULKIO::PrecisionUTCTime& timestamp, bool eos)
    {
        this->queueBurst(data, sri, timestamp, eos, sri.mode != 0);
    }

    template <class Traits>
    void OutPort<Traits>::pushBurst (SequenceType& data, const BURSTIO::BurstSRI& sri, bool eos)
    {
        this->pushBurst(data, sri, burstio::utils::now(), eos);
    }

    template <class Traits>
    void OutPort<Traits>::pushBurst (BurstType& burst)
    {
        this->queueBurst(burst.data, burst.SRI, burst.T, burst.EOS, burst.SRI.mode != 0);
    }

    template <class Traits>
    void OutPort<Traits>::pushBursts(const BurstSequenceType& bursts)
    {
        this->sendBursts(bursts, boost::get_system_time(), 0.0f, std::string());
    }

    template <class Traits>
    void OutPort<Traits>::sendBursts(const BurstSequenceType& bursts, boost::system_time startTime, float queueDepth, const std::string& streamID)
    {
        //LOG_INSTANCE_DEBUG("Sending " << bursts.length() << " bursts");

        // Count up total elements
        size_t total_elements = 0;
        for (CORBA::ULong index = 0; index < bursts.length(); ++index) {
            total_elements += bursts[index].data.length();
        }

        boost::mutex::scoped_lock lock(updatingPortsLock);   // don't want to process while command information is coming in
        for (typename ConnectionMap::iterator ii = connections_.begin(); ii != connections_.end(); ++ii) {
            const std::string& connectionId = ii->first;
            Connection& connection = ii->second;

            if (!isStreamRoutedToConnection(streamID, connectionId)) {
                continue;
            }

            // Record statistics
            boost::posix_time::time_duration delay = boost::get_system_time() - startTime;

            try {
                connection.port->pushBursts(bursts);
                connection.info->alive = true;
                connection.info->stats.record(bursts.length(), total_elements, queueDepth, delay.total_microseconds() * 1e-6);
            } catch (const std::exception& ex) {
                if (connection.info->alive) {
                    LOG_INSTANCE_ERROR("pushBursts to " << ii->first << " failed: " << ex.what());
                }
                connection.info->alive = false;
            } catch (const CORBA::MARSHAL& ex) {
                if (bursts.length() == 1) {
                    // the burst length is 1. There's nothing we can do
                    if (connection.info->alive) {
                        LOG_INSTANCE_ERROR("pushBursts to " << ii->first << " failed because the burst size is too long");
                    }
                    connection.info->alive = false;
                } else {
                    try {
                        partitionBursts(bursts, startTime, queueDepth, streamID, connection);
                    } catch (...) {
                        if (connection.info->alive) {
                            LOG_INSTANCE_ERROR("Paritioned pushBursts to " << ii->first << " failed");
                        }
                        connection.info->alive = false;
                    }
                }
            } catch (const CORBA::Exception& ex) {
                if (connection.info->alive) {
                    LOG_INSTANCE_ERROR("pushBursts to " << ii->first << " failed: CORBA::" << ex._name());
                }
                connection.info->alive = false;
            } catch (...) {
                if (connection.info->alive) {
                    LOG_INSTANCE_ERROR("pushBursts to " << ii->first << " failed");
                }
                connection.info->alive = false;
            }
        }
    }
    
    template <class Traits>
    void OutPort<Traits>::partitionBursts(const BurstSequenceType& bursts, boost::system_time startTime, float queueDepth, const std::string& streamID, const Connection& connection)
    {
        // cut the burst length in half and try again....
        BurstSequenceType first_burst;
        BurstSequenceType second_burst;
        first_burst.length(bursts.length()/2);
        second_burst.length(bursts.length()-first_burst.length());
        boost::posix_time::time_duration delay = boost::get_system_time() - startTime;
        for (int i=0; i<bursts.length(); i++) {
            if (i<first_burst.length()) {
                first_burst[i] = bursts[i];
            } else {
                second_burst[i-first_burst.length()] = bursts[i];
            }
        }
        try {
            size_t total_elements = 0;
            for (CORBA::ULong index = 0; index < first_burst.length(); ++index) {
                total_elements += first_burst[index].data.length();
            }
            connection.port->pushBursts(first_burst);
            connection.info->alive = true;
            connection.info->stats.record(first_burst.length(), total_elements, queueDepth, delay.total_microseconds() * 1e-6);
        } catch (const CORBA::MARSHAL& ex) {
            partitionBursts(first_burst, startTime, queueDepth, streamID, connection);
        }
        try {
            size_t total_elements = 0;
            for (CORBA::ULong index = 0; index < first_burst.length(); ++index) {
                total_elements += first_burst[index].data.length();
            }
            connection.port->pushBursts(second_burst);
            connection.info->alive = true;
            connection.info->stats.record(second_burst.length(), total_elements, queueDepth, delay.total_microseconds() * 1e-6);
        } catch (const CORBA::MARSHAL& ex) {
            partitionBursts(second_burst, startTime, queueDepth, streamID, connection);
        }
    }

    template <class Traits>
    void OutPort<Traits>::queueBurst (SequenceType& data, const BURSTIO::BurstSRI& sri,
                                      const BULKIO::PrecisionUTCTime& timestamp, bool eos, bool isComplex)
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        const std::string streamID(sri.streamID);
        Queue& queue = getQueueForStream(streamID);
        queue.queueBurst(data, sri, timestamp, eos, isComplex);
        if (eos) {
            if (ROUTE_ALL_INTERLEAVED != routingMode_) {
                LOG_INSTANCE_DEBUG("Flushing '" << streamID << " on EOS");
                queue.flush();
                delete streamQueues_[streamID];
            }
            streamQueues_.erase(streamID);
        }
    }

    template <class Traits>
    BULKIO::UsesPortStatisticsSequence * OutPort<Traits>::statistics()
    {
        boost::mutex::scoped_lock lock(updatingPortsLock);
        BULKIO::UsesPortStatisticsSequence_var retval = new BULKIO::UsesPortStatisticsSequence();
        retval->length(connections_.size());
        CORBA::ULong index = 0;
        for (typename ConnectionMap::iterator ii = connections_.begin(); ii != connections_.end(); ++ii, ++index) {
            retval[index].connectionId = ii->first.c_str();
            BULKIO::PortStatistics_var stats = ii->second.info->stats.retrieve();
            for (typename QueueMap::iterator jj = streamQueues_.begin(); jj != streamQueues_.end(); ++jj) {
                const std::string& streamID = jj->first;
                if (isStreamRoutedToConnection(streamID, ii->first)) {
                    burstio::utils::push_back(stats->streamIDs, jj->first.c_str());
                }
            }
            retval[index].statistics = stats;
        }
        return retval._retn();
    }

    template <class Traits>
    void OutPort<Traits>::flush ()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        if (ROUTE_ALL_INTERLEAVED == routingMode_) {
            LOG_INSTANCE_DEBUG("Forcing flush of default queue");
            defaultQueue_.flush();
        } else {
            LOG_INSTANCE_DEBUG("Forcing flush of all queues");
            for (typename QueueMap::iterator queue = streamQueues_.begin(); queue != streamQueues_.end(); ++queue) {
                queue->second->flush();
            }
        }
    }

    template <class Traits>
    void OutPort<Traits>::connectionAdded (const std::string& connectionId, Connection& connection)
    {
        connection.info = new PortStatus(this->name, sizeof(ElementType)*8);
    }

    template <class Traits>
    void OutPort<Traits>::connectionModified (const std::string& connectionId, Connection& connection)
    {
        // Assume that the updated connection is alive
        connection.info->alive = true;
    }

    template <class Traits>
    void OutPort<Traits>::scheduleCheck (boost::system_time when)
    {
        monitor_.schedule(when, &OutPort<Traits>::checkQueues, this);
    }

    template <class Traits>
    void OutPort<Traits>::checkQueues ()
    {
        boost::mutex::scoped_lock lock(queueMutex_);
        if (ROUTE_ALL_INTERLEAVED == routingMode_) {
            defaultQueue_.checkFlush();
        } else {
            for (typename QueueMap::const_iterator queue = streamQueues_.begin(); queue != streamQueues_.end(); ++queue) {
                queue->second->checkFlush();
            }
        }
    }

    template <class Traits>
    const typename OutPort<Traits>::Queue& OutPort<Traits>::getQueueForStream (const std::string& streamID) const
    {
        if (ROUTE_ALL_INTERLEAVED == routingMode_) {
            return defaultQueue_;
        }
        typename QueueMap::const_iterator queue = streamQueues_.find(streamID);
        if (queue == streamQueues_.end()) {
            throw std::invalid_argument("unknown stream " + streamID);
        }
        return *(queue->second);
    }

    template <class Traits>
    typename OutPort<Traits>::Queue& OutPort<Traits>::getQueueForStream (const std::string& streamID)
    {
        typename QueueMap::iterator queue = streamQueues_.find(streamID);
        if (queue != streamQueues_.end()) {
            return *(queue->second);
        } else if (ROUTE_ALL_INTERLEAVED == routingMode_) {
            streamQueues_[streamID] = &defaultQueue_;
            return defaultQueue_;
        } else {
            LOG_INSTANCE_TRACE("Creating new queue for stream " << streamID);
            // Propagate the default queue's settings
            size_t max_bursts = defaultQueue_.getMaxBursts();
            size_t byte_threshold = defaultQueue_.getByteThreshold();
            float latency_threshold = defaultQueue_.getLatencyThreshold();
            Queue* new_queue = new Queue(this, streamID, max_bursts, byte_threshold, latency_threshold);
            streamQueues_[streamID] = new_queue;
            return *new_queue;
        }
    }

    template <class Traits>
    bool OutPort<Traits>::isStreamRoutedToConnection (const std::string& streamID, const std::string& connectionID)
    {
        if (ROUTE_CONNECTION_STREAMS != routingMode_) {
            return true;
        }
        RouteTable::const_iterator stream_routes = routes_.find(streamID);
        if (stream_routes == routes_.end()) {
            return false;
        }
        return stream_routes->second.count(connectionID);
    }
}

#define INSTANTIATE_TEMPLATE(traits, name) \
    PREPARE_CLASS_LOGGING(name);           \
    template class OutPort<traits>;

#endif
