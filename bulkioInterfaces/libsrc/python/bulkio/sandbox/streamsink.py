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
from ossie.utils.sb.io_helpers import _SinkBase

from bulkio.input_ports import *

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


class StreamSink(_SinkBase, ThreadedComponent):
    def __init__(self, format=None):
        if format is None:
            formats = ['char', 'octet', 'short', 'ushort', 'long', 'ulong',
                       'longlong', 'ulonglong', 'float', 'double', 'bit', 'file', 'xml']
        else:
            formats = [format]
        _SinkBase.__init__(self, formats=formats)
        ThreadedComponent.__init__(self)
        self._port = None
        self._portClasses = {
            'char': InCharPort,
            'octet': InOctetPort,
            'short': InShortPort,
            'ushort': InUShortPort,
            'long': InLongPort,
            'ulong': InULongPort,
            'longlong': InLongLongPort,
            'ulonglong': InULongLongPort,
            'float': InFloatPort,
            'double': InDoublePort,
            'file': InFilePort,
            'xml': InXMLPort,
            'bit': InBitPort
        }
        self._streamLock = threading.Lock()
        self._streamReady = threading.Condition(self._streamLock)
        self._streamEnded = threading.Condition(self._streamLock)
        self._finishedStreams = []
        self._activeStreams = {}

    def getPort(self, portName):
        if self._port:
            if portName != self._port.name:
                raise RuntimeError('StreamSink only supports 1 port type at a time')
        else:
            interface = portName[:-2]
            self._port = self._portClasses[interface](portName)

        return self._port._this()

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
        if self._port:
            if hasattr(self._port, 'startPort'):
                self._port.startPort()
        self.startThread()

    def stop(self):
        if self._port and hasattr(self._port, 'stopPort'):
            self._port.stopPort()
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
