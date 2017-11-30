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

namespace {
  template <class T, class Alloc>
  class stealable_vector : public std::vector<T, Alloc> {
  public:
    stealable_vector()
    {
    }

    T* steal()
    {
      T* out = this->_M_impl._M_start;
      this->_M_impl._M_start = 0;
      this->_M_impl._M_finish = 0;
      this->_M_impl._M_end_of_storage = 0;
      return out;
    }
  };

  template <class T, class Alloc>
  T* steal_buffer(std::vector<T,Alloc>& vec)
  {
    stealable_vector<T,Alloc> other;
    std::swap(vec, other);
    return other.steal();
  }
}

using bulkio::InputStream;

template <class PortTraits>
class InputStream<PortTraits>::Impl {
public:
  typedef PortTraits TraitsType;
  typedef DataTransfer<typename TraitsType::DataTransferTraits> DataTransferType;
  typedef typename DataTransferType::NativeDataType NativeType;
  typedef std::vector<NativeType> VectorType;
  typedef DataBlock<NativeType> DataBlockType;

  enum EosState {
    EOS_NONE,
    EOS_RECEIVED,
    EOS_REACHED,
    EOS_REPORTED
  };

  Impl(const BULKIO::StreamSRI& sri, bulkio::InPort<PortTraits>* port) :
    _streamID(sri.streamID),
    _sri(sri),
    _eosState(EOS_NONE),
    _port(port),
    _queue(),
    _pending(0),
    _samplesQueued(0),
    _sampleOffset(0),
    _enabled(true),
    _newstream(true)
  {
  }

  ~Impl()
  {
    delete _pending;
  }

  const std::string& streamID() const
  {
    return _streamID;
  }

  const BULKIO::StreamSRI& sri() const
  {
    return _sri;
  }

  bool eos()
  {
    if (_queue.empty()) {
      // Try a non-blocking fetch to see if there's an empty end-of-stream
      // packet waiting; this helps with the case where the last read consumes
      // exactly the remaining data, and the stream will never report a ready
      // state again
      _fetchPacket(false);
    }
    // At this point, if end-of-stream has been reached, make sure it's been
    // reported
    _reportIfEosReached();
    return (_eosState == EOS_REPORTED);
  }

  size_t samplesAvailable()
  {
    // Start with the samples already in the queue
    size_t queued = _samplesQueued;
    if (queued > 0) {
      // Adjust number of samples to account for complex data, if necessary
      const BULKIO::StreamSRI& sri = _queue.front()->SRI;
      if (sri.mode) {
        queued /= 2;
      }
    }

    // Only search the port's queue if there is no SRI change or input queue
    // flush pending, and an end-of-stream has not been received
    if (!_pending && (_eosState == EOS_NONE)) {
      // If the queue is empty, this is the first read of a segment (i.e.,
      // search can go past the first packet if the SRI change or queue flush
      // flag is set)
      bool first = _queue.empty();
      queued += _port->samplesAvailable(_streamID, first);
    }

    return queued;
  }

  DataBlockType readPacket(bool blocking)
  {
    if (_samplesQueued == 0) {
      _fetchPacket(blocking);
    }

    if (_samplesQueued == 0) {
      // It's possible that there are no samples queued because of an end-of-
      // stream; if so, report it so that this stream can be dissociated from
      // the port
      _reportIfEosReached();
      return DataBlockType();
    }
    const size_t samples = _queue.front()->dataBuffer.size() - _sampleOffset;
    return _readData(samples, samples);
  }

  DataBlockType read(size_t count, size_t consume, bool blocking)
  {
    // Try to get the SRI for the upcoming block of data, fetching it from the
    // port's input queue if necessary
    const BULKIO::StreamSRI* sri = _nextSRI(blocking);
    if (!sri) {
      // No SRI retreived implies no data will be retrieved, either due to end-
      // of-stream or because it would block
      _reportIfEosReached();
      return DataBlockType();
    }

    // If the next block of data is complex, double the read and consume size
    // (which the lower-level I/O handles in terms of scalars) so that the
    // returned block has the right number of samples
    if (sri->mode == 1) {
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
      // As above, it's possible that there are no samples due to an end-of-
      // stream
      _reportIfEosReached();
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
      if (!blocking && !_pending && (_eosState == EOS_NONE)) {
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
    const BULKIO::StreamSRI* sri = _nextSRI(true);
    if (!sri) {
      return 0;
    }

    size_t item_size = sri->mode?2:1;
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

  bool enabled() const
  {
    return _enabled;
  }

  void enable()
  {
    // Changing the enabled flag requires holding the port's dataBufferLock
    // (that controls access to its queue) to ensure that the change is atomic
    // with respect to handling end-of-stream packets. Otherwise, there is a
    // race condition between the port's IO thread and the thread that enables
    // the stream--it could be re-enabled and start reading in between the
    // port checking whether to discard the packet and closing the stream.
    // Because it is assumed that the same thread that calls enable is the one
    // doing the reading, it is not necessary to apply mutual exclusion across
    // the entire public stream API, just enable/disable.
    boost::mutex::scoped_lock lock(_port->dataBufferLock);
    _enabled = true;
  }

  void disable()
  {
    {
      // See above re: locking
      boost::mutex::scoped_lock lock(_port->dataBufferLock);
      _enabled = false;

      // Unless end-of-stream has been received by the port (meaning any further
      // packets with this stream ID are for a different instance), purge any
      // packets for this stream from the port's queue
      if (_eosState == EOS_NONE) {
        _port->discardPacketsForStream(_streamID);
      }
    }
    // NB: The lock is not required to modify the internal stream queue state,
    // because it should only be accessed by the thread that is reading from
    // the stream

    // Purge the packet queue...
    for (typename QueueType::iterator packet = _queue.begin(); packet != _queue.end(); ++packet) {
      _deletePacket(*packet);
    }
    _queue.clear();
    _sampleOffset = 0;
    _samplesQueued = 0;

    // ...and the pending packet
    if (_pending) {
      _deletePacket(_pending);
    }
  }

  void close()
  {
    // NB: This method is always called by the port with dataBufferLock held

    // If this stream is enabled, close() is in response to the stream calling
    // removeStream() on the port, so there's nothing left to do
    if (_enabled) {
      return;
    }

    // Consider end-of-stream reported, since the stream has already been
    // removed from the port; otherwise, there's nothing to do
    _eosState = EOS_REPORTED;
  }

  bool hasBufferedData() const
  {
    // To nudge the caller to check end-of-stream, return true if it has been
    // reached but not reported
    if (_eosState == EOS_REACHED) {
      return true;
    }
    return !_queue.empty() || _pending;
  }

private:
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

  void _consumeData(size_t count)
  {
    while (count > 0) {
      const VectorType& data = _queue.front()->dataBuffer;

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

  void _deletePacket(DataTransferType* packet)
  {
    // The packet buffer was allocated with new[] by the CORBA layer, while
    // vector will use non-array delete, so explicitly delete the buffer
    delete[] steal_buffer(packet->dataBuffer);
    delete packet;
  }

  void _consumePacket()
  {
    // Acknowledge any end-of-stream flag and delete the packet
    DataTransferType* packet = _queue.front();
    if (packet->EOS) {
      _eosState = EOS_REACHED;
    }

    // The packet buffer was allocated with new[] by the CORBA layer, while
    // vector will use non-array delete, so explicitly delete the buffer
    _deletePacket(packet);
    _queue.erase(_queue.begin());

    // If the queue is empty, move the pending packet onto the queue
    if (_queue.empty() && _pending) {
      _queuePacket(_pending);
      _pending = 0;
    }
  }

  DataBlockType _readData(size_t count, size_t consume)
  {
    // Acknowledge pending SRI change
    DataTransferType* front = _queue.front();
    int sriChangeFlags = bulkio::sri::NONE;
    if (front->sriChanged) {
      sriChangeFlags = bulkio::sri::compareFields(_sri, front->SRI);
      front->sriChanged = false;
      _sri = front->SRI;
    }

    if ( _newstream ) {
        // seed sri change flags for new stream
        sriChangeFlags |= bulkio::sri::STREAMID |bulkio::sri::XDELTA | bulkio::sri::YDELTA | bulkio::sri::KEYWORDS | bulkio::sri::MODE;
        _newstream=false;
    }

    // Allocate empty data block and propagate the SRI change and input queue
    // flush flags
    DataBlockType data(_sri);
    data.sriChangeFlags(sriChangeFlags);
    if (front->inputQueueFlushed) {
      data.inputQueueFlushed(true);
      front->inputQueueFlushed = false;
    }

    if ((count <= consume) && (_sampleOffset == 0) && (front->dataBuffer.size() == count)) {
      // Optimization: when the read aligns perfectly with the front packet's
      // data buffer, and the entire packet is being consumed, swap the vector
      // data
      data.addTimestamp(bulkio::SampleTimestamp(front->T, 0));
      data.swap(front->dataBuffer);
      _samplesQueued -= count;
      _consumePacket();
      return data;
    }

    data.resize(count);
    NativeType* data_buffer = data.data();
    size_t data_offset = 0;

    // Assemble data that may span several input packets into the output buffer
    size_t packet_index = 0;
    size_t packet_offset = _sampleOffset;
    while (count > 0) {
      DataTransferType* packet = _queue[packet_index];
      const VectorType& input_data = packet->dataBuffer;

      // Determine the timestamp of this chunk of data; if this is the
      // first chunk, the packet offset (number of samples already read)
      // must be accounted for, so adjust the timestamp based on the SRI.
      // Otherwise, the adjustment is a noop.
      BULKIO::PrecisionUTCTime time = packet->T;
      double time_offset = packet_offset * packet->SRI.xdelta;
      size_t sample_offset = data_offset;
      if (packet->SRI.mode) {
        // Complex data; each sample is two values
        time_offset /= 2.0;
        sample_offset /= 2;
      }

      // If there is a time offset, apply the adjustment and mark the timestamp
      // so that the caller knows it was calculated rather than received
      bool synthetic = false;
      if (time_offset > 0.0) {
        time += time_offset;
        synthetic = true;
      }

      data.addTimestamp(bulkio::SampleTimestamp(time, sample_offset, synthetic));

      // The number of samples copied on this pass may be less than the total
      // remaining
      const size_t available = input_data.size() - packet_offset;
      const size_t pass = std::min(available, count);

      std::copy(&input_data[packet_offset], &input_data[packet_offset+pass], &data_buffer[data_offset]);
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

    // Advance the read pointers
    _consumeData(consume);

    return data;
  }

  const BULKIO::StreamSRI* _nextSRI(bool blocking)
  {
    if (_queue.empty()) {
      if (!_fetchPacket(blocking)) {
        return 0;
      }
    }

    return &(_queue.front()->SRI);
  }

  bool _fetchPacket(bool blocking)
  {
    // Don't fetch a packet from the port if stream is disabled
    if (!_enabled) {
      return false;
    }

    if (_pending) {
      // Cannot read another packet until non-bridging packet is acknowledged
      return false;
    }

    // Any future packets with this stream ID belong to another InputStream
    if (_eosState != EOS_NONE) {
      return false;
    }

    float timeout = blocking?bulkio::Const::BLOCKING:bulkio::Const::NON_BLOCKING;
    DataTransferType* packet = _port->getPacket(timeout, _streamID);
    if (!packet) {
      return false;
    }

    if (packet->EOS) {
      _eosState = EOS_RECEIVED;
    }
    if (_queue.empty() || _canBridge(packet)) {
      return _queuePacket(packet);
    } else {
      _pending = packet;
      return false;
    }
  }

  bool _queuePacket(DataTransferType* packet)
  {
    if (packet->EOS && packet->dataBuffer.empty()) {
      // Handle end-of-stream packet with no data (assuming that timestamps,
      // SRI changes, and queue flushes are irrelevant at this point)
      if (_queue.empty()) {
        // No queued packets, read pointer has reached end-of-stream
        _eosState = EOS_REACHED;
      } else {
        // Assign the end-of-stream flag to the last packet in the queue so
        // that it is handled on read
        _queue.back()->EOS = true;
      }
      delete packet;
      // Return false to let the caller know that no more sample data is
      // forthcoming
      return false;
    } else {
      _samplesQueued += packet->dataBuffer.size();
      _queue.push_back(packet);
      return true;
    }
  }

  bool _canBridge(DataTransferType* packet) const
  {
    return !(packet->sriChanged || packet->inputQueueFlushed);
  }

  const std::string _streamID;
  BULKIO::StreamSRI _sri;
  EosState _eosState;
  InPort<PortTraits>* _port;
  typedef std::vector<DataTransferType*> QueueType;
  QueueType _queue;
  DataTransferType* _pending;
  size_t _samplesQueued;
  size_t _sampleOffset;
  bool _enabled;
  bool _newstream;
};


template <class PortTraits>
InputStream<PortTraits>::InputStream() :
  _impl()
{
}

template <class PortTraits>
InputStream<PortTraits>::InputStream(const BULKIO::StreamSRI& sri, bulkio::InPort<PortTraits>* port) :
  _impl(new Impl(sri, port))
{
}

template <class PortTraits>
const std::string& InputStream<PortTraits>::streamID() const
{
  return _impl->streamID();
}

template <class PortTraits>
const BULKIO::StreamSRI& InputStream<PortTraits>::sri() const
{
  return _impl->sri();
}

template <class PortTraits>
bool InputStream<PortTraits>::eos()
{
  return _impl->eos();
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::read()
{
  return _impl->readPacket(true);
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::read(size_t count)
{
  return _impl->read(count, count, true);
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::read(size_t count, size_t consume)
{
  return _impl->read(count, consume, true);
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::tryread()
{
  return _impl->readPacket(false);
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::tryread(size_t count)
{
  return _impl->read(count, count, false);
}

template <class PortTraits>
typename InputStream<PortTraits>::DataBlockType InputStream<PortTraits>::tryread(size_t count, size_t consume)
{
  return _impl->read(count, consume, false);
}

template <class PortTraits>
size_t InputStream<PortTraits>::skip(size_t count)
{
  return _impl->skip(count);
}

template <class PortTraits>
bool InputStream<PortTraits>::enabled() const
{
  return _impl->enabled();
}

template <class PortTraits>
void InputStream<PortTraits>::enable()
{
  _impl->enable();
}

template <class PortTraits>
void InputStream<PortTraits>::disable()
{
  _impl->disable();
}

template <class PortTraits>
size_t InputStream<PortTraits>::samplesAvailable()
{
  return _impl->samplesAvailable();
}

template <class PortTraits>
bool InputStream<PortTraits>::operator!() const
{
  return !_impl;
}

template <class PortTraits>
bool InputStream<PortTraits>::operator==(const InputStream& other) const
{
  return _impl.get() == other._impl.get();
}

template <class PortTraits>
bool InputStream<PortTraits>::ready()
{
  return _impl->ready();
}

template <class PortTraits>
bool InputStream<PortTraits>::hasBufferedData()
{
  return _impl->hasBufferedData();
}

template <class PortTraits>
void InputStream<PortTraits>::close()
{
  return _impl->close();
}

template class InputStream<bulkio::CharPortTraits>;
template class InputStream<bulkio::OctetPortTraits>;
template class InputStream<bulkio::ShortPortTraits>;
template class InputStream<bulkio::UShortPortTraits>;
template class InputStream<bulkio::LongPortTraits>;
template class InputStream<bulkio::ULongPortTraits>;
template class InputStream<bulkio::LongLongPortTraits>;
template class InputStream<bulkio::ULongLongPortTraits>;
template class InputStream<bulkio::FloatPortTraits>;
template class InputStream<bulkio::DoublePortTraits>;
