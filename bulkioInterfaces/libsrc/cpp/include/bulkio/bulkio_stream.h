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

#ifndef __bulkio_stream_h
#define __bulkio_stream_h

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <ossie/PropertyMap.h>

#include <BULKIO/bulkioDataTypes.h>

namespace bulkio {

    /**
     * @brief  Shared ownership container for StreamSRI.
     *
     * %StreamDescriptor adds shared ownership to the StreamSRI class to
     * provide read-only access to the same underlying SRI instance for
     * multiple readers.
     */
    class StreamDescriptor {
    public:
        StreamDescriptor() :
            _sri()
        {
        }

        StreamDescriptor(const BULKIO::StreamSRI& sri) :
            _sri(boost::make_shared<BULKIO::StreamSRI>(sri))
        {
        }

        std::string streamID() const
        {
            return std::string(_sri->streamID);
        }

        bool blocking() const
        {
            return _sri->blocking;
        }

        bool complex() const
        {
            return (_sri->mode != 0);
        }

        const BULKIO::StreamSRI& sri()
        {
            return *_sri;
        }

        bool operator! () const
        {
            return !_sri;
        }

    protected:
        boost::shared_ptr<BULKIO::StreamSRI> _sri;
    };

    /**
     * @brief  Base class for input and output streams.
     *
     * %StreamBase is a smart-pointer based class that encapsulates a single
     * BulkIO stream. It implements the basic common API for input and output
     * streams, providing accessor methods for StreamSRI fields.
     *
     * @note  User code should typically use the type-specific input and output
     *        stream classes.
     */
    class StreamBase {
    public:
        /**
         * @brief  Returns the stream ID.
         * @pre  Stream is valid.
         *
         * The stream ID is immutable and cannot be changed.
         */
        const std::string& streamID() const;

        /**
         * @brief  Gets the current stream metadata.
         * @returns  Read-only reference to stream SRI.
         * @pre  Stream is valid.
         */
        const BULKIO::StreamSRI& sri() const;

        /**
         * @brief  Implicit conversion to read-only StreamSRI.
         * @pre  Stream is valid.
         */
        operator const BULKIO::StreamSRI& () const;

        /**
         * @brief  Gets the X-axis start value.
         * @returns  Starting coordinate of the first sample in the X
         *           direction.
         * @pre  Stream is valid.
         *
         * For contiguous data, this is the start of the stream in terms of
         * @c xunits. For framed data, this specifies the starting abscissa
         * value, in terms of @c xunits, associated with the first element in
         * each frame.
         */
        double xstart() const;

        /**
         * @brief  Gets the X-axis delta.
         * @returns  The distance between two adjacent samples in the X
         *           direction.
         * @pre  Stream is valid.
         *
         * Because the X-axis is commonly in terms of time (that is,
         * @c sri.xunits is @c BULKIO::UNITS_TIME), this is typically the
         * reciprocal of the sample rate.
         *
         * For framed data, this is the interval between consecutive samples in
         * a frame.
         */
        double xdelta() const;

        /**
         * @brief  Gets the X-axis units.
         * @returns  The unit code for the xstart and xdelta values.
         * @pre  Stream is valid.
         *
         * Axis units are specified using constants in the BULKIO namespace.
         * For contiguous data, the X-axis is commonly in terms of time,
         * @c BULKIO::UNITS_TIME. For framed data, the X-axis is often in terms
         * of frequency, @c BULKIO::UNITS_FREQUENCY.
         */
        short xunits() const;

        /**
         * @brief  Gets the frame size.
         * @returns The length of a row for framed data, or 0 if the data is
         *          contiguous.
         * @pre  Stream is valid.
         *
         * A subsize of 0 indicates that the data is contiguous; this is the
         * default setting. For contiguous data, only the X-axis fields are
         * applicable.
         *
         * A non-zero subsize indicates that the data is framed, with each row
         * having a length of @c subsize. For framed data, both the X-axis and
         * Y-axis fields are applicable.
         */
        int subsize() const;

        /**
         * @brief  Gets the Y-axis start value.
         * @returns  Starting coordinate of the first frame in the Y direction.
         * @pre  Stream is valid.
         * @see  subsize()
         *
         * @note  Y-axis fields are only applicable when subsize is non-zero.
         *
         * This specifies the start of the stream in terms of @c yunits.
         */
        double ystart() const;

        /**
         * @brief  Gets the Y-axis delta.
         * @returns  The distance between two adjacent frames in the Y
         *           direction.
         * @pre  Stream is valid.
         * @see  subsize()
         *
         * @note  Y-axis fields are only applicable when subsize is non-zero.
         *
         * This specifies the interval between frames in terms of @c yunits.
         */
        double ydelta() const;

        /**
         * @brief  Gets the Y-axis units.
         * @returns  The unit code for the ystart and ydelta values.
         * @pre  Stream is valid.
         * @see  subsize()
         * @see  xunits()
         *
         * @note  Y-axis fields are only applicable when subsize is non-zero.
         *
         * Axis units are specified using constants in the BULKIO namespace.
         */
        short yunits() const;

        /**
         * @brief  Gets the complex mode of this stream.
         * @returns  True if data is complex. False if data is not complex.
         * @pre  Stream is valid.
         *
         * A stream is considered complex if @c sri.mode is non-zero.
         */
        bool complex() const;

        /**
         * @brief  Gets the blocking mode of this stream.
         * @returns  True if this stream is blocking. False if stream is non-
         *           blocking.
         * @pre  Stream is valid.
         */
        bool blocking() const;

        /**
         * @brief  Read-only access to the set of SRI keywords.
         * @returns  A read-only reference to the SRI keywords.
         * @pre  Stream is valid.
         *
         * The SRI keywords are reinterpreted as const reference to a
         * PropertyMap, which provides a higher-level interface than the
         * default CORBA sequence.
         */
        const redhawk::PropertyMap& keywords() const;

        /**
         * @brief  Checks for the presence of a keyword in the SRI.
         * @param name  The name of the keyword.
         * @returns  True if the keyword is found. False if keyword is not
         *           found.
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
         * Allows for easy lookup of keyword values in the SRI. To avoid
         * exceptions on missing keywords, the presence of a keyword can be
         * checked with hasKeyword().
         */
        const redhawk::Value& getKeyword(const std::string& name) const;

        /**
         * @brief  Checks stream validity.
         * @returns  True if this stream is not valid. False if the stream is
         *           invalid.
         *
         * Invalid (null) streams are not associated with an active stream in a
         * port. If this method returns true, no other methods except
         * comparison or assignment may be called.
         */
        bool operator! () const;

    protected:
        /// @cond IMPL
        class Impl : public StreamDescriptor {
        public:
            Impl(const StreamDescriptor& sri) :
                StreamDescriptor(sri),
                _streamID(sri.streamID())
            {
            }

            const std::string& streamID() const
            {
                return _streamID;
            }

            virtual ~Impl() { }

        protected:
            const std::string _streamID;
        };

        StreamBase();
        StreamBase(const boost::shared_ptr<Impl>& impl);

        boost::shared_ptr<Impl> _impl;
        /// @endcond
    };
}

#endif // __bulkio_stream_h
