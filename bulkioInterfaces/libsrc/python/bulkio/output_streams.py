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

import copy

from ossie.cf import CF
from redhawk.bitbuffer import bitbuffer

import bulkio
from bulkio.bulkioInterfaces import BULKIO
from bulkio.stream_base import StreamBase

class OutputStream(StreamBase):
    def __init__(self, sri, port, dtype=list):
        StreamBase.__init__(self, sri)
        self._port = port
        self._dtype = dtype
        self.__sriModified = True

    @StreamBase.sri.setter
    def sri(self, sri):
        if not isinstance(sri, BULKIO.StreamSRI):
            raise TypeError('sri must be a BULKIO.StreamSRI') 
        self._modifyingStreamMetadata()
        # Deep copy to avoid accidental updates to the SRI via the caller's
        # reference
        sri = copy.deepcopy(sri)
        # Preserve stream ID
        sri.streamID = self.streamID
        self._sri = sri

    @StreamBase.xstart.setter
    def xstart(self, xstart):
        self._setStreamMetadata('xstart', float(xstart))

    @StreamBase.xdelta.setter
    def xdelta(self, xdelta):
        self._setStreamMetadata('xdelta', float(xdelta))

    @StreamBase.xunits.setter
    def xunits(self, xunits):
        self._setStreamMetadata('xunits', int(xunits))

    @StreamBase.subsize.setter
    def subsize(self, subsize):
        self._setStreamMetadata('subsize', int(subsize))

    @StreamBase.ystart.setter
    def ystart(self, ystart):
        self._setStreamMetadata('ystart', float(ystart))

    @StreamBase.ydelta.setter
    def ydelta(self, ydelta):
        self._setStreamMetadata('ydelta', float(ydelta))

    @StreamBase.yunits.setter
    def yunits(self, yunits):
        self._setStreamMetadata('yunits', int(yunits))

    @StreamBase.complex.setter
    def complex(self, mode):
        self._setStreamMetadata('mode', 1 if mode else 0)

    @StreamBase.blocking.setter
    def blocking(self, blocking):
        self._setStreamMetadata('blocking', 1 if blocking else 0)

    @StreamBase.keywords.setter
    def keywords(self, keywords):
        self._modifyingStreamMetadata()
        # Copy the sequence, but not the values
        self._sri.keywords = keywords[:]

    def setKeyword(self, name, value):
        self._modifyingStreamMetadata()
        bulkio.sri.setKeyword(self._sri, name, value)

    def eraseKeyword(self, name):
        self._modifyingStreamMetadata()
        bulkio.sri.eraseKeyword(self._sri, name)

    def write(self, data, time):
        self._send(data, time, False)

    def close(self):
        data = self._dtype()
        self._send(data, bulkio.timestamp.notSet(), True)

    def _setStreamMetadata(self, attr, value):
        field = getattr(self._sri, attr)
        if field != value:
            self._modifyingStreamMetadata()
            setattr(self._sri, attr, value)

    def _send(self, data, time, eos):
        if self.__sriModified:
            self._port.pushSRI(self._sri)
            self.__sriModified = False
        self._pushPacket(self._port, data, time, eos, self.streamID)

    def _pushPacket(self, port, data, time, eos, streamID):
        port.pushPacket(data, time, eos, streamID)

    def _modifyingStreamMetadata(self):
        self.__sriModified = True


class BufferedOutputStream(OutputStream):
    def __init__(self, sri, port, dtype=list):
        OutputStream.__init__(self, sri, port, dtype)
        self.__buffer = self._dtype()
        self.__bufferSize = 0
        self.__bufferTime = bulkio.timestamp.notSet()

    def write(self, data, time):
        # If buffering is disabled, or the buffer is empty and the input data
        # is large enough for a full buffer, send it immediately
        if self.__bufferSize == 0 or (not self.__buffer and (len(data) >= self.__bufferSize)):
            OutputStream.write(self, data, time);
        else:
            self._doBuffer(data, time);

    def bufferSize(self):
        return self.__bufferSize

    def setBufferSize(self, size):
        # Avoid needless thrashing
        if size == self.__bufferSize:
            return
        self.__bufferSize = int(size)

        # If the new buffer size is less than (or exactly equal to) the
        # currently buffered data size, flush
        if self.__bufferSize <= len(self.__buffer):
            self.flush()

    def flush(self):
        if not self.__buffer:
            return
        self._flush(False)

    def close(self):
        if self.__buffer:
            # Add the end-of-stream marker to the buffered data and its
            # timestamp
            self._flush(True)
        else:
            OutputStream.close(self)

    def _modifyingStreamMetadata(self):
        # Flush any data queued with the old SRI
        self.flush()

        # Post-extend base class method
        OutputStream._modifyingStreamMetadata(self)

    def _flush(self, eos):
        self._send(self.__buffer, self.__bufferTime, eos)
        self.__buffer = self._dtype()

    def _doBuffer(self, data, time):
        # If this is the first data being queued, use its timestamp for the
        # start time of the buffered data
        if not self.__buffer:
            self.__bufferTime = copy.copy(time)

        # Only buffer up to the currently configured buffer size
        count = min(len(data), self.__bufferSize - len(self.__buffer));
        self.__buffer += data[:count]

        # Flush if the buffer is full
        if len(self.__buffer) >= self.__bufferSize:
            self._flush(False)

        # Handle remaining data
        if count < len(data):
            next = time + self.xdelta * count
            self._doBuffer(data[count:], next)

def _unpack_complex(data, dtype):
    for item in data:
        yield dtype(item.real)
        yield dtype(item.imag)
    
def _complex_to_interleaved(data, dtype):
    return list(_unpack_complex(data, dtype))

class NumericOutputStream(BufferedOutputStream):
    def __init__(self, sri, port, dtype, elemType):
        BufferedOutputStream.__init__(self, sri, port, dtype)
        self._elemType = elemType

    def write(self, data, time, formatted=False):
        if not formatted:
            if self.complex:
                data = _complex_to_interleaved(data, self._elemType)
            data = self._port._reformat(data)
        BufferedOutputStream.write(self, data, time)

class OutXMLStream(OutputStream):
    def __init__(self, sri, port):
        OutputStream.__init__(self, sri, port, str)

    def write(self, data):
        OutputStream.write(self, data, None)

    def _pushPacket(self, port, data, time, eos, streamID):
        port.pushPacket(data, eos, streamID)
