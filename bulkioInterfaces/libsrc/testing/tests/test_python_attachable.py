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
from bulkio.bulkioInterfaces import BULKIO
import bulkio
import time
import logging
import threading

class SriListener(object):
    def __init__(self):
        self.sri = None

    def __call__(self, sri):
        self.sri = sri

    def reset(self):
        self.sri = None


class AttachableAPI(unittest.TestCase):
    def __init__( self, methodName='runTest' ):
        unittest.TestCase.__init__(self, methodName)

    def getSddsStreamDef(self, streamId):
        ndef = BULKIO.SDDSStreamDefinition(id=streamId,
                                    dataFormat=BULKIO.SDDS_SB, 
                                    multicastAddress='0.0.0.0', 
                                    vlan=0, 
                                    port=0, 
                                    sampleRate=0,
                                    timeTagValid=False, 
                                    privateInfo='privateInfo')
        return ndef

    def test_stream_container(self):
        streamid="stream1"
        stream_def = self.getSddsStreamDef(streamid)
        scon = bulkio.OutAttachablePort.StreamContainer()
        stream = bulkio.OutAttachablePort.Stream(streamDef=stream_def, name="", streamId=stream_def.id)
        scon.addStream( stream )

        streamid="stream2"
        stream_def = self.getSddsStreamDef(streamid)
        stream = bulkio.OutAttachablePort.Stream(streamDef=stream_def, name="", streamId=stream_def.id)
        scon.addStream( stream )
        logger=logging.getLogger("test1")
        scon.setLogger(logger)

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ AttachableAPI  ] :
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
