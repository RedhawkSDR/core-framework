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

import operator
import threading
import time

from ossie.threadedcomponent import *

from bulkio.input_ports import *

from .helper import SandboxPortHelper

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

    def append(self, block):
        self.blocks.append(block)

    def ready(self):
        return bool(self.blocks)

    def get(self):
        if not self.blocks:
            return None

        # Combine data blocks into a single data object
        front = self.blocks[0]
        sris = [(0, front.sri)]
        data = reduce(operator.add, (block.data for block in self.blocks))
        timestamps = [(0, front.getStartTime())]
        offset = 0
        for block in self.blocks[1:]:
            if block.sriChanged:
                sris.append((offset, block.sri))
            for ts in block.getTimestamps():
                timestamps.append((ts.offset + offset, ts.time))
            offset += block.size

        # Discard references to data
        self.blocks = []

        # Replace SRI with last known SRI
        self.sri = sris[-1][1]

        return StreamData(sris, data, timestamps, self.eos)


class StreamSink(SandboxPortHelper, ThreadedComponent):
    def __init__(self, format=None):
        SandboxPortHelper.__init__(self)
        ThreadedComponent.__init__(self)
        if format:
            formats = [format]
        else:
            formats = _PORT_MAP.keys()
        for format in formats:
            clazz, repo_id = _PORT_MAP[format]
            self._addProvidesPort(format+'In', repo_id, clazz)

        self._port = None

        self._streamLock = threading.Lock()
        self._streamReady = threading.Condition(self._streamLock)
        self._streamEnded = threading.Condition(self._streamLock)
        self._finishedStreams = []
        self._activeStreams = {}

    def streamIDs(self):
        return [sri.streamID for sri in self.activeSRIs()]

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

        with self._streamLock:
            while True:
                data = self._getNextData(eos, streamID)
                if data:
                    return data.get()

                if not self.isRunning():
                    break

                if end is None:
                    cond.wait()
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

    def start(self):
        SandboxPortHelper.start(self)
        self.startThread()

    def stop(self):
        SandboxPortHelper.stop(self)
        self.stopThread()

    def process(self):
        try:
            self._process()
        except Exception:
            import traceback
            traceback.print_exc()
            return FINISH

    def _process(self):
        if not self._port:
            return NOOP

        stream = self._port.getCurrentStream()
        if not stream:
            return
        block = stream.read()
        with self._streamLock:
            data = self._activeStreams.get(stream.streamID, None)
            if data is None:
                data = StreamContainer(stream.sri)
                self._activeStreams[stream.streamID] = data

            if not block:
                if not stream.eos():
                    return NORMAL
            else:
                data.append(block)
                self._streamReady.notify()

            if stream.eos():
                data.eos = True
                del self._activeStreams[stream.streamID]
                self._finishedStreams.append(data)
                self._streamEnded.notify()

        return NORMAL
