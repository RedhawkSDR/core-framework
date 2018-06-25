#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK bulkioInterfaces.
#
# REDHAWK bulkioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK bulkioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import collections
import operator
import threading
import time

from ossie.utils.sandbox.helper import SandboxHelper

from bulkio.bulkioInterfaces import BULKIO
from bulkio.input_ports import *

_PORT_MAP = {
    'char' : (InCharPort, BULKIO.dataChar),
    'octet': (InOctetPort, BULKIO.dataOctet),
    'short' : (InShortPort, BULKIO.dataShort),
    'ushort' : (InUShortPort, BULKIO.dataUshort),
    'long' : (InLongPort, BULKIO.dataLong),
    'ulong' : (InULongPort, BULKIO.dataUlong),
    'longlong' : (InLongLongPort, BULKIO.dataLongLong),
    'ulonglong' : (InULongLongPort, BULKIO.dataUlongLong),
    'float' : (InFloatPort, BULKIO.dataFloat),
    'double' : (InDoublePort, BULKIO.dataDouble),
    'bit' : (InBitPort, BULKIO.dataBit),
    'file' : (InFilePort, BULKIO.dataFile),
    'xml' : (InXMLPort, BULKIO.dataXML)
}

SRI = collections.namedtuple('SRI', 'offset sri')
TimeStamp = collections.namedtuple('TimeStamp', 'offset time')

class StreamData(object):
    def __init__(self, sris, data, timestamps, eos):
        self.sris = sris
        self.data = data
        self.timestamps = timestamps
        self.eos = eos

    @property
    def streamID(self):
        return self.sri.streamID

    @property
    def sri(self):
        offset, sri = self.sris[0]
        return sri

class StreamContainer(object):
    """
    Internal class to cache data read from a stream.
    """
    def __init__(self, sri):
        self.sri = sri
        self.blocks = []
        self.eos = False

    @property
    def streamID(self):
        return self.sri.streamID

    def append(self, block):
        self.blocks.append(block)

    def ready(self):
        if self.eos:
            return True
        else:
            return bool(self.blocks)

    def get(self):
        # Combine blocks' data and metadata into a single data object
        data = StreamData([], self._createEmpty(), [], self.eos)
        framed = False
        for block in self.blocks:
            offset = len(data.data)
            block_data = self._getBlockData(block)

            # Check for framed data mode
            if block.sri.subsize > 0:
                # If the data wasn't framed before, turn it into a list
                if not framed:
                    if data.data:
                        # There was some data, put it in a list
                        data.data = [data.data]
                    else:
                        # Replace empty data with a list (this is mostly for
                        # the benefit of bitbuffer)
                        data.data = []
                    framed = True

                # Reframe the block's data to match the framing
                block_data = self._reframeData(block_data, block.sri.subsize)
            data.data += block_data

            if block.sriChanged or not data.sris:
                data.sris.append(SRI(offset, block.sri))
            for ts in block.getTimestamps():
                data.timestamps.append(TimeStamp(ts.offset + offset, ts.time))

        # In the event that there were no blocks (which can only happen in the
        # case of an end-of-stream with no data), add the saved SRI
        if not data.sris:
            data.sris = [SRI(0, self.sri)]

        return data

    def _createEmpty(self):
        return []

    def _reframeData(self, data, frameSize):
        return [data[pos:pos+frameSize] for pos in xrange(0, len(data), frameSize)]

    def _getBlockSize(self, block):
        if block.complex:
            return block.cxsize
        else:
            return block.size

    def _getBlockData(self, block):
        if block.complex:
            return block.cxdata
        else:
            return block.data

class StringStreamContainer(StreamContainer):
    """
    Internal class for storing data from string-type streams (XML, File).
    """
    def __init__(self, sri):
        StreamContainer.__init__(self, sri)

    def _getBlockSize(self, block):
        return 1

    def _getBlockData(self, block):
        return [block.buffer]

class BitStreamContainer(StreamContainer):
    """
    Internal class for storing data from packed bit streams.
    """
    def __init__(self, sri):
        StreamContainer.__init__(self, sri)

    def _createEmpty(self):
        return bitbuffer()

    def _getBlockSize(self, block):
        return len(block.buffer)

    def _getBlockData(self, block):
        return block.buffer

class StreamSink(SandboxHelper):
    """
    Sandbox helper for reading BulkIO stream data.

    StreamSink provides a simplified interface for reading data from BulkIO
    streams.  It can support any BulkIO data type (with the exception of SDDS
    and VITA49), but only one type is supported per instance.

    Unlike DataSink, reading from StreamSink returns data from only one stream
    at a time. This avoids problems caused by accidental data interleaving.

    If more control over stream reading is desired, the `port` attribute
    provides access to the underlying BulkIO port. The port is an instance of
    the same class used in Python components, and supports the full stream API.

    See Also:
        StreamSource
    """
    def __init__(self, format=None):
        """
        Creates a new StreamSink.

        Args:
            format:    BulkIO port type to support (e.g., "float"). If not
                       given, all BulkIO port types except SDDS and VITA49 are
                       supported.
        """
        SandboxHelper.__init__(self)

        if format:
            formats = [format]
        else:
            formats = _PORT_MAP.keys()
        for format in formats:
            clazz, helper = _PORT_MAP[format]
            if format == 'bit':
                cache = BitStreamContainer
            elif format in ('xml', 'file'):
                cache = StringStreamContainer
            else:
                cache = StreamContainer
            self._addProvidesPort(format+'In', helper._NP_RepositoryId, clazz, {'cache':cache})

        self._cachedStreams = {}
        self._cacheClass = StreamContainer

    def _portCreated(self, port, portDict):
        self._cacheClass = portDict['cache']

    def streamIDs(self):
        """
        Gets the stream IDs for the active streams.

        Returns:
            list(str): The currently active stream IDs.
        """
        return [sri.streamID for sri in self.activeSRIs()]

    def activeSRIs(self):
        """
        Gets the SRIs for the active streams.

        Returns:
            list(BULKIO.StreamSRI): The currently active SRIs.
        """
        sri_list = [c.sri for c in self._cachedStreams.itervalues()]
        if self._port:
            for stream in self._port.getStreams():
                if stream.streamID not in self._cachedStreams:
                    sri_list.append(stream.sri)
        return sri_list

    @property
    def port(self):
        """
        The BulkIO input port in use by this helper.

        The port is created when a connection is made from this helper to
        another object in the sandbox. If no connection exists, the port is
        None.
        """
        return self._port

    def read(self, timeout=-1.0, streamID=None, eos=False):
        """
        Reads stream data.

        Reading attempts to return as much data from a single stream as
        possible. If the read succeeds, returns a StreamData object with the
        following fields:
          * streamID   - Identifier of the stream from which data was read.
          * sri        - SRI in effect at the start of the read.
          * sris       - List of (offset, StreamSRI) tuples that describe the
                         data. The offset gives the element in `data` at which
                         the SRI applies (e.g., data[0]).
          * data       - The data read from the stream, formatted as necessary.
          * timestamps - List of (offset, PrecisionUTCTime) tuples that give
                         time information for data. The offset gives the
                         element in `data` at which the time stamp applies
                         (e.g., data[0]).
          * eos        - True if the stream has ended, False if still active.

        If the SRI indicates that the data is framed (that is, `sri.subsize` is
        non-zero), StreamSink will turn the data into a list of frames. For
        example, with float data and a frame size of 4, the `data` field will
        be a list of 4-element lists, where each item is a float.

        Args:
            timeout:   Maximum time, in seconds, to wait for data to become
                       available. A negative time waits indefinitely (this is
                       the default).
            streamID:  Identifier of stream to read from (default is first
                       available).
            eos:       If True, wait up to `timeout` for a stream to receive an
                       EOS (default is False). This may be combined with the
                       `streamID` argument to wait for a specific stream.

        Returns:
            StreamData object on success.
            None if timeout elapsed or helper was stopped and no data was
            available.
        """
        if eos:
            condition = lambda x: x.eos
        else:
            condition = lambda x: x.ready()

        container = self._read(timeout, streamID, condition)
        if not container:
            return None
        # The read consumes all cached data for the stream, so we can discard
        # the cache object
        self._removeStreamCache(container)
        return container.get()

    def _read(self, timeout, streamID, condition):
        if timeout >= 0.0:
            end = time.time() + timeout
        else:
            end = None

        while self.started:
            # Fetch as much data as possible without blocking
            while self._fetchData():
                pass

            for container in self._cachedStreams.itervalues():
                if streamID and container.streamID != streamID:
                    continue
                if condition(container):
                    return container

            # Sleep to allow more data to come in
            wait_time = 0.1
            if end is not None:
                now = time.time()
                if now >= end:
                    break
                wait_time = min(wait_time, end - now)
            time.sleep(wait_time)

        return None

    def _fetchData(self):
        # Use a polling loop instead of waiting in getCurrentStream so that the
        # operation can be interrupted by ^C
        stream = self._port.getCurrentStream(0.0)
        if not stream:
            return False

        container = self._getStreamCache(stream)
        block = stream.tryread()
        if block:
            container.append(block)
        elif stream.eos():
            container.eos = True
        return True

    def _getStreamCache(self, stream):
        container = self._cachedStreams.get(stream.streamID, None)
        if not container:
            container = self._cacheClass(stream.sri)
            self._cachedStreams[stream.streamID] = container
        return container

    def _removeStreamCache(self, container):
        del self._cachedStreams[container.streamID]
