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
import struct

from ossie.utils.log4py import logging
from redhawk.bitbuffer import bitbuffer

import bulkio
from bulkio.bulkioInterfaces import BULKIO, BULKIO__POA

class InPortStub(object):
    Packet = collections.namedtuple('Packet', 'data T EOS streamID')

    def __init__(self):
        self.H = []
        self.packets = []
        self.logger = logging.getLogger(self.__class__.__name__)

    def pushSRI(self, H):
        self.logger.debug("pushSRI '%s'", H.streamID)
        self.H.append(H)

    def pushPacket(self, data, T, EOS, streamID):
        self.logger.debug("pushPacket '%s'", streamID)
        self._queuePacket(InPortStub.Packet(data, T, EOS, streamID))

    def _queuePacket(self, packet):
        self.packets.append(packet)

# XML requires a special override for pushPacket
class InXMLPortStub(BULKIO__POA.dataXML, InPortStub):
    Packet = collections.namedtuple('Packet', 'data EOS streamID')

    def pushPacket(self, data, EOS, streamID):
        self.logger.debug("pushPacket '%s'", streamID)
        self._queuePacket(InXMLPortStub.Packet(data, EOS, streamID))

# Generate stubs for all standard BULKIO interfaces
for name in ('dataChar', 'dataOctet', 'dataBit', 'dataShort', 'dataUshort',
             'dataLong', 'dataUlong', 'dataLongLong', 'dataUlongLong',
             'dataFloat', 'dataDouble', 'dataFile'):
    class_name = 'In%sPortStub' % (name[4:],)
    port_type = getattr(BULKIO__POA, name)
    globals()[class_name] = type(class_name, (port_type, InPortStub), {})


class PortTestHelper(object):
    def pushPacket(self, port, data, time, eos, streamID):
        port.pushPacket(data, time, eos, streamID)

    def createData(self, length):
        """
        Creates a data object suitable for a pushPacket call.
        """
        data = self.createStreamData(length)
        return self.pack(data)

    def createStreamData(self, length):
        """
        Creates a data object suitable for writing to an output stream.
        """
        return [0]*length

    def pack(self, data):
        """
        Converts a Python array into a data object suitable for a pushPacket
        call.
        """
        return data

    def unpack(self, data):
        """
        Converts the payload from a pushPacket into a Python object.
        """
        return data

    def createInPort(self):
        return self.InPortType(self._getName() + '_in')

    def createOutPort(self):
        return self.OutPortType(self._getName() + '_out')

    def createInStub(self):
        return self.InStubType()

    def packetLength(self, data):
        return len(data)

    def _getName(self):
        return self.PortType.__name__

    def getName(self):
        name = self._getName()
        # Remap BULKIO interface names, where unsigned types have a lowercase
        # letter after the U, to the implementations, where it's uppercase
        if name[4] == 'U':
            name = name[:5] + name[5].upper() + name[6:]
        return name[4:]

class CharTestHelper(PortTestHelper):
    PortType = BULKIO.dataChar
    InPortType = bulkio.InCharPort
    OutPortType = bulkio.OutCharPort
    InStubType = InCharPortStub

    BITS_PER_ELEMENT = 8

    def pack(self, data):
        return struct.pack('%db' % len(data), *data)

    def unpack(self, data):
        return list(struct.unpack('%db' % len(data), data))

class OctetTestHelper(PortTestHelper):
    PortType = BULKIO.dataOctet
    InPortType = bulkio.InOctetPort
    OutPortType = bulkio.OutOctetPort
    InStubType = InOctetPortStub

    BITS_PER_ELEMENT = 8

    def pack(self, data):
        return struct.pack('%dB' % len(data), *data)

    def unpack(self, data):
        return list(struct.unpack('%dB' % len(data), data))

class ShortTestHelper(PortTestHelper):
    PortType = BULKIO.dataShort
    InPortType = bulkio.InShortPort
    OutPortType = bulkio.OutShortPort
    InStubType = InShortPortStub

    BITS_PER_ELEMENT = 16

class UShortTestHelper(PortTestHelper):
    PortType = BULKIO.dataUshort
    InPortType = bulkio.InUShortPort
    OutPortType = bulkio.OutUShortPort
    InStubType = InUshortPortStub

    BITS_PER_ELEMENT = 16

class LongTestHelper(PortTestHelper):
    PortType = BULKIO.dataLong
    InPortType = bulkio.InLongPort
    OutPortType = bulkio.OutLongPort
    InStubType = InLongPortStub

    BITS_PER_ELEMENT = 32

class ULongTestHelper(PortTestHelper):
    PortType = BULKIO.dataUlong
    InPortType = bulkio.InULongPort
    OutPortType = bulkio.OutULongPort
    InStubType = InUlongPortStub

    BITS_PER_ELEMENT = 32

class LongLongTestHelper(PortTestHelper):
    PortType = BULKIO.dataLongLong
    InPortType = bulkio.InLongLongPort
    OutPortType = bulkio.OutLongLongPort
    InStubType = InLongLongPortStub

    BITS_PER_ELEMENT = 64

class ULongLongTestHelper(PortTestHelper):
    PortType = BULKIO.dataUlongLong
    InPortType = bulkio.InULongLongPort
    OutPortType = bulkio.OutULongLongPort
    InStubType = InUlongLongPortStub

    BITS_PER_ELEMENT = 64

class FloatTestHelper(PortTestHelper):
    PortType = BULKIO.dataFloat
    InPortType = bulkio.InFloatPort
    OutPortType = bulkio.OutFloatPort
    InStubType = InFloatPortStub

    BITS_PER_ELEMENT = 32

class DoubleTestHelper(PortTestHelper):
    PortType = BULKIO.dataDouble
    InPortType = bulkio.InDoublePort
    OutPortType = bulkio.OutDoublePort
    InStubType = InDoublePortStub

    BITS_PER_ELEMENT = 64

class BitTestHelper(PortTestHelper):
    PortType = BULKIO.dataBit
    InPortType = bulkio.InBitPort
    OutPortType = bulkio.OutBitPort
    InStubType = InBitPortStub

    BITS_PER_ELEMENT = 1

    def createStreamData(self, length):
        return bitbuffer(bits=length)

    def packetLength(self, data):
        return data.bits

    def unpack(self, data):
        return bitbuffer(bytearray(data.data), data.bits)

class FileTestHelper(PortTestHelper):
    PortType = BULKIO.dataFile
    InPortType = bulkio.InFilePort
    OutPortType = bulkio.OutFilePort
    InStubType = InFilePortStub

    BITS_PER_ELEMENT = 8

    def createStreamData(self, length):
        return ' '*length

class XMLTestHelper(PortTestHelper):
    PortType = BULKIO.dataXML
    InPortType = bulkio.InXMLPort
    OutPortType = bulkio.OutXMLPort
    InStubType = InXMLPortStub

    BITS_PER_ELEMENT = 8

    def createStreamData(self, length):
        return ' '*length

    def pushPacket(self, port, data, time, eos, streamID):
        port.pushPacket(data, eos, streamID)
