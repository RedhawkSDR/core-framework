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
import copy

from   ossie.cf import CF
from   ossie.utils import sb
from   bulkio import *
from   bulkio.bulkioInterfaces import *
from   bulkio.bulkioInterfaces.BULKIO import *
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


class BaseMultiOut(unittest.TestCase):
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
        self.ctx = dict().fromkeys(BaseMultiOut.KEYS)
        self.bio_in_module = bio_in_module
        self.bio_out_module = bio_out_module
        self.burst_type = bio_burst


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
            self.seq = range(50)
            if self.ptype == 'Int8' :
                self.seq = numpy.array(self.seq,numpy.int8).tostring()
            if  self.ptype == 'UInt8' :
                self.seq = numpy.array(self.seq,numpy.uint8).tostring()

        self.orb = CORBA.ORB_init();
        self.rootPOA = self.orb.resolve_initial_references("RootPOA")
        self.logger = logging.getLogger(self.ptype[0])
        self.logger.setLevel(logging.NOTSET)
        #self.logger.setLevel(logging.INFO)


        self.ip1 = self.bio_in_module("sink_1" )
        self.ip1_oid = self.rootPOA.activate_object(self.ip1)
        self.ip2 = self.bio_in_module("sink_2" )
        self.ip2_oid = self.rootPOA.activate_object(self.ip2)
        self.ip3 = self.bio_in_module("sink_3" )
        self.ip3_oid = self.rootPOA.activate_object(self.ip3)
        self.ip4 = self.bio_in_module("sink_4" )
        self.ip4_oid = self.rootPOA.activate_object(self.ip4)
        self.port = self.bio_out_module("multiout_source")
        self.port_oid = self.rootPOA.activate_object(self.port)
        self.setup_connection_table()

    def getData(self):
        self.setContext()
        seq=[]
        if self.srcData:
            seq = copy.deepcopy(self.srcData)
        else:
            seq = range(50)
            if self.ptype == 'Int8' :
                seq = numpy.array(seq,numpy.int8).tostring()
            if  self.ptype == 'UInt8' :
                seq = numpy.array(seq,numpy.uint8).tostring()
        return seq


    def setup_connection_table(self):
        self.desc_list=[]
        self.logger.debug( "Setup - Multiout Connection Table " )
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


    def tearDown(self):

        if self.ip1: self.ip1.stop()
        if self.ip2: self.ip2.stop()
        if self.ip3: self.ip3.stop()
        if self.ip4: self.ip4.stop()
        if self.port: self.port.stop()

        self.rootPOA.deactivate_object(self.ip1_oid)
        self.rootPOA.deactivate_object(self.ip2_oid)
        self.rootPOA.deactivate_object(self.ip3_oid)
        self.rootPOA.deactivate_object(self.ip4_oid)
        self.rootPOA.deactivate_object(self.port_oid)


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

    def check_pkt(self, ip, sid,  oid, npkts):

        cnt=0
        for i in range(npkts):
            pkt = ip.getBurst( self.NON_BLOCKING )
            if pkt == None:
                break

            self.assertNotEqual(pkt,None,"BURSTIO Push/Flush Pkt != None")
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
            cnt+=1

        return cnt


    def send_bursts(self, bio, sid, oid, nbursts ):
        totalBursts=0
        sri=self.make_sri_test( sid, oid )
        bursts=[]
        while  totalBursts < nbursts :
            seq = self.getData()
            burst = self.burst_type( sri, data, utils.now(), False )
            totalBursts +=1
            bursts.append(burst)

        bio.pushBursts( bursts )
        return sri

    def send_bursts2(self, bio, sid, oid, nbursts ):
        totalBursts=0
        sri=self.make_sri_test( sid, oid )
        while  totalBursts < nbursts :
            data=self.getData()
            totalBursts +=1

            bio.pushBurst( data, sri, utils.now() )

        bio.flush()
        return sri


    def test_multiout_data_filter(self):
        self.logger.debug( "Multiout DATA Filtered - BEGIN " )

        clist = self.port._get_connections()
        self.assertEqual( clist != None, True, "Connection List Error" )

        self.logger.debug( "Multiout DATA Filtered - Create Connections and Filter list " )
        self.port.connectPort( self.ip1._this(), "connection_1")
        self.port.connectPort( self.ip2._this(), "connection_2")
        self.port.connectPort( self.ip3._this(), "connection_3")
        self.port.connectPort( self.ip4._this(), "connection_4")
        self.port.updateConnectionFilter( self.desc_list )

        maxTotalBursts = 10
        self.ip1.setQueueThreshold(maxTotalBursts)
        self.ip2.setQueueThreshold(maxTotalBursts)
        self.ip3.setQueueThreshold(maxTotalBursts)
        self.ip4.setQueueThreshold(maxTotalBursts)
        qed = self.ip1.getQueueThreshold()
        self.assertEqual(qed,maxTotalBursts,"BURSTIO Push/Flush setQueueDepth failed")

        self.port.setRoutingMode( ROUTE_CONNECTION_STREAMS )

        self.ip1.start()
        self.ip2.start()
        self.ip3.start()
        self.ip4.start()
        self.port.start()

        ##
        ## Push Burst stream to IP1
        ##
        filter_stream_id="stream-1-1"
        sri = self.send_bursts2( self.port, filter_stream_id, "id-1", maxTotalBursts )

        self.logger.debug( "Multiout DATA Filter - sid:" + filter_stream_id )

        n=self.check_pkt( self.ip1, sri.streamID, sri.id, maxTotalBursts  )

        #
        # make sure other ports did not receive a packet
        #
        n=self.check_pkt( self.ip2, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP2 PKT was NOT empty" )
        n=self.check_pkt( self.ip3, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP3 PKT was NOT empty" )
        n=self.check_pkt( self.ip4, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP4 PKT was NOT empty" )

        ##
        ## Push Burst stream to SELF.IP2
        ##
        filter_stream_id="stream-2-1"
        sri = self.send_bursts2( self.port, filter_stream_id, "id-2", maxTotalBursts )

        self.logger.debug( "Multiout DATA Filter - sid:" + filter_stream_id )

        n=self.check_pkt( self.ip2, sri.streamID, sri.id, maxTotalBursts  )

        #
        # make sure other ports did not receive a packet
        #
        n=self.check_pkt( self.ip1, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP1 PKT was NOT empty" )
        n=self.check_pkt( self.ip3, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP3 PKT was NOT empty" )
        n=self.check_pkt( self.ip4, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP4 PKT was NOT empty" )


        ##
        ## Push Burst stream to IP3
        ##
        filter_stream_id="stream-3-1"
        sri = self.send_bursts2( self.port, filter_stream_id, "id-3", maxTotalBursts )

        self.logger.debug( "Multiout DATA Filter - sid:" + filter_stream_id )

        n=self.check_pkt( self.ip3, sri.streamID, sri.id, maxTotalBursts  )

        #
        # make sure other ports did not receive a packet
        #
        n=self.check_pkt( self.ip1, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP1 PKT was NOT empty" )
        n=self.check_pkt( self.ip2, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP2 PKT was NOT empty" )
        n=self.check_pkt( self.ip4, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP4 PKT was NOT empty" )


        ##
        ## Push Burst stream to IP4
        ##
        filter_stream_id="stream-4-1"
        sri = self.send_bursts2( self.port, filter_stream_id, "id-4", maxTotalBursts )

        self.logger.debug( "Multiout DATA Filter - sid:" + filter_stream_id )

        n=self.check_pkt( self.ip4, sri.streamID, sri.id, maxTotalBursts  )

        #
        # make sure other ports did not receive a packet
        #
        n=self.check_pkt( self.ip1, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP1 PKT was NOT empty" )
        n=self.check_pkt( self.ip2, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP2 PKT was NOT empty" )
        n=self.check_pkt( self.ip3, sri.streamID, sri.id, maxTotalBursts  )
        self.assertEqual( n, 0, "getBurst - IP3 PKT was NOT empty" )


class Test_Burstio_Int8(BaseMultiOut):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseMultiOut.__init__(self,
                                methodName,
                                ptype = 'Int8',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstByteIn,
                                bio_out_module=redhawk.burstio.BurstByteOut,
                                bio_burst = ByteBurst )
        pass

class Test_Burstio_Int16(BaseMultiOut):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseMultiOut.__init__(self,
                                methodName,
                                ptype='Int16',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstShortIn,
                                bio_out_module=redhawk.burstio.BurstShortOut,
                                bio_burst = ShortBurst )
        pass

class Test_Burstio_Int32(BaseMultiOut):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseMultiOut.__init__(self,
                                methodName,
                                ptype='Int32',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstLongIn,
                                bio_out_module=redhawk.burstio.BurstLongOut,
                                bio_burst = LongBurst )
        pass

class Test_Burstio_Int64(BaseMultiOut):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseMultiOut.__init__(self,
                                methodName,
                                ptype='Int64',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstLongLongIn,
                                bio_out_module=redhawk.burstio.BurstLongLongOut,
                                bio_burst = LongLongBurst )
        pass

class Test_Burstio_Double(BaseMultiOut):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseMultiOut.__init__(self,
                                methodName,
                                ptype='Double',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstDoubleIn,
                                bio_out_module=redhawk.burstio.BurstDoubleOut,
                                bio_burst = DoubleBurst )
        pass

class Test_Burstio_Float(BaseMultiOut):
    def __init__(self, methodName='runTest', cname='Python_Ports' ):
        BaseMultiOut.__init__(self,
                                methodName,
                                ptype='Float',
                                cname=cname,
                                bio_in_module=redhawk.burstio.BurstFloatIn,
                                bio_out_module=redhawk.burstio.BurstFloatOut,
                                bio_burst = FloatBurst )
        pass


if __name__ == '__main__':
    suite = unittest.TestSuite()
    for x in [ Test_Burstio_Int8, Test_Burstio_Int16, Test_Burstio_Int32, Test_Burstio_Int64, Test_Burstio_Float, Test_Burstio_Double ]:
        tests = unittest.TestLoader().loadTestsFromTestCase(x)
        suite.addTests(tests)
    try:
        import xmlrunner
        runner = xmlrunner.XMLTestRunner(verbosity=2)
    except ImportError:
        runner = unittest.TextTestRunner(verbosity=2)
    runner.run(suite)
