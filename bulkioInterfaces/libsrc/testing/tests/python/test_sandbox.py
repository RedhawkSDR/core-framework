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

import threading
import time
import unittest

from ossie.utils import sb
from redhawk.bitbuffer import bitbuffer

import bulkio
from bulkio.bulkioInterfaces import BULKIO
import bulkio.sandbox

import helpers

def after(delay, func, *args, **kwargs):
    def delayed_func():
        time.sleep(delay)
        func(*args, **kwargs)
    t = threading.Thread(target=delayed_func)
    t.setDaemon(True)
    t.start()

class StreamSourceTest(unittest.TestCase):
    def setUp(self):
        # Create a source and connect it directly to a port stub, bypassing the
        # normal sandbox connection logic
        self.source = bulkio.sandbox.StreamSource()
        self.stub = helpers.InFloatPortStub()
        port = self.source.getPort('floatOut')
        port.connectPort(self.stub._this(), 'test_connection')

    def tearDown(self):
        sb.release()

        try:
            poa = self.stub._default_POA()
            object_id = poa.servant_to_id(self.stub)
            poa.deactivate_object(object_id)
        except:
            # Ignore CORBA exceptions
            pass

    def testBasicWrite(self):
        data = range(16)
        ts = bulkio.timestamp.now()
        self.source.write(data, ts)

        # Check that the stub received a packet that matches
        self.assertEqual(1, len(self.stub.H))
        self.failUnless(bulkio.sri.compare(self.source.sri, self.stub.H[-1]))
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(self.source.streamID, self.stub.packets[-1].streamID)
        self.assertEqual(data, self.stub.packets[-1].data)
        self.assertEqual(ts, self.stub.packets[-1].T)
        self.failIf(self.stub.packets[-1].EOS)

        # Close the stream, which must send an EOS
        self.source.close()
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(self.source.streamID, self.stub.packets[-1].streamID)
        self.assertEqual(0, len(self.stub.packets[-1].data))
        self.failUnless(self.stub.packets[-1].EOS)

    def testStreamID(self):
        # Default: use instance name
        self.assertEqual(self.source._instanceName, self.source.streamID)

        # Override in constructor
        source2 = bulkio.sandbox.StreamSource(streamID='test_stream_id')
        self.assertEqual('test_stream_id', source2.streamID)

        # streamID is immutable
        self.assertRaises(AttributeError, setattr, source2, 'streamID', 'error')

    def testSriMetadata(self):
        # Configure the source's stream metadata
        self.source.xstart = -2.5
        self.source.xdelta = 0.125
        self.source.xunits = BULKIO.UNITS_FREQUENCY
        self.source.subsize = 1024
        self.source.ystart = 2.5
        self.source.ydelta = 1.0
        self.source.yunits = BULKIO.UNITS_TIME
        self.source.complex = True
        self.source.blocking = True
        self.source.setKeyword('COL_RF', 100.0e6)
        self.source.setKeyword('CHAN_RF', 101.1e6)

        # Check that the raw SRI is correct
        sri = self.source.sri
        self.assertEqual(-2.5, sri.xstart)
        self.assertEqual(0.125, sri.xdelta)
        self.assertEqual(BULKIO.UNITS_FREQUENCY, sri.xunits)
        self.assertEqual(1024, sri.subsize)
        self.assertEqual(2.5, sri.ystart)
        self.assertEqual(1.0, sri.ydelta)
        self.assertEqual(BULKIO.UNITS_TIME, sri.yunits)
        self.assertEqual(1, sri.mode)
        self.assertEqual(1, sri.blocking)
        self.assertEqual(100.0e6, bulkio.sri.getKeyword(sri, 'COL_RF'))
        self.assertEqual(101.1e6, bulkio.sri.getKeyword(sri, 'CHAN_RF'))

        # Write to force an SRI push and compare the SRIs
        self.source.write(range(16))
        self.assertEqual(1, len(self.stub.H))
        self.failUnless(bulkio.sri.compare(self.source.sri, self.stub.H[-1]))

        # Modify a few fields and make sure the SRI is updated
        self.source.xdelta = 0.0625
        self.source.blocking = False
        self.source.eraseKeyword('CHAN_RF')
        self.source.write(range(16))
        self.assertEqual(2, len(self.stub.H))
        self.failUnless(bulkio.sri.compare(self.source.sri, self.stub.H[-1]))


class StreamSinkTest(unittest.TestCase):
    def tearDown(self):
        sb.release()

    def testRead(self):
        # Create a sink and get a direct reference to a port, bypassing the
        # normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('floatIn')
        sb.start()

        # Read from empty sink
        sink_data = sink.read(timeout=0.0)
        self.failUnless(sink_data is None)

        # Push directly to the port
        sri = bulkio.sri.create('test_read')
        port.pushSRI(sri)
        data = range(16)
        ts = bulkio.timestamp.now()
        port.pushPacket(range(16), ts, False, sri.streamID)

        # Read the packet we just pushed
        sink_data = sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual(sri.streamID, sink_data.sri.streamID)
        self.assertEqual(1, len(sink_data.sris))
        self.failUnless(bulkio.sri.compare(sri, sink_data.sri))
        self.assertEqual(data, sink_data.data)
        self.assertEqual((0, ts), sink_data.timestamps[0])

    def testReadComplex(self):
        # Create a sink and get a direct reference to a port, bypassing the
        # normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('floatIn')
        sb.start()

        # Push directly to the port
        sri = bulkio.sri.create('test_read_complex')
        sri.mode = 1
        port.pushSRI(sri)
        ts = bulkio.timestamp.now()
        port.pushPacket(range(32), ts, False, sri.streamID)

        # Data should be returned as complex values
        data = sink.read(timeout=1.0)
        self.failIf(data is None)
        expected = [complex(x,x+1) for x in xrange(0,32,2)]
        self.assertEqual(expected, data.data)

    def testReadStream(self):
        # Create a sink and get a direct reference to a port, bypassing the
        # normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('longIn')
        sb.start()

        # Push directly to the port
        sri = bulkio.sri.create('test_read_stream_1')
        port.pushSRI(sri)
        ts = bulkio.timestamp.now()
        port.pushPacket(range(16), ts, False, sri.streamID)

        # Read from a stream that does not have data should fail
        data = sink.read(timeout=0.1, streamID='not here')
        self.failUnless(data is None)

        # Push to a second stream ID
        sri = bulkio.sri.create('test_read_stream_2')
        port.pushSRI(sri)
        ts = bulkio.timestamp.now()
        port.pushPacket(range(16), ts, False, sri.streamID)

        # Read should return data specifically from the given streamID (need to
        # give it a timeout so that the sink's thread has time to queue the
        # packet data)
        data = sink.read(timeout=1.0, streamID=sri.streamID)
        self.failIf(data is None)
        self.assertEqual(sri.streamID, data.sri.streamID)
        self.assertEqual(1, len(data.sris))
        self.failUnless(bulkio.sri.compare(sri, data.sri))
        self.assertEqual(range(16), data.data)
        self.assertEqual((0, ts), data.timestamps[0])

    def testTimeStamps(self):
        # Create a sink and get a direct reference to a port, bypassing the
        # normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('charIn')
        sb.start()

        # Push a bunch of packets and remember the time stamps
        sri = bulkio.sri.create('test_time_stamps')
        port.pushSRI(sri)
        expected = []
        ts = bulkio.timestamp.now()
        port.pushPacket('\x00'*16, ts, False, sri.streamID)
        expected.append((0, ts))
        ts = bulkio.timestamp.now()
        port.pushPacket('\x00'*32, ts, False, sri.streamID)
        expected.append((16, ts))
        ts = bulkio.timestamp.now()
        port.pushPacket('\x00'*16, ts, False, sri.streamID)
        expected.append((48, ts))
        port.pushPacket('', bulkio.timestamp.notSet(), True, sri.streamID)

        # Read all of the data and check the timestamps against what was sent
        data = sink.read(timeout=1.0, eos=True)
        self.failIf(data is None)
        self.assertEqual(3, len(data.timestamps))
        for (exp_off, exp_ts), (act_off, act_ts) in zip(expected, data.timestamps):
            self.assertEqual(exp_off, act_off)
            self.assertEqual(exp_ts, act_ts)

    def testTimeStampsComplex(self):
        # Create a sink and get a direct reference to a port, bypassing the
        # normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('floatIn')
        sb.start()

        # Push a bunch of packets and remember the time stamps
        sri = bulkio.sri.create('test_time_stamps_cx')
        sri.mode = 1
        port.pushSRI(sri)
        expected = []
        ts = bulkio.timestamp.now()
        port.pushPacket([0] * 32, ts, False, sri.streamID)
        expected.append((0, ts))
        ts = bulkio.timestamp.now()
        port.pushPacket([1] * 64, ts, False, sri.streamID)
        # NB: The offset advances by the number of complex values, not the
        # number of scalars
        expected.append((16, ts))
        ts = bulkio.timestamp.now()
        port.pushPacket([22] * 32, ts, False, sri.streamID)
        expected.append((48, ts))
        port.pushPacket([], bulkio.timestamp.notSet(), True, sri.streamID)

        data = sink.read(timeout=1.0, eos=True)
        self.failIf(data is None)
        self.assertEqual(3, len(data.timestamps))
        for (exp_off, exp_ts), (act_off, act_ts) in zip(expected, data.timestamps):
            self.assertEqual(exp_off, act_off)
            self.assertEqual(exp_ts, act_ts)

    def testSriChanges(self):
        # Create a sink and get a direct reference to the float port, bypassing
        # the normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('octetIn')
        sb.start()
        
        # Push some data with an initial SRI
        sri = bulkio.sri.create('test_sri_changes')
        sri.xdelta = 1.0
        port.pushSRI(sri)
        port.pushPacket('\x00', bulkio.timestamp.now(), False, sri.streamID)

        # Modify the SRI and push some more
        sri2 = bulkio.sri.create(sri.streamID)
        sri2.xdelta = 2.0
        port.pushSRI(sri2)
        port.pushPacket('\x01', bulkio.timestamp.now(), False, sri.streamID)
        port.pushPacket('\x02', bulkio.timestamp.now(), False, sri.streamID)
        port.pushPacket('\x03', bulkio.timestamp.now(), False, sri.streamID)

        # One last modification and some data, followed by an EOS
        sri3 = bulkio.sri.create(sri.streamID)
        sri3.xdelta = 3.0
        port.pushSRI(sri3)
        port.pushPacket('\x04', bulkio.timestamp.now(), False, sri.streamID)
        port.pushPacket('\x05', bulkio.timestamp.now(), False, sri.streamID)
        port.pushPacket('', bulkio.timestamp.notSet(), True, sri.streamID)

        # Read all of the data up to the EOS to ensure we get all of the SRIs,
        # then check the SRIs and offsets
        data = sink.read(timeout=1.0, eos=True)
        self.failIf(data is None)
        self.assertEqual(sri.streamID, data.streamID)
        self.assertEqual(3, len(data.sris))
        self.assertEqual(0, data.sris[0].offset)
        self.failUnless(bulkio.sri.compare(sri, data.sris[0].sri))
        self.assertEqual(1, data.sris[1].offset)
        self.failUnless(bulkio.sri.compare(sri2, data.sris[1].sri))
        self.assertEqual(4, data.sris[2].offset)
        self.failUnless(bulkio.sri.compare(sri3, data.sris[2].sri))

    def testWaitEOS(self):
        # Create a sink and get a direct reference to the float port, bypassing
        # the normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('shortIn')
        sb.start()

        # Push directly to the port
        sri = bulkio.sri.create('test_wait_eos')
        port.pushSRI(sri)
        port.pushPacket([0], bulkio.timestamp.now(), False, sri.streamID)

        # Read with eos=True should fail
        sink_data = sink.read(timeout=0.1, eos=True)

        # Push more data and and end-of-stream packet
        port.pushPacket([1,2], bulkio.timestamp.now(), False, sri.streamID)
        port.pushPacket([3,4,5], bulkio.timestamp.now(), False, sri.streamID)
        port.pushPacket([6,7,8,9], bulkio.timestamp.now(), False, sri.streamID)
        port.pushPacket([], bulkio.timestamp.notSet(), True, sri.streamID)

        # Read until end-of-stream should succeed, returning all the data
        # pushed, with EOS set
        sink_data = sink.read(timeout=1.0, eos=True)
        self.failIf(sink_data is None)
        self.failUnless(sink_data.eos)
        self.assertEqual(sri.streamID, sink_data.sri.streamID)
        self.assertEqual(range(10), sink_data.data)

    def testWaitStreamAndEOS(self):
        # Create a sink and get a direct reference to the float port, bypassing
        # the normal sandbox connection logic
        sink = bulkio.sandbox.StreamSink()
        port = sink.getPort('bitIn')
        sb.start()

        # Push directly to the port
        sri = bulkio.sri.create('test_stream_eos_1')
        port.pushSRI(sri)
        port.pushPacket(BULKIO.BitSequence('\x00', 7), bulkio.timestamp.now(), True, sri.streamID)

        # Read with eos=True and a different stream ID should fail
        sink_data = sink.read(timeout=0.1, streamID='other', eos=True)
        self.failUnless(sink_data is None)

        # Push new data and an end-of-stream packet to a second stream
        sri2 = bulkio.sri.create('test_stream_eos_2')
        port.pushSRI(sri2)
        data = bitbuffer('10'*16)
        packet = BULKIO.BitSequence(data[:21].bytes(), 21)
        port.pushPacket(packet, bulkio.timestamp.now(), False, sri2.streamID)
        packet = BULKIO.BitSequence(data[21:].bytes(), len(data)-21)
        port.pushPacket(packet, bulkio.timestamp.now(), False, sri2.streamID)
        port.pushPacket(BULKIO.BitSequence('', 0), bulkio.timestamp.notSet(), True, sri2.streamID)

        # Read until end-of-stream should succeed, returning all the data
        # pushed, with EOS set
        sink_data = sink.read(timeout=1.0, streamID=sri2.streamID, eos=True)
        self.failIf(sink_data is None)
        self.failUnless(sink_data.eos)
        self.assertEqual(sri2.streamID, sink_data.sri.streamID)
        self.assertEqual(data, sink_data.data)

if __name__ == '__main__':
    import runtests
    runtests.main()
