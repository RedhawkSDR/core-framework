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

#include "bulkio_in_stream.h"
#include "bulkio_time_operators.h"
#include "bulkio_in_port.h"
#include "bulkio_p.h"

#include <boost/make_shared.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/ptr_container/ptr_deque.hpp>

#include <ossie/bitops.h>

using bulkio::InputStream;

template <class PortType>
class InputStream<PortType>::Impl : public StreamBase::Impl {
public:
    typedef StreamBase::Impl ImplBase;

    typedef typename InPortType::Packet PacketType;

    enum EosState {
        EOS_NONE,
        EOS_RECEIVED,
        EOS_REACHED,
        EOS_REPORTED
    };

    Impl(const bulkio::StreamDescriptor& sri, InPortType* port) :
        ImplBase(sri),
        _port(port),
        _eosState(EOS_NONE),
        _enabled(true),
        _newstream(true)
    {
    }

    DataBlockType readPacket(bool blocking)
    {
        boost::scoped_ptr<PacketType> packet(_fetchPacket(blocking));
        if (_eosState == EOS_RECEIVED) {
            _eosState = EOS_REACHED;
        }
        if (!packet || (packet->EOS && packet->buffer.empty())) {
            _reportIfEosReached();
            return DataBlockType();
        }
        DataBlockType block(packet->SRI, packet->buffer);
        // Add timestamp via a templatized method so that dataXML can omit it
        _addTimestamp(block, packet->T);
        _setBlockFlags(block, *packet);
        // Update local SRI from packet
        StreamDescriptor::operator=(packet->SRI);
        return block;
    }

    bool enabled() const
    {
        return _enabled;
    }

    void enable()
    {
        // Changing the enabled flag requires holding the port's streamsMutex
        // (that controls access to the stream map) to ensure that the change
        // is atomic with respect to handling end-of-stream packets. Otherwise,
        // there is a race condition between the port's IO thread and the
        // thread that enables the stream--it could be re-enabled and start
        // reading in between the port checking whether to discard the packet
        // and closing the stream. Because it is assumed that the same thread
        // that calls enable is the one doing the reading, it is not necessary
        // to apply mutual exclusion across the entire public stream API, just
        // enable/disable.
        boost::mutex::scoped_lock lock(_port->streamsMutex);
        _enabled = true;
    }

    virtual void disable()
    {
        // See above re: locking
        boost::mutex::scoped_lock lock(_port->streamsMutex);
        _enabled = false;

        // Unless end-of-stream has been received by the port (meaning any further
        // packets with this stream ID are for a different instance), purge any
        // packets for this stream from the port's queue
        if (_eosState == EOS_NONE) {
            _port->discardPacketsForStream(_streamID);
        }
    }

    void close()
    {
        // NB: This method is always called by the port with streamsMutex held

        // Consider end-of-stream reported, since the stream has already been
        // removed from the port; otherwise, there's nothing to do
        _eosState = EOS_REPORTED;
    }

    virtual bool eos()
    {
        _reportIfEosReached();
        // At this point, if end-of-stream has been reached, the state is
        // reported (it gets set above), so the checking for the latter is
        // sufficient
        return (_eosState == EOS_REPORTED);
    }

    virtual bool hasBufferedData() const
    {
        // For the base class, there is no data to report; however, to nudge
        // the check end-of-stream, return true if it has been reached but not
        // reported
        return (_eosState == EOS_REACHED);
    }

protected:
    PacketType* _fetchPacket(bool blocking)
    {
        // Don't fetch a packet from the port if stream is disabled
        if (!_enabled) {
            return 0;
        }

        // Any future packets with this stream ID belong to another InputStream
        if (_eosState != EOS_NONE) {
            return 0;
        }

        float timeout = blocking?bulkio::Const::BLOCKING:bulkio::Const::NON_BLOCKING;
        PacketType* packet = _port->nextPacket(timeout, _streamID);
        if (packet && packet->EOS) {
            _eosState = EOS_RECEIVED;
        }
        return packet;
    }

    void _reportIfEosReached()
    {
        if (_eosState == EOS_REACHED) {
            // This is the first time end-of-stream has been checked since it
            // was reached; remove the stream from the port now, since the
            // caller knows that the stream ended
            _port->removeStream(_streamID);
            _eosState = EOS_REPORTED;
        }
    }

    size_t _samplesAvailable(bool first)
    {
        if (_eosState == EOS_NONE) {
            return _port->samplesAvailable(_streamID, first);
        } else {
            return 0;
        }
    }

    void _setBlockFlags(DataBlockType& block, PacketType& packet)
    {
        // Allocate empty data block and propagate the SRI change and input
        // queue flush flags
        int flags=0;
        if (packet.sriChanged) {
            flags = bulkio::sri::compareFields(this->sri(), packet.SRI.sri());
            block.sriChangeFlags(flags);
        }
        if (_newstream) {
            _newstream=false;
            flags |= bulkio::sri::STREAMID |bulkio::sri::XDELTA | bulkio::sri::YDELTA | bulkio::sri::KEYWORDS | bulkio::sri::MODE;
            block.sriChangeFlags(flags);
        }
        if (packet.inputQueueFlushed) {
            block.inputQueueFlushed(true);
        }
    }

    void _addTimestamp(DataBlockType& block, const BULKIO::PrecisionUTCTime& time)
    {
        block.addTimestamp(time);
    }

    InPortType* _port;
    EosState _eosState;
    bool _enabled;
    bool _newstream;
};

namespace bulkio {
    template <>
    void InputStream<BULKIO::dataXML>::Impl::_addTimestamp(bulkio::StringDataBlock& block,
                                                           const BULKIO::PrecisionUTCTime&)
    {
        // Discard the time stamp, which was created by the input port to adapt to
        // the common template implementation
    }
}

template <class PortType>
InputStream<PortType>::InputStream() :
    StreamBase()
{
}

template <class PortType>
InputStream<PortType>::InputStream(const StreamDescriptor& sri, InPortType* port) :
    StreamBase(boost::make_shared<Impl>(sri, port))
{
}

template <class PortType>
InputStream<PortType>::InputStream(const boost::shared_ptr<Impl>& impl) :
    StreamBase(impl)
{
}

template <class PortType>
typename InputStream<PortType>::DataBlockType InputStream<PortType>::read()
{
    return impl().readPacket(true);
}

template <class PortType>
typename InputStream<PortType>::DataBlockType InputStream<PortType>::tryread()
{
    return impl().readPacket(false);
}

template <class PortType>
bool InputStream<PortType>::enabled() const
{
    return impl().enabled();
}

template <class PortType>
void InputStream<PortType>::enable()
{
    impl().enable();
}

template <class PortType>
void InputStream<PortType>::disable()
{
    impl().disable();
}

template <class PortType>
bool InputStream<PortType>::eos()
{
    return impl().eos();
}

template <class PortType>
InputStream<PortType>::operator unspecified_bool_type() const
{
    return _impl?static_cast<unspecified_bool_type>(&InputStream::impl):0;
}

template <class PortType>
typename InputStream<PortType>::Impl& InputStream<PortType>::impl()
{
    return static_cast<Impl&>(*this->_impl);
}

template <class PortType>
const typename InputStream<PortType>::Impl& InputStream<PortType>::impl() const
{
    return static_cast<const Impl&>(*this->_impl);
}

template <class PortType>
bool InputStream<PortType>::hasBufferedData()
{
    return impl().hasBufferedData();
}

template <class PortType>
void InputStream<PortType>::close()
{
    impl().close();
}


using bulkio::BufferedInputStream;

template <class PortType>
class BufferedInputStream<PortType>::Impl : public Base::Impl {
public:
    typedef typename Base::Impl ImplBase;

    typedef typename ImplBase::PacketType PacketType;
    typedef typename NativeTraits<PortType>::NativeType NativeType;
    typedef typename BufferTraits<PortType>::BufferType BufferType;
    typedef typename BufferTraits<PortType>::MutableBufferType MutableBufferType;

    Impl(const bulkio::StreamDescriptor& sri, InPortType* port) :
        ImplBase(sri, port),
        _queue(),
        _pending(0),
        _samplesQueued(0),
        _sampleOffset(0)
    {
    }

    ~Impl()
    {
        delete _pending;
    }

    virtual bool eos()
    {
        if (_queue.empty()) {
            // Try a non-blocking fetch to see if there's an empty end-of-stream
            // packet waiting; this helps with the case where the last read consumes
            // exactly the remaining data, and the stream will never report a ready
            // state again
            _fetchPacket(false);
        }
        return ImplBase::eos();
    }

    size_t samplesAvailable()
    {
        // Start with the samples already in the queue
        size_t queued = _samplesQueued;
        if (queued > 0) {
            // Adjust number of samples to account for complex data, if necessary
            if (_queue.front().SRI.complex()) {
                queued /= 2;
            }
        }

        // Only search the port's queue if there is no SRI change or input queue
        // flush pending
        if (!_pending) {
            // If the queue is empty, this is the first read of a segment (i.e.,
            // search can go past the first packet if the SRI change or queue flush
            // flag is set)
            bool first = _queue.empty();
            queued += ImplBase::_samplesAvailable(first);
        }

        return queued;
    }

    DataBlockType readPacket(bool blocking)
    {
        if (_samplesQueued == 0) {
            _fetchPacket(blocking);
        }

        if (_samplesQueued == 0) {
            // It's possible that there are no samples queued because of an
            // end-of-stream; if so, report it so that this stream can be
            // dissociated from the port
            this->_reportIfEosReached();
            return DataBlockType();
        }
        // Only read up to the end of the first packet in the queue
        const size_t samples = _queue.front().buffer.size() - _sampleOffset;
        return _readData(samples, samples);
    }

    DataBlockType read(size_t count, size_t consume, bool blocking)
    {
        // Try to get the SRI for the upcoming block of data, fetching it from the
        // port's input queue if necessary
        const StreamDescriptor* sri = _nextSRI(blocking);
        if (!sri) {
            // No SRI retreived implies no data will be retrieved, either due
            // to end-of-stream or because it would block
            this->_reportIfEosReached();
            return DataBlockType();
        }

        // If the next block of data is complex, double the read and consume size
        // (which the lower-level I/O handles in terms of scalars) so that the
        // returned block has the right number of samples
        if (sri->complex()) {
            count *= 2;
            consume *= 2;
        }

        // Queue up packets from the port until we have enough data to satisfy the
        // requested read amount
        while (_samplesQueued < count) {
            if (!_fetchPacket(blocking)) {
                break;
            }
        }

        if (_samplesQueued == 0) {
            // As above, it's possible that there are no samples due to an end-
            // of-stream
            this->_reportIfEosReached();
            return DataBlockType();
        }

        // Only read as many samples as are available (e.g., if a new SRI is coming
        // or the stream reached the end)
        const size_t samples = std::min(count, _samplesQueued);

        // Handle a partial read, which could mean that there's not enough data at
        // present (non-blocking), or that the read pointer has reached the end of
        // a segment (new SRI, queue flush, end-of-stream)
        if (samples < count) {
            // Non-blocking: return a null block if there's not currently a break in
            // the data, under the assumption that a future read might return the
            // full amount
            if (!blocking && !_pending && (this->_eosState == ImplBase::EOS_NONE)) {
                return DataBlockType();
            }
            // Otherwise, consume all remaining data (when not requested as 0)
            if (consume != 0)
                consume = samples;
        }

        return _readData(samples, consume);
    }

    size_t skip(size_t count)
    {
        // If the next block of data is complex, double the skip size (which the
        // lower-level I/O handles in terms of scalars) so that the right number of
        // samples is skipped
        const StreamDescriptor* sri = _nextSRI(true);
        if (!sri) {
            return 0;
        }

        size_t item_size = sri->complex()?2:1;
        count *= item_size;

        // Queue up packets from the port until we have enough data to satisfy the
        // requested read amount
        while (_samplesQueued < count) {
            if (!_fetchPacket(true)) {
                break;
            }
        }

        count = std::min(count, _samplesQueued);
        _consumeData(count);

        // Convert scalars back to samples
        return count / item_size;
    }

    bool ready()
    {
        if (_samplesQueued) {
            return true;
        } else {
            return samplesAvailable() > 0;
        }
    }

    virtual void disable()
    {
        ImplBase::disable();
        // NB: The lock is not required to modify the internal stream queue
        // state, because it should only be accessed by the thread that is
        // reading from the stream

        // Clear queued packets, which implicitly deletes them
        _queue.clear();
        _sampleOffset = 0;
        _samplesQueued = 0;

        // Delete pending packet (it's safe to delete null pointers)
        delete _pending;
    }

    bool hasBufferedData() const
    {
        if (!_queue.empty() || _pending) {
            // Return true if either there are queued or pending packets
            return true;
        }
        return ImplBase::hasBufferedData();
    }

private:
    void _consumeData(size_t count)
    {
        while (count > 0) {
            const BufferType& data = _queue.front().buffer;

            const size_t available = data.size() - _sampleOffset;
            const size_t pass = std::min(available, count);

            _sampleOffset += pass;
            _samplesQueued -= pass;
            count -= pass;

            if (_sampleOffset >= data.size()) {
                // Read pointer has passed the end of the packet data
                _consumePacket();
                _sampleOffset = 0;
            }
        }
    }

    void _consumePacket()
    {
        // Acknowledge any end-of-stream flag and delete the packet (the queue will
        // automatically delete it when it's removed)
        if (_queue.front().EOS) {
            this->_eosState = ImplBase::EOS_REACHED;
        }
        _queue.pop_front();

        // If the queue is empty, move the pending packet onto the queue
        if (_queue.empty() && _pending) {
            _queuePacket(_pending);
            _pending = 0;
        }
    }

    DataBlockType _readData(size_t count, size_t consume)
    {
        // Acknowledge pending SRI change
        PacketType& front = _queue.front();

        // Allocate empty data block and propagate the SRI change and input queue
        // flush flags
        DataBlockType data(front.SRI);
        this->_setBlockFlags(data, front);
        if (front.sriChanged) {
            // Update the stream metadata
            StreamDescriptor::operator=(front.SRI);
        }

        // Clear flags from packet, since they've been reported
        front.sriChanged = false;
        front.inputQueueFlushed = false;

        size_t last_offset = _sampleOffset + count;
        if (last_offset <= front.buffer.size()) {
            // The requsted sample count can be satisfied from the first packet
            _addTimestamp(data, _sampleOffset, 0, front.T);
            data.buffer(front.buffer.slice(_sampleOffset, last_offset));
        } else {
            // We have to span multiple packets to get the data
            MutableBufferType buffer(count);
            data.buffer(buffer);
            size_t data_offset = 0;

            // Assemble data spanning several input packets into the output buffer
            size_t packet_index = 0;
            size_t packet_offset = _sampleOffset;
            while (count > 0) {
                PacketType& packet = _queue[packet_index];
                const BufferType& input_data = packet.buffer;

                // Add the timestamp for this pass
                _addTimestamp(data, packet_offset, data_offset, packet.T);

                // The number of samples copied on this pass may be less than the total
                // remaining
                const size_t available = input_data.size() - packet_offset;
                const size_t pass = std::min(available, count);

                buffer.replace(data_offset, pass, input_data, packet_offset);
                data_offset += pass;
                packet_offset += pass;
                count -= pass;

                // If all the data from the current packet has been read, move on to
                // the next
                if (packet_offset >= input_data.size()) {
                    packet_offset = 0;
                    ++packet_index;
                }
            }
        }

        // Advance the read pointers
        _consumeData(consume);

        return data;
    }

    void _addTimestamp(DataBlockType& data, size_t inputOffset, size_t outputOffset, BULKIO::PrecisionUTCTime time)
    {
        // Determine the timestamp of this chunk of data; if this is the
        // first chunk, the packet offset (number of samples already read)
        // must be accounted for, so adjust the timestamp based on the SRI.
        // Otherwise, the adjustment is a noop.
        double time_offset = inputOffset * data.xdelta();
        // Check the SRI directly for the complex mode because bit data blocks
        // intentionally do not have a complex() method.
        if (data.sri().mode != 0) {
            // Complex data; each sample is two values
            time_offset /= 2.0;
            outputOffset /= 2;
        }

        // If there is a time offset, apply the adjustment and mark the timestamp
        // so that the caller knows it was calculated rather than received
        bool synthetic = false;
        if (time_offset > 0.0) {
            time += time_offset;
            synthetic = true;
        }

        data.addTimestamp(bulkio::SampleTimestamp(time, outputOffset, synthetic));
    }

    const StreamDescriptor* _nextSRI(bool blocking)
    {
        if (_queue.empty()) {
            if (!_fetchPacket(blocking)) {
                return 0;
            }
        }

        return &(_queue.front().SRI);
    }

    bool _fetchPacket(bool blocking)
    {
        if (_pending) {
            // Cannot read another packet until non-bridging packet is acknowledged
            return false;
        }

        PacketType* packet = ImplBase::_fetchPacket(blocking);
        if (!packet) {
            return false;
        }

        if (_queue.empty() || _canBridge(packet)) {
            return _queuePacket(packet);
        } else {
            _pending = packet;
            return false;
        }
    }

    bool _queuePacket(PacketType* packet)
    {
        if (packet->EOS && packet->buffer.empty()) {
            // Handle end-of-stream packet with no data (assuming that timestamps,
            // SRI changes, and queue flushes are irrelevant at this point)
            if (_queue.empty()) {
                // No queued packets, read pointer has reached end-of-stream
                this->_eosState = ImplBase::EOS_REACHED;
            } else {
                // Assign the end-of-stream flag to the last packet in the queue so
                // that it is handled on read
                _queue.back().EOS = true;
            }
            // Explicitly delete the packet, since it isn't being queued, and
            // return false to let the caller know that no more sample data is
            // forthcoming
            delete packet;
            return false;
        } else {
            // Add the packet to the queue, taking ownership; it will be deleted when
            // it's consumed
            _samplesQueued += packet->buffer.size();
            _queue.push_back(packet);
            return true;
        }
    }

    bool _canBridge(PacketType* packet) const
    {
        return !(packet->sriChanged || packet->inputQueueFlushed);
    }

    boost::ptr_deque<PacketType> _queue;
    PacketType* _pending;
    size_t _samplesQueued;
    size_t _sampleOffset;
};

template <class PortType>
BufferedInputStream<PortType>::BufferedInputStream() :
    Base()
{
}

template <class PortType>
BufferedInputStream<PortType>::BufferedInputStream(const bulkio::StreamDescriptor& sri, InPortType* port) :
    Base(boost::make_shared<Impl>(sri, port))
{
}

template <class PortType>
typename BufferedInputStream<PortType>::DataBlockType BufferedInputStream<PortType>::read()
{
    return impl().readPacket(true);
}

template <class PortType>
typename BufferedInputStream<PortType>::DataBlockType BufferedInputStream<PortType>::read(size_t count)
{
    return impl().read(count, count, true);
}

template <class PortType>
typename BufferedInputStream<PortType>::DataBlockType BufferedInputStream<PortType>::read(size_t count, size_t consume)
{
    return impl().read(count, consume, true);
}

template <class PortType>
typename BufferedInputStream<PortType>::DataBlockType BufferedInputStream<PortType>::tryread()
{
    return impl().readPacket(false);
}

template <class PortType>
typename BufferedInputStream<PortType>::DataBlockType BufferedInputStream<PortType>::tryread(size_t count)
{
    return impl().read(count, count, false);
}

template <class PortType>
typename BufferedInputStream<PortType>::DataBlockType BufferedInputStream<PortType>::tryread(size_t count, size_t consume)
{
    return impl().read(count, consume, false);
}

template <class PortType>
size_t BufferedInputStream<PortType>::skip(size_t count)
{
    return impl().skip(count);
}

template <class PortType>
size_t BufferedInputStream<PortType>::samplesAvailable()
{
    return impl().samplesAvailable();
}

template <class PortType>
bool BufferedInputStream<PortType>::operator==(const BufferedInputStream& other) const
{
    return _impl.get() == other._impl.get();
}

template <class PortType>
bool BufferedInputStream<PortType>::ready()
{
    return impl().ready();
}

template <class PortType>
typename BufferedInputStream<PortType>::Impl& BufferedInputStream<PortType>::impl()
{
    return static_cast<Impl&>(Base::impl());
}

template <class PortType>
const typename BufferedInputStream<PortType>::Impl& BufferedInputStream<PortType>::impl() const
{
    return static_cast<const Impl&>(Base::impl());
}

#define INSTANTIATE_TEMPLATE(x) \
    template class InputStream<x>;

#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    template class BufferedInputStream<x>;

FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
INSTANTIATE_NUMERIC_TEMPLATE(BULKIO::dataBit);
