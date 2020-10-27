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

import CF.DataType;
import org.ossie.component.RHLogger;

import BULKIO.StreamSRI;

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
class InStreamDescriptor {

    public InStreamDescriptor()
    {
        _sri = null;
    };

    public InStreamDescriptor(final BULKIO.StreamSRI sri) {
        _sri = sri;
    };

    public String streamID() {
        return _sri.streamID;
    };

    public boolean blocking() {
        return _sri.blocking;
    };

    public boolean complex() {
        return (_sri.mode != 0);
    };

    public BULKIO.StreamSRI sri() {
        return _sri;
    };

    /*public boolean operator! () {
        return !_sri;
    };*/

    protected BULKIO.StreamSRI _sri;
};
