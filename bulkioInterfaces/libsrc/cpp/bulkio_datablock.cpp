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

#include <stdexcept>
#include <vector>

#include <boost/make_shared.hpp>

#include "bulkio_base.h"
#include "bulkio_datablock.h"
#include "bulkio_time_operators.h"

using bulkio::SampleTimestamp;
using bulkio::DataBlock;

template <class T>
struct DataBlock<T>::Impl
{
  std::vector<T> data;
  BULKIO::StreamSRI sri;
  std::list<SampleTimestamp> timestamps;
  int sriChangeFlags;
  bool inputQueueFlushed;
};

template <class T>
DataBlock<T>::DataBlock() :
  _impl()
{
}

template <class T>
DataBlock<T>::DataBlock(const BULKIO::StreamSRI& sri, size_t size) :
  _impl(new Impl())
{
  _impl->data.resize(size);
  _impl->sri = sri;
}

template <class T>
DataBlock<T> DataBlock<T>::copy() const
{
  DataBlock result;
  if (_impl) {
    result._impl = boost::make_shared<Impl>(*_impl);
  }
  return result;
}

template <class T>
const BULKIO::StreamSRI& DataBlock<T>::sri() const
{
  return _impl->sri;
}

template <class T>
double DataBlock<T>::xdelta() const
{
  return _impl->sri.xdelta;
}

template <class T>
T* DataBlock<T>::data()
{
  return &(_impl->data[0]);
}

template <class T>
const T* DataBlock<T>::data() const
{
  return &(_impl->data[0]);
}

template <class T>
size_t DataBlock<T>::size() const
{
  return _impl->data.size();
}

template <class T>
void DataBlock<T>::resize(size_t count)
{
  _impl->data.resize(count);
}

template <class T>
bool DataBlock<T>::complex() const
{
  return (_impl->sri.mode != 0);
}

template <class T>
std::complex<T>* DataBlock<T>::cxdata()
{
  return reinterpret_cast<ComplexType*>(data());
}

template <class T>
const std::complex<T>* DataBlock<T>::cxdata() const
{
  return reinterpret_cast<const ComplexType*>(data());
}

template <class T>
size_t DataBlock<T>::cxsize() const
{
  return size() / 2;
}

template <class T>
void DataBlock<T>::addTimestamp(const bulkio::SampleTimestamp& timestamp)
{
  std::list<SampleTimestamp>::iterator pos = _impl->timestamps.begin();
  const std::list<SampleTimestamp>::iterator end = _impl->timestamps.end();
  while ((pos != end) && (timestamp.offset >= pos->offset)) {
      // TODO: Replace existing
      ++pos;
  }
  _impl->timestamps.insert(pos, timestamp);
}

template <class T>
std::list<SampleTimestamp> DataBlock<T>::getTimestamps() const
{
  return _impl->timestamps;
}

namespace {
  double get_drift(const bulkio::SampleTimestamp& begin, const bulkio::SampleTimestamp& end, double xdelta)
  {
    double real = end.time - begin.time;
    double expected = (end.offset - begin.offset)*xdelta;
    return real-expected;
  }
}

template <class T>
double DataBlock<T>::getNetTimeDrift() const
{
  const std::list<SampleTimestamp>& timestamps = _impl->timestamps;
  // Validity checks
  if (timestamps.empty()) {
    throw std::logic_error("block contains no timestamps");
  } else if (timestamps.front().offset != 0) {
    throw std::logic_error("no timestamp at offset 0");
  }

  return get_drift(timestamps.front(), timestamps.back(), _impl->sri.xdelta);
}

template <class T>
double DataBlock<T>::getMaxTimeDrift() const
{
  const std::list<SampleTimestamp>& timestamps = _impl->timestamps;
  // Validity checks
  if (timestamps.empty()) {
    throw std::logic_error("block contains no timestamps");
  } else if (timestamps.front().offset != 0) {
    throw std::logic_error("no timestamp at offset 0");
  }

  double max = 0.0;
  std::list<SampleTimestamp>::const_iterator current = timestamps.begin();
  std::list<SampleTimestamp>::const_iterator next = current;
  ++next;
  for (; next != timestamps.end(); ++current, ++next) {
    double drift = get_drift(*current, *next, _impl->sri.xdelta);
    if (std::abs(drift) > std::abs(max)) {
      max = drift;
    }
  }
  return max;
}

template <class T>
bool DataBlock<T>::sriChanged() const
{
  return sriChangeFlags() != bulkio::sri::NONE;
}

template <class T>
int DataBlock<T>::sriChangeFlags() const
{
  return _impl->sriChangeFlags;
}

template <class T>
void DataBlock<T>::sriChangeFlags(int flags)
{
  _impl->sriChangeFlags = flags;
}

template <class T>
bool DataBlock<T>::inputQueueFlushed() const
{
  return _impl->inputQueueFlushed;
}

template <class T>
void DataBlock<T>::inputQueueFlushed(bool flushed)
{
  _impl->inputQueueFlushed = flushed;
}

template <class T>
void DataBlock<T>::swap(std::vector<ScalarType>& other)
{
  _impl->data.swap(other);
}

// Instantiate templates for supported types
template class DataBlock<int8_t>;
template class DataBlock<CORBA::Octet>;
template class DataBlock<CORBA::Short>;
template class DataBlock<CORBA::UShort>;
template class DataBlock<CORBA::Long>;
template class DataBlock<CORBA::ULong>;
template class DataBlock<CORBA::LongLong>;
template class DataBlock<CORBA::ULongLong>;
template class DataBlock<CORBA::Float>;
template class DataBlock<CORBA::Double>;
