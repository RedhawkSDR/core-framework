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

from ossie.utils.log4py import logging

from bulkio.bulkioInterfaces import BULKIO__POA

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
