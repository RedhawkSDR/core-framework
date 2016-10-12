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
#ifndef BURSTIO_OUTPORT_DECL_H
#define BURSTIO_OUTPORT_DECL_H

#include <complex>
#include <map>
#include <set>
#include <string>

#include <boost/thread.hpp>

#include <BULKIO/bio_runtimeStats.h>

#include "UsesPort.h"
#include "BurstStatistics.h"
#include "PortTraits.h"
#include "ExecutorService.h"
#include "utils.h"
#include "debug.h"

namespace burstio {

    struct PortStatus
    {
        PortStatus(const std::string& name, size_t bitsPerElement):
            stats(name, bitsPerElement),
            alive(true)
        {
        }

        SenderStatistics stats;
        bool alive;
    };

    enum RoutingModeType {
        ROUTE_ALL_INTERLEAVED,
        ROUTE_ALL_STREAMS,
        ROUTE_CONNECTION_STREAMS
    };

    class OutputPolicy {
    public:
        virtual size_t getMaxBursts () const = 0;
        virtual void setMaxBursts (size_t count) = 0;
        virtual size_t getByteThreshold () const = 0;
        virtual void setByteThreshold (size_t bytes) = 0;
        virtual long getLatencyThreshold () const = 0;
        virtual void setLatencyThreshold (long usec) = 0;
        virtual void flush () = 0;
    };

    template <class Traits>
    class OutPort : public UsesPort<typename Traits::PortType, PortStatus>,
                    public virtual POA_BULKIO::UsesPortStatisticsProvider
    {
        ENABLE_INSTANCE_LOGGING;

    public:
        typedef UsesPort<typename Traits::PortType, PortStatus> super;

        typedef typename Traits::BurstType BurstType;
        typedef typename Traits::BurstSequenceType BurstSequenceType;
        typedef typename Traits::ElementType ElementType;
        typedef typename Traits::SequenceType SequenceType;
        typedef typename Traits::NativeType NativeType;

        static const size_t DEFAULT_MAX_BURSTS = 100;
        static const long DEFAULT_LATENCY_THRESHOLD = 10000; // 10000 us = 10 ms

        OutPort(std::string port_name);
        ~OutPort();

        void setLogger (LoggerPtr logger);

        // Sets how streams are routed to connections:
        //   ROUTE_ALL_INTERLEAVED    - All connections receive all streams;
        //                              streams are interleaved in one buffer
        //   ROUTE_ALL_STREAMS        - All connections receive all streams;
        //                              streams are buffered independently
        //   ROUTE_CONNECTION_STREAMS - Each connection may subscribe to a set
        //                              of streams; streams are buffered
        //                              independently
        void setRoutingMode (RoutingModeType mode);

        size_t getMaxBursts () const;
        void setMaxBursts (size_t count);

        size_t getByteThreshold () const;
        void setByteThreshold (size_t bytes);

        long getLatencyThreshold () const;
        void setLatencyThreshold (long usec);

        OutputPolicy* getDefaultPolicy ();
        const OutputPolicy* getDefaultPolicy () const;

        OutputPolicy* getStreamPolicy (const std::string& streamID);
        const OutputPolicy* getStreamPolicy (const std::string& streamID) const;

        template <class Container>
        void updateConnectionFilter (const Container& filterTable)
        {
            RouteTable new_routes;
            for (typename Container::const_iterator filter = filterTable.begin(); filter != filterTable.end(); ++filter) {
                if (filter->port_name != this->name) {
                    continue;
                }
                new_routes[filter->stream_id].insert(filter->connection_id);
            }

            boost::mutex::scoped_lock lock(updatingPortsLock);
            routes_.swap(new_routes);
        }

        void addConnectionFilter (const std::string& streamID, const std::string& connectionID);
        void removeConnectionFilter (const std::string& streamID, const std::string& connectionID);

        void start ();
        void stop ();

        BULKIO::PortUsageType state ();

        // STL container-based push
        template <class Container>
        void pushBurst (const Container& data, const BURSTIO::BurstSRI& sri,
                        const BULKIO::PrecisionUTCTime& timestamp=burstio::utils::now(), bool eos=false)
        {
            this->pushBurst(data.begin(), data.end(), sri, timestamp, eos);
        }

        template <class Container>
        void pushBurst (const Container& data, const BURSTIO::BurstSRI& sri, bool eos)
        {
            this->pushBurst(data.begin(), data.end(), sri, eos);
        }

        // Iterator-based push
        template <class Iterator>
        void pushBurst (Iterator first, Iterator last, const BURSTIO::BurstSRI& sri,
                        const BULKIO::PrecisionUTCTime& timestamp=burstio::utils::now(), bool eos=false)
        {
            SequenceType data;
            burstio::utils::copy(data, first, last);
            this->queueBurst(data, sri, timestamp, eos, burstio::utils::is_complex(*first));
        }

        template <class Iterator>
        void pushBurst (Iterator first, Iterator last, const BURSTIO::BurstSRI& sri, bool eos)
        {
            this->pushBurst(first, last, sri, burstio::utils::now(), eos);
        }

        // CORBA sequence-based push
        void pushBurst (SequenceType& data, const BURSTIO::BurstSRI& sri,
                        const BULKIO::PrecisionUTCTime& timestamp=burstio::utils::now(), bool eos=false);
        void pushBurst (SequenceType& data, const BURSTIO::BurstSRI& sri, bool eos);

        void pushBurst (BurstType& burst);
        void pushBursts(const BurstSequenceType& bursts);

        BULKIO::UsesPortStatisticsSequence * statistics();

        // Send all queued packets and wait for completion
        void flush ();

        // Support function for automatic component-managed start.
        virtual void startPort ();

        // Support function for automatic component-managed stop.
        virtual void stopPort ();

    protected:
        class Queue : public OutputPolicy
        {
        public:
            typedef typename Traits::BurstType BurstType;
            typedef typename Traits::BurstSequenceType BurstSequenceType;
            typedef typename Traits::SequenceType SequenceType;
            typedef typename Traits::ElementType ElementType;

            size_t getMaxBursts () const;
            void setMaxBursts (size_t count);

            size_t getByteThreshold () const;
            void setByteThreshold (size_t bytes);

            long getLatencyThreshold () const;
            void setLatencyThreshold (long usec);

            void queueBurst (SequenceType& data, const BURSTIO::BurstSRI& sri,
                             const BULKIO::PrecisionUTCTime& timestamp, bool eos, bool isComplex);

            void flush ();

        protected:
            friend class OutPort;

            Queue(OutPort* port, const std::string& streamID, size_t maxBursts, size_t byteThreshold, long latencyThreshold);

            bool shouldFlush ();
            void checkFlush ();

            void executeThreadedFlush ();

        private:
            void sendBursts_ ();

            OutPort<Traits>* port_;
            LoggerPtr& __logger;

            mutable boost::mutex mutex_;

            size_t maxBursts_;
            size_t thresholdBytes_;
            boost::posix_time::time_duration thresholdLatency_; 

            BurstSequenceType bursts_;
            size_t bytes_;
            boost::system_time startTime_;

            std::string streamID_;
        };

        friend class Queue;

        typedef typename super::ConnectionMap ConnectionMap;
        typedef typename super::Connection Connection;

        typedef std::map<std::string,Queue*> QueueMap;

        typedef std::map<std::string,std::set<std::string> > RouteTable;

        void sendBursts (const BurstSequenceType& bursts, boost::system_time startTime, float queueDepth, const std::string& streamID);

        void scheduleCheck (boost::system_time when);
        void checkQueues ();

        void queueBurst (SequenceType& data, const BURSTIO::BurstSRI& sri,
                         const BULKIO::PrecisionUTCTime& timestamp, bool eos, bool isComplex);

        virtual void connectionAdded (const std::string& connectionId, Connection& connection);
        virtual void connectionModified (const std::string& connectionId, Connection& connection);

        const Queue& getQueueForStream (const std::string& streamID) const;
        Queue& getQueueForStream (const std::string& streamID);

        bool isStreamRoutedToConnection (const std::string& streamID, const std::string& connectionID);

        boost::mutex queueMutex_;
        Queue defaultQueue_;
        QueueMap streamQueues_;

        RoutingModeType routingMode_;
        RouteTable routes_;

        ExecutorService monitor_;

        using super::updatingPortsLock;
        using super::connections_;
    };

    typedef OutPort<ByteTraits>      BurstByteOut;
    typedef OutPort<DoubleTraits>    BurstDoubleOut;
    typedef OutPort<FloatTraits>     BurstFloatOut;
    typedef OutPort<LongLongTraits>  BurstLongLongOut;
    typedef OutPort<LongTraits>      BurstLongOut;
    typedef OutPort<ShortTraits>     BurstShortOut;
    typedef OutPort<UbyteTraits>     BurstUbyteOut;
    typedef OutPort<UlongLongTraits> BurstUlongLongOut;
    typedef OutPort<UlongTraits>     BurstUlongOut;
    typedef OutPort<UshortTraits>    BurstUshortOut;
}

#endif // BURSTIO_OUTPORT_DECL_H
