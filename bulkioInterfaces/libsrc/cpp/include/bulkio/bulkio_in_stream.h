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

#ifndef __bulkio_in_stream_h
#define __bulkio_in_stream_h

#include <string>
#include <boost/shared_ptr.hpp>

#include <BULKIO/bulkioDataTypes.h>

#include "bulkio_typetraits.h"
#include "bulkio_datablock.h"
#include "bulkio_stream.h"

namespace bulkio {

    template <class PortType>
    class InPort;

    template <class PortType>
    struct BlockTraits {
        typedef SampleDataBlock<typename NativeTraits<PortType>::NativeType> DataBlockType;
    };

    template <>
    struct BlockTraits<BULKIO::dataBit> {
        typedef BitDataBlock DataBlockType;
    };

    template <>
    struct BlockTraits<BULKIO::dataXML> {
        typedef StringDataBlock DataBlockType;
    };

    template <>
    struct BlockTraits<BULKIO::dataFile> {
        typedef StringDataBlock DataBlockType;
    };

    /**
     * @brief  Basic BulkIO input stream class.
     * @headerfile  bulkio_in_stream.h <bulkio/bulkio_in_stream.h>
     *
     * %InputStream is a smart pointer-based class that encapsulates a single
     * BulkIO stream for reading. It is associated with the input port that
     * created it, providing a file-like API on top of the classic BulkIO
     * getPacket model.
     *
     * @warning  Do not declare instances of this template class directly in user
     *           code; the template parameter and class name are not considered
     *           API. Use the type-specific @c typedef instead, such as
     *           bulkio::InFloatStream, or the nested @c typedef StreamType from
     *           an %InPort.
     *
     * Notionally, a BulkIO stream represents a contiguous data set and its
     * associated signal-related information (SRI), uniquely identified by a
     * stream ID, from creation until close. The SRI may vary over time, but the
     * stream ID is immutable. Only one stream with a given stream ID can be
     * active at a time.
     *
     * The %InputStream class itself is a lightweight handle; it is inexpensive
     * to copy or store in local variables or nested data types. Assigning one
     * %InputStream to another does not copy the stream state but instead
     * aliases both objects to the same underlying stream.
     *
     * The default constructor creates an invalid "null" %InputStream that cannot
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
     *
     * Input streams are managed by the input port, and created in response to
     * the arrival of a new SRI. Valid input streams are obtained by either
     * querying the port, or registering a callback.
     * @see  InPort::getCurrentStream(float)
     * @see  InPort::getStream(const std::string&)
     * @see  InPort::getStreams()
     * @see  InPort::addStreamListener(Target,Func)
     *
     * @par  End-of-Stream
     * In normal usage, reading continues until the end of the stream is
     * reached, at which point all future read operations will fail
     * immediately. When a read fails, it is incumbent upon the caller to check
     * the stream's end-of-stream state via eos(). Once the end-of-stream has
     * been acknowledged, either by an explicit check or with a subsequent
     * failed read, the stream is removed from the input port. If the input
     * port has another stream with the same streamID pending, it will become
     * active.
     * @par
     * Although the input port may have received and end-of-stream packet, this
     * state is not reflected in eos(). As with Unix pipes or sockets, the
     * recommended pattern is to continually read until a failure occurs,
     * handling the failure as needed.
     */
    template <class PortType>
    class InputStream : public StreamBase {
    public:
        /**
         * @brief  The native type of a real sample, or the real or imaginary
         *         component of a complex sample.
         */
        typedef typename NativeTraits<PortType>::NativeType NativeType;

        /// @brief  The type of data block returned by read methods on this stream.
        typedef typename BlockTraits<PortType>::DataBlockType DataBlockType;

        /**
         * @brief  Reads the next packet.
         * @returns  Valid data block if successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         *
         * Blocking read of the next packet for this stream.
         *
         * Returns a null data block immediately if:
         *   @li End-of-stream has been reached
         *   @li The input port is stopped
         */
        DataBlockType read();

        /**
         * @brief  Non-blocking read of the next packet.
         * @returns  Valid data block if successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         * @see  read()
         *
         * Non-blocking version of read(), returning a null data block
         * immediately when no data is available.
         */
        DataBlockType tryread();

        /**
         * @brief Checks whether this stream can receive data.
         * @returns  True if this stream is enabled. False if stream is
         *           disabled.
         * @pre  Stream is valid.
         * @see  enable()
         * @see  disable()
         *
         * If a stream is enabled, packets received for its stream ID are
         * queued in the input port, and the stream may be used for reading.
         * Conversely, packets for a disabled stream are discarded, and no
         * reading may be performed.
         */
        bool enabled() const;

        /**
         * @brief  Enable this stream for reading data.
         * @pre  Stream is valid.
         * @see  enabled()
         * @see  disable()
         *
         * The input port will resume queuing packets for this stream.
         */
        void enable();

        /**
         * @brief  Disable this stream for reading data.
         * @pre  Stream is valid.
         * @see  enable()
         * @see  enabled()
         *
         * The input port will discard any packets that are currently queued
         * for this stream, and all future packets for this stream will be
         * discarded upon receipt until an end-of-stream is received.
         *
         * Disabling unwanted streams may improve performance and queueing
         * behavior by reducing the number of queued packets on a port.
         */
        void disable();

        /**
         * @brief  Checks whether this stream has ended.
         * @returns  True if this stream has reached the end. False if the end
         *           of stream has not been reached.
         * @pre  Stream is valid.
         *
         * A stream is considered at the end when it has read and consumed all
         * data up to the end-of-stream marker. Once end-of-stream has been
         * reached, all read operations will fail immediately, as no more data
         * will ever be received for this stream.
         *
         * The recommended practice is to check @a eos any time a read
         * operation fails or returns fewer samples than requested. When the
         * end-of-stream is acknowledged, either by checking @a eos or when
         * successive reads fail due to an end-of-stream, the stream is removed
         * from the input port. If the input port has another stream with the
         * same streamID pending, it will become active.
         */
        bool eos();

    protected:
        /// @cond IMPL
        typedef InPort<PortType> InPortType;

        class Impl;
        Impl& impl();
        const Impl& impl() const;

        typedef const Impl& (InputStream::*unspecified_bool_type)() const;

        InputStream();
        InputStream(const boost::shared_ptr<Impl>& impl);

        // Allow matching InPort class to create instances of this stream type
        friend class InPort<PortType>;
        InputStream(const StreamDescriptor& sri, InPortType* port);

        bool hasBufferedData();

        void close();
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
    };

    /**
     * @brief BulkIO input stream class with data buffering.
     * @headerfile  bulkio_in_stream.h <bulkio/bulkio_in_stream.h>
     *
     * %BufferedInputStream extends InputStream with additional methods for
     * data buffering and overlapped reads.
     *
     * @par  Data Buffering
     * Often, signal processing algorithms prefer to work on regular,
     * fixed-size blocks of data. However, because the producer is working
     * independently, data may be received in entirely different packet
     * sizes. For this use case, %BufferedInputStream provides a read(size_t)
     * method that frees the user from managing their own data buffering.
     * @par
     * To maintain the requested size, partial packets may be buffered, or a
     * read may span multiple packets. Packets are fetched from the input port
     * as needed. If an SRI change or input queue flush is encountered during
     * the fetch, the operation will stop and return the data up to that
     * point. The next read operation will continue at the beginning of the
     * packet that contains the new SRI or input queue flush flag.
     *
     * @par  Time Stamps
     * The data block from a successful read always includes as least one time
     * stamp, at a sample offset of 0. Because buffered reads may not begin on
     * a packet boundary, the input stream can interpolate a time stamp based
     * on the SRI @a xdelta value and the prior time stamp. When this occurs,
     * the time stamp will be marked as "synthetic."
     * @par
     * Reads that span multiple packets will contain more than one time stamp.
     * The time stamp offsets indicate at which sample the time stamp occurs,
     * taking real or complex samples into account. Only the first time stamp
     * can be synthetic.
     *
     * @par  Overlapped Reads
     * Certain classes of signal processing algorithms need to preserve a
     * portion of the last data set for the next iteration, such as a power
     * spectral density (PSD) calculation with overlap. The read(size_t,size_t)
     * method supports this mode of operation by allowing the reader to consume
     * fewer samples than are read. This can be thought of as a separate read
     * pointer that trails behind the stream's internal buffer.
     * @par
     * When an overlapped read needs to span multiple packets, but an SRI
     * change, input queue flush, or end-of-stream is encountered, all of the
     * available data is returned and consumed, equivalent to read(size_t). The
     * assumption is that special handling is required due to the pending
     * change, and it is not possible for the stream to interpret the
     * relationship between the read size and consume size.
     * @par
     * When the consume length is zero, the read operation becomes a peek. It
     * returns data following the normal rules but no data is consumed, even in
     * the case of SRI change, input queue flush, or end-of-stream.
     *
     * @par  Non-Blocking Reads
     * For each @a read method, there is a corresponsing @a tryread method that
     * is non-blocking. If there is not enough data currently available to
     * satisfy the request, but more data could become available in the future,
     * the operation will return a null data block immediately.
     *
     * @par  End-of-Stream
     * The end-of-stream behavior of %BufferedInputStream is consistent with
     * %InputStream, with the additional caveat that a read may return fewer
     * samples than requested if an end-of-stream packet is encountered.
     */
    template <class PortType>
    class BufferedInputStream : public InputStream<PortType> {
    public:
        /// @brief  The type of data block returned by read methods on this stream.
        typedef typename InputStream<PortType>::DataBlockType DataBlockType;

        /**
         * @brief  Default constructor.
         * @see  InPort::getCurrentStream()
         * @see  InPort::getStream(const std::string&)
         * @see  InPort::getStreams()
         * @see  InPort::addStreamListener(Target,Func)
         *
         * Creates a null %BufferedInputStream. This stream is not associated
         * with a stream from any InPort instance. No methods may be called on
         * the %BufferedInputStream except for boolean tests and comparison.
         * A null stream will always test as not valid, and will compare equal
         * to another stream if and only if the other stream is also null.
         *
         * To get a handle to a live stream, you must query an input port or
         * register a callback.
         */
        BufferedInputStream();

        /**
         * @brief  Reads the next packet.
         * @returns  Valid data block if successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         *
         * Blocking read up to the next packet boundary. Reading a packet at a
         * time is the most computationally efficent method because it does not
         * require the stream to copy data into an intermediate buffer;
         * instead, it may pass the original buffer along to the reader.
         *
         * Returns a null data block immediately if:
         *   @li End-of-stream has been reached
         *   @li The input port is stopped
         */
        DataBlockType read();

        /**
         * @brief  Reads a specified number of samples.
         * @param count  Number of samples to read.
         * @returns  Data block containing up to @p count samples if
         *           successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         *
         * Blocking read of @a count samples worth of data. For signal
         * processing operations that require a fixed input data size, such as
         * fast Fourier transform (FFT), this simplifies buffer management by
         * offloading it to the stream. This usually incurs some computational
         * overhead to copy data between buffers; however, this cost is
         * intrinsic to the algorithm, and the reduced complexity of
         * implementation avoids common errors.
         *
         * If the SRI indicates that the data is complex, @a count is in terms
         * of complex samples.
         *
         * If any of the following conditions are encountered while fetching
         * packets, the returned data block may contain fewer samples than
         * requested:
         *   @li End-of-stream
         *   @li SRI change
         *   @li Input queue flush
         *
         * Returns a null data block immediately if:
         *   @li End-of-stream has been reached
         *   @li The input port is stopped
         */
        DataBlockType read(size_t count);

        /**
         * @brief  Reads a specified number of samples, with overlap.
         * @param count  Number of samples to read.
         * @param consume  Number of samples to advance read pointer.
         * @returns  Data block containing up to @p count samples if
         *           successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         * @pre  @p consume <= @p count
         * @see  read(size_t)
         *
         * Blocking read of @a count samples worth of data will only advance
         * the read pointer by @a consume samples. The remaining @c
         * count-consume samples are buffered and will be returned on the
         * following read operation. This method is designed to support signal
         * processing operations that require overlapping data sets, such as
         * power spectral density (PSD).
         *
         * If the SRI indicates that the data is complex, @a count and @a
         * consume are in terms of complex samples.
         *
         * If any of the following conditions are encountered while fetching
         * packets, the returned data block may contain fewer samples than
         * requested:
         *   @li End-of-stream
         *   @li SRI change
         *   @li Input queue flush
         *
         * When this occurs, all of the returned samples are consumed unless
         * @a consume is 0, as it is assumed that special handling is required.
         *
         * Returns a null data block immediately if:
         *   @li End-of-stream has been reached
         *   @li The input port is stopped
         */
        DataBlockType read(size_t count, size_t consume);

        /**
         * @brief  Non-blocking read of the next packet.
         * @returns  Valid data block if successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         * @see  read()
         *
         * Non-blocking version of read(), returning a null data block
         * immediately when no data is available.
         */
        DataBlockType tryread();

        /**
         * @brief  Non-blocking sized read.
         * @param count  Number of samples to read.
         * @returns  Data block containing up to @p count samples if
         *           successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         * @see  read(size_t)
         *
         * Non-blocking version of read(size_t), returning a null data block
         * immediately when no data is available.
         */
        DataBlockType tryread(size_t count);

        /**
         * @brief  Non-blocking read with overlap.
         * @param count  Number of samples to read.
         * @param consume  Number of samples to advance read pointer.
         * @returns  Data block containing up to @p count samples if
         *           successful.
         * @returns  Null data block if the read failed.
         * @pre  Stream is valid.
         * @pre  @p consume <= @p count
         * @see  read(size_t,size_t)
         *
         * Non-blocking version of read(size_t,size_t), returning a null data
         * block immediately when no data is available.
         */
        DataBlockType tryread(size_t count, size_t consume);

        /**
         * @brief  Discard a specified number of samples.
         * @param count  Number of samples to skip.
         * @returns  Actual number of samples skipped.
         * @pre  Stream is valid.
         * @see  read(size_t)
         *
         * Skips the next @a count samples worth of data and blocks until the
         * requested amount of data is available. If the data is not being
         * used, this is more computationally efficient than the equivalent
         * call to read(size_t) because no buffering is performed.
         *
         * If the SRI indicates that the data is complex, @a count and the
         * return value are in terms of complex samples.
         *
         * Skipping behaves like read(size_t) when fetching packets. If any of
         * the following conditions are encountered, the returned value may be
         * less than @a count:
         *   @li End-of-stream
         *   @li SRI change
         *   @li Input queue flush
         *
         * Returns 0 immediately if:
         *   @li End-of-stream has been reached
         *   @li The input port is stopped
         */
        size_t skip(size_t count);

        /**
         * @brief  Estimates the number of samples that can be read
         *         immediately.
         * @returns  Number of samples.
         * @pre  Stream is valid.
         *
         * The number of samples returned by this method is an estimate based
         * on the current state of the stream and the input queue. If there are
         * any SRI changes or input queue flushes to report, only samples up to
         * that point are considered, as a read cannot span those packets.
         *
         * If the SRI indicates that the data is complex, the returned value is
         * in terms of complex samples.
         *
         * @warning  The returned value is not guaranteed; if the input queue
         *           flushes in between calls, a subsequent call to @a read may
         *           block or @a tryread may fail.
         */
        size_t samplesAvailable();

        /**
         * @brief  Stream equality comparison.
         * @param other  Another %BufferedInputStream.
         * @returns  True if and only if both BufferedInputStreams reference
         *           the same underlying stream.
         */
        bool operator== (const BufferedInputStream& other) const;

        /**
         * @brief  Returns true if data can be read without blocking.
         * @see  samplesAvailable()
         *
         * A stream is considered ready if samplesAvailable() would return a
         * non-zero value.
         *
         * @warning  Even if this method returns true, if the input queue
         *           flushes in between calls, a subsequent call to @a read may
         *           block or @a tryread may fail.
         */
        bool ready();

    private:
        /// @cond IMPL
        typedef InputStream<PortType> Base;
        using Base::_impl;

        friend class InPort<PortType>;
        typedef InPort<PortType> InPortType;
        BufferedInputStream(const StreamDescriptor&, InPortType*);

        class Impl;
        Impl& impl();
        const Impl& impl() const;
        /// @endcond
    };

    typedef BufferedInputStream<BULKIO::dataChar>      InCharStream;
    typedef BufferedInputStream<BULKIO::dataOctet>     InOctetStream;
    typedef BufferedInputStream<BULKIO::dataShort>     InShortStream;
    typedef BufferedInputStream<BULKIO::dataUshort>    InUShortStream;
    typedef BufferedInputStream<BULKIO::dataLong>      InLongStream;
    typedef BufferedInputStream<BULKIO::dataUlong>     InULongStream;
    typedef BufferedInputStream<BULKIO::dataLongLong>  InLongLongStream;
    typedef BufferedInputStream<BULKIO::dataUlongLong> InULongLongStream;
    typedef BufferedInputStream<BULKIO::dataFloat>     InFloatStream;
    typedef BufferedInputStream<BULKIO::dataDouble>    InDoubleStream;
    typedef BufferedInputStream<BULKIO::dataBit>       InBitStream;
    typedef InputStream<BULKIO::dataXML>               InXMLStream;
    typedef InputStream<BULKIO::dataFile>              InFileStream;

} // end of bulkio namespace

#endif
