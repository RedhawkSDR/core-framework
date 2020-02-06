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
import time
import logging
from omniORB import any
from ossie.utils import sb
import bulkio
from bulkio.bulkioInterfaces import BULKIO
from ossie.utils.testing import main as _ossie_test_main
import omniORB
from omniORB import CORBA


from ossie.utils.sandbox import debugger
#from ossie.utils.log4py import logging
#logging.basicConfig()
#logging.getLogger().setLevel(logging.DEBUG)

# Add the local search paths to find local IDL files
from ossie.utils import model
from ossie.utils.idllib import IDLLibrary
model._idllib = IDLLibrary()
model._idllib.addSearchPath('../../../../../idl')
model._idllib.addSearchPath('/usr/local/redhawk/core/share/idl')
main=_ossie_test_main

class ResourceTests(ossie.utils.testing.ScaComponentTestCase):
    """Test for all resource implementations in multiout_attachable"""

    # ##############
    # Setup/Teardown 
    # ##############

    def setUp(self):
        ossie.utils.testing.ScaComponentTestCase.setUp(self)
        #self.source = sb.launch("../multiout_attachable.spd.xml", impl=self.impl, execparams={"DEBUG_LEVEL":4})
        #self.sink1 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)#, execparams={"DEBUG_LEVEL":4})
        self.source = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink1 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink2 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink3 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        self.sink4 = sb.launch("../multiout_attachable.spd.xml", impl=self.impl)
        sb.start()

    def tearDown(self):
        #ossie.utils.testing.ScaComponentTestCase.tearDown(self)
        self.source.releaseObject()
        self.sink1.releaseObject()
        self.sink2.releaseObject()
        self.sink3.releaseObject()
        self.sink4.releaseObject()
        del self.source
        del self.sink1
        del self.sink2
        del self.sink3
        del self.sink4
        sb.release()

    # ##############
    # Helper Methods 
    # ##############

    def connectAllSdds(self):
        self.source.connect(self.sink1, 'dataSDDS_in', connectionId="conn1")
        self.source.connect(self.sink2, 'dataSDDS_in', connectionId="conn2")
        self.source.connect(self.sink3, 'dataSDDS_in', connectionId="conn3")
        self.source.connect(self.sink4, 'dataSDDS_in', connectionId="conn4")

    def connectAllVita(self):
        self.source.connect(self.sink1, 'dataVITA49_in', connectionId="vita1")
        self.source.connect(self.sink2, 'dataVITA49_in', connectionId="vita2")
        self.source.connect(self.sink3, 'dataVITA49_in', connectionId="vita3")
        self.source.connect(self.sink4, 'dataVITA49_in', connectionId="vita4")
    
    def disconnectAll(self):
        self.source.disconnect(self.sink1)
        self.source.disconnect(self.sink2)
        self.source.disconnect(self.sink3)
        self.source.disconnect(self.sink4)
    
    def addSddsStream(self, streamId):
        newStream = {
            'sdds::privateInfo': "whoops", 
            'sdds::timeTagValid': False, 
            'sdds::sampleRate': 0L, 
            'sdds::id': streamId, 
            'sdds::multicastAddress': '0.0.0.0', 
            'sdds::port': 0L, 
            'sdds::vlan': 0L
        }
        self.source.SDDSStreamDefinitions.append(newStream)

    def addVitaStream(self, streamId):
        newStream = {
            'vita49::data_item_size': 0, 
            'vita49::vlan': 0L, 
            'vita49::vector_size': 0, 
            'vita49::valid_data_format': False, 
            'vita49::event_tag_size': 0, 
            'vita49::channel_tag_size': 0, 
            'vita49::port': 0L, 
            'vita49::repeat_count': 0, 
            'vita49::item_packing_field_size': 0, 
            'vita49::ip_address': '0.0.0.0', 
            'vita49::packing_method_processing_efficient': False, 
            'vita49::id': streamId, 
            'vita49::repeating': False
        }
        self.source.VITA49StreamDefinitions.append(newStream)


    def addConnectionTableEntry(self, connId, streamId, portName):
        entry = {
            'connectionTable::stream_id': streamId, 
            'connectionTable::connection_id': connId, 
            'connectionTable::port_name': portName
        }
        self.source.connectionTable.append(entry)

    def removeAllSddsStreams(self):
        self.source.SDDSStreamDefinitions = []
    
    def removeAllVitaStreams(self):
        self.source.VITA49StreamDefinitions = []

    def removeAllConnectionTableEntries(self):
        self.source.connectionTable = []

    def pushSRI(self, streamId = "defaultStreamId",xstart=0.0):
        sri = bulkio.sri.create(streamId)
        sri.xstart = xstart
        ts = time.time()
        T = BULKIO.PrecisionUTCTime(BULKIO.TCM_CPU, BULKIO.TCS_VALID, 0.0, int(ts), ts - int(ts))
        expected_packets = self.source.packets_ingested + 1
        inFloatPort = self.source.getPort("dataFloat_in")
        time_begin = time.time()
        while time.time()-time_begin < 10:
            try:
                inFloatPort.pushSRI(sri)
                break
            except:
                time.sleep(1)
        inFloatPort.pushPacket(range(1,100),T,False,streamId)

        # Wait until we know that the packet and SRI have been processed by the
        # source; this prevents some tests from reporting spurious failures due
        # to the appearance of an SRI change versus a new SRI, especially when
        # running the Python implemenation.
        self.waitForPacketIngestion(self.source, expected_packets, delay=0.1)
    
    def assertNumNewSRICallbacks(self, num):
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, num)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, num)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, num)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, num)

    def assertNumSRIChangeCallbacks(self, num):
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, num)
        self.assertEquals(self.sink2.callback_stats.num_sri_change_callbacks, num)
        self.assertEquals(self.sink3.callback_stats.num_sri_change_callbacks, num)
        self.assertEquals(self.sink4.callback_stats.num_sri_change_callbacks, num)

    def checkTimeoutException(self, port, initial_length):
        len_attachedSRIs = initial_length
        try:
            len_attachedSRIs = len(port._get_attachedSRIs())
        except Exception, e:
            if type(e) == CORBA.COMM_FAILURE:
                if e.minor != omniORB.COMM_FAILURE_WaitingForReply and e.completed != CORBA.COMPLETED_MAYBE:
                    raise(e)
            else:
                raise(e)
        return len_attachedSRIs

    def checkSRIReceived(self, ports, numSRIs=1, sriFields=None):
        for port in ports:
            timeout = 10
            len_attachedSRIs = 0
            len_attachedSRIs = self.checkTimeoutException(port, len_attachedSRIs)
            while len_attachedSRIs < numSRIs:
                time.sleep(1)
                timeout -= 1
                if timeout == 0:
                    self.assertTrue(False, "Timed-out waiting to receive new SRI")
                    break
                len_attachedSRIs = self.checkTimeoutException(port, len_attachedSRIs)

            timeout = 10
            if sriFields:
                time_begin = time.time()
                while time.time()-time_begin < 10:
                    try:
                        attachedSRIs = port._get_attachedSRIs()
                        self.assertTrue(len(attachedSRIs)>0, "Unable to check SRI parameters...No SRIs received")
                        all_fields = True
                        for key,val in sriFields.items():
                            if getattr(attachedSRIs[0],key) == val:
                                continue
                            else:
                                all_fields = False
                        if not all_fields:
                            time.sleep(1) # key is not there
                        else:
                            break
                    except Exception, e:
                        if type(e) == CORBA.COMM_FAILURE:
                            if e.minor != omniORB.COMM_FAILURE_WaitingForReply and e.completed != CORBA.COMPLETED_MAYBE:
                                raise(e)
                        else:
                            raise(e)
                        time.sleep(1) # _get_attachedSRIs call failed
                    timeout -= 1
                    if timeout == 0:
                        break

    def waitForPacketIngestion(self, component, count, timeout=10, delay=0.5):
        now = time.time()
        endTime = now+timeout
        while (now < endTime):
            if (component.packets_ingested == count):
                return
            time.sleep(delay)
            now = time.time()

        self.assertTrue(component.packets_ingested == count, "Timed-out waiting for expected number of packet ingests")
        

    # ##############
    #     Tests
    # ##############

    def test_input_attach_detach(self):
        self.source.start()
        sdds_stream_def = BULKIO.SDDSStreamDefinition(id='input1', dataFormat=BULKIO.SDDS_SB, multicastAddress='239.0.1.1', vlan=118, port=10000, sampleRate=100000000, timeTagValid=False, privateInfo='privateInfo')
        sdds_in = self.source.getPort("dataSDDS_in")
        attach_id = sdds_in.attach(sdds_stream_def,"")
        sdef = sdds_in.getStreamDefinition(attach_id)
        print 'Attached stream: %s' % sdef
        sdds_in.detach(attach_id) 

    
    def testSddsConnectionsWithNoStreams(self):
        self.connectAllSdds()       
 
        # No attaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 0)
        
        # No detaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 0)
    
    def testVitaConnectionsWithNoStreams(self):
        self.connectAllVita()       
 
        # No attaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 0)
        
        # No detaches should have been made
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 0)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 0)

    # 
    # Test adding streams while connections exist
    #
    def testAddingSDDSStreamToActiveConnections(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")

        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 1)
    
    def testAddingSDDSStreamsToActiveConnections(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 3)
   
    # 
    # Test adding connections after streams
    #
    def testAddingSDDSConnectionsToActiveStream(self):
        self.addSddsStream("Stream1")
        self.connectAllSdds()

        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 1)

    def testAddingSDDSConnectionsToActiveStreams(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 3)

    def testAddingSDDSConnectionsAfterStreamDefinitions(self):
        self.addSddsStream("Stream1")
        self.pushSRI('Stream1')
        self.addSddsStream("Stream2")
        self.pushSRI('Stream2')
        self.addSddsStream("Stream3")
        self.pushSRI('Stream3')
        self.connectAllSdds()
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 3)

        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 3)
    
    #
    # Test removing streams
    #
    def testDisconnectingActiveSDDSStreamsSF(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.disconnectAll()
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_detaches, 3)
    
    def testDisconnectingActiveSDDSStreamsCF(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.disconnectAll()

        self.assertEquals(self.sink1.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_detaches, 3)
    
    #
    # Test reconnecting streams
    #
    def testReconnectingActiveSDDSStreams(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.removeAllSddsStreams()
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")

        self.assertEquals(self.sink1.callback_stats.num_sdds_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_sdds_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_sdds_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_sdds_attaches, 6)
        
        self.assertEquals(self.sink1.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_sdds_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_sdds_detaches, 3)

        self.assertEquals(len(self.sink1.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 3)

    #
    # Test connection table
    #
    def testFilterTablePostSDDSConnection(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTablePreSDDSConnection(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.connectAllSdds()
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTablePreSDDSStreams(self):
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTableInvalidSDDSPort(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","whoopsie")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 3)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 3)

    def testFilterTableInvalidSDDSStream(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","whoopsie","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTableInvalidSDDSConnectionId(self):
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.connectAllSdds()
        self.addConnectionTableEntry("whoopsie","Stream1","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 0)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 0)
    
    def testFilterTableMultiSDDS(self):
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")
        self.addConnectionTableEntry("conn2","Stream2","dataSDDS_out")
        self.connectAllSdds()
        self.addConnectionTableEntry("conn3","Stream3","dataSDDS_out")
        self.addConnectionTableEntry("conn4","Stream3","dataSDDS_out")
        
        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 1)

    # 
    # Test adding streams while connections exist
    #
    def testAddingVITAStreamToActiveConnections(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")

        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 1)
    
    def testAddingVITAStreamsToActiveConnections(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 3)
   
    # 
    # Test adding connections after streams
    #
    def testAddingVITAConnectionsToActiveStream(self):
        self.addVitaStream("Stream1")
        self.connectAllVita()

        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 1)

    def testAddingVITAConnectionsToActiveStreams(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 3)

    def testAddingVITAConnectionsAfterStreamDefinitions(self):
        self.addVitaStream("Stream1")
        self.pushSRI('Stream1')
        self.addVitaStream("Stream2")
        self.pushSRI('Stream2')
        self.addVitaStream("Stream3")
        self.pushSRI('Stream3')
        self.connectAllVita()
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 3)

        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 3)
    
    #
    # Test removing streams
    #
    def testDisconnectingActiveVITAStreamsSF(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.disconnectAll()

        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)
    
    def testDisconnectingActiveVITAStreamsCF(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.disconnectAll()

        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)

    #
    # Test updating streams
    #
    def testUpdatingActiveVITAStreamsSF(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.assertEquals(len(self.source.VITA49StreamDefinitions), 3)
        self.connectAllVita()
       
        self.source.VITA49StreamDefinitions[0].port = 123L
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 4)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 4)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 4)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 4)
        
        self.source.VITA49StreamDefinitions[1].port = 456L
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 5)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 5)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 5)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 5)
        
        self.source.VITA49StreamDefinitions[2].port = 789L
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 6)
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 6)
        
        self.assertEquals(self.sink1.received_vita49_attachments[0].port, 123)
        self.assertEquals(self.sink1.received_vita49_attachments[1].port, 456)
        self.assertEquals(self.sink1.received_vita49_attachments[2].port, 789)
        
        self.assertEquals(self.sink2.received_vita49_attachments[0].port, 123)
        self.assertEquals(self.sink2.received_vita49_attachments[1].port, 456)
        self.assertEquals(self.sink2.received_vita49_attachments[2].port, 789)
        
        self.assertEquals(self.sink3.received_vita49_attachments[0].port, 123)
        self.assertEquals(self.sink3.received_vita49_attachments[1].port, 456)
        self.assertEquals(self.sink3.received_vita49_attachments[2].port, 789)
    
    def testUpdatingActiveVITAStreamsCF(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.assertEquals(len(self.source.VITA49StreamDefinitions), 3)
       
        self.source.VITA49StreamDefinitions[0].port = 123L
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 1)
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 4)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 4)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 4)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 4)
        
        self.source.VITA49StreamDefinitions[1].port = 456L
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 2)
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 5)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 5)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 5)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 5)
        
        self.source.VITA49StreamDefinitions[2].port = 789L
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 6)
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 6)
        
        self.assertEquals(self.sink1.received_vita49_attachments[0].port, 123)
        self.assertEquals(self.sink1.received_vita49_attachments[1].port, 456)
        self.assertEquals(self.sink1.received_vita49_attachments[2].port, 789)
        
        self.assertEquals(self.sink2.received_vita49_attachments[0].port, 123)
        self.assertEquals(self.sink2.received_vita49_attachments[1].port, 456)
        self.assertEquals(self.sink2.received_vita49_attachments[2].port, 789)
        
        self.assertEquals(self.sink3.received_vita49_attachments[0].port, 123)
        self.assertEquals(self.sink3.received_vita49_attachments[1].port, 456)
        self.assertEquals(self.sink3.received_vita49_attachments[2].port, 789)
    
    #
    # Test reconnecting streams
    #
    def testReconnectingActiveVITAStreams(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.removeAllVitaStreams()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")

        self.assertEquals(self.sink1.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink2.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink3.callback_stats.num_vita49_attaches, 6)
        self.assertEquals(self.sink4.callback_stats.num_vita49_attaches, 6)
        
        self.assertEquals(self.sink1.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink2.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink3.callback_stats.num_vita49_detaches, 3)
        self.assertEquals(self.sink4.callback_stats.num_vita49_detaches, 3)

        self.assertEquals(len(self.sink1.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 3)

    #
    # Test connection table
    #
    def testFilterTablePostVITAConnection(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTablePreVITAConnection(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.connectAllVita()
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTablePreVITAStreams(self):
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTableInvalidVITAPort(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","whoopsie")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 3)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 3)

    def testFilterTableInvalidVITAStream(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","whoopsie","dataVITA49_out")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTableInvalidVITAConnectionId(self):
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.connectAllVita()
        self.addConnectionTableEntry("whoopsie","Stream1","dataVITA49_out")
        
        self.assertEquals(len(self.sink1.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 0)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 0)
    
    def testFilterTableMultiVITA(self):#
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.connectAllVita()
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("vita4","Stream3","dataVITA49_out")
       
        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 1)

    def testSDDSAttachmentUpdate(self):
        self.addSddsStream("Stream1")
        self.connectAllSdds()
        newPortValue = 12345
        self.source.SDDSStreamDefinitions[0].port = newPortValue

        self.assertEquals(len(self.sink1.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink2.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink3.received_sdds_attachments), 1)
        self.assertEquals(len(self.sink4.received_sdds_attachments), 1)

        self.assertEquals(self.sink1.received_sdds_attachments[0].port, newPortValue)
        self.assertEquals(self.sink2.received_sdds_attachments[0].port, newPortValue)
        self.assertEquals(self.sink3.received_sdds_attachments[0].port, newPortValue)
        self.assertEquals(self.sink4.received_sdds_attachments[0].port, newPortValue)
    
    def testVITAAttachmentUpdate(self):
        self.addVitaStream("Stream1")
        self.connectAllVita()
        newPortValue = 12345
        self.source.VITA49StreamDefinitions[0].port = newPortValue

        self.assertEquals(len(self.sink1.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink2.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink3.received_vita49_attachments), 1)
        self.assertEquals(len(self.sink4.received_vita49_attachments), 1)

        self.assertEquals(self.sink1.received_vita49_attachments[0].port, newPortValue)
        self.assertEquals(self.sink2.received_vita49_attachments[0].port, newPortValue)
        self.assertEquals(self.sink3.received_vita49_attachments[0].port, newPortValue)
        self.assertEquals(self.sink4.received_vita49_attachments[0].port, newPortValue)

    def testSRIBeforeAddStreamSDDS(self):
        self.connectAllSdds()
        self.pushSRI("Stream1")
        self.addSddsStream("Stream1")

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")
        
        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')

        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(0)


    def testSRIAfterAddStreamSDDS(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")
        self.pushSRI("Stream1")

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')
        
        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(0)

    def testSRIUpdateBeforeAddStreamSDDS(self):
        self.connectAllSdds()
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)
        self.addSddsStream("Stream1")

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4],1,{"xstart":1234.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].xstart, 1234.0)
       
        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(1)

    def testSRIUpdateAfterAddStreamSDDS(self):
        self.connectAllSdds()
        self.addSddsStream("Stream1")
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4],1,{"xstart":1234.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].xstart, 1234.0)
        
        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(1)

    def testSRIBeforeAddStreamVITA(self):
        self.connectAllVita()
        self.pushSRI("Stream1")
        self.addVitaStream("Stream1")

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')
        
        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(0)

    def testSRIAfterAddStreamVITA(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")
        self.pushSRI("Stream1")

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')
        
        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(0)

    def testSRIUpdateBeforeAddStreamVITA(self):
        self.connectAllVita()
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)
        self.addVitaStream("Stream1")

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4],1,{"xstart":1234.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].xstart, 1234.0)
        
        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(1)

    def testSRIUpdateAfterAddStreamVITA(self):
        self.connectAllVita()
        self.addVitaStream("Stream1")
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4],1,{"xstart":1234.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort4._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort4._get_attachedSRIs()[0].xstart, 1234.0)
        
        self.assertNumNewSRICallbacks(1)
        self.assertNumSRIChangeCallbacks(1)

    def testSRIBeforeAddStreamSDDSWithConnectionTable(self):
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addConnectionTableEntry("conn2","Stream2","dataSDDS_out")
        self.addConnectionTableEntry("conn3","Stream3","dataSDDS_out")
        self.addConnectionTableEntry("someconn","Stream5","dataSDDS_out")
        self.pushSRI("Stream1")
        self.addSddsStream("Stream1")
        self.pushSRI("Stream3")
        self.addSddsStream("Stream3")
        self.pushSRI("UnusedStream")
        self.addSddsStream("UnusedStream")

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")

        self.checkSRIReceived([inPort1,inPort3])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs(), [])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort4._get_attachedSRIs(), [])

        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)

    def testSRIAfterAddStreamSDDSWithConnectionTable(self):
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addConnectionTableEntry("conn2","Stream2","dataSDDS_out")
        self.addConnectionTableEntry("conn3","Stream3","dataSDDS_out")
        self.addConnectionTableEntry("someconn","Stream5","dataSDDS_out")
        self.pushSRI("Stream3")
        self.addSddsStream("Stream3")
        self.pushSRI("UnusedStream")
        self.addSddsStream("UnusedStream")
        self.addSddsStream("Stream1")
        self.pushSRI("Stream1")

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")

        self.checkSRIReceived([inPort1,inPort3])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs(),[])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort4._get_attachedSRIs(),[])
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)

    def testSRIUpdateBeforeAddStreamSDDSWithConnectionTable(self):
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addConnectionTableEntry("conn2","Stream2","dataSDDS_out")
        self.addConnectionTableEntry("conn3","Stream3","dataSDDS_out")
        self.addConnectionTableEntry("someconn","Stream5","dataSDDS_out")
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)
        self.addSddsStream("Stream1")
        self.pushSRI("Stream3")
        self.pushSRI("Stream3",xstart=987.0)
        self.addSddsStream("Stream3")
        self.pushSRI("UnusedStream")
        self.pushSRI("UnusedStream",xstart=777.0)
        self.addSddsStream("UnusedStream")

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")

        self.checkSRIReceived([inPort1],1,{"xstart":1234.0})
        self.checkSRIReceived([inPort3],1,{"xstart":987.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs(),[])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 987.0)
        self.assertEquals(inPort4._get_attachedSRIs(),[])
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_sri_change_callbacks, 0)

    def testSRIUpdateAfterAddStreamSDDSWithConnectionTable(self):
        self.connectAllSdds()
        self.addConnectionTableEntry("conn1","Stream1","dataSDDS_out")
        self.addConnectionTableEntry("conn2","Stream2","dataSDDS_out")
        self.addConnectionTableEntry("conn3","Stream3","dataSDDS_out")
        self.addConnectionTableEntry("someconn","Stream5","dataSDDS_out")
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream3")
        self.addSddsStream("UnusedStream")
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)
        self.pushSRI("Stream3")
        self.pushSRI("Stream3",xstart=987.0)
        self.pushSRI("UnusedStream")
        self.pushSRI("UnusedStream",xstart=777.0)

        inPort1 = self.sink1.getPort("dataSDDS_in")
        inPort2 = self.sink2.getPort("dataSDDS_in")
        inPort3 = self.sink3.getPort("dataSDDS_in")
        inPort4 = self.sink4.getPort("dataSDDS_in")

        self.checkSRIReceived([inPort1],1,{"xstart":1234.0})
        self.checkSRIReceived([inPort3],1,{"xstart":987.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs(),[])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 987.0)
        self.assertEquals(inPort4._get_attachedSRIs(),[])
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_sri_change_callbacks, 0)

    def testSRIBeforeAddStreamVITAWithConnectionTable(self):
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("someconn","Stream5","dataVITA49_out")
        self.pushSRI("Stream1")
        self.addVitaStream("Stream1")
        self.pushSRI("Stream3")
        self.addVitaStream("Stream3")
        self.pushSRI("UnusedStream")
        self.addVitaStream("UnusedStream")

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1])
        self.checkSRIReceived([inPort3])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs(), [])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort4._get_attachedSRIs(), [])
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)

    def testSRIAfterAddStreamVITAWithConnectionTable(self):
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("someconn","Stream5","dataVITA49_out")
        self.pushSRI("Stream3")
        self.addVitaStream("Stream3")
        self.pushSRI("UnusedStream")
        self.addVitaStream("UnusedStream")
        self.addVitaStream("Stream1")
        self.pushSRI("Stream1")

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1])
        self.checkSRIReceived([inPort3])

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort2._get_attachedSRIs(),[])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort4._get_attachedSRIs(),[])
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)

    def testSRIUpdateBeforeAddStreamVITAWithConnectionTable(self):
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("someconn","Stream5","dataVITA49_out")
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)
        self.addVitaStream("Stream1")
        self.pushSRI("Stream3")
        self.pushSRI("Stream3",xstart=987.0)
        self.addVitaStream("Stream3")
        self.pushSRI("UnusedStream")
        self.pushSRI("UnusedStream",xstart=777.0)
        self.addVitaStream("UnusedStream")

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1],1,{"xstart":1234.0})
        self.checkSRIReceived([inPort3],1,{"xstart":987.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs(),[])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 987.0)
        self.assertEquals(inPort4._get_attachedSRIs(),[])
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_sri_change_callbacks, 0)

    def testSRIUpdateAfterAddStreamVITAWithConnectionTable(self):
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("someconn","Stream5","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream3")
        self.addVitaStream("UnusedStream")
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)
        self.pushSRI("Stream3")
        self.pushSRI("Stream3",xstart=987.0)
        self.pushSRI("UnusedStream")
        self.pushSRI("UnusedStream",xstart=777.0)
        self.pushSRI("Stream4")

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1],1,{"xstart":1234.0})
        self.checkSRIReceived([inPort3],1,{"xstart":987.0})

        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(inPort1._get_attachedSRIs()[0].xstart, 1234.0)
        self.assertEquals(inPort2._get_attachedSRIs(),[])
        self.assertEquals(inPort3._get_attachedSRIs()[0].streamID, 'Stream3')
        self.assertEquals(inPort3._get_attachedSRIs()[0].xstart, 987.0)
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 0)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink2.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_sri_change_callbacks, 1)
        self.assertEquals(self.sink4.callback_stats.num_sri_change_callbacks, 0)
    
    def testSRIConnectionTableAddedAfterPushSRI(self):
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream2","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)

        # Wait for packets (containing new SRI) to be ingested
        self.waitForPacketIngestion(self.source, 2)

        # Add connection table (should receive latest SRI)
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        inPort1 = self.sink1.getPort("dataVITA49_in")
        self.checkSRIReceived([inPort1],1,{"xstart":1234.0})

        self.assertEquals(len(inPort1._get_attachedSRIs()),1)
        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 0)
    
    def testSRIPushWithoutValidStream(self):
        self.connectAllVita()
        self.pushSRI("Stream1")
        self.pushSRI("Stream1",xstart=1234.0)

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        self.checkSRIReceived([inPort1],1,{"xstart":1234.0})

        self.assertEquals(len(inPort1._get_attachedSRIs()),1)
        self.assertEquals(inPort1._get_attachedSRIs()[0].streamID, 'Stream1')
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 1)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 1)
    
    def testSRIConnectionTableAddedRemoved(self):
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("someconn","Stream5","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream3")
        self.addVitaStream("UnusedStream")
        self.pushSRI("Stream1")
        self.pushSRI("Stream3")
        self.pushSRI("UnusedStream")
        self.pushSRI("UnusedStream",xstart=777.0)

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        # Current connectionTable says ports 1+3 should get one SRI
        self.checkSRIReceived([inPort1,inPort3],1)

        # NOTE: once all connections table entries are removed,
        #       all SRIs get pushed to all connections resulting 
        #       in 3 SRIs for each port 
        self.removeAllConnectionTableEntries()
        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4],3)
      
        self.assertEquals(len(inPort1._get_attachedSRIs()),3)
        self.assertEquals(len(inPort2._get_attachedSRIs()),3)
        self.assertEquals(len(inPort3._get_attachedSRIs()),3)
        self.assertEquals(len(inPort4._get_attachedSRIs()),3)

        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink2.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink4.callback_stats.num_sri_change_callbacks, 0)
    
    def testSRIConnectionTableAddedRemovedAdded(self):
        self.connectAllVita()
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("someconn","Stream5","dataVITA49_out")
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream3")
        self.addVitaStream("UnusedStream")
        self.pushSRI("Stream1")
        self.pushSRI("Stream3")
        self.pushSRI("UnusedStream")
        self.pushSRI("UnusedStream",xstart=777.0)

        inPort1 = self.sink1.getPort("dataVITA49_in")
        inPort2 = self.sink2.getPort("dataVITA49_in")
        inPort3 = self.sink3.getPort("dataVITA49_in")
        inPort4 = self.sink4.getPort("dataVITA49_in")

        # Current connectionTable says ports 1+3 should get one SRI
        self.checkSRIReceived([inPort1,inPort3],1)

        # NOTE: once all connections table entries are removed,
        #       all SRIs get pushed to all connections resulting 
        #       in 3 SRIs for each port 
        self.removeAllConnectionTableEntries()
        
        self.addConnectionTableEntry("vita1","Stream1","dataVITA49_out")
        self.addConnectionTableEntry("vita2","Stream2","dataVITA49_out")
        self.addConnectionTableEntry("vita3","Stream3","dataVITA49_out")
        self.addConnectionTableEntry("someconn","Stream5","dataVITA49_out")
        self.pushSRI("UnusedStream",xstart=888.0)

        self.checkSRIReceived([inPort1,inPort2,inPort3,inPort4],3)

        self.assertEquals(len(inPort1._get_attachedSRIs()),3)
        self.assertEquals(len(inPort2._get_attachedSRIs()),3)
        self.assertEquals(len(inPort3._get_attachedSRIs()),3)
        self.assertEquals(len(inPort4._get_attachedSRIs()),3)
        
        self.assertEquals(self.sink1.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink2.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink3.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink4.callback_stats.num_new_sri_callbacks, 3)
        self.assertEquals(self.sink1.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink2.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink3.callback_stats.num_sri_change_callbacks, 0)
        self.assertEquals(self.sink4.callback_stats.num_sri_change_callbacks, 0)


if __name__ == "__main__":
    try:
        import sys
        import os
        if '--with-xunit' in sys.argv:
            sys.argv=sys.argv[:]+[ __file__ ]
            import use_nose_test
            main=use_nose_test.NoseTestProgram
    except:
        traceback.print_exc()
        pass

    main("../multiout_attachable.spd.xml") # By default tests all implementations

