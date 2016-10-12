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

from base_ports import *
from bulkio import *
import time
import threading

class SriListener(object):
    def __init__(self):
        self.sri = None

    def __call__(self, sri):
        self.sri = sri

    def reset(self):
        self.sri = None

class TestPythonAPI(BaseVectorPort):
    def test_inport_sri_changed(self):
        """
        Tests that SRI changes are reported correctly from getPacket().
        """
        port = self.bio_in_module('test')
        port.startPort()

        # Create a default SRI and push it
        test_sri = sri.create('test-stream')
        port.pushSRI(test_sri)

        # SRI should report changed for first packet
        port.pushPacket([0], timestamp.now(), False, test_sri.streamID)
        packet = port.getPacket(const.NON_BLOCKING)
        self.assertNotEqual(packet[port.DATA_BUFFER], None)
        self.assertEqual(packet[port.SRI_CHG], True)

        # No SRI change for second packet
        port.pushPacket([1], timestamp.now(), False, test_sri.streamID)
        packet = port.getPacket(const.NON_BLOCKING)
        self.assertNotEqual(packet[port.DATA_BUFFER], None)
        self.assertEqual(packet[port.SRI_CHG], False)

        # Reduce the queue size so we can force a flush
        port.setMaxQueueDepth(2)

        # Push a packet, change the SRI, and push two more packets so that the
        # packet with the associated SRI change gets flushed
        port.pushPacket([2], timestamp.now(), False, test_sri.streamID)
        test_sri.xdelta /= 2.0
        port.pushSRI(test_sri)
        port.pushPacket([3], timestamp.now(), False, test_sri.streamID)
        port.pushPacket([4], timestamp.now(), False, test_sri.streamID)

        # Get the last packet and verify that the queue has flushed, and the
        # SRI change is still reported
        packet = port.getPacket(const.NON_BLOCKING)
        self.assertNotEqual(packet[port.DATA_BUFFER], None)
        self.assertEqual(packet[port.QUEUE_FLUSH], True)
        self.assertEqual(packet[port.SRI_CHG], True)

        # Push data without an SRI to check that the sriChanged flag is still
        # set and the SRI callback gets called
        listener = SriListener()
        port.setNewSriListener(listener)
        port.pushPacket([0], timestamp.now(), False, 'invalid_stream')
        packet = port.getPacket(const.NON_BLOCKING)
        self.assertTrue(packet.sriChanged)
        self.assertFalse(listener.sri is None)

        # Push again to the same stream ID; sriChanged should now be false and
        # the SRI callback should not get called
        listener.reset()
        port.pushPacket([0], timestamp.now(), False, 'invalid_stream')
        packet = port.getPacket(const.NON_BLOCKING)
        self.assertFalse(packet.sriChanged)
        self.assertTrue(listener.sri is None)

    def test_inport_getPacket_timeout(self):
        """
        Tests that timeout modes work as expected in getPacket().
        """
        port = self.bio_in_module('test')
        port.startPort()

        # If non-blocking takes more than a millisecond, something is wrong
        import timeit
        def fn_getPacket(fn, timeout):
            def _foo():
                fn(timeout)
            return _foo
        
        timer_fn = timeit.Timer(fn_getPacket(port.getPacket, const.NON_BLOCKING))
        rettime = timer_fn.timeit(number=100)
        self.assert_(rettime < 100e-3)
        packet = port.getPacket(const.NON_BLOCKING)
        self.assertEqual(packet[port.DATA_BUFFER], None)

        # Check that (at least) the timeout period elapses
        timeout = 0.125
        number_iterations = 10
        timer_fn = timeit.Timer(fn_getPacket(port.getPacket, timeout))
        rettime = timer_fn.timeit(number=number_iterations)
        self.assert_(rettime > timeout * number_iterations)
        packet = port.getPacket(timeout)
        self.assertEqual(packet[port.DATA_BUFFER], None)

        # Try a blocking getPacket() on another thread
        results = []
        def get_packet():
            packet = port.getPacket(const.BLOCKING)
            results.append(packet)
        t = threading.Thread(target=get_packet)
        t.setDaemon(True)
        t.start()

        # Wait for a while to ensure that the thread has had a chance to enter
        # getPacket(), then check that it has not returned
        time.sleep(0.125)
        self.assertEqual(len(results), 0)

        # Stop the port and make sure the thread exits
        port.stopPort()
        t.join(timeout=1.0)
        self.failIf(t.isAlive())

    def test_inport_statistics_streamIDs(self):
        """
        Tests that the stream IDs reported in statistics are correct.
        """
        port = self.bio_in_module('test')
        port.startPort()

        # Create a few streams, push an SRI and packet for each, and test that
        # the statistics report the correct stream IDs
        stream_ids = set('sri%d' % ii for ii in xrange(3))
        for streamID in stream_ids:
            stream_sri = sri.create(streamID)
            port.pushSRI(stream_sri)
            port.pushPacket([0], timestamp.now(), False, streamID)
        self.assertEqual(stream_ids, set(port._get_statistics().streamIDs))

        # Push an end-of-stream for one of the streams (doesn't matter which),
        # and test that the stream ID has been removed from the stats
        streamID = stream_ids.pop()
        port.pushPacket([], timestamp.now(), True, streamID)
        self.assertEqual(stream_ids, set(port._get_statistics().streamIDs))

    def test_inport_blocking_deadlock(self):
        """
        Tests that a blocking pushPacket does not prevent other threads from
        interacting with the port.
        """
        port = self.bio_in_module('test')
        port.startPort()

        test_sri = sri.create('blocking-stream')
        test_sri.blocking = True
        port.pushSRI(test_sri)

        port.setMaxQueueDepth(1)

        # Push enough packets to block in one thread
        def push_packet():
            for ii in range(2):
                port.pushPacket([0], timestamp.now(), False, test_sri.streamID)
        push_thread = threading.Thread(target=push_packet)
        push_thread.setDaemon(True)
        push_thread.start()
        
        # Get the queue depth in another thread, which used to lead to deadlock
        # (well, mostly-dead-lock)
        test_thread = threading.Thread(target=port.getCurrentQueueDepth)
        test_thread.setDaemon(True)
        test_thread.start()

        # Wait a while for the queue depth query to complete, which should happen
        # quickly. If the thread is still alive, then deadlock must have occurred
        test_thread.join(1.0)
        deadlock = test_thread.isAlive()

        # Get packets to unblock the push thread, allows all threads to finish
        port.getPacket()
        port.getPacket()
        self.failIf(deadlock)


class Test_Python_Int8(TestPythonAPI):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Python_Ports' ):
        TestPythonAPI.__init__(self, methodName, ptype, cname  )

class Test_Python_Int16(TestPythonAPI):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Python_Ports' ):
        TestPythonAPI.__init__(self, methodName, ptype, cname,  bio_in_module=InShortPort, bio_out_module=OutShortPort )

class Test_Python_Int32(TestPythonAPI):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Python_Ports' ):
        TestPythonAPI.__init__(self, methodName, ptype, cname, bio_in_module=InLongPort, bio_out_module=OutLongPort )

class Test_Python_Int64(TestPythonAPI):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Python_Ports' ):
        TestPythonAPI.__init__(self, methodName, ptype, cname,  bio_in_module=InLongLongPort, bio_out_module=OutLongLongPort )

class Test_Python_Float(TestPythonAPI):
    def __init__(self, methodName='runTest', ptype='Float', cname='Python_Ports' ):
        TestPythonAPI.__init__(self, methodName, ptype, cname,  bio_in_module=InFloatPort, bio_out_module=OutFloatPort )

class Test_Python_Double(TestPythonAPI):
    def __init__(self, methodName='runTest', ptype='Double', cname='Python_Ports' ):
        TestPythonAPI.__init__(self, methodName, ptype, cname,  bio_in_module=InDoublePort, bio_out_module=OutDoublePort )

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Python_Int8, Test_Python_Int16,  Test_Python_Int32, Test_Python_Int64, Test_Python_Float, Test_Python_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
