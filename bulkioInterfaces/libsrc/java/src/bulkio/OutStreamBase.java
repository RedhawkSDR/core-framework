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

import java.util.Arrays;
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
import BULKIO.PrecisionUTCTimeHelper;
import BULKIO.StreamSRI;
import org.omg.CORBA.ORB;
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
public class OutStreamBase {

    protected OutStreamBase(BULKIO.StreamSRI sri) {
        this._sri = sri;
    }

    protected BULKIO.StreamSRI _sri;
    protected boolean _sri_updated;

    /**
     * @brief  Update the SRI.
     * @param sri  New SRI.
     * @pre  Stream is valid.
     *
     * Overwrites all SRI fields except for @c streamID, which is
     * immutable.  If the SRI is changed, it will be pushed on the next
     * write.
     */
    public void sri(BULKIO.StreamSRI sri) {
        bulkio.sri.DefaultComparator comparator = new bulkio.sri.DefaultComparator();
        if (!comparator.compare(this._sri, sri)) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri = sri;
    };

    public BULKIO.StreamSRI sri() {
        return this._sri;
    };
    public boolean complex() {
        if (this._sri == null) {
            return false;
        }
        if (this._sri.mode == 0) {
            return false;
        }
        return true;
    };
    public String streamID() {
        return this._sri.streamID;
    };
    public double xstart() {
        return this._sri.xstart;
    };
    public double xdelta() {
        return this._sri.xdelta;
    }
    public short xunits() {
        return this._sri.xunits;
    };
    public double ystart() {
        return this._sri.ystart;
    };
    public double ydelta() {
        return this._sri.ydelta;
    };
    public short yunits() {
        return this._sri.yunits;
    };
    public int subsize() {
        return this._sri.subsize;
    };
    public int mode() {
        return this._sri.mode;
    };
    public boolean blocking() {
        return this._sri.blocking;
    };
    public CF.DataType[] keywords() {
        return this._sri.keywords;
    };

    public boolean checkSRI() {
        if (this._sri_updated) {
            this._sri_updated = false;
            return true;
        }
        return false;
    }

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
    public void xstart(double start) {
        if (Double.compare(start, this._sri.xstart) != 0) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.xstart = start;
    };

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
    public void xdelta(double delta) {
        if (Double.compare(delta, this._sri.xdelta) != 0) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.xdelta = delta;
    };

    /**
     * @brief  Sets the X-axis units.
     * @param units  Unit code for @c xstart and @c xdelta.
     * @pre  Stream is valid.
     * @see  xunits() const
     *
     * Changing @c xunits updates the SRI, which will be pushed on the next
     * write.
     */
    public void xunits(short units) {
        if (units != this._sri.xunits) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.xunits = units;
    };

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
    public void subsize(int size) {
        if (size != this._sri.subsize) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.subsize = size;
    };

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
    public void ystart(double start) {
        if (Double.compare(start, this._sri.ystart) != 0) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.ystart = start;
    };

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
    public void ydelta(double delta) {
        if (Double.compare(delta, this._sri.ydelta) != 0) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.ydelta = delta;
    };

    /**
     * @brief  Sets the Y-axis units.
     * @returns  Unit code for @c ystart and @c ydelta.
     * @pre  Stream is valid.
     * @see  yunits() const
     *
     * Changing @c yunits updates the SRI, which will be pushed on the next
     * write.
     */
    public void yunits(short units) {
        if (units != this._sri.yunits) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.yunits = units;
    };

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
    public void complex(boolean mode) {
        if (mode) {
            if (this._sri.mode == 0) {
                this.flush();
                this._sri_updated = true;
            }
            this._sri.mode = 1;
        } else {
            if (this._sri.mode == 1) {
                this.flush();
                this._sri_updated = true;
            }
            this._sri.mode = 0;
        }
    };

    /**
     * @brief  Sets the blocking mode of this stream.
     * @param mode  True if blocking. False if stream is non-blocking.
     * @pre  Stream is valid.
     *
     * Changing the %blocking mode updates the SRI, which will be pushed on
     * the next write.
     */
    public void blocking(boolean mode) {
        if (mode != this._sri.blocking) {
            this.flush();
            this._sri_updated = true;
        }
        this._sri.blocking = mode;
    };

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
    public void keywords(CF.DataType[] props) {
        this.flush();
        this._sri.keywords = props;
        this._sri_updated = true;
    };

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
    public void setKeyword(String name, Any value) {
        this.flush();
        this._sri.keywords = Arrays.copyOf(this._sri.keywords, this._sri.keywords.length+1);
        this._sri.keywords[this._sri.keywords.length-1] = new CF.DataType();
        this._sri.keywords[this._sri.keywords.length-1].value = ORB.init().create_any();
        this._sri.keywords[this._sri.keywords.length-1].id = name;
        this._sri.keywords[this._sri.keywords.length-1].value = value;
        this._sri_updated = true;
    };
    public void setKeyword(String name, String value) {
        Any _any = ORB.init().create_any();
        _any.insert_string(value);
        setKeyword(name, _any);
    };
    public void setKeyword(String name, int value) {
        Any _any = ORB.init().create_any();
        _any.insert_long(value);
        setKeyword(name, _any);
    };
    public void setKeyword(String name, short value) {
        Any _any = ORB.init().create_any();
        _any.insert_short(value);
        setKeyword(name, _any);
    };
    public void setKeyword(String name, boolean value) {
        Any _any = ORB.init().create_any();
        _any.insert_boolean(value);
        setKeyword(name, _any);
    };
    public void setKeyword(String name, BULKIO.PrecisionUTCTime value) {
        Any _any = ORB.init().create_any();
        PrecisionUTCTimeHelper.insert(_any, value);
        setKeyword(name, _any);
    };
    public void setKeyword(String name, float value) {
        Any _any = ORB.init().create_any();
        _any.insert_float(value);
        setKeyword(name, _any);
    };
    public void setKeyword(String name, double value) {
        Any _any = ORB.init().create_any();
        _any.insert_double(value);
        setKeyword(name, _any);
    };

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
    public void eraseKeyword(String name) {
        this.flush();
        int ii=0;
        for (; ii<this._sri.keywords.length; ii++) {
            if (this._sri.keywords[ii].id.equals(name)) {
                break;
            }
        }
        if (ii == this._sri.keywords.length) {
            return;
        }
        CF.DataType[] new_keywords = new CF.DataType[this._sri.keywords.length-1];
        int skip_idx = 0;
        for (int jj=0; jj<this._sri.keywords.length; jj++) {
            if (ii==jj) {
                continue;
            }
            new_keywords[skip_idx] = this._sri.keywords[jj];
            skip_idx++;
        }
        this._sri.keywords = new_keywords;
        this._sri_updated = true;
    };

    public Any getKeyword(String name) {
        for (int ii=0; ii<this._sri.keywords.length; ii++) {
            if (this._sri.keywords[ii].id.equals(name)) {
                return this._sri.keywords[ii].value;
            }
        }
        return null;
    };

    public boolean hasKeyword(String name) {
        for (int ii=0; ii<this._sri.keywords.length; ii++) {
            if (this._sri.keywords[ii].id.equals(name)) {
                return true;
            }
        }
        return false;
    };

    /**
     * @brief  Closes this stream and sends an end-of-stream.
     * @pre  Stream is valid.
     * @post  Stream is invalid.
     *
     * Closing a stream sends an end-of-stream packet and resets the stream
     * handle. No further operations may be made on the stream.
     */
    public void close() {

    };

    // Overriden in type-specific port
    public void flush() {
    };
};
