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
#include "bulkio_stream.h"
#include "bulkio_time_operators.h"

namespace bulkio {
    namespace {
        double get_drift(const SampleTimestamp& begin, const bulkio::SampleTimestamp& end, double xdelta)
        {
            double real = end.time - begin.time;
            double expected = (end.offset - begin.offset)*xdelta;
            return real-expected;
        }

        void validate_timestamps(const std::list<SampleTimestamp>& timestamps)
        {
            // Validity checks
            if (timestamps.empty()) {
                throw std::logic_error("block contains no timestamps");
            } else if (timestamps.front().offset != 0) {
                throw std::logic_error("no timestamp at offset 0");
            }
        }
    }
}

using bulkio::SampleTimestamp;
using bulkio::DataBlock;

template <class T>
struct DataBlock<T>::Impl : public StreamDescriptor
{
    Impl(const bulkio::StreamDescriptor& sri, const T& data) :
        StreamDescriptor(sri),
        data(data),
        sriChangeFlags(bulkio::sri::NONE),
        inputQueueFlushed(false),
        dataOwned(false)
    {
    }

    void copy()
    {
        // Default implementation assumes a shared-ownership class for data
        // (shared_buffer, shared_bitbuffer); make a copy and tag the data as
        // owned
        data = data.copy();
        dataOwned = true;
    }

    T data;
    std::list<SampleTimestamp> timestamps;
    int sriChangeFlags;
    bool inputQueueFlushed;

    // Flag to track whether a shared_buffer might share references with other
    // consumers (e.g., other input ports in the same process space). Note that
    // this does *not* track whether there is another reference to the same
    // data block.
    bool dataOwned;
};

namespace bulkio {
    template <>
    void DataBlock<std::string>::Impl::copy()
    {
        // Strings do not have shared ownership, making this a no-op
    }
}

template <class T>
DataBlock<T>::DataBlock() :
    _impl()
{
}

template <class T>
DataBlock<T>::DataBlock(const bulkio::StreamDescriptor& sri, const T& data) :
    _impl(boost::make_shared<Impl>(sri, data))
{
}

template <class T>
DataBlock<T> DataBlock<T>::copy() const
{
    DataBlock result;
    if (_impl) {
        result._impl = boost::make_shared<Impl>(*_impl);
        result._impl->copy();
    }
    return result;
}

template <class T>
const BULKIO::StreamSRI& DataBlock<T>::sri() const
{
    return _impl->sri();
}

template <class T>
double DataBlock<T>::xdelta() const
{
    return sri().xdelta;
}

template <class T>
const T& DataBlock<T>::buffer() const
{
    return _impl->data;
}

template <class T>
void DataBlock<T>::buffer(const T& data)
{
    _impl->data = data;
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

template <class T>
const BULKIO::PrecisionUTCTime& DataBlock<T>::getStartTime() const
{
    const std::list<SampleTimestamp>& timestamps = _impl->timestamps;
    validate_timestamps(timestamps);

    return _impl->timestamps.front().time;
}

template <class T>
double DataBlock<T>::getNetTimeDrift() const
{
    const std::list<SampleTimestamp>& timestamps = _impl->timestamps;
    validate_timestamps(timestamps);

    return get_drift(timestamps.front(), timestamps.back(), xdelta());
}

template <class T>
double DataBlock<T>::getMaxTimeDrift() const
{
    const std::list<SampleTimestamp>& timestamps = _impl->timestamps;
    validate_timestamps(timestamps);

    double max = 0.0;
    std::list<SampleTimestamp>::const_iterator current = timestamps.begin();
    std::list<SampleTimestamp>::const_iterator next = current;
    ++next;
    for (; next != timestamps.end(); ++current, ++next) {
        double drift = get_drift(*current, *next, xdelta());
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
DataBlock<T>::operator unspecified_bool_type() const
{
    return _impl?&DataBlock::_impl:0;
}


//
// SampleDataBlock
//
using bulkio::SampleDataBlock;

template <class T>
SampleDataBlock<T>::SampleDataBlock() :
    Base()
{
}

template <class T>
SampleDataBlock<T>::SampleDataBlock(const bulkio::StreamDescriptor& sri,
                                    const ScalarBuffer& buffer) :
    Base(sri, buffer)
{
}

template <class T>
SampleDataBlock<T>::SampleDataBlock(const BULKIO::StreamSRI& sri, size_t size) :
    Base(bulkio::StreamDescriptor(sri), redhawk::buffer<T>(size))
{
}

template <class T>
SampleDataBlock<T> SampleDataBlock<T>::copy() const
{
    // To support the covariant return type without reimplementing the method,
    // use the base class copy() to return a temporary and then reinterpret it
    // as the derived type to pass to the copy constructor. Since there are no
    // additional data members, the only concern is a potential extra copy of
    // the shared pointer (which may get optimized out by the compiler).
    Base temp = Base::copy();
    return SampleDataBlock(reinterpret_cast<const SampleDataBlock&>(temp));
}

template <class T>
T* SampleDataBlock<T>::data()
{
    // To preserve data integrity of shared buffers received from a port, make
    // a one-time copy of the buffer and assume ownership of it
    if (!_impl->dataOwned) {
        _impl->copy();
    }
    return const_cast<T*>(_impl->data.data());
}

template <class T>
const T* SampleDataBlock<T>::data() const
{
    return _impl->data.data();
}

template <class T>
size_t SampleDataBlock<T>::size() const
{
    return _impl->data.size();
}

template <class T>
void SampleDataBlock<T>::resize(size_t count)
{
    // We have to create a writeable temporary buffer, and although there is a
    // resize() operation, it would potentially require two allocations and two
    // memory copies: once to make a copy of the current buffer, and again on
    // the resize. Instead, we allocate in one step and copy in another.
    redhawk::buffer<T> temp(count);
    temp.replace(0, std::min(_impl->data.size(), count), _impl->data);
    _impl->data = temp;
    // Creating a new buffer also alleviates concern about buffer sharing
    _impl->dataOwned = true;
}

template <class T>
bool SampleDataBlock<T>::complex() const
{
    return _impl->complex();
}

template <class T>
std::complex<T>* SampleDataBlock<T>::cxdata()
{
    // Defer to the data() method to ensure that the data gets copied if it's a
    // potentially shared reference
    return reinterpret_cast<ComplexType*>(data());
}

template <class T>
const std::complex<T>* SampleDataBlock<T>::cxdata() const
{
    return reinterpret_cast<const ComplexType*>(data());
}

template <class T>
size_t SampleDataBlock<T>::cxsize() const
{
    return size() / 2;
}

template <class T>
void SampleDataBlock<T>::swap(std::vector<ScalarType>& other)
{
    // Copy the vector data into a new shared buffer
    ScalarBuffer data = ScalarBuffer::make_transient(&other[0], other.size()).copy();
    // Swap the block's data with the new shared buffer
    _impl->data.swap(data);
    // Assign the old data to the vector
    other.assign(data.begin(), data.end());
}

template <class T>
const typename SampleDataBlock<T>::ScalarBuffer& SampleDataBlock<T>::buffer() const
{
    // This method is overridden to extend the documentation to cover real vs.
    // complex data, but does not modify the behavior
    return Base::buffer();
}

template <class T>
typename SampleDataBlock<T>::ComplexBuffer SampleDataBlock<T>::cxbuffer() const
{
    return ComplexBuffer::recast(buffer());
}

// Instantiate templates for supported types
#define INSTANTIATE_TEMPLATE(x)                 \
    template class DataBlock< x >;

#define INSTANTIATE_NUMERIC_TEMPLATE(x)                 \
    INSTANTIATE_TEMPLATE(redhawk::shared_buffer<x>);    \
    template class SampleDataBlock<x>;

// String (XML, file) and bit blocks use the basic DataBlock class
INSTANTIATE_TEMPLATE(std::string);
INSTANTIATE_TEMPLATE(redhawk::shared_bitbuffer);

// Numeric types support the full SampleDataBlock interface
INSTANTIATE_NUMERIC_TEMPLATE(int8_t);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::Octet);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::Short);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::UShort);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::Long);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::ULong);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::LongLong);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::ULongLong);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::Float);
INSTANTIATE_NUMERIC_TEMPLATE(CORBA::Double);
