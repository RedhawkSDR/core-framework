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
import unittest
import ossie.utils.testing
import os
from omniORB import any
import time
from ossie.utils.testing.unit_test_helpers import getImplId
from ossie.utils import sb

# Add the local search paths to find local IDL files
from ossie.utils import model
from ossie.utils.idllib import IDLLibrary
model._idllib = IDLLibrary()
model._idllib.addSearchPath('../../../../../idl')
model._idllib.addSearchPath('/usr/local/redhawk/core/share/idl')

PAUSE = .1
MAX_WAIT_TIME = 10
EXPECTED_EOS = True

class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in TestLargePush"""

    def checkData(
            self,
            sink,
            isLong=False,
            isOctet=False,
            pause=PAUSE,
            expectedEOS=EXPECTED_EOS,
            maxWaitTime=MAX_WAIT_TIME):

        self.comp.start()
        sink.start()
        if isLong:
            expectedOutput = [long(0) for x in range(self.comp.numSamples)]
        elif isOctet:
            expectedOutput = ['\x00' for x in range(self.comp.numSamples)]
        else:
            expectedOutput = [0 for x in range(self.comp.numSamples)]

        waitCountdown = maxWaitTime
        while not sink.eos():
            time.sleep(pause)
            waitCountdown -= pause
            if waitCountdown <= 0:
                self.fail("Failed to receive an EOS")
        retData = sink.getData()

        # If this assertion fails, make sure the length of the vectors match.
        # if they do not, try increasing the value of pause
        self.assertEquals(retData, expectedOutput)
        self.comp.stop()

    def test_push(self):
        # Launch the resource with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",),
                                         modes=("readwrite", "writeonly"),
                                         includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)

        sink = sb.DataSink()

        class Connection:
            def __init__(self, uses, provides):
                self.uses = uses
                self.provides = provides

        # octet test is diabled until python test component supports it
        connections = (
            Connection(uses = "dataFloat",       provides = "floatIn"),
            Connection(uses = "dataDouble",      provides = "doubleIn"),
            Connection(uses = "dataUshort",      provides = "ushortIn"),
            Connection(uses = "dataShort",       provides = "shortIn"),
            Connection(uses = "dataLong",        provides = "longIn"),
            Connection(uses = "dataUlong",       provides = "ulongIn"),
            Connection(uses = "dataUlongLong",   provides = "ulonglongIn"),
            Connection(uses = "dataLongLong",    provides = "longlongIn"),
         #   Connection(uses = "dataOctet",       provides = "octetIn"),
        )

        for connection in connections:
            print connection.uses
            if connection.uses.lower().find("long") != -1:
                isLong = True
            else:
                isLong = False
            if connection.uses.lower().find("octet") != -1:
                isOctet = True
            else:
                isOctet = False
            self.comp.connect(
                sink,
                usesPortName=connection.uses,
                providesPortName=connection.provides)
            self.checkData(sink, isLong=isLong, isOctet=isOctet)
            self.comp.disconnect(sink)


if __name__ == "__main__":
    ossie.utils.testing.main("../TestLargePush.spd.xml") # By default tests all implementations
