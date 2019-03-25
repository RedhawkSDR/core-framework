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
from omniORB import any
from ossie.utils import sb
import bulkio
from bulkio.bulkioInterfaces import BULKIO
from ossie.utils.testing import main as _ossie_test_main


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


    # ##############
    # Helper Methods 
    # ##############

    def connectAllSdds(self):
        self.source.connect(self.sink1, 'dataSDDS_in', connectionId="conn1")
        self.source.connect(self.sink2, 'dataSDDS_in', connectionId="conn2")
        self.source.connect(self.sink3, 'dataSDDS_in', connectionId="conn3")
        self.source.connect(self.sink4, 'dataSDDS_in', connectionId="conn4")


    def killAllSinks(self):
        os.kill(self.sink1._pid, 9)
        while ( self.sink1._process.isAlive() == True ) : time.sleep(.5)
        os.kill(self.sink2._pid, 9)
        while ( self.sink2._process.isAlive() == True ) : time.sleep(.5)
        os.kill(self.sink3._pid, 9)
        while ( self.sink3._process.isAlive() == True ) : time.sleep(.5)
        os.kill(self.sink4._pid, 9)
        while ( self.sink4._process.isAlive() == True ) : time.sleep(.5)

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
        inFloatPort.pushSRI(sri)
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

    def checkSRIReceived(self, ports, numSRIs=1, sriFields=None):
        for port in ports:
            timeout = 10
            while len(port._get_attachedSRIs()) < numSRIs:
                time.sleep(1)
                timeout -= 1
                if timeout == 0:
                    self.assertTrue(False, "Timed-out waiting to receive new SRI")
                    break
            timeout = 10
            if sriFields:
                for key,val in sriFields.items():
                    attachedSRIs = port._get_attachedSRIs()
                    self.assertTrue(len(attachedSRIs)>0, "Unable to check SRI parameters...No SRIs received")
                    while getattr(port._get_attachedSRIs()[0],key) != val:
                        time.sleep(1)
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
    def testDisconnectingActiveSDDSStreamsCF(self):
        self.connectAllSdds()
        self.killAllSinks()
        self.addSddsStream("Stream1")
        self.addSddsStream("Stream2")
        self.addSddsStream("Stream3")

    def testSRIBeforeAddStreamSDDS(self):
        self.connectAllVita()
        self.killAllSinks()
        self.pushSRI("Stream1")
        self.addSddsStream("Stream1")

    def testDisconnectingActiveVITAStreamsCF(self):
        self.connectAllVita()
        self.killAllSinks()
        self.addVitaStream("Stream1")
        self.addVitaStream("Stream2")
        self.addVitaStream("Stream3")


    def testSRIBeforeAddStreamVITA(self):
        self.connectAllVita()
        self.killAllSinks()
        self.pushSRI("Stream1")
        self.addVitaStream("Stream1")


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
