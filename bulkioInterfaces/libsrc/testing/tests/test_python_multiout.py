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
from  bulkio import *
from ossie.cf import CF
from omniORB import any
from  omniORB import CORBA
import logging
import random
import unittest
import sys
from ossie.utils import sb
import time
from bulkio.bulkioInterfaces.BULKIO import *

# remove when sandbox support for relative path works
test_dir='../'

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None


class BaseMultiOut(unittest.TestCase):
    KEYS = ['c_name', 'c_inport', 'c_outport', 'sink_inport']
    PORT_FLOW = {
               'Int8' : [ 'dataCharIn', 'dataCharOut', 'charIn' ],
               'UInt8' : [ 'dataOctetIn', 'dataOctetOut', 'charIn' ],
               'Int16' : [ 'dataShortIn', 'dataShortOut', 'shortIn' ],
               'UInt16' : [ 'dataUShortIn', 'dataUShortOut', 'shortIn' ],
               'Int32' : [ 'dataLongIn', 'dataLongOut', 'longIn' ],
               'UInt32' : [ 'dataULongIn', 'dataULongOut', 'longIn' ],
               'Int64' : [ 'dataLongLongIn', 'dataLongLongOut', 'longlongIn' ],
               'UInt64' : [ 'dataULongLongIn', 'dataULongLongOut', 'longlongIn' ],
               'Float' : [ 'dataFloatIn', 'dataFloatOut', 'floatIn' ],
               'Double' : [ 'dataDoubleIn', 'dataDoubleOut', 'doubleIn' ],
               'File' : [ 'dataFileIn', 'dataFileOut', 'fileIn' ],
               'Xml' : [ 'dataXMLIn', 'dataXMLOut', 'xmlIn' ]
               }

    def __init__(
            self,
            methodName='runTest',
            ptype='Int8',
            cname=None,
            srcData=None,
            cmpData=None,
            bio_in_module=bulkio.InCharPort,
            bio_out_module=bulkio.OutCharPort ):
        unittest.TestCase.__init__(self, methodName)
        self.c_dir = 'components'
        self.c_name = cname
        self.ptype = ptype
        self.execparams = {}
        self.c_inport = None
        self.c_outport = None
        self.sink_inport = None
        self.srcData = srcData
        self.cmpData = cmpData
        self.ctx = dict().fromkeys(BaseMultiOut.KEYS)
        self.bio_in_module = bio_in_module
        self.bio_out_module = bio_out_module


    def getPortFlow(self, ptype='Int8' ):
        return BaseMultiOut.PORT_FLOW[ptype]

    def setContext(self, ctx=None):
        self.ctx[ BaseMultiOut.KEYS[0] ] = self.c_name
        self.ctx[ BaseMultiOut.KEYS[1] ] = BaseMultiOut.PORT_FLOW[self.ptype][0]
        self.ctx[ BaseMultiOut.KEYS[2] ] = BaseMultiOut.PORT_FLOW[self.ptype][1]
        self.ctx[ BaseMultiOut.KEYS[3] ] = BaseMultiOut.PORT_FLOW[self.ptype][2]
        tmp=self.ctx
        if ctx:
            tmp = ctx
        try:
            self.c_inport = tmp['c_inport']
            self.c_outport = tmp['c_outport']
            self.sink_inport = tmp['sink_inport']
        except:
            pass


    def setUp(self):
        self.setContext()
        if self.srcData:
            self.seq = self.srcData
        else:
            self.seq = range(100)

        self.orb = CORBA.ORB_init();
        self.rootPOA = self.orb.resolve_initial_references("RootPOA")
        self.logger = logging.getLogger(self.ptype[0])
        self.logger.setLevel(logging.NOTSET)
        self.logger.info( "Setup - Multiout Create Ports Table " );

        self.ip1 = bulkio.InFloatPort("sink_1", self.logger );
        self.ip1_oid = self.rootPOA.activate_object(self.ip1);
        self.ip2 = bulkio.InFloatPort("sink_2", self.logger );
        self.ip2_oid = self.rootPOA.activate_object(self.ip2);
        self.ip3 = bulkio.InFloatPort("sink_3", self.logger );
        self.ip3_oid = self.rootPOA.activate_object(self.ip3);
        self.ip4 = bulkio.InFloatPort("sink_4", self.logger );
        self.ip4_oid = self.rootPOA.activate_object(self.ip4);
        self.port = bulkio.OutFloatPort("multiout_source", self.logger );
        self.port_oid = self.rootPOA.activate_object(self.port);

        self.desc_list=[];
        self.logger.info( "Setup - Multiout Connection Table " );
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_1", stream_id="stream-1-1" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_1", stream_id="stream-1-2" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_1", stream_id="stream-1-3" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_2", stream_id="stream-2-1" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_2", stream_id="stream-2-2" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_2", stream_id="stream-2-3" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_3", stream_id="stream-3-1" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_3", stream_id="stream-3-2" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_3", stream_id="stream-3-3" ) )
        self.desc_list.append( bulkio.connection_descriptor_struct( port_name="multiout_source", connection_id="connection_4", stream_id="stream-4-1" ) )


    def tearDown(self):
        self.rootPOA.deactivate_object(self.ip1_oid);
        self.rootPOA.deactivate_object(self.ip2_oid);
        self.rootPOA.deactivate_object(self.ip3_oid);
        self.rootPOA.deactivate_object(self.ip4_oid);
        self.rootPOA.deactivate_object(self.port_oid);


    def test_multiout_sri_filtered(self):
        self.logger.info( "Multiout SRI Filtered - BEGIN " );

        clist = self.port._get_connections();
        self.assertEqual( clist != None, True, "Connection List Error" )

        self.logger.info( "Multiout SRI Filtered - Create Connections and Filter list " );
        self.port.connectPort( self.ip1._this(), "connection_1");
        self.port.connectPort( self.ip2._this(), "connection_2");
        self.port.connectPort( self.ip3._this(), "connection_3");
        self.port.connectPort( self.ip4._this(), "connection_4");
        self.port.updateConnectionFilter( self.desc_list );

        ##
        ## Push SRI for IP1
        ##

        filter_stream_id =  "stream-1-1"
        srate=11.0
        xdelta = 1.0/srate
        TS = bulkio.timestamp.now();
        sri = bulkio.sri.create( filter_stream_id, srate);
        self.port.pushSRI( sri );

        streams = self.ip1._get_activeSRIs();
        self.assertEqual( streams != None, True, "Active SRI List Error" )
        self.assertEqual( len(streams) == 1, True, "Active SRI List Length Error" )
        asri = streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "activeSRIs - Mode Mismatch")


        streams = self.ip2._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI Filtered - Port 2 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI Filtered - Port 2 SRI was Received, Failed")

        streams = self.ip3._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI Filtered - Port 3 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI Filtered - Port 3 SRI was Received, Failed")

        streams = self.ip4._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI Filtered - Port 4 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI Filtered - Port 4 SRI was Received, Failed")

        self.port.updateConnectionFilter( None );

        filter_stream_id =  "stream-1-1"
        srate=11.0
        xdelta = 1.0/srate
        TS = bulkio.timestamp.now();
        sri = bulkio.sri.create( filter_stream_id, srate);
        self.port.pushSRI( sri );

        streams = self.ip1._get_activeSRIs();
        self.assertEqual( streams != None, True, "Multiout SRI Filtered - Port 1 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI Filtered - Port 1 SRI was Received, Failed")
        asri=streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "Multiout SRI Filtered Port 1 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "Multiout SRI Filtered Port 1 activeSRIs - Mode Mismatch")


        streams = self.ip2._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI Filtered - Port 2 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI Filtered - Port 2 SRI was Received, Failed")
        asri=streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "Multiout SRI Filtered Port 2 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "Multiout SRI Filtered Port 2 activeSRIs - Mode Mismatch")

        streams = self.ip3._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI Filtered - Port 3 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI Filtered - Port 3 SRI was Received, Failed")
        asri=streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "Multiout SRI Filtered Port 3 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "Multiout SRI Filtered Port 3 activeSRIs - Mode Mismatch")

        streams = self.ip4._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI Filtered - Port 4 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI Filtered - Port 4 SRI was Received, Failed")
        asri=streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "Multiout SRI Filtered Port 4 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "Multiout SRI Filtered Port 4 activeSRIs - Mode Mismatch")


    def test_multiout_sri_eos_filter(self):
        self.logger.info( "Multiout SRI/EOS Filtered - BEGIN " );

        clist = self.port._get_connections();
        self.assertEqual( clist != None, True, "Connection List Error" )

        self.logger.info( "Multiout SRI/EOS Filtered - Create Connections and Filter list " );
        self.port.connectPort( self.ip1._this(), "connection_1");
        self.port.connectPort( self.ip2._this(), "connection_2");
        self.port.connectPort( self.ip3._this(), "connection_3");
        self.port.connectPort( self.ip4._this(), "connection_4");
        self.port.updateConnectionFilter( self.desc_list );

        ##
        ## Push SRI for IP1
        ##
        filter_stream_id =  "stream-1-1"
        srate=11.0
        xdelta = 1.0/srate
        TS = bulkio.timestamp.now();
        sri = bulkio.sri.create( filter_stream_id, srate);
        self.port.pushSRI( sri );

        streams = self.ip1._get_activeSRIs();
        self.assertEqual( streams != None, True, "Multiout SRI/EOS Filtered - Port 1 Stream Failed")
        self.assertEqual( len(streams) == 1, True,  "Multiout SRI/EOS Filtered - Port 1 SRI was Received, Failed")
        asri = streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "activeSRIs - Mode Mismatch")

        streams = self.ip2._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 2 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 2 SRI was Received, Failed")

        streams = self.ip3._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 3 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 3 SRI was Received, Failed")

        streams = self.ip4._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 4 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 4 SRI was Received, Failed")

        ##
        ## Push SRI for IP2
        ##
        filter_stream_id =  "stream-2-1"
        srate=22.0
        xdelta = 1.0/srate
        sri = bulkio.sri.create( filter_stream_id, srate)
        self.logger.info( "Multiout SRI/EOS Filter - sid:" + filter_stream_id )
        self.port.pushSRI( sri );

        streams = self.ip1._get_activeSRIs();
        self.assertEqual( streams != None, True, "Active SRI List Error" )
        self.assertEqual( len(streams) == 1, True, "Active SRI List Length Error" )
        asri = streams[0]
        self.assertEqual( asri.streamID == "stream-1-1", True, "IP1 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP1 activeSRIs - Mode Mismatch")

        streams = self.ip2._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 2 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI/EOS Filtered - Port 2 SRI was Received, Failed")
        asri = streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "IP2 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP2 activeSRIs - Mode Mismatch")

        streams = self.ip3._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 3 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 3 SRI was Received, Failed")

        streams = self.ip4._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 4 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 4 SRI was Received, Failed")

        ##
        ## Push SRI for IP3
        ##
        filter_stream_id =  "stream-3-1"
        srate=33.0
        xdelta = 1.0/srate
        sri = bulkio.sri.create( filter_stream_id, srate)
        self.logger.info( "Multiout SRI/EOS Filter - sid:" + filter_stream_id )
        self.port.pushSRI( sri );

        streams = self.ip1._get_activeSRIs();
        self.assertEqual( streams != None, True, "Active SRI List Error" )
        self.assertEqual( len(streams) == 1, True, "Active SRI List Length Error" )
        asri = streams[0]
        self.assertEqual( asri.streamID == "stream-1-1", True, "IP1 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP1 activeSRIs - Mode Mismatch")

        streams = self.ip2._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 2 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI/EOS Filtered - Port 2 SRI was Received, Failed")
        asri = streams[0]
        self.assertEqual( asri.streamID == "stream-2-1", True, "IP2 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP2 activeSRIs - Mode Mismatch")

        streams = self.ip3._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 3 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI/EOS Filtered - Port 3 SRI was Received, Failed")
        asri = streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "IP3 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP3 activeSRIs - Mode Mismatch")

        streams = self.ip4._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 4 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 4 SRI was Received, Failed")

        ##
        ## Push SRI for IP4
        ##
        filter_stream_id =  "stream-4-1"
        srate=44.0
        xdelta = 1.0/srate
        sri = bulkio.sri.create( filter_stream_id, srate)
        self.logger.info( "Multiout SRI/EOS Filter - sid:" + filter_stream_id )
        self.port.pushSRI( sri );

        streams = self.ip1._get_activeSRIs();
        self.assertEqual( streams != None, True, "Active SRI List Error" )
        self.assertEqual( len(streams) == 1, True, "Active SRI List Length Error" )
        asri = streams[0]
        self.assertEqual( asri.streamID == "stream-1-1", True, "IP1 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP1 activeSRIs - Mode Mismatch")

        streams = self.ip2._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 2 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI/EOS Filtered - Port 2 SRI was Received, Failed")
        asri = streams[0]
        self.assertEqual( asri.streamID == "stream-2-1", True, "IP2 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP2 activeSRIs - Mode Mismatch")

        streams = self.ip3._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 3 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI/EOS Filtered - Port 3 SRI was Received, Failed")
        asri = streams[0]
        self.assertEqual( asri.streamID == "stream-3-1", True, "IP3 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP3 activeSRIs - Mode Mismatch")

        streams = self.ip4._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 4 Stream Failed")
        self.assertEqual( len(streams) == 1, True, "Multiout SRI/EOS Filtered - Port 4 SRI was Received, Failed")
        asri = streams[0]
        self.assertEqual( asri.streamID == filter_stream_id, True, "IP4 activeSRIs - StreamID Mismatch")
        self.assertEqual( asri.mode == 0, True, "IP4 activeSRIs - Mode Mismatch")


        ##
        ## Push EOS downstream and check SRI Lists
        ##
        filter_stream_id =  "stream-1-1"
        self.logger.info( "Multiout SRI/EOS Filter - EOS sid:" + filter_stream_id )
        self.port.pushPacket( [], TS, True, filter_stream_id );

        pkt = self.ip1.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP1 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP1 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP1 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], True, "getPacket - IP1 EOS Mismatch" )

        filter_stream_id =  "stream-2-1"
        self.logger.info( "Multiout SRI/EOS Filter - EOS sid:" + filter_stream_id )
        self.port.pushPacket( [], TS, True, filter_stream_id );

        pkt = self.ip2.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP2 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP2 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP2 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], True, "getPacket - IP2 EOS Mismatch" )

        filter_stream_id =  "stream-3-1"
        self.logger.info( "Multiout SRI/EOS Filter - EOS sid:" + filter_stream_id )
        self.port.pushPacket( [], TS, True, filter_stream_id );

        pkt = self.ip3.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP3 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP3 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP3 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], True, "getPacket - IP3 EOS Mismatch" )

        filter_stream_id =  "stream-4-1"
        self.logger.info( "Multiout SRI/EOS Filter - EOS sid:" + filter_stream_id )
        self.port.pushPacket( [], TS, True, filter_stream_id );

        pkt = self.ip4.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP4 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP4 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP4 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], True, "getPacket - IP4 EOS Mismatch" )

        streams = self.ip1._get_activeSRIs();
        self.assertEqual( streams != None, True, "Multiout SRI/EOS Filtered - Port 1 Stream Failed")
        self.assertEqual( len(streams) == 0, True,  "Multiout SRI/EOS Filtered - Port 1 SRI was Received, Failed")

        streams = self.ip2._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 2 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 2 SRI was Received, Failed")

        streams = self.ip3._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 3 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 3 SRI was Received, Failed")

        streams = self.ip4._get_activeSRIs();
        self.assertEqual( streams != None, True,  "Multiout SRI/EOS Filtered - Port 4 Stream Failed")
        self.assertEqual( len(streams) == 0, True, "Multiout SRI/EOS Filtered - Port 4 SRI was Received, Failed")

    def test_multiout_data_filter(self):
        self.logger.info( "Multiout DATA Filtered - BEGIN " );

        clist = self.port._get_connections();
        self.assertEqual( clist != None, True, "Connection List Error" )

        self.logger.info( "Multiout DATA Filtered - Create Connections and Filter list " );
        self.port.connectPort( self.ip1._this(), "connection_1");
        self.port.connectPort( self.ip2._this(), "connection_2");
        self.port.connectPort( self.ip3._this(), "connection_3");
        self.port.connectPort( self.ip4._this(), "connection_4");
        self.port.updateConnectionFilter( self.desc_list );

        ##
        ## Push SRI for IP1
        ##
        filter_stream_id =  "stream-1-1"
        srate=11.0
        xdelta = 1.0/srate
        TS = bulkio.timestamp.now();
        sri = bulkio.sri.create( filter_stream_id, srate);
        self.port.pushSRI( sri );

        self.logger.info( "Multiout DATA Filter - sid:" + filter_stream_id )
        self.port.pushPacket( self.seq, TS, False, filter_stream_id );
        pkt = self.ip1.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP1 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP1 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP1 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.SRI].mode == 0, True, "getPacket - IP1 Mode Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], False, "getPacket - IP1 EOS Mismatch" )
        self.assertEqual( len(pkt[bulkio.InPort.DATA_BUFFER])==len(self.seq), True, "getPacket - IP1 DataLength Mismatch" )

        #
        # make sure other ports did not receive a packet
        #
        pkt = self.ip2.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP2 PKT was NOT empty" )
        pkt = self.ip3.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP3 PKT was NOT empty" )
        pkt = self.ip4.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP4 PKT was NOT empty" )

        ##
        ## Push SRI for IP2
        ##
        filter_stream_id =  "stream-2-1"
        srate=22.0
        xdelta = 1.0/srate
        TS = bulkio.timestamp.now();
        sri = bulkio.sri.create( filter_stream_id, srate);
        self.port.pushSRI( sri );

        self.logger.info( "Multiout DATA Filter - sid:" + filter_stream_id )
        self.port.pushPacket( self.seq, TS, False, filter_stream_id );
        pkt = self.ip2.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP2 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP2 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP2 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.SRI].mode == 0, True, "getPacket - IP2 Mode Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], False, "getPacket - IP2 EOS Mismatch" )
        self.assertEqual( len(pkt[bulkio.InPort.DATA_BUFFER])==len(self.seq), True, "getPacket - IP2 DataLength Mismatch" )

        #
        # make sure other ports did not receive a packet
        #
        pkt = self.ip1.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP1 PKT was NOT empty" )
        pkt = self.ip3.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP3 PKT was NOT empty" )
        pkt = self.ip4.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP4 PKT was NOT empty" )


        ##
        ## Push SRI for IP3
        ##
        filter_stream_id =  "stream-3-1"
        srate=33.0
        xdelta = 1.0/srate
        TS = bulkio.timestamp.now();
        sri = bulkio.sri.create( filter_stream_id, srate);
        self.port.pushSRI( sri );

        self.logger.info( "Multiout DATA Filter - sid:" + filter_stream_id )
        self.port.pushPacket( self.seq, TS, False, filter_stream_id );
        pkt = self.ip3.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP3 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP3 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP3 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.SRI].mode == 0, True, "getPacket - IP3 Mode Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], False, "getPacket - IP3 EOS Mismatch" )
        self.assertEqual( len(pkt[bulkio.InPort.DATA_BUFFER])==len(self.seq), True, "getPacket - IP3 DataLength Mismatch" )

        #
        # make sure other ports did not receive a packet
        #
        pkt = self.ip1.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP1 PKT was NOT empty" )
        pkt = self.ip2.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP2 PKT was NOT empty" )
        pkt = self.ip4.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP4 PKT was NOT empty" )

        ##
        ## Push SRI for IP4
        ##
        filter_stream_id = "stream-4-1"
        srate=44.0
        xdelta = 1.0/srate
        TS = bulkio.timestamp.now();
        sri = bulkio.sri.create( filter_stream_id, srate);
        self.port.pushSRI( sri );

        self.logger.info( "Multiout DATA Filter - sid:" + filter_stream_id )
        self.port.pushPacket( self.seq, TS, False, filter_stream_id );
        pkt = self.ip4.getPacket()
        self.assertEqual( pkt != None, True, "getPacket - IP4 PKT was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI] != None, True, "getPacket - IP4 SRI was empty" )
        self.assertEqual( pkt[bulkio.InPort.SRI].streamID == filter_stream_id, True, "getPacket - IP4 StreamID Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.SRI].mode == 0, True, "getPacket - IP4 Mode Mismatch" )
        self.assertEqual( pkt[bulkio.InPort.END_OF_STREAM], False, "getPacket - IP4 EOS Mismatch" )
        self.assertEqual( len(pkt[bulkio.InPort.DATA_BUFFER])==len(self.seq), True, "getPacket - IP4 DataLength Mismatch" )

        #
        # make sure other ports did not receive a packet
        #
        pkt = self.ip1.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP1 PKT was NOT empty" )
        pkt = self.ip2.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP2 PKT was NOT empty" )
        pkt = self.ip3.getPacket()
        self.assertEqual( pkt[0] == None, True, "getPacket - IP3 PKT was NOT empty" )


class Test_Python_Int8(BaseMultiOut):
    def __init__(self, methodName='runTest', ptype='Int8', cname='Python_Ports' ):
        BaseMultiOut.__init__(self, methodName, ptype, cname  )
        pass

class Test_Python_Int16(BaseMultiOut):
    def __init__(self, methodName='runTest', ptype='Int16', cname='Python_Ports' ):
        BaseMultiOut.__init__(self, methodName, ptype, cname  )
        pass

class Test_Python_Int32(BaseMultiOut):
    def __init__(self, methodName='runTest', ptype='Int32', cname='Python_Ports' ):
        BaseMultiOut.__init__(self, methodName, ptype, cname  )
        pass

class Test_Python_Int64(BaseMultiOut):
    def __init__(self, methodName='runTest', ptype='Int64', cname='Python_Ports' ):
        BaseMultiOut.__init__(self, methodName, ptype, cname  )
        pass

class Test_Python_Float(BaseMultiOut):
    def __init__(self, methodName='runTest', ptype='Float', cname='Python_Ports' ):
        BaseMultiOut.__init__(self, methodName, ptype, cname  )
        pass

class Test_Python_Double(BaseMultiOut):
    def __init__(self, methodName='runTest', ptype='Double', cname='Python_Ports' ):
        BaseMultiOut.__init__(self, methodName, ptype, cname  )
        pass

if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Python_Int8, Test_Python_Int16, Test_Python_Int32, Test_Python_Int64, Test_Python_Float, Test_Python_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
