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

from ossie.utils.log4py import logging
from ossie.utils.sandbox.helper import ThreadedSandboxHelper, ThreadStatus

from bulkio.input_ports import *

_PORT_MAP = {
    'char' : (InCharPort, 'IDL:BULKIO/dataChar:1.0'),
    'octet': (InOctetPort, 'IDL:BULKIO/dataOctet:1.0'),
    'short' : (InShortPort, 'IDL:BULKIO/dataShort:1.0'),
    'ushort' : (InUShortPort, 'IDL:BULKIO/dataUshort:1.0'),
    'long' : (InLongPort, 'IDL:BULKIO/dataLong:1.0'),
    'ulong' : (InULongPort, 'IDL:BULKIO/dataUlong:1.0'),
    'longlong' : (InLongLongPort, 'IDL:BULKIO/dataLongLong:1.0'),
    'ulonglong' : (InULongLongPort, 'IDL:BULKIO/dataUlongLong:1.0'),
    'float' : (InFloatPort, 'IDL:BULKIO/dataFloat:1.0'),
    'double' : (InDoublePort, 'IDL:BULKIO/dataDouble:1.0'),
    'bit' : (InBitPort, 'IDL:BULKIO/dataBit:1.0'),
    'file' : (InFilePort, 'IDL:BULKIO/dataFile:1.0'),
    'XML' : (InXMLPort, 'IDL:BULKIO/dataXML:1.0')
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
    def __init__(self, sri, datatype):
        self.sri = sri
        self.blocks = []
        self.eos = False
        self.datatype = datatype

    @property
    def streamID(self):
        return self.sri.streamID

    def append(self, block):
        self.blocks.append(block)

    def ready(self):
        return bool(self.blocks)

    def get(self):
        if not self.blocks:
            if self.eos:
                # Return an emtpy data object with EOS set so the called knows
                # that the stream ended (as opposed to just getting nothing
                # back, and having to guess)
                return StreamData([SRI(0, self.sri)], self.datatype(), [], True)
            return None

        # Combine block data into a single data object
        if self.datatype == str:
            # For string types (XML, File), return a list of strings
            data = [block.data for block in self.blocks]
        else:
            # All other types, concatenate the block data
            data = reduce(operator.add, (self._getBlockData(block) for block in self.blocks))

        # Aggregate block metadata
        offset = 0
        sris = []
        timestamps = []
        for block in self.blocks:
            if block.sriChanged or not sris:
                sris.append(SRI(offset, block.sri))
            for ts in block.getTimestamps():
                timestamps.append(TimeStamp(ts.offset + offset, ts.time))
            offset += block.size

        # Discard references to blocks
        self.blocks = []

        # Replace SRI with last known SRI
        self.sri = sris[-1].sri

        return StreamData(sris, data, timestamps, self.eos)

    def _getBlockData(self, block):
        # If the block data is complex (taking into account that bit and string
        # data blocks do not have a complex attribute), reformat the data as
        # complex; otherwise, just return the data
        if getattr(block, 'complex', False):
            return block.cxdata
        else:
            return block.data

class StreamSink(ThreadedSandboxHelper):
    def __init__(self, format=None):
        ThreadedSandboxHelper.__init__(self)
        if format:
            formats = [format]
        else:
            formats = _PORT_MAP.keys()
        for format in formats:
            clazz, repo_id = _PORT_MAP[format]
            self._addProvidesPort(format+'In', repo_id, clazz)

        self._streamLock = threading.Lock()
        self._streamReady = threading.Condition(self._streamLock)
        self._streamEnded = threading.Condition(self._streamLock)
        self._finishedStreams = []
        self._activeStreams = {}

    def streamIDs(self):
        return [sri.streamID for sri in self.activeSRIs()]

    def _getDataType(self):
        # NB: This is a hack.
        if not self._port:
            return None
        if 'bit' in self._port.name:
            return bitbuffer
        elif 'XML' in self._port.name or 'File' in self._port.name:
            return str
        else:
            return list

    def activeSRIs(self):
        with self._streamLock:
            sris = {}
            for data in self._finishedStreams:
                sris.setdefault(data.streamID, data.sri)
            for stream in self._activeStreams.itervalues():
                sris.setdefault(stream.streamID, stream.sri)
            return sris.values()

    def read(self, timeout=-1.0, streamID=None, eos=False):
        if timeout >= 0.0:
            end = time.time() + timeout
        else:
            end = None
        if eos:
            cond = self._streamEnded
        else:
            cond = self._streamReady

        while True:
            with self._streamLock:
                data = self._getNextData(eos, streamID)
                if data:
                    return data.get()

                if not self.isRunning():
                    break

                if end is None:
                    # Even though we don't want the read call to timeout, using
                    # a timeout argument to read ensures that ^C interrupts the
                    # wait. Otherwise, it can deadlock and the only way to get
                    # out is to terminate the Python interpreter process.
                    cond.wait(timeout=1.0)
                else:
                    now = time.time()
                    if now >= end:
                        break
                    wait_time = end - now
                    cond.wait(timeout=wait_time)

        return None

    def _getNextData(self, eos, streamID):
        if self._finishedStreams:
            if streamID:
                for ii in xrange(len(self._finishedStreams)):
                    if self._finishedStreams[ii].streamID == streamID:
                        return self._finishedStreams.pop(ii)
            else:
                return self._finishedStreams.pop(0)
        elif not eos and self._activeStreams:
            if streamID:
                if streamID in self._activeStreams:
                    stream = self._activeStreams[streamID]
                    if stream.ready():
                        return stream
            else:
                for stream in self._activeStreams.itervalues():
                    if stream.ready():
                        return stream

        return None

    def _threadFunc(self):
        if not self._port:
            return ThreadStatus.NOOP

        stream = self._port.getCurrentStream()
        if not stream:
            return
        block = stream.read()
        with self._streamLock:
            container = self._activeStreams.get(stream.streamID, None)
            if container is None:
                container = StreamContainer(stream.sri, self._getDataType())
                self._activeStreams[stream.streamID] = container

            if not block:
                if not stream.eos():
                    return ThreadStatus.NORMAL
            else:
                container.append(block)
                self._streamReady.notify()

            if stream.eos():
                container.eos = True
                del self._activeStreams[stream.streamID]
                self._finishedStreams.append(container)
                self._streamEnded.notify()

        return ThreadStatus.NORMAL
