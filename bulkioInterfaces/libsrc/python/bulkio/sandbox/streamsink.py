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
        for block in self.blocks:
            offset = len(data.data)
            data.data += self._getBlockData(block)
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
    def __init__(self, sri):
        StreamContainer.__init__(self, sri)

    def _getBlockSize(self, block):
        return 1

    def _getBlockData(self, block):
        return [block.data]

class BitStreamContainer(StreamContainer):
    def __init__(self, sri):
        StreamContainer.__init__(self, sri)

    def _createEmpty(self):
        return bitbuffer()

    def _getBlockSize(self, block):
        return block.size

    def _getBlockData(self, block):
        return block.data

class StreamSink(SandboxHelper):
    def __init__(self, format=None):
        SandboxHelper.__init__(self)

        if format:
            formats = [format]
        else:
            formats = _PORT_MAP.keys()
        for format in formats:
            clazz, helper = _PORT_MAP[format]
            self._addProvidesPort(format+'In', helper._NP_RepositoryId, clazz)

        self._cachedStreams = {}
        self._cacheClass = StreamContainer

    def streamIDs(self):
        return [sri.streamID for sri in self.activeSRIs()]

    def _portCreated(self, port, portDict):
        repo_id = portDict['Port Interface']
        if repo_id == BULKIO.dataBit._NP_RepositoryId:
            self._cacheClass = BitStreamContainer
        elif repo_id in (BULKIO.dataXML._NP_RepositoryId, BULKIO.dataFile._NP_RepositoryId):
            self._cacheClass = StringStreamContainer

    def activeSRIs(self):
        sri_list = [c.sri for c in self._cachedStreams.itervalues()]
        if self._port:
            for stream in self._port.getStreams():
                if stream.streamID not in self._cachedStreams:
                    sri_list.append(stream.sri)
        return sri_list

    @property
    def port(self):
        return self._port

    def read(self, timeout=-1.0, streamID=None, eos=False):
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
