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

  Impl(const BULKIO::StreamSRI& sri, bulkio::InPort<PortTraits>* port) :
    _streamID(sri.streamID),
    _sri(sri),
    _eosReceived(false),
    _eosReached(false),
    _port(port),
    _queue(),
    _pending(0),
    _samplesQueued(0),
    _sampleOffset(0),
    _enabled(true)
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
    return _eosReached;
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
    // flush pending
    if (!_pending) {
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
      if (!blocking && !_pending && !_eosReceived) {
        return DataBlockType();
      }
      // Otherwise, consume all remaining data
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
    if (!_enabled) {
      return false;
    } else if (_samplesQueued) {
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
    _enabled = true;
  }

  void disable()
  {
    _enabled = false;
    // TODO: purge queue
  }

  bool hasBufferedData() const
  {
    return !_queue.empty() || _pending;
  }

private:
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

  void _consumePacket()
  {
    // Acknowledge any end-of-stream flag and delete the packet
    DataTransferType* packet = _queue.front();
    _eosReached = packet->EOS;
    if (_eosReached) {
      _port->removeStream(_streamID);
    }

    // The packet buffer was allocated with new[] by the CORBA layer, while
    // vector will use non-array delete, so explicitly delete the buffer
    delete[] steal_buffer(packet->dataBuffer);
    delete packet;
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
    if (_pending) {
      // Cannot read another packet until non-bridging packet is acknowledged
      return false;
    }

    // Any future packets with this stream ID belong to another InputStream
    if (_eosReceived) {
      return false;
    }

    float timeout = blocking?bulkio::Const::BLOCKING:bulkio::Const::NON_BLOCKING;
    DataTransferType* packet = _port->getPacket(timeout, _streamID);
    if (!packet) {
      return false;
    }

    _eosReceived = packet->EOS;
    if (_queue.empty() || _canBridge(packet)) {
      _queuePacket(packet);
      return true;
    } else {
      _pending = packet;
      return false;
    }
  }

  void _queuePacket(DataTransferType* packet)
  {
    if (packet->EOS && packet->dataBuffer.empty()) {
      // Handle end-of-stream packet with no data (assuming that timestamps,
      // SRI changes, and queue flushes are irrelevant at this point)
      if (_queue.empty()) {
        // No queued packets, read pointer has reached end-of-stream
        _eosReached = true;
        _port->removeStream(_streamID);
      } else {
        // Assign the end-of-stream flag to the last packet in the queue so
        // that it is handled on read
        _queue.back()->EOS = true;
      }
      delete packet;
    } else {
      _samplesQueued += packet->dataBuffer.size();
      _queue.push_back(packet);
    }
  }

  bool _canBridge(DataTransferType* packet) const
  {
    return !(packet->sriChanged || packet->inputQueueFlushed);
  }

  const std::string _streamID;
  BULKIO::StreamSRI _sri;
  bool _eosReceived;
  bool _eosReached;
  InPort<PortTraits>* _port;
  std::vector<DataTransferType*> _queue;
  DataTransferType* _pending;
  size_t _samplesQueued;
  size_t _sampleOffset;
  bool _enabled;
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
