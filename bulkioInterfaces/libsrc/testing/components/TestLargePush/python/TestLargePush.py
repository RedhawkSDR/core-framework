#!/usr/bin/env python
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
#
#
# AUTO-GENERATED
#
# Source: TestLargePush.spd.xml
from ossie.resource import Resource, start_component
import logging

from TestLargePush_base import *

class TestLargePush_i(TestLargePush_base):
    """<DESCRIPTION GOES HERE>"""
    def initialize(self):
        """
        This is called by the framework immediately after your component registers with the NameService.

        In general, you should add customization here and not in the __init__ constructor.  If you have 
        a custom port implementation you can override the specific implementation here with a statement
        similar to the following:
          self.some_port = MyPortImplementation()
        """
        TestLargePush_base.initialize(self)

    def process(self):
        outData     = [0] * self.numSamples
        T           = bulkio.timestamp.create()
        EOS         = True
        streamID    = "test"

        sri         = bulkio.sri.create(streamID)
        sri.xdelta  = 0.001

        self.port_dataFloat.pushSRI(sri)
        self.port_dataFloat.pushPacket(     outData, T, EOS, streamID)
        self.port_dataDouble.pushSRI(sri)
        self.port_dataDouble.pushPacket(    outData, T, EOS, streamID)
        self.port_dataShort.pushSRI(sri)
        self.port_dataShort.pushPacket(     outData, T, EOS, streamID)
        self.port_dataUshort.pushSRI(sri)
        self.port_dataUshort.pushPacket(    outData, T, EOS, streamID)
        self.port_dataLong.pushSRI(sri)
        self.port_dataLong.pushPacket(      outData, T, EOS, streamID)
        self.port_dataUlong.pushSRI(sri)
        self.port_dataUlong.pushPacket(     outData, T, EOS, streamID)
        self.port_dataLongLong.pushSRI(sri)
        self.port_dataLongLong.pushPacket(  outData, T, EOS, streamID)
        self.port_dataUlongLong.pushSRI(sri)
        self.port_dataUlongLong.pushPacket( outData, T, EOS, streamID)

        return FINISH

if __name__ == '__main__':
    logging.getLogger().setLevel(logging.WARN)
    logging.debug("Starting Component")
    start_component(TestLargePush_i)

