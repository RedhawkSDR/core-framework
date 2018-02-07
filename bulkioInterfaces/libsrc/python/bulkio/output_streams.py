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

import bulkio
from bulkio.bulkioInterfaces import BULKIO
from bulkio.stream_base import StreamBase

class OutputStream(StreamBase):
    def __init__(self, sri, port):
        StreamBase.__init__(self, sri)
        self.__port = port
        self.__sriModified = False

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

    def write(self, data, time):
        self._send(data, time, False)

    def close(self):
        data = self._emptyPacket()
        self._send(data, bulkio.timestamp.notSet(), True)

    def _setStreamMetadata(self, attr, value):
        field = getattr(self._sri, attr)
        if field != value:
            self.__sriModified = True
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
        pass

    def _emptyPacket(self):
        return []

class OutCharStream(OutputStream):
    def _emptyPacket(self):
        return str()

class OutOctetStream(OutputStream):
    def _emptyPacket(self):
        return str()

class OutBitStream(OutputStream):
    def _emptyPacket(self):
        return BULKIO.BitSequence('', 0)

class OutXMLStream(OutputStream):
    def _emptyPacket(self):
        return str()

    def _pushPacket(self, port, data, time, eos, streamID):
        port.pushPacket(data, eos, streamID)

class OutFileStream(OutputStream):
    def _emptyPacket(self):
        return str()
