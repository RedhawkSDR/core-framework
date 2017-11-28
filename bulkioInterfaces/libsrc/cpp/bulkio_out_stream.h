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
#include <BULKIO/bulkioDataTypes.h>

#include "bulkio_traits.h"
#include "bulkio_datablock.h"

namespace bulkio {

  template <class PortTraits>
  class OutPort;

  /**
   * @brief  BulkIO output stream class.
   * @headerfile  bulkio_out_stream.h <bulkio/bulkio_out_stream.h>
   *
   * %OutputStream is a smart pointer-based class that encapsulates a single
   * BulkIO stream for writing. It is associated with the OutPort that created
   * it, providing a file-like API on top of the classic BulkIO pushPacket
   * model.
   *
   * @warning  Do not declare instances of this template class directly in user
   *           code; the template parameter and class name are not considered
   *           API. Use the type-specific @c typedef instead, such as
   *           bulkio::OutFloatStream, or the nested @c typedef StreamType from
   *           an %OutPort.
   *
   * Notionally, a BulkIO stream represents a contiguous data set and its
   * associated signal-related information (SRI), uniquely identified by a
   * stream ID, from creation until close. The SRI may vary over time, but the
   * stream ID is immutable. Only one stream with a given stream ID can be
   * active at a time.
   *
   * OutputStreams help manage the stream lifetime by tying that SRI with an
   * %OutPort, and ensuring that all data is associated with a valid stream.
   * When the stream is complete, it may be closed, notifying downstream
   * receivers that no more data is expected.
   *
   * The %OutputStream class itself is a lightweight handle; it is inexpensive
   * to copy or store in local variables or nested data types. Assigning one
   * %OutputStream to another does not copy the stream state, but instead
   * aliases both objects to the same underlying stream.
   *
   * The default constructor creates an invalid "null" %InputStream that cannot
   * be used for any real operations, similar to a null pointer. A stream may
   * be checked for validity with the boolean ! operator:
   *
   * @code
   * if (!stream) {
   *   // handle failure
   * } else {
   *   // use stream
   * }
   * @endcode
   *
   * OutputStreams must be created via an %OutPort. A stream can not be
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
  template <class PortTraits>
  class OutputStream {
  public:
    /// @brief  The native type of a real sample.
    typedef typename PortTraits::DataTransferTraits::NativeDataType ScalarType;

    /// @brief  The native type of a complex sample.
    typedef std::complex<ScalarType> ComplexType;

    /**
     * @brief  Default constructor.
     * @see  OutPort::createStream(const std::string&)
     * @see  OutPort::createStream(const BULKIO::StreamSRI&)
     *
     * Create a null OutputStream. This stream is not associated with a stream
     * from any OutPort instance. No methods may be called on the %OutputStream
     * except for operator!, which will always return true; and operator==,
     * which returns true if the other %OutputStream is also null, or false
     * otherwise.
     *
     * New, valid streams are created via an %OutPort.
     */
    OutputStream();

    /**
     * @brief  Returns the stream ID.
     * @pre  Stream is valid.
     *
     * The stream ID is immutable and cannot be changed.
     */
    const std::string& streamID() const;

    /**
     * @brief  Gets the stream metadata.
     * @returns  Read-only reference to stream SRI.
     * @pre  Stream is valid.
     */
    const BULKIO::StreamSRI& sri() const;

    /**
     * @brief  Update the SRI.
     * @param sri  New SRI.
     * @pre  Stream is valid.
     *
     * Overwrites all SRI fields except for @c streamID, which is immutable.
     * The updated SRI will be pushed on the next call to write().
     */
    void sri(const BULKIO::StreamSRI& sri);

    /**
     * @brief  Gets the X-axis delta.
     * @returns  The distance between two adjacent samples in the X direction.
     * @pre  Stream is valid.
     *
     * Since the X-axis is commonly in terms of time (that is, @c sri.xunits is
     * @c BULKIO::UNITS_TIME), this is typically the reciprocal of the sample
     * rate.
     */
    double xdelta() const;

    /**
     * @brief  Sets the X-axis delta.
     * @param delta  The distance between two adjacent samples in the X
     *               direction.
     * @pre  Stream is valid.
     * @see  xdelta() const
     *
     * Changing the %xdelta updates the SRI, which will be pushed on the next
     * call to write().
     */
    void xdelta(double delta);

    /**
     * @brief  Gets the complex mode of this stream.
     * @returns  True if data is complex, false if data is real.
     * @pre  Stream is valid.
     *
     * A stream is considered complex if @c sri.mode is non-zero.
     */
    bool complex() const;

    /**
     * @brief  Sets the complex mode of this stream.
     * @param mode  True if data is complex, false if data is real.
     * @pre  Stream is valid.
     * @see  complex() const
     *
     * Changing the %complex mode indicates that all subsequent data is real or
     * complex based on the value of @a mode. The updated SRI will be pushed on
     * the next call to write().
     */
    void complex(bool mode);

    /**
     * @brief  Gets the blocking mode of this stream.
     * @returns  True if this stream is blocking, false if it is non-blocking.
     * @pre  Stream is valid.
     */
    bool blocking() const;

    /**
     * @brief  Sets the blocking mode of this stream.
     * @param mode  True if blocking, false if non-blocking.
     * @pre  Stream is valid.
     *
     * Changing the %blocking mode updates the SRI, which will be pushed on the
     * next call to write().
     */
    void blocking(bool mode);

    /**
     * @brief  Read-only access to the set of SRI keywords.
     * @returns  A read-only reference to the SRI keywords.
     * @pre  Stream is valid.
     *
     * The SRI keywords are reinterpreted as const reference to a PropertyMap,
     * which provides a higher-level interface than the default CORBA sequence.
     */
    const redhawk::PropertyMap& keywords() const;

    /**
     * @brief  Overwrites the SRI keywords.
     * @param props  New SRI keywords.
     * @pre  Stream is valid.
     * @see  setKeyword
     *
     * The current SRI keywords are replaced with @a props. The updated SRI
     * will be pushed on the next call to write().
     */
    void keywords(const _CORBA_Unbounded_Sequence<CF::DataType>& props);

    /**
     * @brief  Checks for the presence of a keyword in the SRI.
     * @param name  The name of the keyword.
     * @returns  True if the keyword is found, false otherwise.
     * @pre  Stream is valid.
     */
    bool hasKeyword(const std::string& name) const;

    /**
     * @brief  Gets the current value of a keyword in the SRI.
     * @param name  The name of the keyword.
     * @returns  A read-only reference to the keyword's value.
     * @throw std::invalid_argument  If no keyword @a name exists.
     * @pre  Stream is valid.
     * @see  hasKeyword
     *
     * Allows for easy lookup of keyword values in the SRI. To avoid exceptions
     * on missing keywords, the presence of a keyword can be checked with
     * hasKeyword().
     */
    const redhawk::Value& getKeyword(const std::string& name) const;

    /**
     * @brief  Sets the current value of a keyword in the SRI.
     * @param name  The name of the keyword.
     * @param value  The new value.
     * @pre  Stream is valid.
     * @see  setKeyword(const std::string&, const redhawk::Value&)
     * @see  setKeyword(const std::string&, const T&)
     *
     * If the keyword @a name already exists, its value is updated to @a value,
     * otherwise a new keyword is appended.
     *
     * Setting a keyword updates the SRI, which will be pushed on the next
     * call to write().
     */
    void setKeyword(const std::string& name, const CORBA::Any& value);

    /**
     * @brief  Sets the current value of a keyword in the SRI.
     * @param name  The name of the keyword.
     * @param value  The new value.
     * @pre  Stream is valid.
     * @see  setKeyword(const std::string&, const T&)
     *
     * If the keyword @a name already exists, its value is updated to @a value,
     * otherwise a new keyword is appended.
     *
     * Setting a keyword updates the SRI, which will be pushed on the next
     * call to write().
     */
    void setKeyword(const std::string& name, const redhawk::Value& value);

    /**
     * @brief  Sets the current value of a keyword in the SRI.
     * @param name  The name of the keyword.
     * @param value  The new value.
     * @tparam T  Any type that can be converted to a redhawk::Value.
     * @pre  Stream is valid.
     *
     * If the keyword @a name already exists, its value is updated to @a value,
     * otherwise a new keyword is appended.
     *
     * Setting a keyword updates the SRI, which will be pushed on the next
     * call to write().
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
     * Erases the keyword named @a name from the SRI keywords. If no keyword
     * @a name is found, the keywords are not modified.
     *
     * Removing a keyword updates the SRI, which will be pushed on the next
     * call to write().
     */
    void eraseKeyword(const std::string& name);

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
     * Sends the contents of a real or complex vector as a single packet. This
     * is a convenience wrapper that defers to one of the write methods that
     * takes a pointer and size, depending on on whether @a T is real or
     * complex.
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
     * Sends the contents of a real or complex vector as one or more packets.
     * This is a convenience wrapper that defers to one of the write methods
     * that takes a pointer and size, depending on on whether @a T is real or
     * complex.
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
     * Sends @a count samples of scalar data as single packet with the time
     * stamp @a time via the associated OutPort.
     *
     * If there are any pending SRI changes, the new SRI is pushed first.
     */
    void write(const ScalarType* data, size_t count, const BULKIO::PrecisionUTCTime& time);

    /**
     * @brief  Writes one or more packets of real data.
     * @param data  Pointer to real sample data.
     * @param count  Number of samples to write.
     * @param times  List of time stamps, with offsets.
     * @pre  Stream is valid.
     * @pre  @p times is sorted order in order of offset.
     * @throw std::logic_error  If @p times is empty.
     * @see  write(const ScalarType*,size_t,const BULKIO::PrecisionUTCTime&)
     *
     * Writes @a count samples of scalar data to the steam, where each element
     * of @a times gives the offset and time stamp of an individual packet. The
     * offset of the first time stamp is ignored and assumed to be 0, while
     * subsequent offsets determine the length of the prior packet. All offsets
     * should be less than @a count.
     *
     * For example, given three time stamps with offsets 0, 10, and 20, and a
     * @a count of 25, @a data is broken into three packets of size 10, 10 and
     * 5 samples.
     *
     * If there are any pending SRI changes, the new SRI is pushed first.
     *
     * @note  This method may be used when the stream is configured for complex
     *        data, though this usage is not recommended. In this case, the
     *        offsets in @a times are interpreted in terms of complex samples.
     */
    void write(const ScalarType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times);

    /**
     * @brief  Writes a packet of complex data.
     * @param data  Pointer to complex sample data.
     * @param count  Number of samples to write.
     * @param time  Time stamp of first sample.
     * @throw std::logic_error  If stream is not configured for complex data.
     * @pre  Stream is valid.
     *
     * Sends @a count samples of complex data as single packet with the time
     * stamp @a time via the associated OutPort.
     *
     * If there are any pending SRI changes, the new SRI is pushed first.
     */
    void write(const ComplexType* data, size_t count, const BULKIO::PrecisionUTCTime& time);

    /**
     * @brief  Writes one or more packets of complex data.
     * @param data  Pointer to complex sample data.
     * @param count  Number of samples to write.
     * @param times  List of time stamps, with offsets.
     * @pre  Stream is valid.
     * @pre  @p times is sorted order in order of offset.
     * @throw std::logic_error  If stream is not configured for complex data.
     * @throw std::logic_error  If @p times is empty.
     * @see  write(const ComplexType*,size_t,const BULKIO::PrecisionUTCTime&)
     *
     * Writes @a count samples of complex data to the steam, where each element
     * of @a times gives the offset and time stamp of an individual packet. The
     * offset of the first time stamp is ignored and assumed to be 0, while
     * subsequent offsets determine the length of the prior packet. All offsets
     * should be less than @a count.
     *
     * For example, given three time stamps with offsets 0, 10, and 20, and a
     * @a count of 25, @a data is broken into three packets of size 10, 10 and
     * 5 samples.
     *
     * If there are any pending SRI changes, the new SRI is pushed first.
     */
    void write(const ComplexType* data, size_t count, const std::list<bulkio::SampleTimestamp>& times);

    /**
     * @brief  Closes this stream and sends an end-of-stream.
     * @pre  Stream is valid.
     * @post  Stream is invalid.
     *
     * Closing a stream sends an end-of-stream packet and resets the stream
     * handle. No further operations may be made on the stream.
     */
    void close();

    /**
     * @brief  Checks stream validity.
     * @returns  True if this stream is not valid, false if it is valid.
     *
     * Invalid (null) OutputStreams are not associated with an active stream in
     * an %OutPort. If this method returns true, no other methods except
     * comparison or assignment may be called.
     */
    bool operator! () const
    {
      return !_impl;
    }

  private:
    /// @cond IMPL
    friend class OutPort<PortTraits>;
    OutputStream(const BULKIO::StreamSRI& sri, OutPort<PortTraits>* port);

    class Impl;
    boost::shared_ptr<Impl> _impl;
    /// @endcond
  };

  typedef OutputStream<bulkio::CharPortTraits>      OutCharStream;
  typedef OutputStream<bulkio::OctetPortTraits>     OutOctetStream;
  typedef OutputStream<bulkio::ShortPortTraits>     OutShortStream;
  typedef OutputStream<bulkio::UShortPortTraits>    OutUShortStream;
  typedef OutputStream<bulkio::LongPortTraits>      OutLongStream;
  typedef OutputStream<bulkio::ULongPortTraits>     OutULongStream;
  typedef OutputStream<bulkio::LongLongPortTraits>  OutLongLongStream;
  typedef OutputStream<bulkio::ULongLongPortTraits> OutULongLongStream;
  typedef OutputStream<bulkio::FloatPortTraits>     OutFloatStream;
  typedef OutputStream<bulkio::DoublePortTraits>    OutDoubleStream;

} // end of bulkio namespace

#endif
