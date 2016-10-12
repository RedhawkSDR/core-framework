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
#ifndef BURSTIO_INPORT_DECL_H
#define BURSTIO_INPORT_DECL_H

#include <list>
#include <set>

#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/shared_ptr.hpp>

#include <ossie/Port_impl.h>

#include "BurstStatistics.h"
#include "PortTraits.h"
#include "debug.h"

namespace burstio {
    template <class Traits> class BurstPacket;

    template <class Traits>
    class InPort : public Port_Provides_base_impl, public virtual Traits::POAType
    {
        ENABLE_INSTANCE_LOGGING;

    public:
		typedef typename Traits::PortType PortType;
        typedef typename Traits::BurstType BurstType;
        typedef typename Traits::BurstSequenceType BurstSequenceType;
        typedef typename Traits::ElementType ElementType;
        typedef typename BurstSequenceType::_var_type BurstSequenceVar;

        typedef BurstPacket<Traits> PacketType;

        static const size_t DEFAULT_QUEUE_THRESHOLD = 100;

        InPort(std::string port_name);
        ~InPort();

        // Replace the current logger with the given logger.
        void setLogger (LoggerPtr logger);

        // Set or get the queue threshold, which is the maximum number of
        // bursts that can be queued before subsequence pushBursts calls will
        // block. There is no guarantee that the queue will not exceed the
        // threshold at any given time, as an upstream output port may send
        // arbitrarily large numbers of bursts (within the limits of the max
        // GIOP message size).
        size_t getQueueThreshold () const;
        void setQueueThreshold (size_t count);

        // Start accepting burst data on this port. If the port is not started,
        // received bursts will be dropped.
        void start ();

        // Stop accepting burst data on this port. Any calls to getBurst() or
        // getBursts() currently waiting on a burst will return immediately,
        // and received bursts will be dropped. Any blocked senders will be
        // unblocked.
        void stop ();

        // High-level interface to retrieve a single burst as a smart pointer
        // to C++ wrapper class, which eliminates the need to delete the
        // returned value. 
        //
        // The timeout value is the amount of time, in seconds, to wait for a
        // burst to become available; negative timeout values indicate an
        // indefinite wait. If the operation times out, the returned pointer
        // will be null (i.e., !ptr evalutes to true).
        //
        // There are typedefs for the specific types, e.g., BytePacket, so that
        // the caller can use those instead of the dependent type PacketType.
        //
        //   burstio::BytePacket* burst = byte_in->getBurst(0.125);
        //
        PacketType* getBurst (float timeout);
        
        // Lower-level interface to get at the received CORBA sequence of
        // bursts; if the operation times out, or the port is stopped, returns
        // an empty sequence.
        BurstSequenceType* getBursts (float timeout);

        // Returns true if a CORBA pushBursts call blocked since the last time
        // this method (or getBurst) was called; clears the flag, so subsequent
        // calls will return false unless another call blocks.
        bool blockOccurred ();

        // CORBA interface for upstream output ports. User code should not call
        // this method.
        void pushBursts(const BurstSequenceType& bursts);

        BULKIO::PortUsageType state();
        BULKIO::PortStatistics* statistics();

        // Returns the current depth of this port's queue. Note that this can
        // exceed the configured burst threshold.
        size_t getQueueDepth ();

        // Drop all queued bursts. Any senders that are currently blocked will
        // be unblocked, and may start queueing bursts again.
        void flush ();

        // Support function for automatic component-managed start.
        virtual void startPort ();

        // Support function for automatic component-managed stop.
        virtual void stopPort ();

		std::string getRepid() const;

    protected:
        // Wait timeout seconds for a burst to become available; the caller
        // must already hold the queue mutex, passing in the scoped lock as an
        // argument. Returns true if a burst can be retrieved from the queue,
        // false if the timeout expired or the port is stopped.
        bool waitBurst (float timeout, boost::mutex::scoped_lock& lock);

    private:
        typedef std::list<BurstSequenceVar> BurstQueue;
        boost::mutex queueMutex_;
        boost::condition_variable queueNotEmpty_;
        boost::condition_variable queueNotFull_;
        BurstQueue queue_;
        size_t queueOffset_;
        size_t queuedBursts_;
        size_t queueThreshold_;
        volatile bool blockOccurred_;
        volatile bool started_;

        ReceiverStatistics statistics_;
        std::set<std::string> streamIDs_;
    };

    typedef InPort<ByteTraits>      BurstByteIn;
    typedef InPort<DoubleTraits>    BurstDoubleIn;
    typedef InPort<FloatTraits>     BurstFloatIn;
    typedef InPort<LongTraits>      BurstLongIn;
    typedef InPort<LongLongTraits>  BurstLongLongIn;
    typedef InPort<ShortTraits>     BurstShortIn;
    typedef InPort<UbyteTraits>     BurstUbyteIn;
    typedef InPort<UlongTraits>     BurstUlongIn;
    typedef InPort<UlongLongTraits> BurstUlongLongIn;
    typedef InPort<UshortTraits>    BurstUshortIn;
}

#endif // BURSTIO_INPORT_DECL_H
