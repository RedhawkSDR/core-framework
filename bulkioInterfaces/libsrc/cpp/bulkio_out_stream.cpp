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

#include "bulkio_out_stream.h"
#include "bulkio_out_port.h"
#include "bulkio_time_operators.h"
#include "bulkio_p.h"

using bulkio::OutputStream;

template <class PortTraits>
class OutputStream<PortTraits>::Impl : public bulkio::StreamBase::Impl {
public:
  typedef typename OutputStream<PortTraits>::ScalarBuffer ScalarBuffer;
  typedef typename OutputStream<PortTraits>::ComplexBuffer ComplexBuffer;

  Impl(const BULKIO::StreamSRI& sri, OutPortType* port) :
    bulkio::StreamBase::Impl(sri),
    _port(port),
    _bufferSize(0),
    _bufferOffset(0)
  {
  }

  void write(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time)
  {
    // If buffering is disabled, or the buffer is empty and the input data is
    // large enough for a full buffer, send it immediately
    if ((_bufferSize == 0) || (_bufferOffset == 0 && (data.size() >= _bufferSize))) {
      _send(data, time, false);
    } else {
      _doBuffer(data, time);
    }
  }

  void write(const ComplexBuffer& data, const BULKIO::PrecisionUTCTime& time)
  {
    if (_sri.mode == 0) {
      throw std::logic_error("stream mode is not complex");
    }
    write(ScalarBuffer::recast(data), time);
  }

  template <class Sample>
  void write(const redhawk::shared_buffer<Sample>& data, const std::list<bulkio::SampleTimestamp>& times)
  {
    std::list<bulkio::SampleTimestamp>::const_iterator timestamp = times.begin();
    if (timestamp == times.end()) {
      throw std::logic_error("no timestamps given");
    }

    size_t first = 0;
    while (first < data.size()) {
      size_t last = 0;
      const BULKIO::PrecisionUTCTime& when = timestamp->time;
      if (++timestamp == times.end()) {
        last = data.size();
      } else {
        last = timestamp->offset;
      }
      write(data.slice(first, last), when);
      first = last;
    }
  }

  size_t bufferSize() const
  {
    return _bufferSize;
  }

  void setBufferSize(size_t samples)
  {
    // Avoid needless thrashing
    if (samples == _bufferSize) {
      return;
    }
    _bufferSize = samples;

    // If the new buffer size is less than the currently buffered data, flush
    if (_bufferSize < _bufferOffset) {
      flush();
    } else if (_bufferSize > _buffer.size()) {
      // The buffer size is increasing beyond the existing allocation; allocate
      // a new buffer of the desired size and copy existing data
      redhawk::buffer<ScalarType> new_buffer(_bufferSize);
      if (_bufferOffset > 0) {
        std::copy(&_buffer[0], &_buffer[_bufferOffset], &new_buffer[0]);
      }

      // Replace the old buffer with the new one
      _buffer.swap(new_buffer);
    }
  }

  void flush()
  {
    if (_bufferOffset == 0) {
      return;
    }

    _flush(false);
  }

  virtual void close()
  {
    if (_bufferOffset > 0) {
      // Add the end-of-stream marker to the buffered data and its timestamp
      _flush(true);
    } else {
      // Send an empty packet with an end-of-stream marker; since there is no
      // sample data, the timestamp does not matter
      _send(ScalarBuffer(), bulkio::time::utils::notSet(), true);
    }
  }

private:
  void _send(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time, bool eos)
  {
    _port->pushPacket(data, time, eos, _streamID);
  }

  void _flush(bool eos)
  {
    // Push out all buffered data, which must be less than the full allocated
    // size otherwise it would have already been sent
    _send(_buffer.slice(0, _bufferOffset), _bufferTime, eos);

    // Allocate a new buffer and reset the offset index
    _buffer = redhawk::buffer<ScalarType>(_bufferSize);
    _bufferOffset = 0;
  }

  void _doBuffer(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time)
  {
    // If this is the first data being queued, use its timestamp for the start
    // time of the buffered data
    if (_bufferOffset == 0) {
      _bufferTime = time;
    }

    // Only buffer up to the currently configured buffer size
    size_t count = std::min(data.size(), _bufferSize - _bufferOffset);
    std::copy(&data[0], &data[count], &_buffer[_bufferOffset]);

    // Advance buffer offset, flushing if the buffer is full
    _bufferOffset += count;
    if (_bufferOffset >= _bufferSize) {
      _flush(false);
    }

    // Handle remaining data
    if (count < data.size()) {
      BULKIO::PrecisionUTCTime next = time + (_sri.xdelta * count);
      _doBuffer(data.slice(count), next);
    }
  }

  OutPortType* _port;

  redhawk::buffer<ScalarType> _buffer;
  BULKIO::PrecisionUTCTime _bufferTime;
  size_t _bufferSize;
  size_t _bufferOffset;
};

template <class PortTraits>
OutputStream<PortTraits>::OutputStream() :
  StreamBase()
{
}

template <class PortTraits>
OutputStream<PortTraits>::OutputStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
  StreamBase(boost::make_shared<Impl>(sri, port))
{
}

template <class PortTraits>
size_t OutputStream<PortTraits>::bufferSize() const
{
  return impl().bufferSize();
}

template <class PortTraits>
void OutputStream<PortTraits>::setBufferSize(size_t samples)
{
  impl().setBufferSize(samples);
}

template <class PortTraits>
void OutputStream<PortTraits>::flush()
{
  impl().flush();
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time)
{
  impl().write(data, time);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ScalarBuffer& data, const std::list<bulkio::SampleTimestamp>& times)
{
  impl().write(data, times);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ComplexBuffer& data, const BULKIO::PrecisionUTCTime& time)
{
  impl().write(data, time);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ComplexBuffer& data, const std::list<bulkio::SampleTimestamp>& times)
{
  impl().write(data, times);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ScalarType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
{
  impl().write(ScalarBuffer::make_transient(data, count), time);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ScalarType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
{
  impl().write(ScalarBuffer::make_transient(data, count), times);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ComplexType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
{
  impl().write(ComplexBuffer::make_transient(data, count), time);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ComplexType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
{
  impl().write(ComplexBuffer::make_transient(data, count), times);
}

template <class PortTraits>
typename OutputStream<PortTraits>::Impl& OutputStream<PortTraits>::impl()
{
    return static_cast<Impl&>(*_impl);
}

template <class PortTraits>
const typename OutputStream<PortTraits>::Impl& OutputStream<PortTraits>::impl() const
{
    return static_cast<const Impl&>(*_impl);
}

template <class PortTraits>
OutputStream<PortTraits>::operator unspecified_bool_type() const
{
    return _impl?static_cast<unspecified_bool_type>(&OutputStream::impl):0;
}


//
// XML
//
using bulkio::XMLPortTraits;

class OutputStream<XMLPortTraits>::Impl : public bulkio::StreamBase::Impl {
public:
    Impl(const BULKIO::StreamSRI& sri, OutPortType* port) :
        bulkio::StreamBase::Impl(sri),
        _port(port)
    {
    }

    void write(const std::string& data)
    {
        _send(data, false);
    }

    virtual void close()
    {
        // Send an empty packet with an end-of-stream marker
        _send(std::string(), true);
    }

private:
    void _send(const std::string& data, bool eos)
    {
        _port->pushPacket(data, true, _streamID);
    }

    OutPortType* _port;
};

OutputStream<XMLPortTraits>::OutputStream() :
    StreamBase()
{
}

OutputStream<XMLPortTraits>::OutputStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    StreamBase(boost::make_shared<OutputStream::Impl>(sri, port))
{
}

void OutputStream<XMLPortTraits>::write(const std::string& xmlString)
{
    impl().write(xmlString);
}

OutputStream<XMLPortTraits>::Impl& OutputStream<XMLPortTraits>::impl()
{
    return static_cast<OutputStream::Impl&>(*_impl);
}

OutputStream<XMLPortTraits>::operator unspecified_bool_type() const
{
    return _impl?&OutputStream::impl:0;
}

//
// File
//
using bulkio::FilePortTraits;

class OutputStream<FilePortTraits>::Impl : public bulkio::StreamBase::Impl {
public:
    Impl(const BULKIO::StreamSRI& sri, OutPortType* port) :
        bulkio::StreamBase::Impl(sri),
        _port(port)
    {
    }

    void write(const std::string& data, const BULKIO::PrecisionUTCTime& time)
    {
        _send(data, time, false);
    }

    virtual void close()
    {
        // Send an empty packet with an end-of-stream marker; since there is no
        // sample data, the timestamp does not matter
        _send(std::string(), bulkio::time::utils::notSet(), true);
    }

private:
    void _send(const std::string& data, const BULKIO::PrecisionUTCTime& time, bool eos)
    {
        _port->pushPacket(data, time, true, _streamID);
    }

    OutPortType* _port;
};

OutputStream<FilePortTraits>::OutputStream() :
    StreamBase()
{
}

OutputStream<FilePortTraits>::OutputStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    StreamBase(boost::make_shared<OutputStream::Impl>(sri, port))
{
}

void OutputStream<FilePortTraits>::write(const std::string& URL, const BULKIO::PrecisionUTCTime& time)
{
    impl().write(URL, time);
}

OutputStream<FilePortTraits>::Impl& OutputStream<FilePortTraits>::impl()
{
    return static_cast<OutputStream::Impl&>(*_impl);
}

OutputStream<FilePortTraits>::operator unspecified_bool_type() const
{
    return _impl?&OutputStream::impl:0;
}

template class OutputStream<bulkio::CharPortTraits>;
template class OutputStream<bulkio::OctetPortTraits>;
template class OutputStream<bulkio::ShortPortTraits>;
template class OutputStream<bulkio::UShortPortTraits>;
template class OutputStream<bulkio::LongPortTraits>;
template class OutputStream<bulkio::ULongPortTraits>;
template class OutputStream<bulkio::LongLongPortTraits>;
template class OutputStream<bulkio::ULongLongPortTraits>;
template class OutputStream<bulkio::FloatPortTraits>;
template class OutputStream<bulkio::DoublePortTraits>;
