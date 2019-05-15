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

#ifndef __bulkio_out_stream_h
#define __bulkio_out_stream_h

#include <string>
#include <complex>
#include <boost/shared_ptr.hpp>

#include <ossie/PropertyMap.h>
#include <ossie/shared_buffer.h>
#include <BULKIO/bulkioDataTypes.h>

#include "bulkio_typetraits.h"
#include "bulkio_datablock.h"
#include "bulkio_stream.h"

namespace bulkio {

    template <class PortType>
    class OutPort;

    /**
     * @brief  Abstract BulkIO output stream class.
     * @headerfile  bulkio_out_stream.h <bulkio/bulkio_out_stream.h>
     *
     * %OutputStream is a smart pointer-based class that encapsulates a single
     * BulkIO stream for writing. It is associated with the output port that
     * created it, providing a file-like API on top of the classic BulkIO
     * pushPacket model.
     *
     * @warning  Do not declare instances of this template class directly in
     *           user code; the template parameter and class name are not
     *           considered API. Use the type-specific @c typedef instead, such
     *           as bulkio::OutFloatStream, or the nested @c typedef StreamType
     *           from an %OutPort.
     *
     * Notionally, a BulkIO stream represents a contiguous data set and its
     * associated signal-related information (SRI), uniquely identified by a
     * stream ID, from creation until close. The SRI may vary over time, but
     * the stream ID is immutable. Only one stream with a given stream ID can
     * be active at a time.
     *
     * OutputStreams help manage the stream lifetime by tying that SRI with an
     * output port and ensuring that all data is associated with a valid stream.
     * When the stream is complete, it may be closed, notifying downstream
     * receivers that no more data is expected.
     *
     * The %OutputStream class itself is a lightweight handle; it is inexpensive
     * to copy or store in local variables or nested data types. Assigning one
     * %OutputStream to another does not copy the stream state, but instead, it
     * aliases both objects to the same underlying stream.
     *
     * The default constructor creates an invalid "null" %OutputStream that cannot
     * be used for any real operations, similar to a null pointer. A stream may
     * be checked for validity with boolean tests:
     *
     * @code
     * if (!stream) {
     *     // handle failure
     * }
     * @endcode
     * or
     * @code
     * if (stream) {
     *     // operate on stream
     * }
     * @endcode
     *
     * OutputStreams must be created via an %OutPort. A stream cannot be
     * associated with more than one port.
     * @see  OutPort::createStream(const std::string&)
     * @see  OutPort::createStream(const BULKIO::StreamSRI&)
     *
     * @par  SRI Changes
     * Updates to the stream that modify its SRI are cached locally until the
     * next write to minimize the number of updates that are published. When
     * there are pending SRI changes, the %OutputStream pushes the updated SRI
     * first, followed by the data.
     */
    template <class PortType>
    class OutputStream : public StreamBase {
    public:
        using StreamBase::sri;

        /**
         * @brief  Update the SRI.
         * @param sri  New SRI.
         * @pre  Stream is valid.
         *
         * Overwrites all SRI fields except for @c streamID, which is
         * immutable.  If the SRI is changed, it will be pushed on the next
         * write.
         */
        void sri(const BULKIO::StreamSRI& sri);

        using StreamBase::xstart;

        /**
         * @brief  Sets the X-axis start value.
         * @param start  Starting coordinate of the first sample in the X
         *               direction.
         * @pre  Stream is valid.
         * @see  xstart() const
         *
         * Changing @c xstart updates the SRI, which will be pushed on the next
         * write.
         */
        void xstart(double start);

        using StreamBase::xdelta;

        /**
         * @brief  Sets the X-axis delta.
         * @param delta  The distance between two adjacent samples in the X
         *               direction.
         * @pre  Stream is valid.
         * @see  xdelta() const
         *
         * Changing @c xdelta updates the SRI, which will be pushed on the next
         * write.
         */
        void xdelta(double delta);

        using StreamBase::xunits;

        /**
         * @brief  Sets the X-axis units.
         * @param units  Unit code for @c xstart and @c xdelta.
         * @pre  Stream is valid.
         * @see  xunits() const
         *
         * Changing @c xunits updates the SRI, which will be pushed on the next
         * write.
         */
        void xunits(short units);

        using StreamBase::subsize;

        /**
         * @brief  Sets the frame size.
         * @param size  Length of a row for framed data, or 0 for contiguous
         *              data.
         * @pre  Stream is valid.
         * @see  subsize() const
         *
         * Changing @c subsize updates the SRI, which will be pushed on the
         * next write.
         */
        void subsize(int size);

        using StreamBase::ystart;

        /**
         * @brief  Sets the Y-axis start value.
         * @param start  Starting coordinate of the first frame in the Y
         *               direction.
         * @pre  Stream is valid.
         * @see  ystart() const
         *
         * Changing @c ystart updates the SRI, which will be pushed on the next
         * write.
         */
        void ystart(double start);

        using StreamBase::ydelta;

        /**
         * @brief  Sets the Y-axis delta.
         * @param delta  The distance between two adjacent frames in the Y
         *               direction.
         * @pre  Stream is valid.
         * @see  ydelta() const
         *
         * Changing @c ydelta updates the SRI, which will be pushed on the next
         * write.
         */
        void ydelta(double delta);

        using StreamBase::yunits;

        /**
         * @brief  Sets the Y-axis units.
         * @returns  Unit code for @c ystart and @c ydelta.
         * @pre  Stream is valid.
         * @see  yunits() const
         *
         * Changing @c yunits updates the SRI, which will be pushed on the next
         * write.
         */
        void yunits(short units);

        using StreamBase::complex;

        /**
         * @brief  Sets the complex mode of this stream.
         * @param mode  True if data is complex. False if data is not complex.
         * @pre  Stream is valid.
         * @see  complex() const
         *
         * Changing the %complex mode indicates that all subsequent data is
         * real or complex based on the value of @a mode. The updated SRI will
         * be pushed on the next write.
         */
        void complex(bool mode);

        using StreamBase::blocking;

        /**
         * @brief  Sets the blocking mode of this stream.
         * @param mode  True if blocking. False if stream is non-blocking.
         * @pre  Stream is valid.
         *
         * Changing the %blocking mode updates the SRI, which will be pushed on
         * the next write.
         */
        void blocking(bool mode);

        using StreamBase::keywords;

        /**
         * @brief  Overwrites the SRI keywords.
         * @param props  New SRI keywords.
         * @pre  Stream is valid.
         * @see  setKeyword
         *
         * If @a props differ from the current SRI keywords, the keywords are
         * replaced with @a props. The updated SRI will be pushed on the next
         * write.
         */
        void keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props);

        /**
         * @brief  Sets the current value of a keyword in the SRI.
         * @param name  The name of the keyword.
         * @param value  The new value.
         * @pre  Stream is valid.
         * @see  setKeyword(const std::string&, const redhawk::Value&)
         * @see  setKeyword(const std::string&, const T&)
         *
         * If keyword @a name does not exist, the new keyword is appended.
         * If the keyword @a name exists, and its value differs from @a value,
         * then its value is updated to @a value.
         *
         * Changing a keyword updates the SRI, which will be pushed on the next
         * write.
         */
        void setKeyword(const std::string& name, const CORBA::Any& value);

        /**
         * @brief  Sets the current value of a keyword in the SRI.
         * @param name  The name of the keyword.
         * @param value  The new value.
         * @pre  Stream is valid.
         * @see  setKeyword(const std::string&, const T&)
         *
         * If keyword @a name does not exist, the new keyword is appended.
         * If the keyword @a name exists, and its value differs from @a value,
         * then its value is updated to @a value.
         *
         * Changing a keyword updates the SRI, which will be pushed on the next
         * write.
         */
        void setKeyword(const std::string& name, const redhawk::Value& value);

        /**
         * @brief  Sets the current value of a keyword in the SRI.
         * @param name  The name of the keyword.
         * @param value  The new value.
         * @tparam T  Any type that can be converted to a redhawk::Value.
         * @pre  Stream is valid.
         *
         * If keyword @a name does not exist, the new keyword is appended.
         * If the keyword @a name exists, and its value differs from @a value,
         * then its value is updated to @a value.
         *
         * Changing a keyword updates the SRI, which will be pushed on the next
         * write.
         */
        template <typename T>
        void setKeyword(const std::string& name, const T& value)
        {
            setKeyword(name, redhawk::Value(value));
        }

        /**
         * @brief  Removes a keyword from the SRI.
         * @param name  The name of the keyword.
         * @pre  Stream is valid.
         *
         * Erases the keyword named @a name from the SRI keywords. If no
         * keyword @a name is found, the keywords are not modified.
         *
         * Removing a keyword updates the SRI, which will be pushed on the next
         * write.
         */
        void eraseKeyword(const std::string& name);

        /**
         * @brief  Closes this stream and sends an end-of-stream.
         * @pre  Stream is valid.
         * @post  Stream is invalid.
         *
         * Closing a stream sends an end-of-stream packet and resets the stream
         * handle. No further operations may be made on the stream.
         */
        void close();

    protected:
        /// @cond IMPL
        typedef OutPort<PortType> OutPortType;

        class Impl;
        Impl& impl();
        const Impl& impl() const;

        OutputStream();
        OutputStream(const BULKIO::StreamSRI& sri, OutPortType* port);
        OutputStream(boost::shared_ptr<Impl> impl);

        int modcount() const;

        typedef const Impl& (OutputStream::*unspecified_bool_type)() const;
        /// @endcond
    public:
        /**
         * @brief  Checks stream validity.
         * @returns  Value convertible to true if this stream is valid.
         *           Value convertible to false if this stream is invalid.
         * @see  StreamBase::operator!() const
         *
         * This operator supports affirmative boolean tests:
         * @code
         * if (stream) {
         *     // operate on stream
         * }
         * @endcode
         *
         * If this method returns true, it is safe to call any method.
         */
        operator unspecified_bool_type() const;

        /*
         * @brief  Stream equality comparison.
         * @param other  Another %OutputStream.
         * @returns  True if and only if both OutputStreams reference the same
         *           underlying stream.
         */
        bool operator==(const OutputStream& other) const;

        bool operator!=(const OutputStream& other) const;
    };


    /**
     * @brief BulkIO output stream class with data buffering.
     * @headerfile  bulkio_out_stream.h <bulkio/bulkio_out_stream.h>
     *
     * %BufferedOutputStream can use an internal buffer to queue up multiple
     * packets worth of data into a single push.  By default, buffering is
     * disabled.
     *
     * @warning  Do not declare instances of this template class directly in
     *           user code; the template parameter and class name are not
     *           considered API. Use the type-specific @c typedef instead, such
     *           as bulkio::OutFloatStream, or the nested @c typedef StreamType
     *           from an %OutPort.
     *
     * @par  Data Buffering
     *
     * BufferedOutputStreams can combine multiple small chunks of data into a
     * single packet for reduced I/O overhead. Data buffering is enabled by
     * setting a non-zero buffer size via the setBufferSize() method. The
     * output stream creates an internal buffer of the requested size; the
     * stream's complex mode is not taken into account.
     *
     * With buffering enabled, each write copies its data into the internal
     * buffer, up to the maximum of the buffer size. When the internal buffer
     * is full, a packet is sent via the output port, using the time stamp of
     * the first buffered sample.  After the packet is sent, the internal
     * buffer is reset to its initial state. If there is any remaining data
     * from the write, it is copied into a new buffer and a new starting time
     * stamp is interpolated.
     *
     * @par  Time Stamps
     *
     * When buffering is enabled, the time stamps provided to the write()
     * methods may be discarded. Furthermore, when write sizes do not align
     * exactly with the buffer size, the output time stamp may be interpolated.
     * If precise time stamps are required, buffering should not be used.
     */
    template <class PortType>
    class BufferedOutputStream : public OutputStream<PortType> {
    public:
        /// @brief  Data type for write().
        typedef typename BufferTraits<PortType>::BufferType BufferType;

        /**
         * @brief  Default constructor.
         * @see  OutPort::createStream(const std::string&)
         * @see  OutPort::createStream(const BULKIO::StreamSRI&)
         *
         * Create a null %BufferedOutputStream. This stream is not associated
         * with a stream from any output port. No methods may be called on the
         * the %BufferedOutputStream except for boolean tests and comparison.
         * A null stream will always test as not valid, and will compare equal
         * to another stream if and only if the other stream is also null.
         *
         * New, valid streams are created via an output port.
         */
        BufferedOutputStream();

        /**
         * @brief  Gets the internal buffer size.
         * @returns  Number of real samples to buffer per push.
         * @pre  Stream is valid.
         *
         * The buffer size is in terms of real samples, ignoring the complex
         * mode of the stream. Complex samples count as two real samples for
         * the purposes of buffering.
         *
         * A buffer size of 0 indicates that buffering is disabled.
         */
        size_t bufferSize() const;

        /**
         * @brief  Sets the internal buffer size.
         * @param samples  Number of real samples to buffer per push.
         * @pre  Stream is valid.
         * @see bufferSize() const
         *
         * The internal buffer is flushed if @a samples is less than the number
         * of real samples currently buffered.
         *
         * A buffer size of 0 disables buffering, flushing any buffered data.
         */
        void setBufferSize(size_t samples);

        /**
         * @brief  Flushes the internal buffer.
         * @pre  Stream is valid.
         *
         * Any data in the internal buffer is sent to the port to be pushed.
         */
        void flush();

        /**
         * @brief  Writes data to the stream.
         * @param data  The data to write.
         * @param time  Time stamp of first element.
         *
         * If buffering is disabled, @a data is sent as a single packet with
         * the given time stamp.
         *
         * When buffering is enabled, @a data is copied into the internal
         * buffer. If the internal buffer exceeds the configured buffer size,
         * one or more packets will be sent.
         *
         * If there are any pending SRI changes, the new SRI is pushed first.
         */
        void write(const BufferType& data, const BULKIO::PrecisionUTCTime& time);

    protected:
        /// @cond IMPL
        typedef OutputStream<PortType> Base;

        friend class OutPort<PortType>;
        typedef OutPort<PortType> OutPortType;
        BufferedOutputStream(const BULKIO::StreamSRI& sri, OutPortType* port);

        class Impl;
        Impl& impl();
        const Impl& impl() const;
        /// @endcond
    };


    /**
     * @brief BulkIO output stream class for numeric data types.
     * @headerfile  bulkio_out_stream.h <bulkio/bulkio_out_stream.h>
     *
     * %NumericOutputStream provides overloaded write methods for both real and
     * complex sample data.
     *
     * @warning  Do not declare instances of this template class directly in
     *           user code; the template parameter and class name are not
     *           considered API. Use the type-specific @c typedef instead, such
     *           as bulkio::OutFloatStream, or the nested @c typedef StreamType
     *           from an %OutPort.
     */
    template <class PortType>
    class NumericOutputStream : public BufferedOutputStream<PortType> {
    public:
        /// @brief  The native type of a real sample.
        typedef typename NativeTraits<PortType>::NativeType ScalarType;
        /// @brief  The native type of a complex sample.
        typedef std::complex<ScalarType> ComplexType;

        /// @brief  The shared_buffer type for real data.
        typedef redhawk::shared_buffer<ScalarType> ScalarBuffer;
        /// @brief  The shared_buffer type for complex data.
        typedef redhawk::shared_buffer<ComplexType> ComplexBuffer;
       
        /**
         * @brief  Default constructor.
         * @see  OutPort::createStream(const std::string&)
         * @see  OutPort::createStream(const BULKIO::StreamSRI&)
         *
         * Create a null %NumericOutputStream. This stream is not associated
         * with a stream from any output port. No methods may be called on the
         * the %NumericOutputStream except for boolean tests and comparison. A
         * null stream will always test as not valid, and will compare equal to
         * another stream if and only if the other stream is also null.
         *
         * New, valid streams are created via an output port.
         */
        NumericOutputStream();

        /*
         * @brief  Write real sample data to the stream.
         * @param data  %shared_buffer containing real data.
         * @param time  Time stamp of first sample.

         * Sends the real data in @a data as single packet with the time stamp
         * @a time via the associated output port.
         *
         * If there are any pending SRI changes, the new SRI is pushed first.
         */
        void write(const ScalarBuffer& data, const BULKIO::PrecisionUTCTime& time);

        /**
         * @brief  Write real sample data to the stream.
         * @param data   %shared_buffer containing real data.
         * @param times  List of time stamps, with offsets.
         * @pre  Stream is valid.
         * @pre  @p times is sorted in order of offset.
         * @throw std::logic_error  If @p times is empty.
         *
         * Writes the real data in @a data to the stream, where each element of
         * @a times gives the offset and time stamp of an individual packet.
         * The offset of the first time stamp is ignored and assumed to be 0,
         * while subsequent offsets determine the length of the prior packet.
         * All offsets should be less than @a data.size().
         *
         * For example, given @a data with size 25 and three time stamps with
         * offsets 0, 10, and 20, @a data is broken into three packets of size
         * 10, 10, and 5 samples.
         *
         * If there are any pending SRI changes, the new SRI is pushed first.
         */
        void write(const ScalarBuffer& data, const std::list<bulkio::SampleTimestamp>& times);

        /**
         * @brief  Write complex sample data to the stream.
         * @param data  %shared_buffer containing complex data.
         * @param time  Time stamp of the first sample.
         * @throw std::logic_error  If stream is not configured for complex
         *                          data.
         *
         * Sends the complex data in @a data as single packet with the time
         * stamp @a time via the associated output port.
         *
         * If there are any pending SRI changes, the new SRI is pushed first.
         */
        void write(const ComplexBuffer& data, const BULKIO::PrecisionUTCTime& time);

        /**
         * @brief  Write complex data to the stream.
         * @param data   %shared_buffer containing complex data.
         * @param times  List of time stamps, with offsets.
         * @pre  Stream is valid.
         * @pre  @p times is sorted in order of offset.
         * @throw std::logic_error  If stream is not configured for complex
         *                          data.
         * @throw std::logic_error  If @p times is empty.
         *
         * Writes the complex data in @a data to the stream, where each element
         * of @a times gives the offset and time stamp of an individual packet.
         * The offset of the first time stamp is ignored and assumed to be 0,
         * while subsequent offsets determine the length of the prior packet.
         * All offsets should be less than @a data.size().
         *
         * For example, given @a data with size 25 and three time stamps with
         * offsets 0, 10, and 20, @a data is broken into three packets of size
         * 10, 10, and 5 samples.
         *
         * If there are any pending SRI changes, the new SRI is pushed first.
         */
        void write(const ComplexBuffer& data, const std::list<bulkio::SampleTimestamp>& times);

        /**
         * @brief  Writes a packet of data.
         * @tparam T  Sample type (must be ScalarType or ComplexType).
         * @param data  Vector containing real or complex sample data.
         * @param time  Time stamp of first sample.
         * @pre  Stream is valid.
         * @throw std::logic_error  If @p T is complex but stream is not.
         * @see  write(const ScalarType*,size_t,const BULKIO::PrecisionUTCTime&)
         * @see  write(const ComplexType*,size_t,const BULKIO::PrecisionUTCTime&)
         *
         * Sends the contents of a real or complex vector as a single packet.
         * This is a convenience wrapper that defers to one of the write
         * methods that takes a pointer and size, depending on whether @a T is
         * real or complex.
         */
        template <class T>
        void write(const std::vector<T>& data, const BULKIO::PrecisionUTCTime& time)
        {
            write(&data[0], data.size(), time);
        }

        /**
         * @brief  Writes one or more packets.
         * @tparam T  Sample type (must be ScalarType or ComplexType).
         * @param data  Vector containing real or complex sample data.
         * @param times  List of time stamps, with offsets.
         * @pre  Stream is valid.
         * @throw std::logic_error  If @p T is complex but stream is not.
         * @throw std::logic_error  If @p times is empty.
         * @see  write(const ScalarType*,size_t,const std::list<bulkio::SampleTimestamp>&)
         * @see  write(const ComplexType*,size_t,const std::list<bulkio::SampleTimestamp>&)
         *
         * Sends the contents of a real or complex vector as one or more
         * packets.  This is a convenience wrapper that defers to one of the
         * write methods that takes a pointer and size, depending on whether
         * @a T is real or complex.
         */
        template <class T>
        void write(const std::vector<T>& data, const std::list<bulkio::SampleTimestamp>& times)
        {
            write(&data[0], data.size(), times);
        }

        /**
         * @brief  Writes a packet of real data.
         * @param data  Pointer to real sample data.
         * @param count  Number of samples to write.
         * @param time  Time stamp of first sample.
         * @pre  Stream is valid.
         *
         * Convenience wrapper for write(const ScalarBuffer&,const BULKIO::PrecisionUTCTime&)
         * that creates a transient buffer from @a data and @a count.
         */
        void write(const ScalarType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
        {
            write(ScalarBuffer::make_transient(data, count), time);
        }

        /**
         * @brief  Writes one or more packets of real data.
         * @param data  Pointer to real sample data.
         * @param count  Number of samples to write.
         * @param times  List of time stamps, with offsets.
         * @pre  Stream is valid.
         * @pre  @p times is sorted in order of offset.
         * @throw std::logic_error  If @p times is empty.
         *
         * Convenience wrapper for write(const ScalarBuffer&,const std::list<bulkio::SampleTimestamp>&)
         * that creates a transient buffer from @a data and @a count.
         */
        void write(const ScalarType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
        {
            write(ScalarBuffer::make_transient(data, count), times);
        }

        /**
         * @brief  Writes a packet of complex data.
         * @param data  Pointer to complex sample data.
         * @param count  Number of samples to write.
         * @param time  Time stamp of first sample.
         * @throw std::logic_error  If stream is not configured for complex
         *                          data.
         * @pre  Stream is valid.
         *
         * Convenience wrapper for write(const ComplexBuffer&,const BULKIO::PrecisionUTCTime&)
         * that creates a transient buffer from @a data and @a count.
         */
        void write(const ComplexType* data, size_t count, const BULKIO::PrecisionUTCTime& time)
        {
            write(ComplexBuffer::make_transient(data, count), time);
        }

        /**
         * @brief  Writes one or more packets of complex data.
         * @param data  Pointer to complex sample data.
         * @param count  Number of samples to write.
         * @param times  List of time stamps, with offsets.
         * @pre  Stream is valid.
         * @pre  @p times is sorted in order of offset.
         * @throw std::logic_error  If stream is not configured for complex
         *                          data.
         * @throw std::logic_error  If @p times is empty.
         *
         * Convenience wrapper for write(const ComplexBuffer&,const std::list<bulkio::SampleTimestamp>&)
         * that creates a transient buffer from @a data and @a count.
         */
        void write(const ComplexType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times)
        {
            write(ComplexBuffer::make_transient(data, count), times);
        }

    private:
        /// @cond IMPL
        typedef BufferedOutputStream<PortType> Base;

        friend class OutPort<PortType>;
        typedef OutPort<PortType> OutPortType;
        NumericOutputStream(const BULKIO::StreamSRI& sri, OutPortType* port);

        template <typename Sample>
        inline void _writeMultiple(const redhawk::shared_buffer<Sample>& data,
                                   const std::list<bulkio::SampleTimestamp>& times);
        /// @endcond
    };


    /**
     * @brief BulkIO XML output stream class.
     * @headerfile  bulkio_out_stream.h <bulkio/bulkio_out_stream.h>
     */
    class OutXMLStream : public OutputStream<BULKIO::dataXML> {
    public:
        /**
         * @brief  Default constructor.
         * @see  OutPort::createStream(const std::string&)
         * @see  OutPort::createStream(const BULKIO::StreamSRI&)
         *
         * Create a null %OutXMLStream. This stream is not associated with a
         * stream from any output port. No methods may be called on the the
         * %OutXMLStream except for boolean tests and comparison. A null stream
         * will always test as not valid, and will compare equal to another
         * stream if and only if the other stream is also null.
         *
         * New, valid streams are created via an output port.
         */
        OutXMLStream();

        /**
         * @brief  Writes XML data to the stream.
         * @param xmlString  An XML string.
         *
         * The XML string @a data is sent as a single packet.
         */
        void write(const std::string& xmlString);

    private:
        /// @cond IMPL
        typedef OutputStream<BULKIO::dataXML> Base;

        friend class OutPort<BULKIO::dataXML>;
        typedef OutPort<BULKIO::dataXML> OutPortType;
        OutXMLStream(const BULKIO::StreamSRI& sri, OutPortType* port);
        /// @endcond IMPL
    };


    /**
     * @brief BulkIO file output stream class.
     * @headerfile  bulkio_out_stream.h <bulkio/bulkio_out_stream.h>
     */
    class OutFileStream : public OutputStream<BULKIO::dataFile> {
    public:
        /**
         * @brief  Default constructor.
         * @see  OutPort::createStream(const std::string&)
         * @see  OutPort::createStream(const BULKIO::StreamSRI&)
         *
         * Create a null %OutFileStream. This stream is not associated with a
         * stream from any output port. No methods may be called on the the
         * %OutFileStream except for boolean tests and comparison. A null
         * stream will always test as not valid, and will compare equal to
         * another stream if and only if the other stream is also null.
         *
         * New, valid streams are created via an output port.
         */
        OutFileStream();

        /**
         * @brief  Writes a file URI to the stream.
         * @param URL  The file URI to write.
         * @param time  Time stamp of file data.
         *
         * The URI is sent as a single packet with the given time stamp.
         */
        void write(const std::string& URL, const BULKIO::PrecisionUTCTime& time);

    private:
        /// @cond IMPL
        typedef OutputStream<BULKIO::dataFile> Base;

        friend class OutPort<BULKIO::dataFile>;
        typedef OutPort<BULKIO::dataFile> OutPortType;
        OutFileStream(const BULKIO::StreamSRI& sri, OutPortType* port);
        /// @endcond
    };


    typedef BufferedOutputStream<BULKIO::dataBit>      OutBitStream;
    typedef NumericOutputStream<BULKIO::dataChar>      OutCharStream;
    typedef NumericOutputStream<BULKIO::dataOctet>     OutOctetStream;
    typedef NumericOutputStream<BULKIO::dataShort>     OutShortStream;
    typedef NumericOutputStream<BULKIO::dataUshort>    OutUShortStream;
    typedef NumericOutputStream<BULKIO::dataLong>      OutLongStream;
    typedef NumericOutputStream<BULKIO::dataUlong>     OutULongStream;
    typedef NumericOutputStream<BULKIO::dataLongLong>  OutLongLongStream;
    typedef NumericOutputStream<BULKIO::dataUlongLong> OutULongLongStream;
    typedef NumericOutputStream<BULKIO::dataFloat>     OutFloatStream;
    typedef NumericOutputStream<BULKIO::dataDouble>    OutDoubleStream;

} // end of bulkio namespace

#endif
