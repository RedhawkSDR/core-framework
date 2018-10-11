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

from ossie.utils.log4py import logging

import bulkio

class Test_Python_File(unittest.TestCase):
    def setUp(self):
        self.port = bulkio.InFilePort('dataFile_in')

    def test_queue_flush_flags(self):
        """
        Tests that EOS and sriChanged flags are preserved on a per-stream basis
        when a queue flush occurs.
        """
        self.port.startPort()

        # Push 1 packet for the normal data stream
        sri_data = bulkio.sri.create('stream_data')
        sri_data.blocking = False
        self.port.pushSRI(sri_data)
        self.port.pushPacket('file:///var/tmp/test', bulkio.timestamp.now(), False, sri_data.streamID)

        # Push 1 packet for the EOS test stream
        sri_eos = bulkio.sri.create('stream_eos')
        sri_eos.blocking = False
        self.port.pushSRI(sri_eos)
        self.port.pushPacket('file:///var/tmp/test', bulkio.timestamp.now(), False, sri_eos.streamID)

        # Push 1 packet for the SRI change stream
        sri_change = bulkio.sri.create('stream_sri')
        sri_change.blocking = False
        self.port.pushSRI(sri_change)
        self.port.pushPacket('file:///var/tmp/test', bulkio.timestamp.now(), False, sri_change.streamID)

        # Grab the packets to ensure the initial conditions are correct
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_data.streamID, packet.streamID)

        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_eos.streamID, packet.streamID)

        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_change.streamID, packet.streamID)

        # Push an EOS packet for the EOS stream
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri_eos.streamID)

        # Modify the SRI for the SRI change stream and push another packet
        sri_change.mode = 1
        self.port.pushSRI(sri_change)
        self.port.pushPacket('file:///var/tmp/test', bulkio.timestamp.now(), False, sri_change.streamID)

        # Cause a queue flush by lowering the ceiling and pushing packets
        self.port.setMaxQueueDepth(3)
        self.port.pushPacket('file:///var/tmp/test', bulkio.timestamp.now(), False, sri_data.streamID)
        self.port.pushPacket('file:///var/tmp/test', bulkio.timestamp.now(), False, sri_data.streamID)

        # Push another packet for the SRI change stream
        self.port.pushPacket('file:///var/tmp/test', bulkio.timestamp.now(), False, sri_change.streamID)

        # 1st packet should be for EOS stream, with no data or SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_eos.streamID, packet.streamID)
        self.assertTrue(packet.inputQueueFlushed)
        self.assertTrue(packet.EOS)
        self.assertFalse(packet.sriChanged)
        self.assertEqual(0, len(packet.dataBuffer))

        # 2nd packet should be for data stream, with no EOS or SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_data.streamID, packet.streamID)
        self.assertFalse(packet.inputQueueFlushed)
        self.assertFalse(packet.EOS)
        self.assertFalse(packet.sriChanged)

        # 3rd packet should contain the "lost" SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_change.streamID, packet.streamID)
        self.assertFalse(packet.inputQueueFlushed)
        self.assertFalse(packet.EOS)
        self.assertTrue(packet.sriChanged)

class Test_Python_XML(unittest.TestCase):
    def setUp(self):
        self.port = bulkio.InXMLPort('dataXML_in')

    def test_queue_flush_flags(self):
        """
        Tests that EOS and sriChanged flags are preserved on a per-stream basis
        when a queue flush occurs.
        """
        self.port.startPort()

        # Push 1 packet for the normal data stream
        sri_data = bulkio.sri.create('stream_data')
        sri_data.blocking = False
        self.port.pushSRI(sri_data)
        self.port.pushPacket('<document/>', False, sri_data.streamID)

        # Push 1 packet for the EOS test stream
        sri_eos = bulkio.sri.create('stream_eos')
        sri_eos.blocking = False
        self.port.pushSRI(sri_eos)
        self.port.pushPacket('<document/>', False, sri_eos.streamID)

        # Push 1 packet for the SRI change stream
        sri_change = bulkio.sri.create('stream_sri')
        sri_change.blocking = False
        self.port.pushSRI(sri_change)
        self.port.pushPacket('<document/>', False, sri_change.streamID)

        # Grab the packets to ensure the initial conditions are correct
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_data.streamID, packet.streamID)

        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_eos.streamID, packet.streamID)

        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_change.streamID, packet.streamID)

        # Push an EOS packet for the EOS stream
        self.port.pushPacket([], True, sri_eos.streamID)

        # Modify the SRI for the SRI change stream and push another packet
        sri_change.mode = 1
        self.port.pushSRI(sri_change)
        self.port.pushPacket('<document/>', False, sri_change.streamID)

        # Cause a queue flush by lowering the ceiling and pushing packets
        self.port.setMaxQueueDepth(3)
        self.port.pushPacket('<document/>', False, sri_data.streamID)
        self.port.pushPacket('<document/>', False, sri_data.streamID)

        # Push another packet for the SRI change stream
        self.port.pushPacket('<document/>', False, sri_change.streamID)

        # 1st packet should be for EOS stream, with no data or SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_eos.streamID, packet.streamID)
        self.assertTrue(packet.inputQueueFlushed)
        self.assertTrue(packet.EOS)
        self.assertFalse(packet.sriChanged)
        self.assertEqual(0, len(packet.dataBuffer))

        # 2nd packet should be for data stream, with no EOS or SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_data.streamID, packet.streamID)
        self.assertFalse(packet.inputQueueFlushed)
        self.assertFalse(packet.EOS)
        self.assertFalse(packet.sriChanged)

        # 3rd packet should contain the "lost" SRI change flag
        packet = self.port.getPacket(bulkio.const.NON_BLOCKING)
        self.failIf(packet is None)
        self.assertEqual(sri_change.streamID, packet.streamID)
        self.assertFalse(packet.inputQueueFlushed)
        self.assertFalse(packet.EOS)
        self.assertTrue(packet.sriChanged)

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Python_File, Test_Python_XML ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
