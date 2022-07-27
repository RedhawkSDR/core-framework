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
package bulkio;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Iterator;
import java.util.Map;
import java.util.ArrayDeque;
import java.util.HashSet;
import java.util.Set;
import java.util.concurrent.Semaphore;
import java.util.concurrent.TimeUnit;

import org.apache.log4j.Logger;

import bulkio.InStreamDescriptor;

import CF.DataType;
import org.ossie.component.RHLogger;

import BULKIO.PortStatistics;
import BULKIO.PortUsageType;
import BULKIO.PrecisionUTCTime;
import BULKIO.StreamSRI;
import org.omg.CORBA.Any;

/**
 * 
 */
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
public class InStreamBase {

    static int EOS_NONE = 0;
    static int EOS_RECEIVED = 1;
    static int EOS_REACHED = 2;
    static int EOS_REPORTED = 3;

    /**
        * @brief  Gets the current stream metadata.
        * @returns  Read-only reference to stream SRI.
        * @pre  Stream is valid.
        */
    public BULKIO.StreamSRI sri() {
        if (_impl == null) {
            return null;
        }
        return _impl.sri();
    };

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
    public double xstart() {
        if (_impl == null) {
            return 0;
        }
        return _impl.sri().xstart;
    };

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
    public double xdelta() {
        if (_impl == null) {
            return 0;
        }
        return _impl.sri().xdelta;
    };

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
    public short xunits() {
        if (_impl == null) {
            return 0;
        }
        return _impl.sri().xunits;
    };

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
    public int subsize() {
        if (_impl == null) {
            return 0;
        }
        return _impl.sri().subsize;
    };

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
    public double ystart() {
        if (_impl == null) {
            return 0;
        }
        return _impl.sri().ystart;
    };

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
    public double ydelta() {
        if (_impl == null) {
            return 0;
        }
        return _impl.sri().ydelta;
    };

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
    public short yunits() {
        if (_impl == null) {
            return 0;
        }
        return _impl.sri().yunits;
    };

    /**
        * @brief  Gets the complex mode of this stream.
        * @returns  True if data is complex. False if data is not complex.
        * @pre  Stream is valid.
        *
        * A stream is considered complex if @c sri.mode is non-zero.
        */
    public boolean complex() {
        if (_impl == null) {
            return false;
        }
        return _impl.complex();
    };

    /**
        * @brief  Gets the blocking mode of this stream.
        * @returns  True if this stream is blocking. False if stream is non-
        *           blocking.
        * @pre  Stream is valid.
        */
    public boolean blocking() {
        if (_impl == null) {
            return false;
        }
        return _impl.sri().blocking;
    };

    /**
        * @brief  Read-only access to the set of SRI keywords.
        * @returns  A read-only reference to the SRI keywords.
        * @pre  Stream is valid.
        *
        * The SRI keywords are reinterpreted as const reference to a
        * PropertyMap, which provides a higher-level interface than the
        * default CORBA sequence.
        */
    public  CF.DataType[] keywords() {
        if (_impl == null) {
            return null;
        }
        return _impl.sri().keywords;
    };

    /**
        * @brief  Checks for the presence of a keyword in the SRI.
        * @param name  The name of the keyword.
        * @returns  True if the keyword is found. False if keyword is not
        *           found.
        * @pre  Stream is valid.
        */
    public boolean hasKeyword(final String name) {
        for (DataType keyword : _impl.sri().keywords) {
            if (keyword.id.equals(name)) {
                return true;
            }
        }
        return false;
    };

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
    public Any getKeyword(final String name) {
        if (_impl == null) {
            return null;
        }
        for (DataType keyword : _impl.sri().keywords) {
            if (keyword.id.equals(name)) {
                return keyword.value;
            }
        }
        return null;
    };

    public InStreamBase() {
        _impl = null;
    };
    public InStreamBase(final InStreamDescriptor impl) {
        _impl = impl;
    };

    public InStreamDescriptor _impl;
    /// @endcond
};
