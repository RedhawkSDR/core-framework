#
# This file is protected by Copyright. Please refer to the COPYRIGHT file
# distributed with this source distribution.
#
# This file is part of REDHAWK burstioInterfaces.
#
# REDHAWK burstioInterfaces is free software: you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation, either version 3 of the License, or (at your option) any
# later version.
#
# REDHAWK burstioInterfaces is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#
import random
import unittest
import sys
import time
import logging
from   omniORB import CORBA
from   omniORB import any
import numpy

from   ossie.cf import CF
from   ossie.utils import sb
from   bulkio import *
from   bulkio.bulkioInterfaces import *
from   redhawk.burstioInterfaces.BURSTIO import *
from   redhawk.burstio import *

# remove when sandbox support for relative path works
test_dir='../'

# Add the local search paths to find local IDL files
from ossie.utils import model
from ossie.utils.idllib import IDLLibrary
model._idllib = IDLLibrary()
model._idllib.addSearchPath('../../../idl')
model._idllib.addSearchPath('/usr/local/redhawk/core/share/idl')

def str_to_class(s):
    if s in globals() and isinstance(globals()[s], types.ClassType):
        return globals()[s]
    return None

class BaseVectorPort(unittest.TestCase):
    BLOCKING=-1.0
    NON_BLOCKING=0.0
    KEYS = ['c_name', 'c_inport', 'c_outport', 'sink_inport']
    PORT_FLOW = {
               'Int8' : [ 'burstByteIn', 'burstByteOut', 'burstByteIn' ],
               'UInt8' : [ 'burstOctetIn', 'burstOctetOut', 'burstOctetIn' ],
               'Int16' : [ 'burstShortIn', 'burstShortOut', 'burstShortIn' ],
               'UInt16' : [ 'burstUShortIn', 'burstUShortOut', 'burstShortIn' ],
               'Int32' : [ 'burstLongIn', 'burstLongOut', 'busrtLongIn' ],
               'UInt32' : [ 'burstULongIn', 'burstULongOut', 'burstLongIn' ],
               'Int64' : [ 'burstLongLongIn', 'burstLongLongOut', 'burstLonglongIn' ],
               'UInt64' : [ 'burstULongLongIn', 'burstULongLongOut', 'burstLonglongIn' ],
               'Float' : [ 'burstFloatIn', 'burstFloatOut', 'burstFloatIn' ],
               'Double' : [ 'burstDoubleIn', 'burstDoubleOut', 'burstDoubleIn' ]
               }

    def __init__(
            self,
            methodName='runTest',
            ptype='Int8',
            cname=None,
            srcData=None,
            cmpData=None,
            bio_in_module=BurstByteIn,
            bio_out_module=BurstByteOut,
            bio_burst=ByteBurst ):
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
        self.ctx = dict().fromkeys(BaseVectorPort.KEYS)
        self.bio_in_module = bio_in_module
        self.bio_out_module = bio_out_module
        self.burst_type = bio_burst

    def getPortFlow(self, ptype='Int8' ):
        return BaseVectorPort.PORT_FLOW[ptype]

    def setContext(self, ctx=None):
        ##print "cname " + str(self.c_name) + " ptype= " + str(self.ptype)
        self.ctx[ BaseVectorPort.KEYS[0] ] = self.c_name
        self.ctx[ BaseVectorPort.KEYS[1] ] = BaseVectorPort.PORT_FLOW[self.ptype][0]
        self.ctx[ BaseVectorPort.KEYS[2] ] = BaseVectorPort.PORT_FLOW[self.ptype][1]
        self.ctx[ BaseVectorPort.KEYS[3] ] = BaseVectorPort.PORT_FLOW[self.ptype][2]
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
            self.seq = range(50)
            if self.ptype == 'Int8' :
                self.seq = numpy.array(self.seq,numpy.int8).tostring()
            if  self.ptype == 'UInt8' :
                self.seq = numpy.array(self.seq,numpy.uint8).tostring()

        self.orb = CORBA.ORB_init();
        self.rootPOA = self.orb.resolve_initial_references("RootPOA")
        self.logger = logging.getLogger(self.ptype[0])
        self.logger.setLevel(logging.NOTSET)
        self.conn_cnt=0
        self.disconn_cnt=0

    def setup_connection_table(self):
        self.desc_list=[];
        self.logger.debug( "Setup - Multiout Connection Table " );
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_1", stream_id="stream-1-1" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_1", stream_id="stream-1-2" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_1", stream_id="stream-1-3" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_2", stream_id="stream-2-1" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_2", stream_id="stream-2-2" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_2", stream_id="stream-2-3" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_3", stream_id="stream-3-1" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_3", stream_id="stream-3-2" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_3", stream_id="stream-3-3" ) )
        self.desc_list.append( connection_descriptor_struct( port_name="multiout_source", connection_id="connection_4", stream_id="stream-4-1" ) )

    def make_sri_test(self, sid,  oid):
        sri = utils.createSRI(sid)
        sri.streamID = sid
        sri.id = oid
        sri.xdelta = 1.0
        sri.mode = 0
        sri.flags = 0
        sri.tau = 1.1
        sri.theta = 1.2
        sri.gain = 1.3
        sri.uwlength = 128
        sri.bursttype = 2
        sri.burstLength = 1024
        sri.CHAN_RF = 1e6
        sri.baudestimate = 2.0
        sri.carrieroffset = 2.1
        sri.SNR = 2.3
        sri.modulation = "mod"
        sri.baudrate = 56000.0
        sri.fec = "vit"
        sri.fecrate = "7/8"
        sri.randomizer="R20"
        sri.overhead="unknown"
        sri.expectedStartOfBurstTime=utils.now()
        return sri

    def make_sri_pkt1(self):
        sri = self.make_sri_test( "packet 1", "id-1" )
        sri.expectedStartOfBurstTime=utils.now()
        return sri

    def make_sri_pkt2(self):
        sri = self.make_sri_test( "packet 2", "id-2" )
        sri.expectedStartOfBurstTime=utils.now()
        return sri

    def check_pkt(self, pkt, sid,  oid, pktVal=None ):
        self.assertNotEqual( pkt, pktVal, "BURSTIO PKT CHECK mismatch ");
        self.assertEqual(    pkt.getEOS(), False, "BURSTIO  PKT CHECK  EOS mismatch ")
        self.assertEqual(    pkt.getSize(), 50, "BURSTIO PKT CHECK  Data Length mismatch " )
        self.assertEqual(    pkt.isComplex(), False, "BURSTIO PKT CHECK  Mode mismatch " )
        self.assertEqual(    pkt.getStreamID(), sid, "BURSTIO PKT CHECK SRI mismatch " )
        sri = pkt.getSRI()
        self.assertEqual(    sri.modulation, "mod", "BURSTIO PKT CHECK modulation mismatch " )
        self.assertEqual(    sri.fec, "vit", "BURSTIO PKT CHECK FEC mismatch " )
        self.assertEqual(    sri.fecrate, "7/8", "BURSTIO PKT CHECK FEC rate mismatch " )
        self.assertEqual(    sri.randomizer, "R20", "BURSTIO PKT CHECK randomizer mismatch " )
        self.assertEqual(    sri.overhead, "unknown", "BURSTIO PKT CHECK overhead mismatch " )

    def connect_cb( self, conn_id ):
        self.conn_cnt+=1

    def disconnect_cb( self, conn_id ):
        self.disconn_cnt+=1


    def test_inport_python_api(self):
        ##
        ## test bulkio base class standalone
        ##
        bio = self.bio_in_module("xxx")

        ps = bio._get_statistics()
        self.assertNotEqual(ps,None,"Cannot get Port Statistics")

        s = bio._get_state()
        self.assertNotEqual(s,None,"Cannot get Port State")
        self.assertEqual(s,BULKIO.IDLE,"Invalid Port State")

        qed = bio.getQueueDepth()
        self.assertEqual(qed,0,"Get Stream Depth Failed")

        qed = bio.getQueueThreshold()
        self.assertEqual(qed,100,"Get Stream Depth Failed")

        bio.setQueueThreshold(22)
        qed = bio.getQueueThreshold()
        self.assertEqual(qed,22,"Set/Get Burst Queue Depth Failed")

        bio.start()

        pkt = bio.getBurst( self.NON_BLOCKING )
        self.assertEqual(pkt,None,"getBurst Failed - should be empty")

        bursts = bio.getBursts(  self.NON_BLOCKING )
        self.assertEqual(bursts,[],"getBursts Failed - should be empty list")

        sid = "test_port_api"
        oid = "id-1"
        ts = utils.now()
        sri = self.make_sri_test( sid, oid )
        burst = self.burst_type( sri, self.seq, ts, False )

        # push burst to port
        bio.pushBursts( [ burst ] )

        qed = bio.getQueueDepth()
        self.assertEqual(qed,1,"Get Stream Depth Failed,should be 1")

        pkt = bio.getBurst( self.NON_BLOCKING )
        self.assertNotEqual(pkt,None,"getBurst Failed - should NOT be empty")

        self.check_pkt( pkt, sid, oid  )

        bio.stop()

        bio.pushBursts( [burst] )

        qed = bio.getQueueDepth()
        self.assertEqual(qed,0,"Get Stream Depth Failed,should be empty")

        pkt = bio.getBurst( self.NON_BLOCKING )
        self.assertEqual(pkt,None,"getBurst Failed - should be empty")


    def test_push_flush_sequence(self):
        ##
        ## test bulkio base class standalone
        ##
        bio = self.bio_in_module("xxx")

        totalBursts = 55
        bio.setQueueThreshold(totalBursts)
        qed = bio.getQueueThreshold()
        self.assertEqual(qed,totalBursts,"BURSTIO Push/Flush setQueueDepth failed")

        bio.start()

        totalBursts=0
        sri1=self.make_sri_pkt1()
        sri2=self.make_sri_pkt2()
        i=0
        while  i<10 and totalBursts < 55:
            j=0
            bursts=[]
            while  j<i*2 and totalBursts <55 :
                burst = self.burst_type( self.make_sri_pkt1(), self.seq, utils.now(), False )
                totalBursts +=1
                bursts.append(burst)

                burst = self.burst_type( self.make_sri_pkt2(), self.seq, utils.now(), False )
                totalBursts +=1
                bursts.append(burst)
                j += 2


            bio.pushBursts( bursts )
            i +=1

        qed = bio.getQueueDepth()
        self.assertEqual(qed,totalBursts,"BURSTIO Push/Flush getQueueDepth totalBursts mismatch")

        pkt = bio.getBurst( self.NON_BLOCKING )
        self.assertNotEqual(pkt,None,"BURSTIO Push/Flush Pkt != None")
        self.check_pkt( pkt, sri1.streamID, sri1.id  )

        pkt = bio.getBurst( self.NON_BLOCKING )
        self.assertNotEqual(pkt,None,"BURSTIO Push/Flush Pkt != None")
        self.check_pkt( pkt, sri2.streamID, sri2.id  )

        bio.flush()

        qed = bio.getQueueDepth()
        self.assertEqual(qed,0,"Get Stream Depth Failed,should be 0")

        pkt = bio.getBurst(  self.NON_BLOCKING )
        self.assertEqual(pkt, None,"getBurst Failed - should be empty")

        bursts = bio.getBursts(  self.NON_BLOCKING )
        self.assertEqual(bursts, [],"getBursts Failed - should be empty list")


    def test_outport_api(self):

        bio = self.bio_out_module("xxx")

        # try and assign logger to port
        ##bio.setLogger(self.logger);

        stats = bio._get_statistics();
        self.assertEqual(stats, [],"Set/Get API, Statistics should be empty")

        state = bio.state();
        self.assertEqual(state, BULKIO.IDLE,"Set/Get API, State mismatch")

        clist = bio._get_connections();
        self.assertNotEqual(clist,None,"Set/Get API, Cannot get connections list")

        ip1 = self.bio_in_module("sink_1")
        ip1_oid = self.rootPOA.activate_object(ip1);
        connectionName="testing-connection-list"
        bio.connectPort(ip1._this(), connectionName )

        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),1,"Incorrect Connections List Length")

        bio.disconnectPort(connectionName)

        cl = bio._get_connections()
        self.assertNotEqual(cl,None,"Cannot get Connections List")
        self.assertEqual(len(cl),0,"Incorrect Connections List Length")


        bio.addConnectListener( self.connect_cb )
        bio.addDisconnectListener( self.disconnect_cb)

        inport = CORBA.Object()
        self.assertRaises( CF.Port.InvalidPort, bio.connectPort,  inport,"connection_1")

        bio.connectPort( ip1._this(), "connection_1" )
        bio.disconnectPort( "connection_1" )
        self.assertRaises( CF.Port.InvalidPort, bio.disconnectPort, "connection_1")

        bio.removeConnectListener( self.connect_cb )
        bio.removeDisconnectListener( self.disconnect_cb)

        self.assertEqual( self.conn_cnt, 1, "Connect Callback Failed.")
        self.assertEqual( self.disconn_cnt, 1, "Disconnect Callback Failed.")

        cl = bio._get_connections()
        self.assertNotEqual(cl,None, "Cannot get Connections List")
        self.assertEqual(len(cl),0, "Incorrect Connections List Length")

        tmp = bio.getMaxBursts()
        self.assertEqual(tmp,100, "BURSTIO_OUT_PORT_TEST Get Max Bursts Failed" )

        tmp=22
        bio.setMaxBursts(tmp)
        tmp = bio.getMaxBursts()
        self.assertEqual(tmp,22, "BURSTIO_OUT_PORT_TEST Get Max Bursts Failed" )

        tmp=0xbeef
        bio.setByteThreshold(tmp)
        tmp = bio.getByteThreshold()
        self.assertEqual(tmp, 0xbeef, "BURSTIO_OUT_PORT_TEST Set/Get Byte Threshold Failed")

        tmp=123456789
        bio.setLatencyThreshold(tmp)
        tmp = bio.getLatencyThreshold()
        self.assertEqual(tmp, 123456789, "BURSTIO_OUT_PORT_TEST Set/Get Byte latency Threshold Failed")

        bio.connectPort( ip1._this(), "connection_1" )

        ts = utils.now()
        sid = "test_port_api"
        id = "id-1"
        sri =  self.make_sri_test( sid, id )

        bursts=[]
        burst = self.burst_type( sri, self.seq, ts, False )
        bursts.append(burst)

        bio.pushBursts( bursts )

        bio.pushBurst( self.seq, sri )

        bio.start()
        bio.stop()

        bio.start()
        bio.stop()

        bio.start()
        bio.flush()
        bio.flush()
        bio.flush()
        bio.stop()


    def test_connection_filter(self):
        bio = self.bio_out_module("xxx")

        pname = bio.getName();
        desc_list=[];
        desc_list.append( connection_descriptor_struct( port_name=pname, connection_id="connection_1", stream_id="stream-1-1" ) )
        desc_list.append( connection_descriptor_struct( port_name=pname, connection_id="connection_1", stream_id="stream-1-2" ) )
        desc_list.append( connection_descriptor_struct( port_name=pname, connection_id="connection_2", stream_id="stream-2-1" ) )
        desc_list.append( connection_descriptor_struct( port_name=pname, connection_id="connection_2", stream_id="stream-2-2" ) )
        desc_list.append( connection_descriptor_struct( port_name=pname, connection_id="connection_3", stream_id="stream-3-1" ) )
        desc_list.append( connection_descriptor_struct( port_name=pname, connection_id="connection_3", stream_id="stream-3-2" ) )
        desc_list.append( connection_descriptor_struct( port_name=pname, connection_id="connection_4", stream_id="stream-4-1" ) )

        bio.updateConnectionFilter( desc_list );

        bio.addConnectionFilter( "stream-4-2", "connection-4");
        bio.addConnectionFilter( "stream-4-3", "connection-4");

        bio.removeConnectionFilter( "stream-4-2", "connection-4");
        bio.removeConnectionFilter( "stream-4-3", "connection-4");

        bio.addConnectionFilter( "stream-4-3", "connection-4");

        bio.removeConnectionFilter( "stream-4-3", "connection-4");
        bio.removeConnectionFilter( "stream-4-3", "connection-4");
        bio.removeConnectionFilter( "stream-4-3", "connection-4");
        bio.removeConnectionFilter( "stream-4-3", "connection-4");


class Test_Burstio_Int8(BaseVectorPort):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseVectorPort.__init__(self,
                                methodName,
                                ptype = 'Int8',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstByteIn,
                                bio_out_module=redhawk.burstio.BurstByteOut,
                                bio_burst = ByteBurst )
        pass

class Test_Burstio_Int16(BaseVectorPort):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseVectorPort.__init__(self,
                                methodName,
                                ptype='Int16',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstShortIn,
                                bio_out_module=redhawk.burstio.BurstShortOut,
                                bio_burst = ShortBurst )
        pass

class Test_Burstio_Int32(BaseVectorPort):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseVectorPort.__init__(self,
                                methodName,
                                ptype='Int32',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstLongIn,
                                bio_out_module=redhawk.burstio.BurstLongOut,
                                bio_burst = LongBurst )
        pass

class Test_Burstio_Int64(BaseVectorPort):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseVectorPort.__init__(self,
                                methodName,
                                ptype='Int64',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstLongLongIn,
                                bio_out_module=redhawk.burstio.BurstLongLongOut,
                                bio_burst = LongLongBurst )
        pass

class Test_Burstio_Double(BaseVectorPort):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseVectorPort.__init__(self,
                                methodName,
                                ptype='Double',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstDoubleIn,
                                bio_out_module=redhawk.burstio.BurstDoubleOut,
                                bio_burst = DoubleBurst )
        pass

class Test_Burstio_Float(BaseVectorPort):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseVectorPort.__init__(self,
                                methodName,
                                ptype='Float',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstFloatIn,
                                bio_out_module=redhawk.burstio.BurstFloatOut,
                                bio_burst = FloatBurst )
        pass


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Burstio_Int8, Test_Burstio_Int16,  Test_Burstio_Int32, Test_Burstio_Int64, Test_Burstio_Float, Test_Burstio_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)

