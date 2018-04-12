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

from omniORB import CORBA

from ossie.utils.sandbox import LocalSandbox
from redhawk.bitbuffer import bitbuffer

import bulkio
from bulkio.bulkioInterfaces import BULKIO
from bulkio.sandbox import StreamSource, StreamSink

import helpers

def after(delay, func, *args, **kwargs):
    def delayed_func():
        time.sleep(delay)
        func(*args, **kwargs)
    t = threading.Thread(target=delayed_func)
    t.setDaemon(True)
    t.start()

class format(object):
    """
    Decorator to declare the data format for an individual unit test. The
    TestCase can use this format in setUp to perform connection logic.
    """
    def __init__(self, format):
        self.format = format

    def __call__(self, obj):
        obj.format = self.format
        return obj

bithelper = helpers.BitTestHelper()

class StreamSourceTest(unittest.TestCase):
    def setUp(self):
        self.sandbox = LocalSandbox()
        self.source = StreamSource(sandbox=self.sandbox)

        # Connect source directly to a port stub, bypassing the normal sandbox
        # connection logic
        format = self._getTestFormat()
        if not format:
            format = 'float'
        self.stub = self._createStub(format)
        port = self.source.getPort(format+'Out')
        port.connectPort(self.stub._this(), 'test_connection')

    def tearDown(self):
        self.sandbox.shutdown()

        try:
            poa = self.stub._default_POA()
            object_id = poa.servant_to_id(self.stub)
            poa.deactivate_object(object_id)
        except:
            # Ignore CORBA exceptions
            pass

    def _getTestFormat(self):
        method = getattr(self, self._testMethodName, None)
        if not method:
            return None
        return getattr(method, 'format', None)

    def _createStub(self, format):
        stubs = {
            'char'     : helpers.InCharPortStub,
            'octet'    : helpers.InOctetPortStub,
            'short'    : helpers.InShortPortStub,
            'ushort'   : helpers.InUshortPortStub,
            'long'     : helpers.InLongPortStub,
            'ulong'    : helpers.InUlongPortStub,
            'longlong' : helpers.InLongLongPortStub,
            'ulonglong': helpers.InUlongLongPortStub,
            'float'    : helpers.InFloatPortStub,
            'double'   : helpers.InDoublePortStub,
            'bit'      : helpers.InBitPortStub,
            'xml'      : helpers.InXMLPortStub,
            'file'     : helpers.InFilePortStub
        }
        return stubs[format]()

    @format('ulonglong')
    def testWrite(self):
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

    @format('long')
    def testWriteTimestamp(self):
        # Explicit timestamp
        data = range(16)
        ts = bulkio.timestamp.create(10000.0, 0.75)
        self.source.write(data, ts)

        # Check that the stub received the timestamp given
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(ts, self.stub.packets[-1].T)

        # No timestamp given; should use current time (giving a fair amount of
        # leeway here)
        ts = bulkio.timestamp.now()
        self.source.write(data)
        self.assertEqual(2, len(self.stub.packets))
        self.failIf(abs(self.stub.packets[-1].T - ts) > 1.0)

    @format('double')
    def testWriteComplex(self):
        # Write a 40-element complex list where the interleaved values are a
        # ramp
        self.source.complex = True
        data = [complex(ii,ii+1) for ii in xrange(0, 80, 2)]
        self.source.write(data)

        # Check that the stub received a packet that matches (i.e., data is an
        # 80-element ramp)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, self.stub.H[-1].mode)
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(self.source.streamID, self.stub.packets[-1].streamID)
        self.assertEqual(range(80), self.stub.packets[-1].data)
        self.failIf(self.stub.packets[-1].EOS)

        # Write a 20-element scalar list, which will be interpreted as 20
        # complex values with 0 for the imaginary portion
        self.source.write(range(20))
        # To generate the expected output, walk through twice the range,
        # turning odd numbers into zeros and dividing even numbers in half
        # [ 0, 1, 2, 3, 4, 5...] => [ 0, 0, 1, 0, 2, 0...]
        expected = [ii/2 if ii % 2 == 0 else 0 for ii in xrange(0, 40)]
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(expected, self.stub.packets[-1].data)

        # Write pre-interleaved data
        data = range(36)
        self.source.write(data, interleaved=True)
        self.assertEqual(3, len(self.stub.packets))
        self.assertEqual(data, self.stub.packets[-1].data)
        

    @format('ushort')
    def testWriteFramed(self):
        # Write a list of 4 ramps
        self.source.subsize = 16
        data = [range(x,x+16) for x in xrange(0,64,16)]
        self.source.write(data)

        # Check that the stub received a packet that matches (i.e., data is an
        # 64-element ramp)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(16, self.stub.H[-1].subsize)
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(range(64), self.stub.packets[-1].data)

    @format('double')
    def testWriteFramedComplex(self):
        # Create a set of complex values where each alternating real/imaginary
        # value forms a ramp, and reframe it a a list of 4-item lists
        data = [complex(x,x+1) for x in xrange(0, 32, 2)]
        data = [data[x:x+4] for x in xrange(0, len(data), 4)]
        self.source.complex = True
        self.source.subsize = 4
        self.source.write(data)

        # Check that the stub received a packet that matches (i.e., data is an
        # 32-element ramp)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, self.stub.H[-1].mode)
        self.assertEqual(4, self.stub.H[-1].subsize)
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(range(32), self.stub.packets[-1].data)

        # Write a 4 frames of 4-element real lists; each element will be
        # interpreted as a complex value with 0 for the imaginary portion
        data = range(16)
        data = [data[x:x+4] for x in xrange(0, len(data), 4)]
        self.source.write(data)
        # To generate the expected output, walk through twice the range,
        # turning odd numbers into zeros and dividing even numbers in half
        # [ 0, 1, 2, 3, 4, 5...] => [ 0, 0, 1, 0, 2, 0...]
        expected = [ii/2 if ii % 2 == 0 else 0 for ii in xrange(0, 32)]
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(expected, self.stub.packets[-1].data)

    @format('ulong')
    def testWriteFramedPreformatted(self):
        # Test that writing a 1-dimensional list for framed data still works as
        # expected
        self.source.subsize = 8
        data = range(48)
        self.source.write(data)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(8, self.stub.H[-1].subsize)
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(data, self.stub.packets[-1].data)

    @format('bit')
    def testWriteBit(self):
        # No timestamp
        data = bitbuffer('101011010100101011010101101111')
        self.source.write(data)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(data, bithelper.unpack(self.stub.packets[-1].data))

        # Provided time should pass through unmodified
        ts = bulkio.timestamp.now()
        data = bitbuffer('10001001001001001')
        self.source.write(data, ts)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(data, bithelper.unpack(self.stub.packets[-1].data))
        self.assertEqual(ts, self.stub.packets[-1].T)

        # Write a string literal and make sure it gets translated
        literal = '101011010100101011010101101111'
        self.source.write(literal)
        self.assertEqual(3, len(self.stub.packets))
        self.assertEqual(literal, bithelper.unpack(self.stub.packets[-1].data))

    @format('bit')
    def testWriteBitFramed(self):
        # Create frames of all 0s and all 1s for a good indicator of framing
        # problems
        zeros = bitbuffer(bits=17)
        zeros[:] = 0
        ones = bitbuffer(bits=17)
        ones[:] = 1

        # Write five frames, alterning all 0s and all 1s
        self.source.subsize = 17
        data = [ones,zeros,ones,zeros,ones]
        self.source.write(data)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(17, self.stub.H[-1].subsize)
        self.assertEqual(1, len(self.stub.packets))
        expected = sum(data, bitbuffer())
        self.assertEqual(expected, bithelper.unpack(self.stub.packets[-1].data))

    @format('file')
    def testWriteFile(self):
        # Write data without a timestamp
        uri1 = 'file:///tmp/file1.dat'
        self.source.write(uri1)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(uri1, self.stub.packets[0].data)

        # Provided time should pass through unmodified
        ts = bulkio.timestamp.now()
        uri2 = 'file:///tmp/file2.dat'
        self.source.write(uri2, ts)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(uri2, self.stub.packets[1].data)
        self.assertEqual(ts, self.stub.packets[-1].T)

    @format('xml')
    def testWriteXML(self):
        # No timestamp needed
        data1 = '<document/>'
        self.source.write(data1)
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(1, len(self.stub.packets))
        self.assertEqual(data1, self.stub.packets[0].data)

        # Provided timestamp should be ignored
        data2 = '<document></document>'
        self.source.write(data2, bulkio.timestamp.now())
        self.assertEqual(1, len(self.stub.H))
        self.assertEqual(2, len(self.stub.packets))
        self.assertEqual(data2, self.stub.packets[1].data)

    @format('octet')
    def testClose(self):
        # Normal write, EOS should be false
        self.source.write([0] * 10)
        self.assertEqual(1, len(self.stub.packets))
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
        source2 = StreamSource(streamID='test_stream_id', sandbox=self.sandbox)
        self.assertEqual('test_stream_id', source2.streamID)

        # streamID is immutable
        self.assertRaises(AttributeError, setattr, source2, 'streamID', 'error')

    @format('short')
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

        # Set an explicitly typed keyword
        self.source.setKeyword('typed', 0.25, 'float')
        self.assertEqual(0.25, bulkio.sri.getKeyword(sri, 'typed'))
        any_value = self.source.sri.keywords[-1].value
        self.assertEqual(CORBA.TC_float, any_value.typecode())

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

    def testPortAccess(self):
        # New source should have no port yet
        source2 = StreamSource(sandbox=self.sandbox)
        self.failUnless(source2.port is None)

        # A connection has already been made for the test fixture's source, so
        # the port must be defined
        self.failIf(self.source.port is None)

        # Use direct access to create another stream
        stream = self.source.port.createStream('test_stream')
        stream.close()

    def testStreamAccess(self):
        # New source should have no stream yet
        source2 = StreamSource(sandbox=self.sandbox)
        self.failUnless(source2.stream is None)

        # A connection has already been made for the test fixture's source, so
        # a new stream should be created on access
        stream = self.source.stream
        self.failIf(stream is None)
        self.assertEqual(self.source.streamID, stream.streamID)

    def testFormat(self):
        source = StreamSource(format='float', sandbox=self.sandbox)
        port = source.getPort('floatOut')
        self.assertRaises(RuntimeError, source.getPort, 'shortOut')

class StreamSinkTest(unittest.TestCase):
    def setUp(self):
        self.sandbox = LocalSandbox()
        self.sink = StreamSink(sandbox=self.sandbox)

        format = self._getTestFormat()
        if not format:
            format = 'float'

        # Get a direct reference to a port, bypassing the normal sandbox
        # connection logic
        self.port = self.sink.getPort(format+'In')

        # StreamSink has to be started to return data, because the underlying
        # BulkIO ports require it
        self.sandbox.start()

    def tearDown(self):
        self.sandbox.shutdown()

    def _getTestFormat(self):
        method = getattr(self, self._testMethodName, None)
        if not method:
            return None
        return getattr(method, 'format', None)

    @format('ulong')
    def testRead(self):
        # Read from empty sink
        sink_data = self.sink.read(timeout=0.0)
        self.failUnless(sink_data is None)

        # Push directly to the port
        sri = bulkio.sri.create('test_read')
        self.port.pushSRI(sri)
        data = range(16)
        ts = bulkio.timestamp.now()
        self.port.pushPacket(range(16), ts, False, sri.streamID)

        # Read the packet we just pushed
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual(sri.streamID, sink_data.sri.streamID)
        self.assertEqual(1, len(sink_data.sris))
        self.failUnless(bulkio.sri.compare(sri, sink_data.sri))
        self.assertEqual(data, sink_data.data)
        self.assertEqual((0, ts), sink_data.timestamps[0])

    @format('float')
    def testReadComplex(self):
        # Push directly to the port
        sri = bulkio.sri.create('test_read_complex')
        sri.mode = 1
        self.port.pushSRI(sri)
        ts = bulkio.timestamp.now()
        self.port.pushPacket(range(32), ts, False, sri.streamID)

        # Data should be returned as complex values
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        expected = [complex(x,x+1) for x in xrange(0,32,2)]
        self.assertEqual(expected, data.data)

    @format('ushort')
    def testReadFramed(self):
        # Push four frames directly to the port
        sri = bulkio.sri.create('test_read_framed')
        sri.subsize = 16
        self.port.pushSRI(sri)
        ts1 = bulkio.timestamp.now()
        self.port.pushPacket(range(64), ts1, False, sri.streamID)

        # Data should be returned as a 4-item list of lists
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        self.assertEqual(4, len(data.data))
        expected = [range(x, x+16) for x in xrange(0, 64, 16)]
        self.assertEqual(expected, data.data)

        # Push another four frames across two packets
        ts2 = bulkio.timestamp.now()
        self.port.pushPacket(range(32,64), ts2, False, sri.streamID)
        ts3 = bulkio.timestamp.now()
        self.port.pushPacket(range(64,96), ts3, False, sri.streamID)

        # Data should be returned as a 4-item list of lists
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        self.assertEqual(4, len(data.data))
        expected = [range(x, x+16) for x in xrange(32, 96, 16)]
        self.assertEqual(expected, data.data)

        # Time stamp offsets should be frame-based
        self.assertEqual(2, len(data.timestamps))
        self.assertEqual(0, data.timestamps[0].offset)
        self.assertEqual(ts2, data.timestamps[0].time)
        self.assertEqual(2, data.timestamps[1].offset)
        self.assertEqual(ts3, data.timestamps[1].time)

    @format('double')
    def testReadFramedComplex(self):
        # Push a single frame directly to the port
        sri = bulkio.sri.create('test_read_framed_cx')
        sri.mode = 1
        sri.subsize = 10
        self.port.pushSRI(sri)
        ts1 = bulkio.timestamp.now()
        self.port.pushPacket(range(20), ts1, False, sri.streamID)

        # Data should be returned as a 1-item list of lists of complex values
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        self.assertEqual(1, len(data.data))
        expected = [[complex(x,x+1) for x in xrange(0, 20,2)]]
        self.assertEqual(expected, data.data)

        # Push three frames across two packets
        ts2 = bulkio.timestamp.now()
        self.port.pushPacket(range(40), ts2, False, sri.streamID)
        ts3 = bulkio.timestamp.now()
        self.port.pushPacket(range(40,60), ts3, False, sri.streamID)

        # Data should come back as a 3-item list
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        self.assertEqual(3, len(data.data))
        expected = [[complex(x,x+1) for x in xrange(y, y+20,2)] for y in xrange(0, 60, 20)]
        self.assertEqual(expected, data.data)

        # Time stamp offsets should be frame-based
        self.assertEqual(2, len(data.timestamps))
        self.assertEqual(0, data.timestamps[0].offset)
        self.assertEqual(ts2, data.timestamps[0].time)
        self.assertEqual(2, data.timestamps[1].offset)
        self.assertEqual(ts3, data.timestamps[1].time)

    @format('bit')
    def testReadBit(self):
        # Push a bit sequence directly to the port
        sri = bulkio.sri.create('test_read_bit')
        self.port.pushSRI(sri)
        data1 = bitbuffer('10101101010101')
        ts1 = bulkio.timestamp.now()
        self.port.pushPacket(self._formatBitPacket(data1), ts1, False, sri.streamID)

        # Read should return the equivalent bitbuffer
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual(data1, sink_data.data)
        self.assertEqual(1, len(sink_data.timestamps))
        self.assertEqual(0, sink_data.timestamps[0].offset)
        self.assertEqual(ts1, sink_data.timestamps[0].time)

        # Push a couple more strings of bits with new timestamps
        ts1 = bulkio.timestamp.now()
        self.port.pushPacket(self._formatBitPacket(data1), ts1, False, sri.streamID)
        data2 = bitbuffer('1010010101011')
        ts2 = bulkio.timestamp.now()
        self.port.pushPacket(self._formatBitPacket(data2), ts2, False, sri.streamID)
        data3 = bitbuffer('1000101011100')
        ts3 = bulkio.timestamp.now()
        self.port.pushPacket(self._formatBitPacket(data3), ts3, False, sri.streamID)

        # Read should merge all of the bits into a single bitbuffer
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual(data1+data2+data3, sink_data.data)
        self.assertEqual(3, len(sink_data.timestamps))

        # Check each timestamp's offset
        expected_timestamps = [(0, ts1), (len(data1), ts2), (len(data1)+len(data2), ts3)]
        for (exp_offset, exp_ts), actual_ts in zip(expected_timestamps, sink_data.timestamps):
            self.assertEqual(exp_offset, actual_ts.offset)
            self.assertEqual(exp_ts, actual_ts.time)

    def _formatBitPacket(self, bitdata):
        return BULKIO.BitSequence(bitdata.bytes(), len(bitdata))

    @format('bit')
    def testReadBitFramed(self):
        sri = bulkio.sri.create('test_read_bit_framed')
        sri.subsize = 25
        self.port.pushSRI(sri)

        # Create frames of all 0s and all 1s for a good indicator of framing
        # problems
        zeros = bitbuffer(bits=sri.subsize)
        zeros[:] = 0
        ones = bitbuffer(bits=sri.subsize)
        ones[:] = 1

        # Push 4 frames in one packet
        ts1 = bulkio.timestamp.now()
        self.port.pushPacket(self._formatBitPacket(zeros+ones+zeros+ones), ts1, False, sri.streamID)

        # Data should be returned as a 4-item list of bitbuffers
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual(4, len(sink_data.data))
        self.assertEqual([zeros,ones,zeros,ones], sink_data.data)

        # Push 6 frames across two packets
        ts2 = bulkio.timestamp.now()
        self.port.pushPacket(self._formatBitPacket(zeros+ones+zeros), ts2, False, sri.streamID)
        ts3 = bulkio.timestamp.now()
        self.port.pushPacket(self._formatBitPacket(ones+zeros+ones), ts3, False, sri.streamID)

        # Data should be returned as a 6-item list of bitbuffers
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual(6, len(sink_data.data))
        self.assertEqual([zeros,ones,zeros,ones,zeros,ones], sink_data.data)

        # Time stamp offsets should be frame-based
        self.assertEqual(2, len(sink_data.timestamps))
        self.assertEqual(0, sink_data.timestamps[0].offset)
        self.assertEqual(ts2, sink_data.timestamps[0].time)
        self.assertEqual(3, sink_data.timestamps[1].offset)
        self.assertEqual(ts3, sink_data.timestamps[1].time)

    @format('file')
    def testReadFile(self):
        # Push a file URI directly to the port
        sri = bulkio.sri.create('test_read_file')
        self.port.pushSRI(sri)
        uri1 = 'file:///tmp/file1.dat'
        ts1 = bulkio.timestamp.now()
        self.port.pushPacket(uri1, ts1, False, sri.streamID)

        # Read should return a list of URIs with 1 timestamp per URI
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual([uri1], sink_data.data)
        self.assertEqual(1, len(sink_data.timestamps))
        self.assertEqual(0, sink_data.timestamps[0].offset)
        self.assertEqual(ts1, sink_data.timestamps[0].time)

        # Push a couple more URIs with new timestamps
        ts1 = bulkio.timestamp.now()
        self.port.pushPacket(uri1, ts1, False, sri.streamID)
        uri2 = 'file:///tmp/file2.dat'
        ts2 = bulkio.timestamp.now()
        self.port.pushPacket(uri2, ts2, False, sri.streamID)
        uri3 = 'file:///tmp/file3.dat'
        ts3 = bulkio.timestamp.now()
        self.port.pushPacket(uri3, ts3, False, sri.streamID)

        # Again, read should return a list of URIs with 1 timestamp per URI
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual([uri1, uri2, uri3], sink_data.data)
        self.assertEqual(3, len(sink_data.timestamps))

        # Check each timestamp; offset should increase by 1 each time
        for (exp_offset, exp_ts), actual_ts in zip(enumerate([ts1, ts2, ts3]), sink_data.timestamps):
            self.assertEqual(exp_offset, actual_ts.offset)
            self.assertEqual(exp_ts, actual_ts.time)

    @format('xml')
    def testReadXML(self):
        # Push an XML string directly to the port
        sri = bulkio.sri.create('test_read_xml')
        self.port.pushSRI(sri)
        data1 = '<document/>'
        self.port.pushPacket(data1, False, sri.streamID)

        # Read should return a list of complete XML strings, and there should
        # be no timestamps; in this case, there's only one XML string
        sink_data = self.sink.read(timeout=1.0)
        self.failIf(sink_data is None)
        self.assertEqual([data1], sink_data.data)
        self.assertEqual(0, len(sink_data.timestamps))

        # Push a couple more XML strings
        self.port.pushPacket(data1, False, sri.streamID)
        data2 = '<document><body/></document>'
        self.port.pushPacket(data2, False, sri.streamID)
        data3 = '<document><body></body></document>'
        self.port.pushPacket(data3, True, sri.streamID)

        # This time, it should return 3 XML strings, but still no timestamps
        sink_data = self.sink.read(timeout=1.0, eos=True)
        self.failIf(sink_data is None)
        self.assertEqual([data1, data2, data3], sink_data.data)
        self.assertEqual(0, len(sink_data.timestamps))

    @format('long')
    def testReadStreamID(self):
        # Push directly to the port
        sri = bulkio.sri.create('test_read_stream_1')
        self.port.pushSRI(sri)
        ts = bulkio.timestamp.now()
        self.port.pushPacket(range(16), ts, False, sri.streamID)

        # Read from a stream that does not have data should fail
        data = self.sink.read(timeout=0.1, streamID='not here')
        self.failUnless(data is None)

        # Push to a second stream ID
        sri = bulkio.sri.create('test_read_stream_2')
        self.port.pushSRI(sri)
        ts = bulkio.timestamp.now()
        self.port.pushPacket(range(16), ts, False, sri.streamID)

        # Read should return data specifically from the given streamID (need to
        # give it a timeout so that the sink's thread has time to queue the
        # packet data)
        data = self.sink.read(timeout=1.0, streamID=sri.streamID)
        self.failIf(data is None)
        self.assertEqual(sri.streamID, data.sri.streamID)
        self.assertEqual(1, len(data.sris))
        self.failUnless(bulkio.sri.compare(sri, data.sri))
        self.assertEqual(range(16), data.data)
        self.assertEqual((0, ts), data.timestamps[0])

    @format('char')
    def testTimeStamps(self):
        # Push a bunch of packets and remember the time stamps
        sri = bulkio.sri.create('test_time_stamps')
        self.port.pushSRI(sri)
        expected = []
        ts = bulkio.timestamp.now()
        self.port.pushPacket('\x00'*16, ts, False, sri.streamID)
        expected.append((0, ts))
        ts = bulkio.timestamp.now()
        self.port.pushPacket('\x00'*32, ts, False, sri.streamID)
        expected.append((16, ts))
        ts = bulkio.timestamp.now()
        self.port.pushPacket('\x00'*16, ts, False, sri.streamID)
        expected.append((48, ts))
        self.port.pushPacket('', bulkio.timestamp.notSet(), True, sri.streamID)

        # Read all of the data and check the timestamps against what was sent
        data = self.sink.read(timeout=1.0, eos=True)
        self.failIf(data is None)
        self.assertEqual(3, len(data.timestamps))
        for (exp_off, exp_ts), (act_off, act_ts) in zip(expected, data.timestamps):
            self.assertEqual(exp_off, act_off)
            self.assertEqual(exp_ts, act_ts)

    @format('double')
    def testTimeStampsComplex(self):
        # Push a bunch of packets and remember the time stamps
        sri = bulkio.sri.create('test_time_stamps_cx')
        sri.mode = 1
        self.port.pushSRI(sri)
        expected = []
        ts = bulkio.timestamp.now()
        self.port.pushPacket([0] * 32, ts, False, sri.streamID)
        expected.append((0, ts))
        ts = bulkio.timestamp.now()
        self.port.pushPacket([1] * 64, ts, False, sri.streamID)
        # NB: The offset advances by the number of complex values, not the
        # number of scalars
        expected.append((16, ts))
        ts = bulkio.timestamp.now()
        self.port.pushPacket([22] * 32, ts, False, sri.streamID)
        expected.append((48, ts))
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri.streamID)

        data = self.sink.read(timeout=1.0, eos=True)
        self.failIf(data is None)
        self.assertEqual(3, len(data.timestamps))
        for (exp_off, exp_ts), (act_off, act_ts) in zip(expected, data.timestamps):
            self.assertEqual(exp_off, act_off)
            self.assertEqual(exp_ts, act_ts)

    @format('octet')
    def testSriChanges(self):
        # Push some data with an initial SRI
        sri = bulkio.sri.create('test_sri_changes')
        sri.xdelta = 1.0
        self.port.pushSRI(sri)
        self.port.pushPacket('\x00', bulkio.timestamp.now(), False, sri.streamID)

        # Modify the SRI and push some more
        sri2 = bulkio.sri.create(sri.streamID)
        sri2.xdelta = 2.0
        self.port.pushSRI(sri2)
        self.port.pushPacket('\x01', bulkio.timestamp.now(), False, sri.streamID)
        self.port.pushPacket('\x02', bulkio.timestamp.now(), False, sri.streamID)
        self.port.pushPacket('\x03', bulkio.timestamp.now(), False, sri.streamID)

        # One last modification and some data, followed by an EOS
        sri3 = bulkio.sri.create(sri.streamID)
        sri3.xdelta = 3.0
        self.port.pushSRI(sri3)
        self.port.pushPacket('\x04', bulkio.timestamp.now(), False, sri.streamID)
        self.port.pushPacket('\x05', bulkio.timestamp.now(), False, sri.streamID)
        self.port.pushPacket('', bulkio.timestamp.notSet(), True, sri.streamID)

        # Read all of the data up to the EOS to ensure we get all of the SRIs,
        # then check the SRIs and offsets
        data = self.sink.read(timeout=1.0, eos=True)
        self.failIf(data is None)
        self.assertEqual(sri.streamID, data.streamID)
        self.assertEqual(3, len(data.sris))
        self.assertEqual(0, data.sris[0].offset)
        self.failUnless(bulkio.sri.compare(sri, data.sris[0].sri))
        self.assertEqual(1, data.sris[1].offset)
        self.failUnless(bulkio.sri.compare(sri2, data.sris[1].sri))
        self.assertEqual(4, data.sris[2].offset)
        self.failUnless(bulkio.sri.compare(sri3, data.sris[2].sri))

    @format('short')
    def testWaitEOS(self):
        # Push directly to the port
        sri = bulkio.sri.create('test_wait_eos')
        self.port.pushSRI(sri)
        self.port.pushPacket([0], bulkio.timestamp.now(), False, sri.streamID)

        # Read with eos=True should fail
        sink_data = self.sink.read(timeout=0.1, eos=True)

        # Push more data and and end-of-stream packet
        self.port.pushPacket([1,2], bulkio.timestamp.now(), False, sri.streamID)
        self.port.pushPacket([3,4,5], bulkio.timestamp.now(), False, sri.streamID)
        self.port.pushPacket([6,7,8,9], bulkio.timestamp.now(), False, sri.streamID)
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri.streamID)

        # Read until end-of-stream should succeed, returning all the data
        # pushed, with EOS set
        sink_data = self.sink.read(timeout=1.0, eos=True)
        self.failIf(sink_data is None)
        self.failUnless(sink_data.eos)
        self.assertEqual(sri.streamID, sink_data.sri.streamID)
        self.assertEqual(range(10), sink_data.data)

    @format('float')
    def testWaitStreamAndEOS(self):
        # Push directly to the port
        sri = bulkio.sri.create('test_stream_eos_1')
        self.port.pushSRI(sri)
        self.port.pushPacket(range(7), bulkio.timestamp.now(), True, sri.streamID)

        # Read with eos=True and a different stream ID should fail
        sink_data = self.sink.read(timeout=0.1, streamID='other', eos=True)
        self.failUnless(sink_data is None)

        # Push new data and an end-of-stream packet to a second stream
        sri2 = bulkio.sri.create('test_stream_eos_2')
        self.port.pushSRI(sri2)
        self.port.pushPacket(range(21), bulkio.timestamp.now(), False, sri2.streamID)
        self.port.pushPacket(range(21, 32), bulkio.timestamp.now(), False, sri2.streamID)
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri2.streamID)

        # Read until end-of-stream should succeed, returning all the data
        # pushed, with EOS set
        sink_data = self.sink.read(timeout=1.0, streamID=sri2.streamID, eos=True)
        self.failIf(sink_data is None)
        self.failUnless(sink_data.eos)
        self.assertEqual(sri2.streamID, sink_data.sri.streamID)
        self.assertEqual(range(32), sink_data.data)

    @format('double')
    def testStreamIDs(self):
        # Start with no streams
        self.assertEqual([], self.sink.streamIDs())

        # First stream
        sri_1 = bulkio.sri.create('test_streamids_1')
        self.port.pushSRI(sri_1)
        self.port.pushPacket([0], bulkio.timestamp.now(), False, sri_1.streamID)
        self.assertEqual([sri_1.streamID], self.sink.streamIDs())

        # Push a second stream; note that order is undefined, so we need to
        # sort the stream IDs
        sri_2 = bulkio.sri.create('test_streamids_2')
        self.port.pushSRI(sri_2)
        self.port.pushPacket([0], bulkio.timestamp.now(), False, sri_2.streamID)
        stream_ids = self.sink.streamIDs()
        stream_ids.sort()
        self.assertEqual([sri_1.streamID, sri_2.streamID], stream_ids)

        # Send an end-of-stream packet for the first stream and read it all;
        # only the second stream's ID should remain
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri_1.streamID)
        data = self.sink.read(timeout=1.0, streamID=sri_1.streamID)
        self.failIf(data is None)
        self.failUnless(data.eos)
        self.assertEqual([sri_2.streamID], self.sink.streamIDs())

        # Read all the data from the second stream; it should still be active
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        self.failIf(data.eos)
        self.assertEqual([sri_2.streamID], self.sink.streamIDs())

        # Close out the second stream
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri_2.streamID)
        self.assertEqual([sri_2.streamID], self.sink.streamIDs())
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        self.failUnless(data.eos)

        # Should end up with no streams
        self.assertEqual([], self.sink.streamIDs())

    @format('ulonglong')
    def testActiveSRIs(self):
        # Start with no SRIs
        self.assertEqual([], self.sink.activeSRIs())

        # First stream
        sri_1 = bulkio.sri.create('test_activesris_1')
        sri_1.xdelta = 1.0
        self.port.pushSRI(sri_1)
        self.port.pushPacket([0], bulkio.timestamp.now(), False, sri_1.streamID)
        active_sris = self.sink.activeSRIs()
        self.assertEqual(1, len(active_sris))
        self.failUnless(bulkio.sri.compare(sri_1, active_sris[0]))

        # Push a second stream
        sri_2 = bulkio.sri.create('test_streamids_2')
        self.port.pushSRI(sri_2)
        self.port.pushPacket([0], bulkio.timestamp.now(), False, sri_2.streamID)

        # Note that order is undefined, so we need to sort on the stream IDs
        active_sris = self.sink.activeSRIs()
        self._sortSRIs(active_sris)
        self.assertEqual(2, len(active_sris))
        self.failUnless(bulkio.sri.compare(sri_1, active_sris[0]))
        self.failUnless(bulkio.sri.compare(sri_2, active_sris[1]))

        # Send an end-of-stream for the second stream, but don't read it yet
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri_2.streamID)
        active_sris = self.sink.activeSRIs()
        self._sortSRIs(active_sris)
        self.assertEqual(2, len(active_sris))
        self.failUnless(bulkio.sri.compare(sri_1, active_sris[0]))
        self.failUnless(bulkio.sri.compare(sri_2, active_sris[1]))

        # Modify first SRI and push some more data; the active SRI should still
        # have the original values (xdelta == 1.0)
        sri_1.xdelta = 2.0
        self.port.pushSRI(sri_1)
        self.port.pushPacket([0], bulkio.timestamp.now(), False, sri_1.streamID)
        active_sris = self.sink.activeSRIs()
        self._sortSRIs(active_sris)
        self.assertEqual(2, len(active_sris))
        self.assertEqual(1.0, active_sris[0].xdelta)

        # Read all of the data from the first stream; afterwards, the active
        # SRI should have the updated values
        data = self.sink.read(timeout=1.0, streamID=sri_1.streamID)
        self.failIf(data is None)
        active_sris = self.sink.activeSRIs()
        self._sortSRIs(active_sris)
        self.assertEqual(2, len(active_sris))
        self.assertEqual(2.0, active_sris[0].xdelta)

        # Acknowledge the end-of-stream for the second stream, and make sure it
        # no longer shows up in the active SRIs
        data = self.sink.read(timeout=1.0, streamID=sri_2.streamID)
        self.failIf(data is None)
        self.failUnless(data.eos)
        active_sris = self.sink.activeSRIs()
        self.assertEqual(1, len(active_sris))
        self.failUnless(bulkio.sri.compare(sri_1, active_sris[0]))

        # Close out the first stream and make sure there are no more active
        # SRIs
        self.port.pushPacket([], bulkio.timestamp.notSet(), True, sri_1.streamID)
        data = self.sink.read(timeout=1.0)
        self.failIf(data is None)
        self.failUnless(data.eos)
        self.assertEqual([], self.sink.activeSRIs())

    def _sortSRIs(self, sris):
        # Sorts a list of SRIs by stream ID
        sris.sort(cmp=lambda x,y: cmp(x.streamID, y.streamID))

    def testPortAccess(self):
        # New sink should have no port yet
        sink2 = StreamSink(sandbox=self.sandbox)
        self.failUnless(sink2.port is None)

        # A connection has already been made for the test fixture's sink, so
        # the port must be defined
        port = self.sink.port
        self.failIf(port is None)

        # Try calling a port method as a quick sanity check
        self.assertEqual([], port.getStreams())
        
    def testFormat(self):
        sink = StreamSink(format='float', sandbox=self.sandbox)
        port = sink.getPort('floatIn')
        self.assertRaises(RuntimeError, sink.getPort, 'shortIn')


if __name__ == '__main__':
    import runtests
    runtests.main()
