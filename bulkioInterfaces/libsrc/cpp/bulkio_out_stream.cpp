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

using bulkio::OutputStream;

template <class PortTraits>
class OutputStream<PortTraits>::Impl {
public:
  typedef typename PortTraits::DataTransferTraits::NativeDataType ScalarType;
  typedef std::complex<ScalarType> ComplexType;
  typedef typename PortTraits::DataTransferTraits::TransportType TransportType;

  Impl(const std::string& streamID, bulkio::OutPort<PortTraits>* port) :
    _streamID(streamID),
    _port(port),
    _sri(bulkio::sri::create(streamID)),
    _sriUpdated(true)
  {
  }

  Impl(const BULKIO::StreamSRI& sri, bulkio::OutPort<PortTraits>* port) :
    _streamID(sri.streamID),
    _port(port),
    _sri(sri),
    _sriUpdated(true)
  {
  }

  const std::string& streamID() const
  {
    return _streamID;
  }

  const BULKIO::StreamSRI& sri() const
  {
    return _sri;
  }

  void setSRI(const BULKIO::StreamSRI& sri)
  {
    // Copy the new SRI, except for the stream ID, which is immutable
    _sri = sri;
    _sri.streamID = _streamID.c_str();
    _sriUpdated = true;
  }

  void setXDelta(double delta)
  {
    _setStreamMetadata(_sri.xdelta, delta);
  }

  void setComplex(bool mode)
  {
    _setStreamMetadata(_sri.mode, mode?1:0);
  }

  void setBlocking(bool blocking)
  {
    _setStreamMetadata(_sri.blocking, blocking?1:0);
  }

  void setKeywords(const _CORBA_Unbounded_Sequence<CF::DataType>& properties)
  {
    _sri.keywords = properties;
    _sriUpdated = true;
  }

  void setKeyword(const std::string& name, const CORBA::Any& value)
  {
    redhawk::PropertyMap::cast(_sri.keywords)[name] = value;
    _sriUpdated = true;
  }

  void eraseKeyword(const std::string& name)
  {
    redhawk::PropertyMap::cast(_sri.keywords).erase(name);
    _sriUpdated = true;
  }

  template <class Sample>
  void write(const Sample* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
  {
    std::list<bulkio::SampleTimestamp>::const_iterator timestamp = times.begin();
    if (timestamp == times.end()) {
      throw std::logic_error("no timestamps given");
    }

    size_t first = 0;
    while (first < count) {
      size_t last = 0;
      const BULKIO::PrecisionUTCTime& when = timestamp->time;
      if (++timestamp == times.end()) {
        last = count;
      } else {
        last = timestamp->offset;
      }
      const size_t pass = last-first;
      write(data+first, pass, when);
      first += pass;
    }
  }

  void write(const ScalarType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
  {
    if (_sriUpdated) {
      _port->pushSRI(_sri);
      _sriUpdated = false;
    }
    _send(reinterpret_cast<const TransportType*>(data), count, time, false);
  }

  void write(const ComplexType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
  {
    if (_sri.mode == 0) {
      throw std::logic_error("stream mode is not complex");
    }
    write(reinterpret_cast<const ScalarType*>(data), count*2, time);
  }

  void close()
  {
    // Send an empty packet with an end-of-stream marker; since there is no
    // sample data, the timestamp does not matter
    _send(0, 0, bulkio::time::utils::notSet(), true);
  }

private:
  void _send(const TransportType* data, size_t count, const BULKIO::PrecisionUTCTime& time, bool eos)
  {
    _port->pushPacket(data, count, time, eos, _streamID);
  }

  template <typename Field, typename Value>
  void _setStreamMetadata(Field& field, Value value)
  {
    if (field == value) {
      return;
    }
    field = value;
    _sriUpdated = true;
  }

  const std::string _streamID;
  OutPort<PortTraits>* _port;
  BULKIO::StreamSRI _sri;
  bool _sriUpdated;
};

template <class PortTraits>
OutputStream<PortTraits>::OutputStream() :
  _impl()
{
}

template <class PortTraits>
OutputStream<PortTraits>::OutputStream(const BULKIO::StreamSRI& sri, bulkio::OutPort<PortTraits>* port) :
  _impl(new Impl(sri, port))
{
}

template <class PortTraits>
const std::string& OutputStream<PortTraits>::streamID() const
{
  return _impl->streamID();
}

template <class PortTraits>
const BULKIO::StreamSRI& OutputStream<PortTraits>::sri() const
{
  return _impl->sri();
}

template <class PortTraits>
void OutputStream<PortTraits>::sri(const BULKIO::StreamSRI& sri)
{
  _impl->setSRI(sri);
}

template <class PortTraits>
double OutputStream<PortTraits>::xdelta() const
{
  return _impl->sri().xdelta;
}

template <class PortTraits>
void OutputStream<PortTraits>::xdelta(double delta)
{
  _impl->setXDelta(delta);
}

template <class PortTraits>
bool OutputStream<PortTraits>::complex() const
{
  return (_impl->sri().mode != 0);
}

template <class PortTraits>
void OutputStream<PortTraits>::complex(bool mode)
{
  _impl->setComplex(mode);
}

template <class PortTraits>
bool OutputStream<PortTraits>::blocking() const
{
  return _impl->sri().blocking;
}

template <class PortTraits>
void OutputStream<PortTraits>::blocking(bool mode)
{
  _impl->setBlocking(mode);
}

template <class PortTraits>
const redhawk::PropertyMap& OutputStream<PortTraits>::keywords() const
{
  return redhawk::PropertyMap::cast(_impl->sri().keywords);
}

template <class PortTraits>
void OutputStream<PortTraits>::keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props)
{
  _impl->setKeywords(props);
}

template <class PortTraits>
bool OutputStream<PortTraits>::hasKeyword(const std::string& name) const
{
  return keywords().contains(name);
}

template <class PortTraits>
const redhawk::Value& OutputStream<PortTraits>::getKeyword(const std::string& name) const
{
  return keywords()[name];
}

template <class PortTraits>
void OutputStream<PortTraits>::setKeyword(const std::string& name, const CORBA::Any& value)
{
    _impl->setKeyword(name, value);
}

template <class PortTraits>
void OutputStream<PortTraits>::setKeyword(const std::string& name, const redhawk::Value& value)
{
    _impl->setKeyword(name, value);
}

template <class PortTraits>
void OutputStream<PortTraits>::eraseKeyword(const std::string& name)
{
    _impl->eraseKeyword(name);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ScalarType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
{
  _impl->write(data, count, time);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ScalarType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
{
  _impl->write(data, count, times);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ComplexType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
{
  _impl->write(data, count, time);
}

template <class PortTraits>
void OutputStream<PortTraits>::write(const ComplexType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
{
  _impl->write(data, count, times);
}

template <class PortTraits>
void OutputStream<PortTraits>::close()
{
  _impl->close();
  _impl.reset();
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
