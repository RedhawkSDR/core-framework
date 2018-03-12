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

    def ready(self, size=None):
        if self.eos:
            return True

        if size is None:
            # No size given, any data is sufficient
            size = 1

        # Count up the block sizes until it exceeds the requested size, or we
        # run out of blocks
        count = 0
        for block in self.blocks:
            count += self._getBlockSize(block)
            if count >= size:
                return True
        return False

    def _mergeBlocks(self, data, blocks):
        for block in blocks:
            offset = len(data.data)
            data.data += self._getBlockData(block)
            if block.sriChanged or not data.sris:
                data.sris.append(SRI(offset, block.sri))
            for ts in block.getTimestamps():
                data.timestamps.append(TimeStamp(ts.offset + offset, ts.time))

    def get(self, size=None):
        if not self.blocks:
            if self.eos:
                # Return an empty data object with EOS set so the called knows
                # that the stream ended (as opposed to just getting nothing
                # back, and having to guess)
                return StreamData([SRI(0, self.sri)], self._createEmpty(), [], True)
            return None

        # Combine block data into a single data object
        data = StreamData([], self._createEmpty(), [], False)

        # Aggregate block metadata
        if size is None:
            self._mergeBlocks(data, self.blocks)

            # Discard references to blocks
            self.blocks = []
        else:
            # Count up the data from each block to identify the last block
            # needed to get enough data
            count = 0
            for index in xrange(len(self.blocks)):
                count += self._getBlockSize(self.blocks[index])
                if count >= size:
                    break

            # Merge all the blocks up to the last index
            self._mergeBlocks(data, self.blocks[:index])

            # Discard up to last used block
            del self.blocks[:index]

            if count == size:
                # Exact size, merge in and discard the last block
                self._mergeBlocks(data, self.blocks[0:1])
                del self.blocks[:1]
            else:
                offset = len(data.data)
                remain = size - offset
                block = self.blocks[0]
                chunk = self._getBlockData(block)
                data.data += chunk[:remain]
                if block.sriChanged or not data.sris:
                    data.sris.append(SRI(offset, block.sri))
                for ts in block.getTimestamps():
                    if ts.offset < remain:
                        data.timestamps.append(TimeStamp(ts.offset + offset, ts.time))

                # TODO: save leftover block

        # Replace SRI with last known SRI
        self.sri = data.sris[-1].sri

        # Apply EOS flag if stream has ended and there is no saved data
        if not self.blocks and self.eos:
            data.eos = self.eos

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
        sris = [c.sri for c in self._cachedStreams.itervalues()]
        if self._port:
            for stream in self._port.getStreams():
                if stream.streamID not in self._cachedStreams:
                    sris.append(stream.sri)
        return sris

    def read(self, size=None, timeout=-1.0, streamID=None, eos=False):
        if eos:
            if size is not None:
                raise ValueError('size cannot be specified with eos')
            condition = lambda x: x.eos
        else:
            condition = lambda x: x.ready(size)

        container = self._read(timeout, streamID, condition)
        if not container:
            return None
        if container.eos:
            self._removeStreamCache(container)
        return container.get(size)

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
