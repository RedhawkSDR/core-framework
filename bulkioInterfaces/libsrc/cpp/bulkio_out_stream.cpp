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
class OutputStream<PortTraits>::Impl : public StreamBase::Impl {
public:
    typedef StreamBase::Impl ImplBase;
    typedef typename PortTraits::SharedBufferType SharedBufferType;

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
        _send(SharedBufferType(), bulkio::time::utils::notSet(), true);
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
        _modifyingStreamMetadata();
        _sri->keywords = properties;
        ++_modcount;
    }

    void setKeyword(const std::string& name, const CORBA::Any& value)
    {
        _modifyingStreamMetadata();
        redhawk::PropertyMap::cast(_sri->keywords)[name] = value;
        ++_modcount;
    }

    void eraseKeyword(const std::string& name)
    {
        _modifyingStreamMetadata();
        redhawk::PropertyMap::cast(_sri->keywords).erase(name);
        ++_modcount;
    }

    void setSRI(const BULKIO::StreamSRI& sri)
    {
        _modifyingStreamMetadata();
        // Copy the new SRI, except for the stream ID, which is immutable
        *_sri = sri;
        _sri->streamID = _streamID.c_str();
        ++_modcount;
    }

    virtual void write(const SharedBufferType& data, const BULKIO::PrecisionUTCTime& time)
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

    void _send(const SharedBufferType& data, const BULKIO::PrecisionUTCTime& time, bool eos)
    {
        _port->_sendPacket(data, time, eos, _streamID);
    }

    int _modcount;
    OutPortType* _port;
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
OutputStream<PortTraits>::OutputStream(boost::shared_ptr<Impl> impl) :
    StreamBase(impl)
{
}

template <class PortTraits>
void OutputStream<PortTraits>::sri(const BULKIO::StreamSRI& sri)
{
    impl().setSRI(sri);
}

template <class PortTraits>
void OutputStream<PortTraits>::xstart(double start)
{
    impl().setXStart(start);
}

template <class PortTraits>
void OutputStream<PortTraits>::xdelta(double delta)
{
    impl().setXDelta(delta);
}

template <class PortTraits>
void OutputStream<PortTraits>::xunits(short units)
{
    impl().setXUnits(units);
}

template <class PortTraits>
void OutputStream<PortTraits>::subsize(int size)
{
    impl().setSubsize(size);
}

template <class PortTraits>
void OutputStream<PortTraits>::ystart(double start)
{
    impl().setYStart(start);
}

template <class PortTraits>
void OutputStream<PortTraits>::ydelta(double delta)
{
    impl().setYDelta(delta);
}

template <class PortTraits>
void OutputStream<PortTraits>::yunits(short units)
{
    impl().setYUnits(units);
}

template <class PortTraits>
void OutputStream<PortTraits>::complex(bool mode)
{
    impl().setComplex(mode);
}

template <class PortTraits>
void OutputStream<PortTraits>::blocking(bool mode)
{
    impl().setBlocking(mode);
}

template <class PortTraits>
void OutputStream<PortTraits>::keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props)
{
    impl().setKeywords(props);
}

template <class PortTraits>
void OutputStream<PortTraits>::setKeyword(const std::string& name, const CORBA::Any& value)
{
    impl().setKeyword(name, value);
}

template <class PortTraits>
void OutputStream<PortTraits>::setKeyword(const std::string& name, const redhawk::Value& value)
{
    impl().setKeyword(name, value);
}

template <class PortTraits>
void OutputStream<PortTraits>::eraseKeyword(const std::string& name)
{
    impl().eraseKeyword(name);
}

template <class PortTraits>
void OutputStream<PortTraits>::close()
{
    impl().close();
    _impl.reset();
}

template <class PortTraits>
OutputStream<PortTraits>::operator unspecified_bool_type() const
{
    return _impl?static_cast<unspecified_bool_type>(&OutputStream::impl):0;
}

template <class PortTraits>
typename OutputStream<PortTraits>::Impl& OutputStream<PortTraits>::impl()
{
    return static_cast<Impl&>(*this->_impl);
}

template <class PortTraits>
const typename OutputStream<PortTraits>::Impl& OutputStream<PortTraits>::impl() const
{
    return static_cast<const Impl&>(*this->_impl);
}

template <class PortTraits>
int OutputStream<PortTraits>::modcount() const
{
    return impl().modcount();
}


using bulkio::BufferedOutputStream;

template <class PortTraits>
class BufferedOutputStream<PortTraits>::Impl : public Base::Impl {
public:
    typedef typename Base::Impl ImplBase;

    typedef typename BufferedOutputStream<PortTraits>::ScalarBuffer ScalarBuffer;
    typedef typename BufferedOutputStream<PortTraits>::ComplexBuffer ComplexBuffer;

    using ImplBase::_sri;
    using ImplBase::_streamID;

    Impl(const BULKIO::StreamSRI& sri, OutPortType* port) :
        ImplBase::Impl(sri, port),
        _bufferSize(0),
        _bufferOffset(0)
    {
    }

    void write(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time)
    {
        // If buffering is disabled, or the buffer is empty and the input data is
        // large enough for a full buffer, send it immediately
        if ((_bufferSize == 0) || (_bufferOffset == 0 && (data.size() >= _bufferSize))) {
            ImplBase::write(data, time);
        } else {
            _doBuffer(data, time);
        }
    }

    void write(const ComplexBuffer& data, const BULKIO::PrecisionUTCTime& time)
    {
        if (_sri->mode == 0) {
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
            BULKIO::PrecisionUTCTime next = time + (_sri->xdelta * count);
            _doBuffer(data.slice(count), next);
        }
    }

    redhawk::buffer<ScalarType> _buffer;
    BULKIO::PrecisionUTCTime _bufferTime;
    size_t _bufferSize;
    size_t _bufferOffset;
};

template <class PortTraits>
BufferedOutputStream<PortTraits>::BufferedOutputStream() :
    Base()
{
}

template <class PortTraits>
BufferedOutputStream<PortTraits>::BufferedOutputStream(const BULKIO::StreamSRI& sri, OutPortType* port) :
    Base(boost::make_shared<Impl>(sri, port))
{
}

template <class PortTraits>
size_t BufferedOutputStream<PortTraits>::bufferSize() const
{
    return impl().bufferSize();
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::setBufferSize(size_t samples)
{
    impl().setBufferSize(samples);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::flush()
{
    impl().flush();
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time)
{
    impl().write(data, time);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ScalarBuffer& data, const std::list<bulkio::SampleTimestamp>& times)
{
    impl().write(data, times);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ComplexBuffer& data, const BULKIO::PrecisionUTCTime& time)
{
    impl().write(data, time);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ComplexBuffer& data, const std::list<bulkio::SampleTimestamp>& times)
{
    impl().write(data, times);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ScalarType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
{
    impl().write(ScalarBuffer::make_transient(data, count), time);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ScalarType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
{
    impl().write(ScalarBuffer::make_transient(data, count), times);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ComplexType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
{
    impl().write(ComplexBuffer::make_transient(data, count), time);
}

template <class PortTraits>
void BufferedOutputStream<PortTraits>::write(const ComplexType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
{
    impl().write(ComplexBuffer::make_transient(data, count), times);
}

template <class PortTraits>
typename BufferedOutputStream<PortTraits>::Impl& BufferedOutputStream<PortTraits>::impl()
{
    return static_cast<Impl&>(*this->_impl);
}

template <class PortTraits>
const typename BufferedOutputStream<PortTraits>::Impl& BufferedOutputStream<PortTraits>::impl() const
{
    return static_cast<const Impl&>(*this->_impl);
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

#define INSTANTIATE_NUMERIC_TEMPLATE(x) \
    INSTANTIATE_TEMPLATE(x); template class BufferedOutputStream<x>;

INSTANTIATE_NUMERIC_TEMPLATE(bulkio::CharPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::OctetPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::ShortPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::UShortPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::LongPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::ULongPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::LongLongPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::ULongLongPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::FloatPortTraits);
INSTANTIATE_NUMERIC_TEMPLATE(bulkio::DoublePortTraits);
INSTANTIATE_TEMPLATE(bulkio::XMLPortTraits);
INSTANTIATE_TEMPLATE(bulkio::FilePortTraits);
