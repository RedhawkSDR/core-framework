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

import omniORB.any

from ossie.cf import CF
from redhawk.bitbuffer import bitbuffer

import bulkio
from bulkio.bulkioInterfaces import BULKIO
from bulkio.stream_base import StreamBase

class OutputStream(StreamBase):
    def __init__(self, sri, port, dtype=list):
        StreamBase.__init__(self, sri)
        self.__port = port
        self.__sriModified = False
        self._dtype = dtype

    @property
    def sri(self):
        return StreamBase.sri.fget(self)

    @sri.setter
    def sri(self, sri):
        if not isinstance(sri, BULKIO.StreamSRI):
            raise TypeError('sri must be a BULKIO.StreamSRI') 
        self._modifyingStreamMetadata()
        self._sri = copy.deepcopy(sri)

    @property
    def xstart(self):
        return StreamBase.xstart.fget(self)

    @xstart.setter
    def xstart(self, xstart):
        self._setStreamMetadata('xstart', float(xstart))

    @property
    def xdelta(self):
        return StreamBase.xdelta.fget(self)

    @xdelta.setter
    def xdelta(self, xdelta):
        self._setStreamMetadata('xdelta', float(xdelta))

    @property
    def xunits(self):
        return StreamBase.xunits.fget(self)

    @xunits.setter
    def xunits(self, xunits):
        self._setStreamMetadata('xunits', int(xunits))

    @property
    def subsize(self):
        return StreamBase.subsize.fget(self)

    @subsize.setter
    def subsize(self, subsize):
        self._setStreamMetadata('subsize', int(subsize))

    @property
    def ystart(self):
        return StreamBase.ystart.fget(self)

    @ystart.setter
    def ystart(self, ystart):
        self._setStreamMetadata('ystart', float(ystart))

    @property
    def ydelta(self):
        return StreamBase.ydelta.fget(self)

    @ydelta.setter
    def ydelta(self, ydelta):
        self._setStreamMetadata('ydelta', float(ydelta))

    @property
    def yunits(self):
        return StreamBase.yunits.fget(self)

    @yunits.setter
    def yunits(self, yunits):
        self._setStreamMetadata('yunits', int(yunits))

    @property
    def complex(self):
        return StreamBase.complex.fget(self)

    @complex.setter
    def complex(self, mode):
        self._setStreamMetadata('mode', 1 if mode else 0)

    @property
    def blocking(self):
        return StreamBase.blocking.fget(self)

    @blocking.setter
    def blocking(self, blocking):
        self._setStreamMetadata('blocking', 1 if blocking else 0)

    @property
    def keywords(self):
        return StreamBase.keywords.fget(self)

    @keywords.setter
    def keywords(self, keywords):
        self._modifyingStreamMetadata()
        # Copy the sequence, but not the values
        self._sri.keywords = keywords[:]

    def setKeyword(self, name, value):
        self._modifyingStreamMetadata()
        for dt in self._sri.keywords:
            if dt.id == name:
                dt.value = omniORB.any.to_any(value)
                return
        self._sri.keywords.append(CF.DataType(name, omniORB.any.to_any(value)))

    def eraseKeyword(self, name):
        for index in xrange(len(self._sri.keywords)):
            if self._sri.keywords[index].id == name:
                self._modifyingStreamMetadata();
                del self._sri.keywords[index]
                return

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
            self.__port.pushSRI(self._sri)
            self.__sriModified = False
        self._pushPacket(self.__port, data, time, eos, self.streamID)

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


class OutXMLStream(OutputStream):
    def _pushPacket(self, port, data, time, eos, streamID):
        port.pushPacket(data, eos, streamID)
