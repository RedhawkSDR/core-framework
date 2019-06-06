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
#include <ossie/prop_helpers.h>

#include "bulkio_out_stream.h"
#include "bulkio_out_port.h"
#include "bulkio_time_operators.h"
#include "bulkio_p.h"

using bulkio::OutputStream;

namespace {
  // Traits class to distinguish between scalar (real) and complex data
  template <typename T>
  struct is_complex {
    static const bool value = false;
  };

  // Specialization of traits class to identify complex data types
  template <typename T>
  struct is_complex<std::complex<T> > {
    static const bool value = true;
  };
}

template <class PortType>
class OutputStream<PortType>::Impl : public StreamBase::Impl {
public:
    typedef StreamBase::Impl ImplBase;
    typedef typename BufferTraits<PortType>::BufferType BufferType;

    Impl(const BULKIO::StreamSRI& sri, OutPortType* port) :
        ImplBase(sri),
        _modcount(0),
        _port(port)
    {
    }

    virtual ~Impl()
    {
    }

    virtual void close()
    {
        // Send an empty packet with an end-of-stream marker; since there is no
        // sample data, the timestamp does not matter
        _send(BufferType(), bulkio::time::utils::notSet(), true);
    }

    void setXStart(double start)
    {
        _setStreamMetadata(_sri->xstart, start);
    }

    void setXDelta(double delta)
    {
        _setStreamMetadata(_sri->xdelta, delta);
    }

    void setXUnits(short units)
    {
        _setStreamMetadata(_sri->xunits, units);
    }

    void setSubsize(int size)
    {
        _setStreamMetadata(_sri->subsize, size);
    }

    void setYStart(double start)
    {
        _setStreamMetadata(_sri->ystart, start);
    }

    void setYDelta(double delta)
    {
        _setStreamMetadata(_sri->ydelta, delta);
    }

    void setYUnits(short units)
    {
        _setStreamMetadata(_sri->yunits, units);
    }

    void setComplex(bool mode)
    {
        _setStreamMetadata(_sri->mode, mode?1:0);
    }

    void setBlocking(bool mode)
    {
        _setStreamMetadata(_sri->blocking, mode?1:0);
    }

    void setKeywords(const _CORBA_Unbounded_Sequence<CF::DataType>& properties)
    {
        if ( !bulkio::sri::compareKeywords(_sri->keywords, properties) ) {
            _modifyingStreamMetadata();
            _sri->keywords = properties;
            ++_modcount;
        }
    }

    void setKeyword(const std::string& name, const CORBA::Any& value)
    {
        redhawk::PropertyMap & sri_keywords = redhawk::PropertyMap::cast(_sri->keywords);
        redhawk::PropertyMap::const_iterator it = sri_keywords.find(name);
        if ( it != sri_keywords.end() ) {
            const CORBA::Any & value_orig = it->getValue();
            std::string action("eq");
            if ( ossie::compare_anys(value_orig, value, action) ) {
                return;
            }
        }
        _modifyingStreamMetadata();
        sri_keywords[name] = value;
        ++_modcount;
    }

    void eraseKeyword(const std::string& name)
    {
        redhawk::PropertyMap & sri_keywords = redhawk::PropertyMap::cast(_sri->keywords);
        if (sri_keywords.contains(name)) {
            _modifyingStreamMetadata();
            sri_keywords.erase(name);
            ++_modcount;
        }
    }

    void setSRI(const BULKIO::StreamSRI& sri)
    {
        // If the SRI is the same, or differs only by streamID, do nothing.
        int comparison = bulkio::sri::compareFields(*_sri, sri);
        if ( (comparison | bulkio::sri::STREAMID) != bulkio::sri::STREAMID ) {
            _modifyingStreamMetadata();
            // Copy the new SRI, except for the stream ID, which is immutable
            *_sri = sri;
            _sri->streamID = _streamID.c_str();
            ++_modcount;
        }
    }

    virtual void write(const BufferType& data, const BULKIO::PrecisionUTCTime& time)
    {
        _send(data, time, false);
    }

    int modcount() const
    {
        return _modcount;
    }

protected:
    virtual void _modifyingStreamMetadata()
    {
        // By default, do nothing
    }

    template <typename Field, typename Value>
    void _setStreamMetadata(Field& field, Value value)
    {
        if (field != value) {
            _modifyingStreamMetadata();
            field = value;
            ++_modcount;
        }
    }

    void _send(const BufferType& data, const BULKIO::PrecisionUTCTime& time, bool eos)
    {
        _port->_sendPacket(data, time, eos, _streamID);
    }

    int _modcount;
    OutPortType* _port;
};

template <class PortType>
OutputStream<PortType>::OutputStream() :
    StreamBase()
{
}

template <class PortType>
OutputStream<PortType>::OutputStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    StreamBase(boost::make_shared<Impl>(sri, port))
{
}

template <class PortType>
OutputStream<PortType>::OutputStream(boost::shared_ptr<Impl> impl) :
    StreamBase(impl)
{
}

template <class PortType>
void OutputStream<PortType>::sri(const BULKIO::StreamSRI& sri)
{
    impl().setSRI(sri);
}

template <class PortType>
void OutputStream<PortType>::xstart(double start)
{
    impl().setXStart(start);
}

template <class PortType>
void OutputStream<PortType>::xdelta(double delta)
{
    impl().setXDelta(delta);
}

template <class PortType>
void OutputStream<PortType>::xunits(short units)
{
    impl().setXUnits(units);
}

template <class PortType>
void OutputStream<PortType>::subsize(int size)
{
    impl().setSubsize(size);
}

template <class PortType>
void OutputStream<PortType>::ystart(double start)
{
    impl().setYStart(start);
}

template <class PortType>
void OutputStream<PortType>::ydelta(double delta)
{
    impl().setYDelta(delta);
}

template <class PortType>
void OutputStream<PortType>::yunits(short units)
{
    impl().setYUnits(units);
}

template <class PortType>
void OutputStream<PortType>::complex(bool mode)
{
    impl().setComplex(mode);
}

template <class PortType>
void OutputStream<PortType>::blocking(bool mode)
{
    impl().setBlocking(mode);
}

template <class PortType>
void OutputStream<PortType>::keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props)
{
    impl().setKeywords(props);
}

template <class PortType>
void OutputStream<PortType>::setKeyword(const std::string& name, const CORBA::Any& value)
{
    impl().setKeyword(name, value);
}

template <class PortType>
void OutputStream<PortType>::setKeyword(const std::string& name, const redhawk::Value& value)
{
    impl().setKeyword(name, value);
}

template <class PortType>
void OutputStream<PortType>::eraseKeyword(const std::string& name)
{
    impl().eraseKeyword(name);
}

template <class PortType>
void OutputStream<PortType>::close()
{
    impl().close();
    _impl.reset();
}

template <class PortType>
OutputStream<PortType>::operator unspecified_bool_type() const
{
    return _impl?static_cast<unspecified_bool_type>(&OutputStream::impl):0;
}

template <class PortType>
bool OutputStream<PortType>::operator==(const OutputStream& other) const
{
    return _impl == other._impl;
}

template <class PortType>
bool OutputStream<PortType>::operator!=(const OutputStream& other) const
{
    return !(*this == other);
}

template <class PortType>
typename OutputStream<PortType>::Impl& OutputStream<PortType>::impl()
{
    return static_cast<Impl&>(*this->_impl);
}

template <class PortType>
const typename OutputStream<PortType>::Impl& OutputStream<PortType>::impl() const
{
    return static_cast<const Impl&>(*this->_impl);
}

template <class PortType>
int OutputStream<PortType>::modcount() const
{
    return impl().modcount();
}


using bulkio::BufferedOutputStream;

template <class PortType>
class BufferedOutputStream<PortType>::Impl : public Base::Impl {
public:
    typedef typename Base::Impl ImplBase;

    typedef typename BufferTraits<PortType>::BufferType BufferType;
    typedef typename BufferTraits<PortType>::MutableBufferType MutableBufferType;

    using ImplBase::_sri;
    using ImplBase::_streamID;

    Impl(const BULKIO::StreamSRI& sri, OutPortType* port) :
        ImplBase::Impl(sri, port),
        _bufferSize(0),
        _bufferOffset(0)
    {
    }

    void write(const BufferType& data, const BULKIO::PrecisionUTCTime& time)
    {
        // If buffering is disabled, or the buffer is empty and the input data is
        // large enough for a full buffer, send it immediately
        if ((_bufferSize == 0) || (_bufferOffset == 0 && (data.size() >= _bufferSize))) {
            ImplBase::write(data, time);
        } else {
            _doBuffer(data, time);
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

        // If the new buffer size is less than (or exactly equal to) the
        // currently buffered data size, flush
        if (_bufferSize <= _bufferOffset) {
            flush();
        } else if (_bufferSize > _buffer.size()) {
            // The buffer size is increasing beyond the existing allocation
            _buffer.resize(_bufferSize);
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
            ImplBase::close();
        }
    }

private:
    virtual void _modifyingStreamMetadata()
    {
        // Flush any data queued with the old SRI
        flush();
    }

    void _flush(bool eos)
    {
        // Push out all buffered data, which must be less than the full allocated
        // size otherwise it would have already been sent
        this->_send(_buffer.slice(0, _bufferOffset), _bufferTime, eos);

        // Allocate a new buffer and reset the offset index
        _buffer = MutableBufferType(_bufferSize);
        _bufferOffset = 0;
    }

    void _doBuffer(const BufferType& data, const BULKIO::PrecisionUTCTime& time)
    {
        // If this is the first data being queued, use its timestamp for the start
        // time of the buffered data
        if (_bufferOffset == 0) {
            _bufferTime = time;
        }

        // Only buffer up to the currently configured buffer size
        size_t count = std::min(data.size(), _bufferSize - _bufferOffset);
        _buffer.replace(_bufferOffset, count, data);

        // Advance buffer offset, flushing if the buffer is full
        _bufferOffset += count;
        if (_bufferOffset >= _bufferSize) {
            _flush(false);
        }

        // Handle remaining data
        if (count < data.size()) {
            BULKIO::PrecisionUTCTime next = time + (_sri->xdelta * count);
            _doBuffer(data.slice(count), next);
        }
    }

    MutableBufferType _buffer;
    BULKIO::PrecisionUTCTime _bufferTime;
    size_t _bufferSize;
    size_t _bufferOffset;
};

template <class PortType>
BufferedOutputStream<PortType>::BufferedOutputStream() :
    Base()
{
}

template <class PortType>
BufferedOutputStream<PortType>::BufferedOutputStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    Base(boost::make_shared<Impl>(sri, port))
{
}

template <class PortType>
size_t BufferedOutputStream<PortType>::bufferSize() const
{
    return impl().bufferSize();
}

template <class PortType>
void BufferedOutputStream<PortType>::setBufferSize(size_t samples)
{
    impl().setBufferSize(samples);
}

template <class PortType>
void BufferedOutputStream<PortType>::flush()
{
    impl().flush();
}

template <class PortType>
void BufferedOutputStream<PortType>::write(const BufferType& data, const BULKIO::PrecisionUTCTime& time)
{
    impl().write(data, time);
}

template <class PortType>
typename BufferedOutputStream<PortType>::Impl& BufferedOutputStream<PortType>::impl()
{
    return static_cast<Impl&>(*this->_impl);
}

template <class PortType>
const typename BufferedOutputStream<PortType>::Impl& BufferedOutputStream<PortType>::impl() const
{
    return static_cast<const Impl&>(*this->_impl);
}

//
// Numeric streams add addtional complex/scalar and extended timestamp methods
//
using bulkio::NumericOutputStream;

template <class PortType>
NumericOutputStream<PortType>::NumericOutputStream() :
    Base()
{
}

template <class PortType>
NumericOutputStream<PortType>::NumericOutputStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    Base(sri, port)
{
}

template <class PortType>
void NumericOutputStream<PortType>::write(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time)
{
    Base::write(data, time);
}

template <class PortType>
void NumericOutputStream<PortType>::write(const ScalarBuffer& data, const std::list<bulkio::SampleTimestamp>& times)
{
    _writeMultiple(data, times);
}

template <class PortType>
void NumericOutputStream<PortType>::write(const ComplexBuffer& data, const BULKIO::PrecisionUTCTime& time)
{
    if (!this->complex()) {
        throw std::logic_error("stream mode is not complex");
    }
    write(ScalarBuffer::recast(data), time);
}

template <class PortType>
void NumericOutputStream<PortType>::write(const ComplexBuffer& data, const std::list<bulkio::SampleTimestamp>& times)
{
    _writeMultiple(data, times);
}

template <class PortType>
template <typename Sample>
inline void NumericOutputStream<PortType>::_writeMultiple(const redhawk::shared_buffer<Sample>& data,
                                                          const std::list<bulkio::SampleTimestamp>& times)
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
            last = timestamp->offset < data.size()? timestamp->offset : data.size();
            if (!is_complex<Sample>::value && this->complex()) {
                // If the stream is complex but the data type is not, adjust sample
                // offset to account for the fact that each real/imaginary pair is
                // actually two values
                last *= 2;
            }
        }
        write(data.slice(first, last), when);
        first = last;
    }
}

//
// XML
//
using bulkio::OutXMLStream;

OutXMLStream::OutXMLStream() :
    Base()
{
}

OutXMLStream::OutXMLStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    Base(sri, port)
{
}

void OutXMLStream::write(const std::string& xmlString)
{
    // XML ports do not officially support timestamps, although the port
    // implementation includes it (because it's templatized); always pass
    // "not set" for consistency
    impl().write(xmlString, bulkio::time::utils::notSet());
}

//
// File
//
using bulkio::OutFileStream;

OutFileStream::OutFileStream() :
    Base()
{
}

OutFileStream::OutFileStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    Base(sri, port)
{
}

void OutFileStream::write(const std::string& URL, const BULKIO::PrecisionUTCTime& time)
{
    impl().write(URL, time);
}

#define INSTANTIATE_TEMPLATE(x) \
    template class OutputStream<x>;

#define INSTANTIATE_NUMERIC_TEMPLATE(x)         \
    template class BufferedOutputStream<x>;     \
    template class NumericOutputStream<x>;

FOREACH_PORT_TYPE(INSTANTIATE_TEMPLATE);
FOREACH_NUMERIC_PORT_TYPE(INSTANTIATE_NUMERIC_TEMPLATE);
// Bit data gets output stream buffering, but does not support scalar/complex
// data APIs
template class BufferedOutputStream<BULKIO::dataBit>;
