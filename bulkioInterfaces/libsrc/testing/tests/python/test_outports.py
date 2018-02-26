#!/usr/bin/python
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

from ossie.cf import CF

import bulkio
from bulkio.bulkioInterfaces import BULKIO

from helpers import *

class OutPortTest(object):
    def setUp(self):
        self.__servants = []
        self.port = self.helper.createOutPort()
        self.stub = self._createStub()
        self.connectionTable = []

        objref = self.stub._this()
        self.port.connectPort(objref, 'test_connection')

    def tearDown(self):
        try:
            self._disconnectPorts()
        except:
            # Ignore disconnection errors
            pass

        self._releaseServants()

    def _createStub(self):
        stub = self.helper.createInStub()
        self.__servants.append(stub)
        return stub

    def _disconnectPorts(self):
        for connection in self.port._get_connections():
            self.port.disconnectPort(connection.connectionId)

    def _releaseServants(self):
        for servant in self.__servants:
            try:
                poa = servant._default_POA()
                object_id = poa.servant_to_id(servant)
                poa.deactivate_object(object_id)
            except:
                # Ignore CORBA exceptions
                pass
        self.__servants = []

    def testLegacyAPI(self):
        # Test to ensure old API methods and some inadvertently public members
        # still behave as expected (within reason)

        # sriDict member
        self.assertEqual(0, len(self.port.sriDict))

        sri = bulkio.sri.create('test_legacy_api')
        self.port.pushSRI(sri)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri.streamID)
        self.assertEqual(1, len(self.port.sriDict))

        sri2 = bulkio.sri.create('test_legacy_api_2')
        self.port.pushSRI(sri2)
        self._pushTestPacket(1, bulkio.timestamp.now(), False, sri2.streamID)
        self.assertEqual(2, len(self.port.sriDict))

        self._pushTestPacket(0, bulkio.timestamp.notSet(), True, sri.streamID)
        self.assertEqual(1, len(self.port.sriDict))

        # enableStats
        self.port.enableStats(False)

    def testConnections(self):
        # Should start with one connection, to the in port stub
        connections = self.port._get_connections()
        self.assertEqual(1, len(connections))
        self.assertEqual(BULKIO.ACTIVE, self.port._get_state())

        # Should throw an invalid port on a nil
        self.assertRaises(CF.Port.InvalidPort, self.port.connectPort, None, 'connection_nil')

        # Normal connection
        stub2 = self._createStub()
        objref = stub2._this()
        self.port.connectPort(objref, 'connection_2')
        connections = self.port._get_connections()
        self.assertEqual(2, len(connections))

        # Cannot reuse connection ID
        self.assertRaises(CF.Port.OccupiedPort, self.port.connectPort, objref, 'connection_2')

        # Disconnect second connection
        self.port.disconnectPort('connection_2')
        connections = self.port._get_connections()
        self.assertEqual(1, len(connections))

        # Bad connection ID on disconnect
        self.assertRaises(CF.Port.InvalidPort, self.port.disconnectPort, 'connection_bad')

        # Disconnect the default stub; port should go to idle
        self.port.disconnectPort('test_connection')
        connections = self.port._get_connections()
        self.assertEqual(0, len(connections))
        self.assertEqual(BULKIO.IDLE, self.port._get_state())

    def testStatistics(self):
        # Even if there are no active SRIs, there should still be statistics
        # for existing connections
        uses_stats = self.port._get_statistics()
        self.assertEqual(1, len(uses_stats))
        self.assertEqual('test_connection', uses_stats[0].connectionId)

        # Push a packet of data to trigger meaningful statistics
        sri = bulkio.sri.create("port_stats")
        self.port.pushSRI(sri)
        self._pushTestPacket(1024, bulkio.timestamp.now(), False, sri.streamID);
        uses_stats = self.port._get_statistics()
        self.assertEqual(1, len(uses_stats))

        # Check that the statistics report the right element size
        stats = uses_stats[0].statistics
        self.failUnless(stats.elementsPerSecond > 0.0)
        bits_per_element = int(round(stats.bitsPerSecond / stats.elementsPerSecond))
        self.assertEqual(self.helper.BITS_PER_ELEMENT, bits_per_element)

        # Test that statistics are returned for all connections
        stub2 = self._createStub()
        self.port.connectPort(stub2._this(), 'connection_2')
        uses_stats = self.port._get_statistics()
        self.assertEqual(2, len(uses_stats))

    def testMultiOut(self):
        stub2 = self._createStub()
        self.port.connectPort(stub2._this(), 'connection_2')

        # Set up a connection table that only routes the filtered stream to the
        # second stub, and another stream to both connections
        filter_stream_id = 'filter_stream'
        self._addStreamFilter(filter_stream_id, 'connection_2')
        all_stream_id = 'all_stream'
        self._addStreamFilter(all_stream_id, 'test_connection')
        self._addStreamFilter(all_stream_id, 'connection_2')

        # Push an SRI for the filtered stream; it should only be received by
        # the second stub
        sri = bulkio.sri.create(filter_stream_id, 2.5e6)
        self.port.pushSRI(sri)
        self.assertEqual(0, len(self.stub.H))
        self.assertEqual(1, len(stub2.H))
        self.assertEqual(filter_stream_id, stub2.H[-1].streamID)

        # Push a packet for the filtered stream; again, only received by #2
        self._pushTestPacket(91, bulkio.timestamp.now(), False, filter_stream_id)
        self.assertEqual(0, len(self.stub.packets))
        self.assertEqual(1, len(stub2.packets))
        self.assertEqual(91, self.helper.packetLength(stub2.packets[-1].data))

        # Unknown (to the connection filter) stream should get dropped
        unknown_stream_id = 'unknown_stream'
        sri = bulkio.sri.create(unknown_stream_id)
        self.port.pushSRI(sri)
        self.assertEqual(0, len(self.stub.H))
        self.assertEqual(1, len(stub2.H))
        self._pushTestPacket(50, bulkio.timestamp.now(), False, unknown_stream_id)
        self.assertEqual(0, len(self.stub.packets))
        self.assertEqual(1, len(stub2.packets))

        # Check SRI routed to both connections...
        sri = bulkio.sri.create(all_stream_id, 1e6)
        self.port.pushSRI(sri)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(2, len(stub2.H))
        self.assertEqual(all_stream_id, self.stub.H[-1].streamID)
        self.assertEqual(all_stream_id, stub2.H[-1].streamID)

        # ...and data
        self._pushTestPacket(256, bulkio.timestamp.now(), False, all_stream_id)
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(256, self.helper.packetLength(self.stub.packets[-1].data))
        self.assertEqual(2, len(stub2.packets))
        self.assertEqual(256, self.helper.packetLength(stub2.packets[-1].data))

        # Reset the connection filter and push data for the filtered stream again,
        # which should trigger an SRI push to the first stub
        self.connectionTable = []
        self.port.updateConnectionFilter(self.connectionTable)
        self._pushTestPacket(9, bulkio.timestamp.now(), False, filter_stream_id)
        self.assertEqual(2, len(self.stub.H))
        self.assertEqual(filter_stream_id, self.stub.H[-1].streamID)
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(9, self.helper.packetLength(self.stub.packets[-1].data))
        self.assertEqual(3, len(stub2.packets))
        self.assertEqual(9, self.helper.packetLength(stub2.packets[-1].data))

    def _addStreamFilter(self, streamId, connectionId):
        desc = bulkio.connection_descriptor_struct(connectionId, streamId, self.port.name)
        self.connectionTable.append(desc)
        self.port.updateConnectionFilter(self.connectionTable)

    def _pushTestPacket(self, length, time, eos, streamID):
        data = self.helper.createData(length)
        self.helper.pushPacket(self.port, data, time, eos, streamID)

class ChunkingOutPortTest(OutPortTest):
    def testPushChunking(self):
        # Set up a basic stream
        stream_id = 'push_chunking'
        sri = bulkio.sri.create(stream_id)
        sri.xdelta = 0.125
        self.port.pushSRI(sri)

        # Test that the push is properly chunked
        time = bulkio.timestamp.create(0.0, 0.0)
        self._testPushOversizedPacket(time, False, stream_id)

        # Check that the synthesized time stamp(s) advanced by the expected
        # time
        last = self.stub.packets[0]
        for packet in self.stub.packets[1:]:
            expected = self.helper.packetLength(last.data) * sri.xdelta
            elapsed = packet.T - last.T
            self.assertEqual(expected, elapsed, 'Incorrect time stamp delta')
            last = packet

    def testPushChunkingEOS(self):
        # Set up a basic stream
        stream_id = 'push_chunking_eos'
        sri = bulkio.sri.create(stream_id)
        self.port.pushSRI(sri)

        # Send a packet with end-of-stream set
        self._testPushOversizedPacket(bulkio.timestamp.notSet(), True, stream_id)

        # Check that only the final packet has end-of-stream set
        self.failUnless(self.stub.packets[-1].EOS, 'Last packet does not have EOS set')
        for packet in self.stub.packets[:-1]:
            self.failIf(packet.EOS, 'Intermediate packet has EOS set')

    def testPushChunkingSubsize(self):
        # Set up a 2-dimensional stream
        stream_id = 'push_chunking_subsize'
        sri = bulkio.sri.create(stream_id)
        sri.subsize = 1023
        self.port.pushSRI(sri)

        self._testPushOversizedPacket(bulkio.timestamp.notSet(), False, stream_id)

        # Check that each packet is a multiple of the subsize (except the last,
        # because the oversized packet was not explicitly quantized to be an
        # exact multiple)
        for packet in self.stub.packets[:-1]:
            self.assertEqual(0, self.helper.packetLength(packet.data) % 1023, 'Packet size is not a multiple of subsize')

    def _testPushOversizedPacket(self, time, eos, streamID):
        # Pick a sufficiently large number of samples that the packet has to
        # span multiple packets
        max_bits = 8 * bulkio.const.MAX_TRANSFER_BYTES
        bits_per_element = self.helper.BITS_PER_ELEMENT
        count = 2 * max_bits / bits_per_element
        self._pushTestPacket(count, time, eos, streamID)

        # More than one packet must have been received, and no packet can
        # exceed the max transfer size
        self.failUnless(len(self.stub.packets) > 1)
        for packet in self.stub.packets:
            packet_bits = self.helper.packetLength(packet.data) * bits_per_element
            self.failUnless(packet_bits < max_bits, 'Packet too large')

class NumericOutPortTest(ChunkingOutPortTest):
    def testPushChunkingComplex(self):
        # Set up a complex stream
        stream_id = 'push_chunking_complex'
        sri = bulkio.sri.create(stream_id)
        sri.mode = 1
        sri.xdelta = 0.0625
        self.port.pushSRI(sri)

        # Test that the push is properly chunked
        time = bulkio.timestamp.create(0.0, 0.0)
        self._testPushOversizedPacket(time, False, stream_id)

        # Check that each packet contains an even number of samples (i.e., no
        # complex value was split)
        for packet in self.stub.packets:
            self.assertEqual(0, self.helper.packetLength(packet.data) % 2, 'Packet contains a partial complex value')

        # Check that the synthesized time stamp(s) advanced by the expected time
        last = self.stub.packets[0]
        for packet in self.stub.packets[1:]:
            expected = self.helper.packetLength(last.data) * 0.5 * sri.xdelta
            elapsed = packet.T - last.T
            self.assertEqual(expected, elapsed, 'Incorrect time stamp delta')
            last = packet

    def testPushChunkingSubsizeComplex(self):
        # Set up a 2-dimensional complex stream
        stream_id = 'push_chunking_subsize_complex'
        sri = bulkio.sri.create(stream_id)
        sri.subsize = 2048
        sri.mode = 1
        self.port.pushSRI(sri)

        self._testPushOversizedPacket(bulkio.timestamp.notSet(), False, stream_id)

        # Check that each packet is a multiple of the subsize (except the last,
        # because the oversized packet was not explicitly quantized to be an exact
        # multiple)
        for packet in self.stub.packets[:-1]:
            self.assertEqual(0, self.helper.packetLength(packet.data) % 4096, 'Packet size is not a multiple of subsize')


def register_test(name, testbase, **kwargs):
    globals()[name] = type(name, (testbase, unittest.TestCase), kwargs)

register_test('OutBitPortTest', ChunkingOutPortTest, helper=BitTestHelper())
register_test('OutXMLPortTest', OutPortTest, helper=XMLTestHelper())
register_test('OutFilePortTest', OutPortTest, helper=FileTestHelper())
register_test('OutCharPortTest', NumericOutPortTest, helper=CharTestHelper())
register_test('OutOctetPortTest', NumericOutPortTest, helper=OctetTestHelper())
register_test('OutShortPortTest', NumericOutPortTest, helper=ShortTestHelper())
register_test('OutUShortPortTest', NumericOutPortTest, helper=UShortTestHelper())
register_test('OutLongPortTest', NumericOutPortTest, helper=LongTestHelper())
register_test('OutULongPortTest', NumericOutPortTest, helper=ULongTestHelper())
register_test('OutLongLongPortTest', NumericOutPortTest, helper=LongLongTestHelper())
register_test('OutULongLongPortTest', NumericOutPortTest, helper=ULongLongTestHelper())
register_test('OutFloatPortTest', NumericOutPortTest, helper=FloatTestHelper())
register_test('OutDoublePortTest', NumericOutPortTest, helper=DoubleTestHelper())

if __name__ == '__main__':
    import runtests
    runtests.main()
