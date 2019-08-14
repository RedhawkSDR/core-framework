#!/usr/bin/env python
#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK codegenTesting.
#
# REDHAWK codegenTesting is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK codegenTesting is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import threading, time
import unittest
import ossie.utils.testing
import os
from omniORB import any, CORBA
import commands

from ossie.utils.bulkio import bulkio_helpers
from ossie.utils import sb
from bulkio.bulkioInterfaces import BULKIO__POA
from bulkio.bulkioInterfaces import BULKIO

class ComponentTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all component implementations in sri"""

    def testScaBasicBehavior(self):
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        
        #######################################################################
        # Verify the basic state of the component
        self.assertNotEqual(self.comp_obj, None)
        self.assertEqual(self.comp_obj._non_existent(), False)
        self.assertEqual(self.comp_obj._is_a("IDL:CF/Resource:1.0"), True)
        
        #######################################################################
        # Simulate regular component startup
        # Verify that initialize nor configure throw errors
        self.comp_obj.initialize()
        configureProps = self.getPropertySet(kinds=("configure",), modes=("readwrite", "writeonly"), includeNil=False)
        self.comp_obj.configure(configureProps)
        
        #######################################################################
        # Validate that query returns all expected parameters
        # Query of '[]' should return the following set of properties
        expectedProps = []
        expectedProps.extend(self.getPropertySet(kinds=("configure", "execparam"), modes=("readwrite", "readonly"), includeNil=True))
        expectedProps.extend(self.getPropertySet(kinds=("allocate",), action="external", includeNil=True))
        props = self.comp_obj.query([])
        props = dict((x.id, any.from_any(x.value)) for x in props)
        # Query may return more than expected, but not less
        for expectedProp in expectedProps:
            self.assertEquals(props.has_key(expectedProp.id), True)
        
        #######################################################################
        # Verify that all expected ports are available
        for port in self.scd.get_componentfeatures().get_ports().get_uses():
            port_obj = self.comp_obj.getPort(str(port.get_usesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a("IDL:CF/Port:1.0"),  True)
            
        for port in self.scd.get_componentfeatures().get_ports().get_provides():
            port_obj = self.comp_obj.getPort(str(port.get_providesname()))
            self.assertNotEqual(port_obj, None)
            self.assertEqual(port_obj._non_existent(), False)
            self.assertEqual(port_obj._is_a(port.get_repid()),  True)
            
        #######################################################################
        # Make sure start and stop can be called without throwing exceptions
        self.comp_obj.start()
        self.comp_obj.stop()
        
        #######################################################################
        # Simulate regular component shutdown
        self.comp_obj.releaseObject()
    
    #test that the sri changed flags are tied to the right streamIDs
    def testSRIChanged(self):
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        #######################################################################
        # Launch the component with the default execparams
        execparams = self.getPropertySet(kinds=("execparam",), modes=("readwrite", "writeonly"), includeNil=False)
        execparams = dict([(x.id, any.from_any(x.value)) for x in execparams])
        self.launch(execparams)
        self.comp_obj.initialize()
        self.comp_obj.start()
        
        self.helperShortInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataShort)
        self.dataShortInput = self.comp_obj.getPort('dataShortIn')
        self.dataShortOutput = self.comp_obj.getPort('dataShortOut')
        self.dataShortOutput.connectPort(self.helperShortInput, 'dataShortOut')
        
        shortData = bulkio_helpers.genRandomDataSet(16, True, 1000)
        
        s1_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 300, 0.0, 0.001, 1, 1, "s1", False, [])
        s2_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 200, 0.0, 0.001, 1, 1, "s2", False, [])
        
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s1")
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s2")
        
        #pushPacket with no previous pushSRI, should have the correct streamIDs but the flags 
        #should not be set in either getPacket call
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(sriChanged)
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's2')
        self.assertTrue(sriChanged)
        
        self.dataShortInput.pushSRI(s1_sri)
        self.dataShortInput.pushSRI(s2_sri)
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s1")
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s2")
        
        #pushSRIs followed by pushPackets.  The flag should be set in both getPacket 
        #calls and the received SRIs should match the ones sent in 
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s1_sri))
        self.assertTrue(sriChanged)
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's2')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s2_sri))
        self.assertTrue(sriChanged)
        
        s1_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 400, 0.0, 0.001, 1, 1, "s1", False, [])
        self.dataShortInput.pushSRI(s1_sri)
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s1")
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s2")
        
        #Changed the SRI for stream s1, and pushed it, did nothing for stream s2.  Should see the flag set
        #in the first getPacket call and the streamID should be s1, also the SRI should match the new one.
        #The second getPacket call should return streamID s2, flag not set, and the SRI should match the old one
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s1_sri))
        self.assertTrue(sriChanged)
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's2')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s2_sri))
        self.assertFalse(sriChanged)
        
        #Changed the SRI for stream s2, and pushed it, did nothing for stream s1.  Should see the flag set
        #in the second getPacket call and the streamID should be s2, also the SRI should match the new one.
        #The first getPacket call should return streamID s1, flag not set, and the SRI should match the old one
        s2_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 600, 0.0, 0.001, 1, 1, "s2", False, [])
        self.dataShortInput.pushSRI(s2_sri)
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s1")
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s2")
        
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s1_sri))
        self.assertFalse(sriChanged)
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's2')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s2_sri))
        self.assertTrue(sriChanged)
        
        #Did nothing to change either stream s1 or stream s2.  Should see no changes to SRI, and should not
        #see the flag set in either getPacket call.  The streamIDs should still come back in order
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s1")
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s2")
        
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s1_sri))
        self.assertFalse(sriChanged)
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's2')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s2_sri))
        self.assertFalse(sriChanged)
        
        #Changed the SRI for stream s2, and pushed it, Then repeated the operation changing the SRI again before
        #making a pushPacket call for either stream,  Should see the flag set in the getPacket call, also the SRI 
        #should match the latest one pushed.
        s2_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 500, 0.0, 0.001, 1, 1, "s2", False, [])
        self.dataShortInput.pushSRI(s2_sri)
        s2_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 700, 0.0, 0.001, 1, 1, "s2", False, [])
        self.dataShortInput.pushSRI(s2_sri)
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s2")
        
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's2')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s2_sri))
        self.assertTrue(sriChanged)
        
        #Testing that a pushPacket with an EOS flag does not break anything with SRI.  The SRI is supposed
        #to be removed from the list that the port maintains if EOS was received for a particular streamID.
        #Flags should not be set and SRIs should still come back as the latest ones 
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), True, "s1")
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), True, "s2")
        
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s1_sri))
        self.assertFalse(sriChanged)
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's2')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s2_sri))
        self.assertFalse(sriChanged)
        
        #This final test looks to see if the same SRI is pushed once before a pushPacket, then again after
        #the pushPacket, that the sriChanged flag won't be set for the next data set to be returned.  A
        #pushSRI with the exact same SRI that already exists should not cause the flag to be set. The first
        #getPacket should see new SRI and the flag set, the second one should see the same SRI, no flag set
        s1_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 300, 0.0, 0.001, 1, 1, "s1", False, [])
        self.dataShortInput.pushSRI(s1_sri)
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s1")
        self.dataShortInput.pushSRI(s1_sri)
        self.dataShortInput.pushPacket(shortData, bulkio_helpers.createCPUTimestamp(), False, "s1")
        
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s1_sri))
        self.assertTrue(sriChanged)
        data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket(-1)
        self.assertEquals(streamID, 's1')
        self.assertTrue(bulkio_helpers.compareSRI(sri, s1_sri))
        self.assertFalse(sriChanged)
        
        self.comp_obj.stop()
        self.comp_obj.releaseObject()

class BackPressureTests(ossie.utils.testing.ScaComponentTestCase):
            
    def testFlush(self):
        """
        Test the standard flush behavior.
        """
        self.utility()
        numPackets = 4
        
        # Set the queue depth to the number of packets intending to be sent - 1
        self.comp.mqd = numPackets-1
        
        # Create a non-blocking source
        source = sb.StreamSource()
        source.connect(self.comp)

        # Create a sink for the output
        sink = sb.StreamSink()
        self.comp.connect(sink)
        
        # Send numPackets without starting the component, should generate a flush
        for x in xrange(numPackets):
            source.write([x])
        source.start()
        source.close()

        # Start the component and let the stream complete
        self.comp.start()
        sink.start()
        streamData = sink.read(timeout=1, eos=True)
        data = []
        if streamData:
            data = streamData.data

        self.assertEqual(len(data), 1)

        source.stop()
        sink.stop()
        self.comp.stop()
    
    def testBackPressure(self):
        """
        Tests the back pressure behavior.
        """
        self.utility()
        numPackets = 4
        
        # Set the queue depth to the number of packets intending to be sent - 1
        self.comp.mqd = numPackets-1
        
        # Create a blocking source (non-default behavior for StreamSink)
        # Requires a separate thread to prevent deadlock.
        class StreamSourceThread(threading.Thread):
            def __init__(self, parent):
                threading.Thread.__init__(self)
                self.src = sb.StreamSource()
                self.src.blocking = True
                self.src.connect(parent.comp)
                self.src.start()

            def run(self):
                for x in range(numPackets):
                    self.src.write([x])
                self.src.close()

            def stop(self):
                self.src.stop()

        src_thread = StreamSourceThread(self)

        # Create a sink for the output
        sink = sb.StreamSink()
        sink.start()
        self.comp.connect(sink)

        # Fill up the queue
        src_thread.start()

        # Start the component and let the stream complete
        self.comp.start()
        streamData = sink.read(timeout=1, eos=True)
        data = []
        if streamData:
            data = streamData.data

        # Should not have flushed, should have gotten all packets
        self.assertEquals(len(data), numPackets)

        src_thread.stop()
        sink.stop()
        self.comp.stop()

    #test that sending a packet with an EOS on it will lift the block 
    #if that stream was the last one needing the blocking behavior
    def testEOSBlockReset(self):
        self.utility()
        numPackets = 4
        
        #set the queueDepth to the number of packets intending to be sent - 1
        self.comp.mqd = numPackets-1
        
        #send two streamSRI, one for blocking and one to not block, should tell the port to block
        s1_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 300, 0.0, 0.001, 1, 1, 's1', True, [])
        s2_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 300, 0.0, 0.001, 1, 1, 's2', False, [])
        self.dataShortInput.pushSRI(s1_sri)
        self.dataShortInput.pushSRI(s2_sri)
        
        #start the 'quasi' component thread, tell him to receive numPackets*2
        t1 = threading.Thread(None, self.getPacketThread, 't1', (numPackets*2,), {})
        t1.setDaemon(True)
        t1.start()
        
        #fill the queue
        for x in range(numPackets-1):
            self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's1')

        #send the last one and the rest of the packets in a separate thread
        t2 = threading.Thread(None, self.pushOnePlusPacketsThread, 't2', ('s1','s2',numPackets), {})
        t2.setDaemon(True)
        t2.start()
        
        #start the component and let the 'quasi' component thread complete
        self.comp.start()
        t2.join()
        t1.join()
        
        #should not have flushed, should have gotten all packets     
        self.assertFalse(self.queueFlushed)
        self.assertEquals(self.packetsReceived, numPackets*2)
        self.comp.stop()
        
        #reset packetsReceived to 0
        self.packetsReceived = 0
        
        #start another receiver thread
        t1 = threading.Thread(None, self.getPacketThread, 't1', (numPackets*2,), {})
        t1.setDaemon(True)
        t1.start()
        
        #send 2 packets, the second one with an EOS which should lift the block
        #since s1 was calling for the block and s2 wasn't
        self.comp.start()
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's1')
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), True, 's1')

        #wait to see that the EOS has passed through the component
        #which should reset the block to false
        while not self.EOSReceived:
            pass
        
        self.comp.stop()

        #push enough packets in to cause a flush
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        
        self.comp.start()
        t1.join()
        self.comp.stop()
        
        #should have received the 2 packets from s1, and the single packet from s2
        self.assertTrue(self.queueFlushed)
        self.assertTrue(self.packetsReceived == 3)

    #test that changing a port to blocking while the queue
    #is not empty doesn't cause an error
    def testChangeBlockWithNonEmptyQueue(self):
        self.utility()
        numPackets = 6
        
        #set the queueDepth to the number of packets intending to be sent - 1
        self.comp.mqd = numPackets-1
        
        #set the port to not block (this is default behavior
        s2_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 300, 0.0, 0.001, 1, 1, 's2', False, [])
        self.dataShortInput.pushSRI(s2_sri)
        
        #fill the queue half way
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        
        #now set the port the be blocking
        s1_sri = BULKIO.StreamSRI(1, 0.0, 0.001, 1, 300, 0.0, 0.001, 1, 1, 's1', True, [])
        self.dataShortInput.pushSRI(s1_sri)
        
        #should still be able to push 2 more before the queue is full
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, 's2')
        
        #send the last packet in a separate thread
        #this call should block, not flush
        t2 = threading.Thread(None, self.pushOnePacketThread, 't2', ('s2',), {})
        t2.setDaemon(True)
        t2.start()
        
        #start the 'quasi' component thread, tell him to receive numPackets
        t1 = threading.Thread(None, self.getPacketThread, 't1', (numPackets,), {})
        t1.setDaemon(True)
        t1.start()
        
        self.comp.start()
        t2.join()
        t1.join()
        self.comp.stop()
        
        #should not have flushed
        self.assertTrue(not self.queueFlushed)
        self.assertTrue(self.packetsReceived == numPackets)
        self.comp.releaseObject()
        
    def setUp(self):
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        #######################################################################
        # Launch the component with the default execparams
        self.launch()
        
        self.helperShortInput = bulkio_helpers.createBULKIOInputPort(BULKIO__POA.dataShort)
        self.dataShortInput = self.comp.getPort('dataShortIn')
        self.dataShortOutput = self.comp.getPort('dataShortOut')
        self.dataShortOutput.connectPort(self.helperShortInput, 'dataShortOut')
        
        self.queueFlushed = False
        self.packetsReceived = 0
        self.EOSReceived = False
    
    def utility(self):
        pass

    #Thread function to mimic a simple service function for a component
    def getPacketThread(self, numPacketsSent):
        numPacketsReceived = 0
        runLoop = True
        while runLoop:
            data, T, EOS, streamID, sri, sriChanged, flushed = self.helperShortInput.getPacket()
            if data != None:
                if EOS:
                    self.EOSReceived = True
                
                #terminate the thread if a flush occurs
                if data == [0]:
                    numPacketsReceived = numPacketsReceived + 1
                    self.queueFlushed = True
                    runLoop = False
                else:         
                    numPacketsReceived = numPacketsReceived + 1
                    if numPacketsReceived == numPacketsSent:
                        runLoop = False
            self.packetsReceived = numPacketsReceived
    
    def pushOnePacketThread(self, streamID):
            self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, streamID)
    
    def pushOnePlusPacketsThread(self, firstStreamID, restStreamID, numPacketsToSend):
            self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, firstStreamID)
            for x in range(numPacketsToSend):
                self.dataShortInput.pushPacket([1], bulkio_helpers.createCPUTimestamp(), False, restStreamID)
    
        
    # TODO Add additional tests here
    #
    # See:
    #   ossie.utils.testing.bulkio_helpers,
    #   ossie.utils.testing.bluefile_helpers
    # for modules that will assist with testing components with BULKIO ports
    
if __name__ == "__main__":
    ossie.utils.testing.main("../sri.spd.xml") # By default tests all implementations
